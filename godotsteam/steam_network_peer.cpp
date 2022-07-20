#include "steam_network_peer.h"
#include "core/io/networked_multiplayer_peer.h"
#include "core/os/os.h"
#include "core/resource.h"
#include "godotsteam.h"

#if DEBUG_ENABLED
#define TODO_PRINT(str) WARN_PRINT(String("TODO:") + String(str))
#else
#define TODO_PRINT(ignored)
#endif

#define MAIN_COM_CHANNEL 1
#define MAX_LOBBY_KEY_LENGTH 255
#define MAX_CHAT_METADATA 8192

SteamNetworkPeer::SteamNetworkPeer() :
		callbackLobbyMessage(this, &SteamNetworkPeer::lobbyMessage),
		callbackLobbyChatUpdate(this, &SteamNetworkPeer::lobbyChatUpdate),
		callbackNetworkMessagesSessionRequest(this, &SteamNetworkPeer::networkMessagesSessionRequest)

{
	auto steam = Steam::get_singleton();
	// s->set_steam_network_peer(this);
	if (steam != nullptr) {
		// steam->connect( "lobby_chat_update", this,"lobbyChatUpdate");
		steam->connect("lobby_created", this, "lobbyCreated");
		steam->connect("lobby_data_update", this, "lobbyDataUpdate");
		steam->connect("lobby_joined", this, "lobbyJoined");
		steam->connect("lobby_game_created", this, "lobbyGameCreated");
		steam->connect("lobby_invite", this, "lobbyInvite");
		steam->connect("lobby_match_list", this, "lobbyMatchList");
		steam->connect("lobby_kicked", this, "lobbyKicked");
		steam->connect("steamworks_error", this, "continue");
		// steam->connect("join_game_requested",this,"joinGameRequested");
		// steam->connect("join_requested",this,"joinRequested");
	}
	this->connectionStatus = ConnectionStatus::CONNECTION_DISCONNECTED;
};
SteamNetworkPeer::~SteamNetworkPeer(){
	closeConnection();
	clear_packets();
};

void SteamNetworkPeer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_all_lobby_data"), &SteamNetworkPeer::getAllLobbyData);
	ClassDB::bind_method(D_METHOD("set_lobby_data", "key", "value"), &SteamNetworkPeer::setLobbyData);
	ClassDB::bind_method(D_METHOD("get_lobby_data", "key"), &SteamNetworkPeer::getLobbyData);

	// ClassDB::bind_method(D_METHOD())
	//I hate this. None of the fucntions should be called externally from script...
	ClassDB::bind_method(D_METHOD("lobbyCreated", "connect", "lobbyId"), &SteamNetworkPeer::lobbyCreated);
	ClassDB::bind_method(D_METHOD("lobbyDataUpdate", "success", "lobbyId", "memberId"), &SteamNetworkPeer::lobbyDataUpdate);
	ClassDB::bind_method(D_METHOD("lobbyJoined", "lobbyId", "permissions", "locked", "response"), &SteamNetworkPeer::lobbyJoined);
	ClassDB::bind_method(D_METHOD("lobbyGameCreated", "lobbyId", "serverId", "serverIp", "port"), &SteamNetworkPeer::lobbyGameCreated);
	ClassDB::bind_method(D_METHOD("lobbyInvite", "inviter", "lobbyId", "game"), &SteamNetworkPeer::lobbyInvite);
	ClassDB::bind_method(D_METHOD("lobbyMatchList", "lobbies"), &SteamNetworkPeer::lobbyMatchList);
	ClassDB::bind_method(D_METHOD("lobbyKicked", "lobbyId", "adminId", "dueToDisconnect"), &SteamNetworkPeer::lobbyKicked);
	// ClassDB::bind_method(D_METHOD("joinGameRequested"), &SteamNetworkPeer::joinGameRequested);
	// ClassDB::bind_method(D_METHOD("joinRequested"), &SteamNetworkPeer::joinRequested);

	ClassDB::bind_method(D_METHOD("create_server", "lobby_type", "max_members"), &SteamNetworkPeer::createServer, DEFVAL(FRIENDS_ONLY), DEFVAL(2));
	ClassDB::bind_method(D_METHOD("create_client", "lobby_id"), &SteamNetworkPeer::createClient);
	ClassDB::bind_method(D_METHOD("close_connection"), &SteamNetworkPeer::closeConnection);

	// peer_connected //pre existing signals
	// peer_disconnectedf
	// connection_succeeded
	// connection_failed
	// server_disconnected

	ADD_SIGNAL(MethodInfo("continue", PropertyInfo(Variant::STRING, "message")));
	ADD_SIGNAL(MethodInfo("lobbyEvent", PropertyInfo(Variant::STRING, "message")));

	// BIND_CONSTANT(DISCONNECTED);
	// BIND_CONSTANT(HOST_PENDING);
	// BIND_CONSTANT(HOST);
	// BIND_CONSTANT(CLIENT_PENDING);
	// BIND_CONSTANT(CLIENT);
	BIND_CONSTANT(PRIVATE);
	BIND_CONSTANT(FRIENDS_ONLY);
	BIND_CONSTANT(PUBLIC);
	BIND_CONSTANT(INVISIBLE);
	BIND_CONSTANT(PRIVATE_UNIQUE);
};

