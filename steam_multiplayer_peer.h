#ifndef STEAM_MULTIPLAYER_PEER_H
#define STEAM_MULTIPLAYER_PEER_H



// Include Godot headers
#include "scene/main/multiplayer_peer.h"

//Steam APIs
#include "godotsteam.h"
#include "core/os/os.h"
//MultiplayerPeer
class SteamMultiplayerPeer : public MultiplayerPeer {
	GDCLASS(SteamMultiplayerPeer, MultiplayerPeer);

protected:
	static void _bind_methods();

public:

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

public:
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
	ConnectionStatus connection_status;

	struct Packet {
		//  *packet = nullptr;
		int from = 0;
		int channel = 0;
		TransferMode transfer_mode = TRANSFER_MODE_RELIABLE;
	};
	Packet current_packet;
	List<Packet> incoming_packets;

	_FORCE_INLINE_ bool _is_active() const { return lobbyState != LOBBY_STATE::NOT_CONNECTED; }

public:
	Error create_server(int max_players);
	Error create_client();//todo function signature here!
	
};

#endif
