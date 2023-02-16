#include "steam_multiplayer_peer.h"

#include "godotsteam.h"

#define MAIN_COM_CHANNEL 1

VARIANT_ENUM_CAST(SteamMultiplayerPeer::LOBBY_TYPE);
VARIANT_ENUM_CAST(SteamMultiplayerPeer::CHAT_CHANGE);
VARIANT_ENUM_CAST(SteamMultiplayerPeer::LOBBY_STATE);

SteamMultiplayerPeer::SteamMultiplayerPeer() :
		callbackLobbyMessage(this, &SteamMultiplayerPeer::lobby_message),
		callbackLobbyChatUpdate(this, &SteamMultiplayerPeer::lobby_chat_update),
		callbackNetworkMessagesSessionRequest(this, &SteamMultiplayerPeer::network_messages_session_request),
		callbackLobbyJoined(this, &SteamMultiplayerPeer::lobby_joined)
{
	// connection_status = ConnectionStatus::CONNECTION_DISCONNECTED;
}

uint64 SteamMultiplayerPeer::get_lobby_id() {
	return lobby_id.ConvertToUint64();
}

void SteamMultiplayerPeer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_lobby", "lobby_type", "max_players"), &SteamMultiplayerPeer::create_lobby, DEFVAL(32));
	ClassDB::bind_method(D_METHOD("connect_lobby", "lobby_id"), &SteamMultiplayerPeer::join_lobby);

	BIND_ENUM_CONSTANT(PRIVATE);
	BIND_ENUM_CONSTANT(FRIENDS_ONLY);
	BIND_ENUM_CONSTANT(PUBLIC);
	BIND_ENUM_CONSTANT(INVISIBLE);
	BIND_ENUM_CONSTANT(PRIVATE_UNIQUE);

	BIND_ENUM_CONSTANT(ENTERED);
	BIND_ENUM_CONSTANT(LEFT);
	BIND_ENUM_CONSTANT(DISCONNECTED);
	BIND_ENUM_CONSTANT(KICKED);
	BIND_ENUM_CONSTANT(BANNED);

	BIND_ENUM_CONSTANT(DISCONNECTED);
	BIND_ENUM_CONSTANT(HOST_PENDING);
	BIND_ENUM_CONSTANT(HOSTING);
	BIND_ENUM_CONSTANT(CLIENT_PENDING);
	BIND_ENUM_CONSTANT(CLIENT);

	// MATCHMAKING SIGNALS //////////////////////
	ADD_SIGNAL(MethodInfo("favorites_list_accounts_updated", PropertyInfo(Variant::INT, "result")));
	ADD_SIGNAL(MethodInfo("favorites_list_changed", PropertyInfo(Variant::DICTIONARY, "favorite")));
	ADD_SIGNAL(MethodInfo("lobby_message", PropertyInfo(Variant::INT, "lobby_id"), PropertyInfo(Variant::INT, "user"), PropertyInfo(Variant::STRING, "message"), PropertyInfo(Variant::INT, "chat_type")));
	ADD_SIGNAL(MethodInfo("lobby_chat_update", PropertyInfo(Variant::INT, "lobby_id"), PropertyInfo(Variant::INT, "changed_id"), PropertyInfo(Variant::INT, "making_change_id"), PropertyInfo(Variant::INT, "chat_state")));
	ADD_SIGNAL(MethodInfo("lobby_created", PropertyInfo(Variant::INT, "connect"), PropertyInfo(Variant::INT, "lobby_id")));
	ADD_SIGNAL(MethodInfo("lobby_data_update"));
	ADD_SIGNAL(MethodInfo("lobby_joined", PropertyInfo(Variant::INT, "lobby"), PropertyInfo(Variant::INT, "permissions"), PropertyInfo(Variant::BOOL, "locked"), PropertyInfo(Variant::INT, "response")));
	ADD_SIGNAL(MethodInfo("lobby_game_created", PropertyInfo(Variant::INT, "lobby_id"), PropertyInfo(Variant::INT, "server_id"), PropertyInfo(Variant::STRING, "server_ip"), PropertyInfo(Variant::INT, "port")));
	ADD_SIGNAL(MethodInfo("lobby_invite", PropertyInfo(Variant::INT, "inviter"), PropertyInfo(Variant::INT, "lobby"), PropertyInfo(Variant::INT, "game")));
	ADD_SIGNAL(MethodInfo("lobby_match_list", PropertyInfo(Variant::ARRAY, "lobbies")));
	ADD_SIGNAL(MethodInfo("lobby_kicked", PropertyInfo(Variant::INT, "lobby_id"), PropertyInfo(Variant::INT, "admin_id"), PropertyInfo(Variant::INT, "due_to_disconnect")));
}

