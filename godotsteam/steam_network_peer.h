#ifndef STEAM_NETWORK_PEER_H
#define STEAM_NETWORK_PEER_H

#include "core/resource.h"
#include "godotsteam.h"
#include "core/io/networked_multiplayer_peer.h"


class SteamNetworkPeer : public NetworkedMultiplayerPeer {
	GDCLASS(SteamNetworkPeer, NetworkedMultiplayerPeer);


protected:
	static void _bind_methods();

public:
	SteamNetworkPeer();
	~SteamNetworkPeer();


	/* Specific to PacketPeer */
	virtual Error get_packet(const uint8_t **r_buffer, int &r_buffer_size);
	virtual Error put_packet(const uint8_t *p_buffer, int p_buffer_size);
	virtual int get_max_packet_size() const;
	virtual int get_available_packet_count() const;

	/* Specific to NetworkedMultiplayerPeer */
	virtual void set_transfer_mode(TransferMode p_mode);
	virtual TransferMode get_transfer_mode() const;
	virtual void set_target_peer(int p_peer_id);

	virtual int get_packet_peer() const;

	virtual bool is_server() const;

	virtual void poll();

	virtual int get_unique_id() const;

	virtual void set_refuse_new_connections(bool p_enable);
	virtual bool is_refusing_new_connections() const;

	virtual ConnectionStatus get_connection_status() const;

};


#endif // STEAM_NETWORK_PEER_H