/* Specific to SteamNetworkPeer */
void SteamNetworkPeer::createServer(int lobby_type, int max_members) {
	isServer = true;
	connectionStatus = ConnectionStatus::CONNECTION_CONNECTING;
	Steam::get_singleton()->createLobby(lobby_type, max_members);
}
void SteamNetworkPeer::createClient(uint64_t lobbyId) {
	// ERR_FAIL_COND_MSG(SteamMatchmaking() == nullptr, "Steam is likely not init'd.");
	isServer = false;
	connectionStatus = ConnectionStatus::CONNECTION_CONNECTING;
	SteamMatchmaking()->JoinLobby((uint64)lobbyId);
}
void SteamNetworkPeer::closeConnection(){
	if( lobbyId != CSteamID() ){
		SteamMatchmaking()->LeaveLobby(lobbyId);
		lobbyId = CSteamID();
	}
}
Dictionary SteamNetworkPeer::getAllLobbyData() {
	return lobbyData;
}
void SteamNetworkPeer::setLobbyData(String key, String value) {
	TODO_PRINT("add lobby data");
}
String SteamNetworkPeer::getLobbyData(String key) {
	return lobbyData[key];
}

/* Specific to PacketPeer */
Error SteamNetworkPeer::get_packet(const uint8_t **r_buffer, int &r_buffer_size) {
	delete lastPacket;
	lastPacket = receivedPackets.front()->get();
	if (receivedPackets.size() != 0) {
		r_buffer_size = lastPacket->size;
		*r_buffer = (const uint8_t *)(&lastPacket->data);
		receivedPackets.pop_front();
		return OK;
	}
	return Error::ERR_DOES_NOT_EXIST;
};

void SteamNetworkPeer::clear_packets(){
	while(receivedPackets.front()){
		delete receivedPackets.front();
		receivedPackets.pop_front();
	}
}

Error SteamNetworkPeer::put_packet(const uint8_t *p_buffer, int p_buffer_size) {
	if (activeConnection == nullptr) { //send to ALL
		EResult returnValue = k_EResultOK;
		for (int i = 0; i < connections.size(); i++) {
			auto errorCode = SteamNetworkingMessages()->SendMessageToUser(connections[i].networkIdentity, p_buffer, p_buffer_size, steamNetworkFlag, MAIN_COM_CHANNEL);
			if (errorCode != k_EResultOK) {
				returnValue = errorCode;
			}
		}
		if (returnValue == k_EResultOK) {
			return Error::OK;
		} else {
			return Error(returnValue);
		}
	} else {
		auto errorCode = SteamNetworkingMessages()->SendMessageToUser(activeConnection->networkIdentity, p_buffer, p_buffer_size, steamNetworkFlag, MAIN_COM_CHANNEL);
		if (errorCode == k_EResultOK) {
			return Error::OK;
		}
		return Error(errorCode);
	}
	return Error::ERR_DOES_NOT_EXIST;

	// SteamNetworkingMessages()->SendMessageToUser()
	// bool sentValid = SteamMatchmaking()->SendLobbyChatMsg(lobbyId, p_buffer, p_buffer_size);
	// if (sentValid) {
	// 	return Error::OK;
	// } else {
	// 	ERR_PRINT("packed failed to send!");
	// 	return Error::ERR_CONNECTION_ERROR;
	// }
	// WARN_PRINT("not yet implemented!");
	// return Error::ERR_PRINTER_ON_FIRE;
};