int SteamMultiplayerPeer::get_available_packet_count() const {
	return incoming_packets.size();
}

Error SteamMultiplayerPeer::get_packet(const uint8_t **r_buffer, int &r_buffer_size) {
	ERR_PRINT("ERROR:: SteamMultiplayerPeer::put_packet not yet implemented");
	ERR_FAIL_COND_V_MSG(incoming_packets.size() == 0, ERR_UNAVAILABLE, "No incoming packets available.");

	current_packet = incoming_packets.front()->get();
	incoming_packets.pop_front();

	// *r_buffer = (const uint8_t *)(current_packet.packet->data);
	// r_buffer_size = current_packet.packet->dataLength;

	return OK;
}

Error SteamMultiplayerPeer::put_packet(const uint8_t *p_buffer, int p_buffer_size) {
	ERR_PRINT("ERROR:: SteamMultiplayerPeer::put_packet not yet implemented");
	return Error::ERR_PRINTER_ON_FIRE;
}

int SteamMultiplayerPeer::get_max_packet_size() const {
	return MAX_STEAM_PACKET_SIZE; //from ENet
}

void SteamMultiplayerPeer::set_target_peer(int p_peer_id) {
	target_peer = p_peer_id;
}

int SteamMultiplayerPeer::get_packet_peer() const {
	ERR_FAIL_COND_V_MSG(!_is_active(), 1, "The multiplayer instance isn't currently active.");
	ERR_FAIL_COND_V(incoming_packets.size() == 0, 1);

	return incoming_packets.front()->get()->from;
}

SteamMultiplayerPeer::TransferMode SteamMultiplayerPeer::get_packet_mode() const {
	ERR_FAIL_COND_V_MSG(!_is_active(), TRANSFER_MODE_RELIABLE, "The multiplayer instance isn't currently active.");
	ERR_FAIL_COND_V(incoming_packets.size() == 0, TRANSFER_MODE_RELIABLE);

	return incoming_packets.front()->get()->transfer_mode;
}

int SteamMultiplayerPeer::get_packet_channel() const {
	ERR_FAIL_COND_V_MSG(!_is_active(), 1, "The multiplayer instance isn't currently active.");
	ERR_FAIL_COND_V(incoming_packets.size() == 0, 1);
	int ch = incoming_packets.front()->get()->channel;
	// if (ch >= SYSCH_MAX) { // First 2 channels are reserved.
	// 	return ch - SYSCH_MAX + 1;
	// }
	return 0;
}

void SteamMultiplayerPeer::disconnect_peer(int p_peer, bool p_force) {
	ERR_PRINT("ERROR:: SteamMultiplayerPeer::disconnect_peer not yet implemented");
}

bool SteamMultiplayerPeer::is_server() const {
	return unique_id == 1;
}

#define MAX_MESSAGE_COUNT 255
void SteamMultiplayerPeer::poll() {
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
		incoming_packets.push_back(packet);
		msg->Release();
	}
}

void SteamMultiplayerPeer::close() {
	ERR_PRINT("ERROR:: SteamMultiplayerPeer::close not yet implemented");
}

int SteamMultiplayerPeer::get_unique_id() const {
	ERR_FAIL_COND_V_MSG(!_is_active(), 0, "The multiplayer instance isn't currently active.");
	return unique_id;
}

SteamMultiplayerPeer::ConnectionStatus SteamMultiplayerPeer::get_connection_status() const {
	if (lobbyState == LOBBY_STATE::NOT_CONNECTED) {
		return ConnectionStatus::CONNECTION_DISCONNECTED;
	} else if (lobbyState == LOBBY_STATE::CLIENT || lobbyState == LOBBY_STATE::HOSTING) {
		return ConnectionStatus::CONNECTION_CONNECTED;
	} else {
		return ConnectionStatus::CONNECTION_CONNECTING;
	}
}

bool SteamMultiplayerPeer::add_connection_peer(const CSteamID &steamId) {
	if (steamId == SteamUser()->GetSteamID()) {
		return false;
	} else {
		int godotId = steamId == lobby_owner ? 1 : steamId.GetAccountID(); //maybe this account ID needs to be something else?
		auto connection = connections.push_back(ConnectionData(steamId, godotId))->get();
		emit_signal("peer_connected", godotId);
		SteamNetworkingMessages()->AcceptSessionWithUser(connection.networkIdentity);
		return true;
	}
}

void SteamMultiplayerPeer::removed_connection_peer(const CSteamID &steamId) {
	auto a = connections.front();
	while (a) {
		auto element = a->get();
		if (element.steamId == steamId) {
			SteamNetworkingMessages()->CloseSessionWithUser(element.networkIdentity);
			emit_signal("peer_disconnected", element.godotId);
			connections.erase(a);
			return;
		} else {
			a = a->next();
		}
	}
}

