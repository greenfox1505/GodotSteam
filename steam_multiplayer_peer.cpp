#include "steam_multiplayer_peer.h"



#include "godotsteam.h"


#define MAX_STEAM_PACKET_SIZE k_cbMaxSteamNetworkingSocketsMessageSizeSend


VARIANT_ENUM_CAST(SteamMultiplayerPeer::LOBBY_TYPE);
VARIANT_ENUM_CAST(SteamMultiplayerPeer::CHAT_CHANGE);
VARIANT_ENUM_CAST(SteamMultiplayerPeer::LOBBY_STATE);


void SteamMultiplayerPeer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_server","max_players"), &SteamMultiplayerPeer::create_server,DEFVAL(32));
	ClassDB::bind_method(D_METHOD("create_client"), &SteamMultiplayerPeer::create_client);

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
	BIND_ENUM_CONSTANT(HOST);
	BIND_ENUM_CONSTANT(CLIENT_PENDING);
	BIND_ENUM_CONSTANT(CLIENT);
	
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
;}

int SteamMultiplayerPeer::get_max_packet_size() const {
	return MAX_STEAM_PACKET_SIZE;//from ENet
}

void SteamMultiplayerPeer::set_target_peer(int p_peer_id) {
	target_peer = p_peer_id;
}

int SteamMultiplayerPeer::get_packet_peer() const {
	ERR_FAIL_COND_V_MSG(!_is_active(), 1, "The multiplayer instance isn't currently active.");
	ERR_FAIL_COND_V(incoming_packets.size() == 0, 1);

	return incoming_packets.front()->get().from;
}

SteamMultiplayerPeer::TransferMode SteamMultiplayerPeer::get_packet_mode() const {
	ERR_FAIL_COND_V_MSG(!_is_active(), TRANSFER_MODE_RELIABLE, "The multiplayer instance isn't currently active.");
	ERR_FAIL_COND_V(incoming_packets.size() == 0, TRANSFER_MODE_RELIABLE);

	return incoming_packets.front()->get().transfer_mode;
}

int SteamMultiplayerPeer::get_packet_channel() const {
	ERR_FAIL_COND_V_MSG(!_is_active(), 1, "The multiplayer instance isn't currently active.");
	ERR_FAIL_COND_V(incoming_packets.size() == 0, 1);
	int ch = incoming_packets.front()->get().channel;
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

void SteamMultiplayerPeer::poll() {
	ERR_PRINT("ERROR:: SteamMultiplayerPeer::poll not yet implemented");
}

void SteamMultiplayerPeer::close() {
	ERR_PRINT("ERROR:: SteamMultiplayerPeer::close not yet implemented");
}

int SteamMultiplayerPeer::get_unique_id() const {
	ERR_FAIL_COND_V_MSG(!_is_active(), 0, "The multiplayer instance isn't currently active.");
	return unique_id;
}

SteamMultiplayerPeer::ConnectionStatus SteamMultiplayerPeer::get_connection_status() const {
	return connection_status;
}

Error SteamMultiplayerPeer::create_server(int max_players) {
	ERR_PRINT("ERROR:: SteamMultiplayerPeer::create_server not yet implemented");
	unique_id = 1;
	return Error();
}

Error SteamMultiplayerPeer::create_client() {
	ERR_PRINT("ERROR:: SteamMultiplayerPeer::create_client not yet implemented");
	unique_id = generate_unique_id();

	return Error();
}
