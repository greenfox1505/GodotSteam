#ifndef STEAM_MULTIPLAYER_PEER_H
#define STEAM_MULTIPLAYER_PEER_H

// Include Godot headers
#include "scene/main/multiplayer_peer.h"

//Steam APIs
#include "core/os/os.h"
#include "godotsteam.h"

class SteamID : public RefCounted {
	GDCLASS(SteamID, RefCounted);

	CSteamID data;

public:
	uint64 to_int() {
		return data.ConvertToUint64();
	}
	void from_int(uint64 i) {
		data.SetFromUint64(i);
	}

protected:
	static void _bind_methods();
};

class SteamMultiplayerPeer : public MultiplayerPeer {
public:
	GDCLASS(SteamMultiplayerPeer, MultiplayerPeer);

public:
	// Matchmaking call results ///////////// stolen
	CCallResult<SteamMultiplayerPeer, LobbyCreated_t> callResultCreateLobby;
	void lobby_created(LobbyCreated_t *call_data, bool io_failure);
	CCallResult<SteamMultiplayerPeer, LobbyMatchList_t> callResultLobbyList;
	void lobby_match_list(LobbyMatchList_t *call_data, bool io_failure);

	CSteamID lobby_id = CSteamID();
	CSteamID lobby_owner = CSteamID();

	SteamMultiplayerPeer();
	uint64 get_lobby_id();

	static void _bind_methods();

	//MultiplayerPeer stuff
	virtual int get_available_packet_count() const override;
	virtual Error get_packet(const uint8_t **r_buffer, int &r_buffer_size) override;
	virtual Error put_packet(const uint8_t *p_buffer, int p_buffer_size) override;
	virtual int get_max_packet_size() const override;

	virtual void set_target_peer(int p_peer_id) override;
	virtual int get_packet_peer() const override;
	virtual TransferMode get_packet_mode() const override;
	virtual int get_packet_channel() const override;
	virtual void disconnect_peer(int p_peer, bool p_force = false) override;
	virtual bool is_server() const override;
	virtual void poll() override;
	virtual void close() override;
	virtual int get_unique_id() const override;
	virtual ConnectionStatus get_connection_status() const override;

	// all SteamGodot from here on down

	enum LOBBY_TYPE {
		PRIVATE = ELobbyType::k_ELobbyTypePrivate,
		FRIENDS_ONLY = ELobbyType::k_ELobbyTypeFriendsOnly,
		PUBLIC = ELobbyType::k_ELobbyTypePublic,
		INVISIBLE = ELobbyType::k_ELobbyTypeInvisible,
		PRIVATE_UNIQUE = ELobbyType::k_ELobbyTypePrivateUnique,
	};

	enum CHAT_CHANGE {
		ENTERED = k_EChatMemberStateChangeEntered,
		LEFT = k_EChatMemberStateChangeLeft,
		DISCONNECTED = k_EChatMemberStateChangeDisconnected,
		KICKED = k_EChatMemberStateChangeKicked,
		BANNED = k_EChatMemberStateChangeBanned
	};

	enum LOBBY_STATE {
		NOT_CONNECTED,
		HOST_PENDING,
		HOST,
		CLIENT_PENDING,
		CLIENT
	} lobbyState = LOBBY_STATE::NOT_CONNECTED;

	int32_t target_peer = -1;
	int32_t unique_id = -1;
	ConnectionStatus connection_status = ConnectionStatus::CONNECTION_DISCONNECTED;
	TransferMode transferMode = TransferMode::TRANSFER_MODE_RELIABLE;

	struct Packet {
		uint8_t data[MAX_STEAM_PACKET_SIZE];
		uint32_t size;
		CSteamID sender;

		int from = 0;
		int channel = 0;
		TransferMode transfer_mode = TRANSFER_MODE_RELIABLE;
	};
	Packet *current_packet;
	List<Packet *> incoming_packets;

	_FORCE_INLINE_ bool _is_active() const { return lobbyState != LOBBY_STATE::NOT_CONNECTED; }

	struct ConnectionData {
		CSteamID steamId;
		uint32_t godotId;
		SteamNetworkingIdentity networkIdentity;
		ConnectionData(const CSteamID &steamId, const uint32_t &godotId) {
			this->steamId = steamId;
			this->godotId = godotId;
			networkIdentity = SteamNetworkingIdentity();
			networkIdentity.SetSteamID(steamId);
		}
		ConnectionData(){};
		bool operator==(const ConnectionData &data) {
			return godotId = data.godotId;
		}
		// bool operator==(const ConnectionData& data){
		// 	return godotId = data.godotId;
		// }
	};

	List<ConnectionData> connections; //list is a linked list and a bad data structure for this. todo fix
	const ConnectionData *activeConnection;

	bool add_connection_peer(const CSteamID &steamId);
	void removed_connection_peer(const CSteamID &steamId);

	Error create_lobby(LOBBY_TYPE lobbyType, int max_players);
	Error connect_lobby(uint64 lobbyId); //todo function signature here!

	STEAM_CALLBACK(SteamMultiplayerPeer, lobby_message, LobbyChatMsg_t, callbackLobbyMessage);
	STEAM_CALLBACK(SteamMultiplayerPeer, lobby_chat_update, LobbyChatUpdate_t, callbackLobbyChatUpdate);
	STEAM_CALLBACK(SteamMultiplayerPeer, network_messages_session_request, SteamNetworkingMessagesSessionRequest_t, callbackNetworkMessagesSessionRequest);
};

#endif