int SteamNetworkPeer::get_max_packet_size() const {
	WARN_PRINT("not yet implemented!");
	return -1;
};

int SteamNetworkPeer::get_available_packet_count() const {
	return this->receivedPackets.size();
};

/* Specific to NetworkedMultiplayerPeer */
void SteamNetworkPeer::set_transfer_mode(SteamNetworkPeer::TransferMode p_mode) {
	// there is a better way to handle this.
	this->transferMode = p_mode;
	switch (p_mode) {
		case SteamNetworkPeer::TransferMode::TRANSFER_MODE_RELIABLE:
			if (noNagle) {
				steamNetworkFlag = k_nSteamNetworkingSend_ReliableNoNagle;
			} else {
				steamNetworkFlag = k_nSteamNetworkingSend_Reliable;
			}
			break;
		case SteamNetworkPeer::TransferMode::TRANSFER_MODE_UNRELIABLE:
		case SteamNetworkPeer::TransferMode::TRANSFER_MODE_UNRELIABLE_ORDERED:
			if (noDelay) {
				steamNetworkFlag = k_nSteamNetworkingSend_UnreliableNoDelay;
			} else if (noNagle) {
				steamNetworkFlag = k_nSteamNetworkingSend_UnreliableNoNagle;
			} else {
				steamNetworkFlag = k_nSteamNetworkingSend_Unreliable;
			}
			break;
	}
};

SteamNetworkPeer::TransferMode SteamNetworkPeer::get_transfer_mode() const {
	return this->transferMode;
};

void SteamNetworkPeer::set_target_peer(int p_peer_id) {
	targetPeer = p_peer_id;
	if (p_peer_id == 0) {
		activeConnection = nullptr;
	} else {
		activeConnection = getConnectionByGodotId(p_peer_id);
	}
};

int SteamNetworkPeer::get_packet_peer() const {
	auto a = receivedPackets.front()->get()->sender;
	return a == lobbyOwner ? 1 : a.GetAccountID();
};

bool SteamNetworkPeer::is_server() const {
	return isServer;
};

#define MAX_MESSAGE_COUNT 255
void SteamNetworkPeer::poll() {
	SteamNetworkingMessage_t *messages[MAX_MESSAGE_COUNT];
	int count = SteamNetworkingMessages()->ReceiveMessagesOnChannel(MAIN_COM_CHANNEL, messages, MAX_MESSAGE_COUNT);
	for (int i = 0; i < count; i++) {
		auto msg = messages[i];
		Packet *packet = new Packet;
		packet->channel = MAIN_COM_CHANNEL;
		packet->sender = msg->m_identityPeer.GetSteamID();
		packet->size = msg->GetSize();
		ERR_FAIL_COND_MSG(packet->size > MAX_STEAM_PACKET_SIZE, "PACKET TOO LARGE!");
		auto rawData = (char *)msg->GetData();
		for (uint32_t j = 0; j < packet->size; j++) {
			packet->data[j] = rawData[j];
		}
		receivedPackets.push_back(packet);
		msg->Release();
	}
};

int SteamNetworkPeer::get_unique_id() const {
	// return receivedPackets.front()->get()->sender.GetAccountID();
	if (isServer) {
		return 1;
	}
	return SteamUser()->GetSteamID().GetAccountID();
};

void SteamNetworkPeer::set_refuse_new_connections(bool p_enable) {
	refuseConnections = p_enable;
	TODO_PRINT("figure out if I need to do something on the steam side here");
};

bool SteamNetworkPeer::is_refusing_new_connections() const {
	return refuseConnections;
};

SteamNetworkPeer::ConnectionStatus SteamNetworkPeer::get_connection_status() const {
	return connectionStatus;
};

/* Callbacks */

