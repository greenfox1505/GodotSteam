#ifndef STEAM_MULTIPLAYER_PEER_H
#define STEAM_MULTIPLAYER_PEER_H

// Include Godot headers
#include "scene/main/multiplayer_peer.h"

// Steam APIs
#include "core/os/os.h"
#include "godotsteam.h"
// #include "steam_id.h"


class SteamMultiplayerPeer : public MultiplayerPeer
{
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

	// MultiplayerPeer stuff
	virtual int get_available_packet_count() const override;
	virtual Error get_packet(const uint8_t **r_buffer, int &r_buffer_size) override;
	virtual Error put_packet(const uint8_t *p_buffer, int p_buffer_size) override;
	virtual int get_max_packet_size() const override;
	virtual bool is_server_relay_supported() const override;

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

	enum CHANNEL_MANAGEMENT
	{
		PING_CHANNEL,
		SIZE
	};

	struct PingPayload
	{
		int peer_id;
		CSteamID steam_id;
	};

	enum LOBBY_TYPE
	{
		PRIVATE = ELobbyType::k_ELobbyTypePrivate,
		FRIENDS_ONLY = ELobbyType::k_ELobbyTypeFriendsOnly,
		PUBLIC = ELobbyType::k_ELobbyTypePublic,
		INVISIBLE = ELobbyType::k_ELobbyTypeInvisible,
		PRIVATE_UNIQUE = ELobbyType::k_ELobbyTypePrivateUnique,
	};

	enum CHAT_CHANGE
	{
		ENTERED = k_EChatMemberStateChangeEntered,
		LEFT = k_EChatMemberStateChangeLeft,
		DISCONNECTED = k_EChatMemberStateChangeDisconnected,
		KICKED = k_EChatMemberStateChangeKicked,
		BANNED = k_EChatMemberStateChangeBanned
	};

	enum LOBBY_STATE
	{
		NOT_CONNECTED,
		HOST_PENDING,
		HOSTING,
		CLIENT_PENDING,
		CLIENT
	} lobby_state = LOBBY_STATE::NOT_CONNECTED;
	LOBBY_STATE get_state() { return lobby_state; }

	bool no_nagle = true;
	bool no_delay = false;

	int32_t target_peer = -1;
	int32_t unique_id = -1;
	// ConnectionStatus connection_status = ConnectionStatus::CONNECTION_DISCONNECTED;
	TransferMode transfer_mode = TransferMode::TRANSFER_MODE_RELIABLE;

	struct Packet
	{
		uint8_t data[MAX_STEAM_PACKET_SIZE];
		uint32_t size;
		CSteamID sender;

		int channel = 0;
		TransferMode transfer_mode = TRANSFER_MODE_RELIABLE;
	};
	Packet *current_packet = new Packet; // this packet gets deleted at the first get_packet request
	List<Packet *> incoming_packets;

	_FORCE_INLINE_ bool _is_active() const { return lobby_state != LOBBY_STATE::NOT_CONNECTED; }

	class ConnectionData : public RefCounted
	{
		GDCLASS(ConnectionData, RefCounted);

	public:
		int peer_id;
		CSteamID steamId;
		SteamNetworkingIdentity networkIdentity;

		ConnectionData(CSteamID steamId)
		{
			this->peer_id = -1;
			this->steamId = steamId;
			networkIdentity = SteamNetworkingIdentity();
			networkIdentity.SetSteamID(steamId);
		}
		ConnectionData(){};
		~ConnectionData()
		{
			SteamNetworkingMessages()->CloseSessionWithUser(networkIdentity);
		}
		bool operator==(const ConnectionData &data)
		{
			return steamId == data.steamId;
		}
		Error send(const void *p_buffer, uint32 p_buffer_size, int transferMode, int channel)
		{
			switch (SteamNetworkingMessages()->SendMessageToUser(
				networkIdentity,
				p_buffer,
				p_buffer_size,
				transferMode,
				channel))
			{
			case k_EResultOK:
				return OK;
			case k_EResultNoConnection:
				ERR_FAIL_V_MSG(ERR_DOES_NOT_EXIST, "Send Error: k_EResultNoConnection");
			case k_nSteamNetworkingSend_AutoRestartBrokenSession:
				ERR_FAIL_V_MSG(ERR_UNAUTHORIZED, "Send Error: k_nSteamNetworkingSend_AutoRestartBrokenSession");
			default:
				ERR_FAIL_V_MSG(ERR_BUG, "Send Error: don't know what this error is, but it's not on the expected errors list...");
			}
		}
	};

	HashMap<__int64, Ref<ConnectionData>> connections_by_steamId;

	HashMap<__int64, int> steamId_to_peerId;
	HashMap<int, CSteamID> peerId_to_steamId;

	int get_peer_id(CSteamID steamId);
	CSteamID get_steam_id(int peer);
	void set_steam_id_peer(CSteamID steamId, int peer_id);
	Ref<ConnectionData> get_connection_by_peer(int peer_id);

	void SteamMultiplayerPeer::add_connection_peer(const CSteamID &steamId, int peer_id);
	void add_pending_peer(const CSteamID &steamId);
	void removed_connection_peer(const CSteamID &steamId);

	Error create_lobby(LOBBY_TYPE lobbyType, int max_players);
	Error join_lobby(uint64 lobbyId);

	STEAM_CALLBACK(SteamMultiplayerPeer, lobby_message_scb, LobbyChatMsg_t, callbackLobbyMessage);
	STEAM_CALLBACK(SteamMultiplayerPeer, lobby_chat_update_scb, LobbyChatUpdate_t, callbackLobbyChatUpdate);
	STEAM_CALLBACK(SteamMultiplayerPeer, network_messages_session_request_scb, SteamNetworkingMessagesSessionRequest_t, callbackNetworkMessagesSessionRequest);
	STEAM_CALLBACK(SteamMultiplayerPeer, network_messages_session_failed_scb, SteamNetworkingMessagesSessionFailed_t, callbackNetworkMessagesSessionFailed);
	STEAM_CALLBACK(SteamMultiplayerPeer, lobby_joined_scb, LobbyEnter_t, callbackLobbyJoined);

	int _get_steam_transfer_flag();

	void process_message(const SteamNetworkingMessage_t *msg);
	void process_ping(const SteamNetworkingMessage_t *msg);
	// void poll_channel(int nLocalChannel, void (*func)(SteamNetworkingMessage_t));
};

#endif