Error SteamMultiplayerPeer::create_lobby(LOBBY_TYPE lobby_type, int max_players) {
	if (SteamMatchmaking() != NULL) {
		SteamAPICall_t api_call = SteamMatchmaking()->CreateLobby((ELobbyType)lobby_type, max_players);
		callResultCreateLobby.Set(api_call, this, &SteamMultiplayerPeer::lobby_created);
		return OK;
	} else {
		ERR_PRINT("`SteamMatchmaking()` is null.");
		return ERR_DOES_NOT_EXIST;
	}
}

void SteamMultiplayerPeer::lobby_created(LobbyCreated_t *lobby_data, bool io_failure) {
	if (io_failure) {
		lobbyState = LOBBY_STATE::NOT_CONNECTED;
		ERR_FAIL_MSG("lobby_created failed? idk wtf is happening");
		// steamworksError("lobby_created");
	} else {
		unique_id = 1;
		lobbyState = LOBBY_STATE::HOSTING;
		int connect = lobby_data->m_eResult;
		lobby_id = lobby_data->m_ulSteamIDLobby;
		uint64 lobby = lobby_id.ConvertToUint64();
		emit_signal("lobby_created", connect, lobby);
	}
}

Error SteamMultiplayerPeer::join_lobby(uint64 lobbyId) {
	if (SteamMatchmaking() != NULL) {
		lobbyState = LOBBY_STATE::CLIENT_PENDING;
		this->lobby_id = lobbyId;
		SteamMatchmaking()->JoinLobby(CSteamID(lobbyId));
	}
	return OK;
}

void SteamID::_bind_methods() {
	ClassDB::bind_method(D_METHOD("to_int"), &SteamID::to_int);
	ClassDB::bind_method(D_METHOD("from_int", "id"), &SteamID::from_int);
}

void SteamMultiplayerPeer::lobby_message(LobbyChatMsg_t *call_data) {
	if (lobby_id != call_data->m_ulSteamIDLobby) {
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
	packet->size = SteamMatchmaking()->GetLobbyChatEntry(lobby_id, call_data->m_iChatID, &(packet->sender), &(packet->data), MAX_STEAM_PACKET_SIZE, &type);
	packet->channel = -1;

	incoming_packets.push_back(packet);
};

void SteamMultiplayerPeer::lobby_chat_update(LobbyChatUpdate_t *call_data) {
	if (lobby_id != call_data->m_ulSteamIDLobby) {
		return;
	}
	CSteamID userChanged = CSteamID(call_data->m_ulSteamIDUserChanged);
	switch (CHAT_CHANGE(call_data->m_rgfChatMemberStateChange)) {
		case CHAT_CHANGE::ENTERED:
			if (userChanged != SteamUser()->GetSteamID()) {
				add_connection_peer(userChanged);
			}
			break;
		case CHAT_CHANGE::LEFT:
		case CHAT_CHANGE::DISCONNECTED:
		case CHAT_CHANGE::KICKED:
		case CHAT_CHANGE::BANNED:
			if (userChanged != SteamUser()->GetSteamID()) {
				removed_connection_peer(userChanged);
			}
			break;
	}
};

void SteamMultiplayerPeer::network_messages_session_request(SteamNetworkingMessagesSessionRequest_t *t) {
	//search for lobby member
	CSteamID requester = t->m_identityRemote.GetSteamID();
	int currentLobbySize = SteamMatchmaking()->GetNumLobbyMembers(lobby_id);
	for (int i = 0; i < currentLobbySize; i++) {
		if (SteamMatchmaking()->GetLobbyMemberByIndex(lobby_id, i) == requester) {
			SteamNetworkingMessages()->AcceptSessionWithUser(t->m_identityRemote);
			return;
		}
	}
	ERR_PRINT(String("CONNECTION ATTEMPTED BY PLAYER NOT IN LOBBY!:") + String::num_uint64(requester.GetAccountID()));
};

void SteamMultiplayerPeer::lobby_joined(LobbyEnter_t* lobbyData){
	if(lobbyData->m_ulSteamIDLobby != this->lobby_id.ConvertToUint64()){
		return;
	}
	lobbyState = LOBBY_STATE::CLIENT;



	// CSteamID steam_lobby_id = lobbyData->m_ulSteamIDLobby;
	// uint64_t lobby_id = steam_lobby_id.ConvertToUint64();
	// uint32_t permissions = lobbyData->m_rgfChatPermissions;
	// bool locked = lobbyData->m_bLocked;
	// uint32_t response = lobbyData->m_EChatRoomEnterResponse;
	// emit_signal("lobby_joined", lobby_id, permissions, locked, response);
	//TODO 
	}