void SteamNetworkPeer::lobbyChatUpdate(LobbyChatUpdate_t *call_data) {
	if (lobbyId != call_data->m_ulSteamIDLobby) {
		return;
	}
	CSteamID userChanged = CSteamID(call_data->m_ulSteamIDUserChanged);
	switch (CHAT_CHANGE(call_data->m_rgfChatMemberStateChange)) {
		case CHAT_CHANGE::ENTERED:
			if (userChanged != SteamUser()->GetSteamID()) {
				addConnectionPeer(userChanged);
			}
			break;
		case CHAT_CHANGE::LEFT:
		case CHAT_CHANGE::DISCONNECTED:
		case CHAT_CHANGE::KICKED:
		case CHAT_CHANGE::BANNED:
			if (userChanged != SteamUser()->GetSteamID()) {
				removeConnectionPeer(userChanged);
			}
			break;
	}
};
void SteamNetworkPeer::networkMessagesSessionRequest(SteamNetworkingMessagesSessionRequest_t *t) {
	//search for lobby member
	CSteamID requester = t->m_identityRemote.GetSteamID();
	int currentLobbySize = SteamMatchmaking()->GetNumLobbyMembers(lobbyId);
	for (int i = 0; i < currentLobbySize; i++) {
		if (SteamMatchmaking()->GetLobbyMemberByIndex(lobbyId, i) == requester) {
			SteamNetworkingMessages()->AcceptSessionWithUser(t->m_identityRemote);
			return;
		}
	}
	ERR_PRINT(String("CONNECTION ATTEMPTED BY PLAYER NOT IN LOBBY!:") + String::num_uint64(requester.GetAccountID()));
}

void SteamNetworkPeer::lobbyCreated(int connect, uint64_t lobbyId) {
	if (connect == 1) {
		this->lobbyId = CSteamID(uint64(lobbyId));
		this->lobbyOwner = SteamMatchmaking()->GetLobbyOwner(this->lobbyId);
		connectionStatus = ConnectionStatus::CONNECTION_CONNECTED;
		isServer = true;
		// emit_signal("connection_succeeded");
		// emit_signal("peer_connected",1);
	} else {
		ERR_PRINT("ERROR ON LOBBY CREATION!");
		emit_signal("connection_failed");
	}
};

void SteamNetworkPeer::lobbyDataUpdate(uint8_t success, uint64_t lobbyId, uint64_t memberId) {
	if (this->lobbyId.ConvertToUint64() != lobbyId) {
		return;
	} else if (success == EResult::k_EResultOK) {
		if (lobbyId == memberId) {
			// WARN_PRINT("lobbyGameCreated: todo, update lobby itself!");
			//update the entire lobby
			int playerCount = SteamMatchmaking()->GetNumLobbyMembers(this->lobbyId);
			for (int i = 0; i < playerCount; i++) {
				CSteamID lobbyMember = SteamMatchmaking()->GetLobbyMemberByIndex(this->lobbyId, i);
				addConnectionPeer(lobbyMember);
			}
			// TODO_PRINT("Update entire lobby!");
		} else {
			TODO_PRINT("todo, user data!");
		}
		updateLobbyData();
	} else
		ERR_PRINT("failed!");
};

void SteamNetworkPeer::updateLobbyData() {
	//set all lobby data
	Dictionary data;
	int dataCount = SteamMatchmaking()->GetLobbyDataCount(lobbyId);
	char key[MAX_LOBBY_KEY_LENGTH];
	char value[MAX_CHAT_METADATA];
	for (int i = 0; i < dataCount; i++) {
		bool success = SteamMatchmaking()->GetLobbyDataByIndex(lobbyId, i, key, MAX_LOBBY_KEY_LENGTH, value, MAX_CHAT_METADATA);
		if (success) {
			data["index"] = i;
			data["key"] = key;
			data["value"] = value;
		}
	}
	lobbyData = data;
};

