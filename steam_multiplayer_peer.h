#ifndef STEAM_MULTIPLAYER_PEER_H
#define STEAM_MULTIPLAYER_PEER_H

// Include Godot headers
#include "scene/main/multiplayer_peer.h"

// Steam APIs
#include "core/os/os.h"
#include "godotsteam.h"

// #include "steam_id.h"

#define MAX_TIME_WITHOUT_MESSAGE 1000

Dictionary steamIdToDict(CSteamID input);

class SteamMultiplayerPeer : public MultiplayerPeer {
public:
	GDCLASS(SteamMultiplayerPeer, MultiplayerPeer);
	String convertEResultToString(EResult e);

	Dictionary get_peer_info(int i);

public:
	// Matchmaking call results ///////////// stolen
	CCallResult<SteamMultiplayerPeer, LobbyCreated_t> callResultCreateLobby;
	void lobby_created_scb(LobbyCreated_t *call_data, bool io_failure);
	CCallResult<SteamMultiplayerPeer, LobbyMatchList_t> callResultLobbyList;
	void lobby_match_list_scb(LobbyMatchList_t *call_data, bool io_failure);

	CSteamID lobby_id = CSteamID();
	CSteamID lobby_owner = CSteamID();
	CSteamID steam_id = CSteamID();

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

	enum CHANNEL_MANAGEMENT {
		PING_CHANNEL,
		SIZE
	};

	struct PingPayload {
		int peer_id = -1;
		CSteamID steam_id = CSteamID();
	};

	enum LOBBY_TYPE {
		PRIVATE = ELobbyType::k_ELobbyTypePrivate,
		FRIENDS_ONLY = ELobbyType::k_ELobbyTypeFriendsOnly,
		PUBLIC = ELobbyType::k_ELobbyTypePublic,
		INVISIBLE = ELobbyType::k_ELobbyTypeInvisible,
		// PRIVATE_UNIQUE = ELobbyType::k_ELobbyTypePrivateUnique, //this type must be created from Steam's web api.
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
		HOSTING,
		CLIENT_PENDING,
		CLIENT
	} lobby_state = LOBBY_STATE::NOT_CONNECTED;
	LOBBY_STATE get_state() { return lobby_state; }

	bool no_nagle = false;
	bool no_delay = false;

	int32_t target_peer = -1;
	int32_t unique_id = -1;
	// ConnectionStatus connection_status = ConnectionStatus::CONNECTION_DISCONNECTED;
	// TransferMode transfer_mode = TransferMode::TRANSFER_MODE_RELIABLE;

	struct Packet {
		uint8_t data[MAX_STEAM_PACKET_SIZE];
		uint32_t size = 0;
		CSteamID sender = CSteamID();
		int channel = 0;
		int transfer_mode = k_nSteamNetworkingSend_Reliable;
		Packet() {}
		Packet(const void *p_buffer, uint32 p_buffer_size, int transferMode, int channel) {
			ERR_FAIL_COND_MSG(p_buffer_size > MAX_STEAM_PACKET_SIZE, "ERROR TRIED TO SEND A PACKET LARGER THAN MAX_STEAM_PACKET_SIZE");
			memcpy(data, p_buffer, p_buffer_size);
			size = p_buffer_size;
			sender = CSteamID();
			channel = channel;
			transfer_mode = transferMode;
		}
	};
	Packet *next_send_packet = new Packet;
	Packet *next_received_packet = new Packet; // this packet gets deleted at the first get_packet request
	List<Packet *> incoming_packets;

	_FORCE_INLINE_ bool _is_active() const { return lobby_state != LOBBY_STATE::NOT_CONNECTED; }

	class ConnectionData : public RefCounted {
		GDCLASS(ConnectionData, RefCounted);

	public:
		int peer_id;
		CSteamID steam_id;
		uint64_t last_msg_timestamp;
		SteamNetworkingIdentity networkIdentity;
		List<Packet *> pending_retry_packets;