void SteamNetworkPeer::lobbyJoined(uint64_t lobbyId, uint32_t permissions, bool locked, uint32_t response) {
	if (response == k_EChatRoomEnterResponseSuccess) {
		this->lobbyId = CSteamID(uint64(lobbyId));
		this->lobbyOwner = SteamMatchmaking()->GetLobbyOwner(this->lobbyId);
		connectionStatus = ConnectionStatus::CONNECTION_CONNECTED;
		if (isServer) {
			//don't do stuff if you're already the host
		} else {
			emit_signal("connection_succeeded");
		}
	} else {
		String output = "";
		switch (response) {
			// k_EChatRoomEnterResponseSuccess: 			output = "k_EChatRoomEnterResponseSuccess"; break;
			case k_EChatRoomEnterResponseDoesntExist:
				output = "Doesn't Exist";
				break;
			case k_EChatRoomEnterResponseNotAllowed:
				output = "Not Allowed";
				break;
			case k_EChatRoomEnterResponseFull:
				output = "Full";
				break;
			case k_EChatRoomEnterResponseError:
				output = "Error";
				break;
			case k_EChatRoomEnterResponseBanned:
				output = "Banned";
				break;
			case k_EChatRoomEnterResponseLimited:
				output = "Limited";
				break;
			case k_EChatRoomEnterResponseClanDisabled:
				output = "Clan Disabled";
				break;
			case k_EChatRoomEnterResponseCommunityBan:
				output = "Community Ban";
				break;
			case k_EChatRoomEnterResponseMemberBlockedYou:
				output = "Member Blocked You";
				break;
			case k_EChatRoomEnterResponseYouBlockedMember:
				output = "You Blocked Member";
				break;
			case k_EChatRoomEnterResponseRatelimitExceeded:
				output = "Ratelimit Exceeded";
				break;
		};
		if (output.length() != 0) {
			ERR_PRINT("Joined lobby failed!" + output);
			emit_signal("connection_failed");
			return;
		}
	}
};

void SteamNetworkPeer::lobbyGameCreated(uint64_t lobbyId, uint64_t serverId, String serverIp, uint16_t port) {
	WARN_PRINT("not yet implemented!");
};

void SteamNetworkPeer::lobbyInvite(uint64_t inviter, uint64_t lobbyId, uint64_t game) {
	WARN_PRINT("not yet implemented!");
};

void SteamNetworkPeer::lobbyMatchList(Array lobbies) {
	WARN_PRINT("not yet implemented!");
};

void SteamNetworkPeer::lobbyKicked(uint64_t lobbyId, uint64_t adminId, uint8_t dueToDisconnect) {
	WARN_PRINT("not yet implemented!");
};

/* Steam_API direct callbacks */
void SteamNetworkPeer::lobbyMessage(LobbyChatMsg_t *call_data) {
	if (lobbyId != call_data->m_ulSteamIDLobby) {
		return;
	}
	Packet *packet = new Packet;

	packet->sender = call_data->m_ulSteamIDUser;
	if (SteamUser()->GetSteamID() == packet->sender) {
		return;
	}
	uint8 chat_type = call_data->m_eChatEntryType;
	// Convert the chat type over
	EChatEntryType type = (EChatEntryType)chat_type;
	// Get the chat message data
	packet->size = SteamMatchmaking()->GetLobbyChatEntry(lobbyId, call_data->m_iChatID, &(packet->sender), &(packet->data), MAX_STEAM_PACKET_SIZE, &type);
	packet->channel = -1;

	receivedPackets.push_back(packet);

	// 	lobbyMessage( lobby, user, out, chat_type);
	// }
	// void SteamNetworkPeer::lobbyMessage( uint64_t lobbyId, uint64_t user, PoolByteArray message, uint8 chatType){
	// p.channel
	// WARN_PRINT("not yet implemented!");
	// TODO_PRINT("here I need to store messages. Packets are consumed with get_packet calls!");
};

// void SteamNetworkPeer::joinGameRequested(uint64_t lobbyId, String connect){
// 	WARN_PRINT("not yet implemented!");
// }

// void SteamNetworkPeer::joinRequested(GameLobbyJoinRequested_t* call_data){
// 	CSteamID lobby_id = call_data->m_steamIDLobby;
// 	uint64_t lobby = lobby_id.ConvertToUint64();
// 	CSteamID friend_id = call_data->m_steamIDFriend;
// 	uint64_t steam_id = friend_id.ConvertToUint64();
// 	// emit_signal("join_requested", lobby, steam_id);
// }
// void SteamNetworkPeer::joinRequested(int lobby,int steam_id){
// 	WARN_PRINT("not yet implemented!");
// }