		ConnectionData(CSteamID steamId) {
			this->peer_id = -1;
			this->steam_id = steamId;
			this->last_msg_timestamp = 0;
			networkIdentity = SteamNetworkingIdentity();
			networkIdentity.SetSteamID(steamId);
		}
		ConnectionData(){};
		~ConnectionData() {
			SteamNetworkingMessages()->CloseSessionWithUser(networkIdentity);
			while (pending_retry_packets.size()) {
				delete pending_retry_packets.front()->get();
				pending_retry_packets.pop_front();
			}
		}
		bool operator==(const ConnectionData &data) {
			return steam_id == data.steam_id;
		}
		EResult rawSend(Packet *packet) {
			return SteamNetworkingMessages()->SendMessageToUser(networkIdentity, packet->data, packet->size, packet->transfer_mode, packet->channel);
		}
		Error send(Packet *packet) {
			auto errorCode = rawSend(packet);
			if (errorCode == k_EResultOK) {
				delete packet;
				int startingSize = pending_retry_packets.size();
				packet = pending_retry_packets.front()->get();
				while (pending_retry_packets.size() != 0) {
					auto error = rawSend(packet);
					if (error == k_EResultOK) {
						delete packet;
						pending_retry_packets.pop_front();
						packet = pending_retry_packets.front()->get();
					} else {
						ERR_PRINT("RESNED ERROR!");
						break;
					}
				}
				// try to resend old packets?
				return OK;
			} else {
				if (packet->transfer_mode & k_nSteamNetworkingSend_Reliable) {
					pending_retry_packets.push_back(packet);
				}
				switch (errorCode) {
					case k_EResultNoConnection:
						ERR_FAIL_V_MSG(ERR_DOES_NOT_EXIST, "Send Error: k_EResultNoConnection");
					case k_EResultRateLimitExceeded:
						ERR_FAIL_V_MSG(ERR_BUSY, "Send Error: k_EResultRateLimitExceeded");
					case k_EResultConnectFailed:
						ERR_FAIL_V_MSG(FAILED, "Send Error: k_EResultConnectFailed");
					default:
						ERR_FAIL_V_MSG(ERR_BUG, "Send Error: don't know what this error is, but it's not on the expected errors list...");
				}
				// return errorCode;
			}
		}
		void attemptToResendOldPackets() {
		}
		Error ping(const PingPayload &p) {
			last_msg_timestamp = OS::get_singleton()->get_ticks_msec(); // only ping once per maxDeltaT;

			auto packet = new Packet((void *)&p, sizeof(PingPayload), TRANSFER_MODE_RELIABLE, PING_CHANNEL);
			return send(packet);
		}
		Error ping() {
			auto p = PingPayload();
			return ping(p);
		}
		Dictionary collect_debug_data() {
			Dictionary output;
			output["peer_id"] = peer_id;
			output["steam_id"] = steam_id.GetAccountID();
			output["pending_packet_count"] = pending_retry_packets.size();
			SteamNetConnectionRealTimeStatus_t info;
			SteamNetworkingMessages()->GetSessionConnectionInfo(networkIdentity, nullptr, &info);
			switch (info.m_eState) {
				case k_ESteamNetworkingConnectionState_None:
					output["connection_status"] = "None";
					break;
				case k_ESteamNetworkingConnectionState_Connecting:
					output["connection_status"] = "Connecting";
					break;
				case k_ESteamNetworkingConnectionState_FindingRoute:
					output["connection_status"] = "FindingRoute";
					break;
				case k_ESteamNetworkingConnectionState_Connected:
					output["connection_status"] = "Connected";
					break;
				case k_ESteamNetworkingConnectionState_ClosedByPeer:
					output["connection_status"] = "ClosedByPeer";
					break;
				case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
					output["connection_status"] = "ProblemDetectedLocally";
					break;
			}
			output["ping"] = info.m_nPing;
			return output;
		}
	};

	HashMap<__int64, Ref<ConnectionData>> connections_by_steamId64;

	HashMap<__int64, int> steamId64_to_peerId;
	HashMap<int, CSteamID> peerId_to_steamId;

	int get_peer_by_steam_id(CSteamID steamId);
	CSteamID get_steam_id_by_peer(int peer);
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

	Dictionary collect_debug_data() {
		auto output = Dictionary();

		output["lobby_id"] = steamIdToDict(lobby_id);
		output["lobby_owner"] = steamIdToDict(lobby_owner);
		output["steam_id"] = steamIdToDict(steam_id);
		output["lobby_state"] = lobby_state;
		output["no_nagle"] = no_nagle;
		output["no_delay"] = no_delay;
		output["target_peer"] = target_peer;
		output["unique_id"] = unique_id;

		Array connections;
		for (auto E = connections_by_steamId64.begin(); E; ++E) {
			auto qwer = E->value->collect_debug_data();
			connections.push_back(qwer);
		}
		output["connections"] = connections;

		return output;
	}
};

// todo: make these empty for release builds
#define DEBUG_DATA_SIGNAL_V(msg, value) \
	Dictionary a;                       \
	a["msg"] = msg;                     \
	a["value"] = value;                 \
	emit_signal("debug_data", a)

#define DEBUG_DATA_SIGNAL(msg) \
	Dictionary a;              \
	a["msg"] = msg;            \
	emit_signal("debug_data", a)

#endif
