#include "steam_multiplayer_peer.h"

int SteamMultiplayerPeer::get_available_packet_count() const {
	ERR_PRINT("ERROR:: SteamMultiplayerPeer::get_available_packet_count not yet implemented");
    return -1;
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
	return 1 << 24;//from ENet
}

void SteamMultiplayerPeer::set_target_peer(int p_peer_id) {
	ERR_PRINT("ERROR:: SteamMultiplayerPeer::set_target_peer not yet implemented");
}

int SteamMultiplayerPeer::get_packet_peer() const {
	ERR_PRINT("ERROR:: SteamMultiplayerPeer::get_packet_peer not yet implemented");
	return -1;
}

SteamMultiplayerPeer::TransferMode SteamMultiplayerPeer::get_packet_mode() const {
	ERR_PRINT("ERROR:: SteamMultiplayerPeer::get_packet_mode not yet implemented");
	return TransferMode::TRANSFER_MODE_RELIABLE;
}

int SteamMultiplayerPeer::get_packet_channel() const {
	ERR_PRINT("ERROR:: SteamMultiplayerPeer::get_packet_channel not yet implemented");
	return -1;
}

void SteamMultiplayerPeer::disconnect_peer(int p_peer, bool p_force) {
	ERR_PRINT("ERROR:: SteamMultiplayerPeer::disconnect_peer not yet implemented");
}

bool SteamMultiplayerPeer::is_server() const {
	ERR_PRINT("ERROR:: SteamMultiplayerPeer::is_server not yet implemented");
	return false;
}

void SteamMultiplayerPeer::poll() {
	ERR_PRINT("ERROR:: SteamMultiplayerPeer::poll not yet implemented");
}

void SteamMultiplayerPeer::close() {
	ERR_PRINT("ERROR:: SteamMultiplayerPeer::close not yet implemented");
}

int SteamMultiplayerPeer::get_unique_id() const {
	ERR_PRINT("ERROR:: SteamMultiplayerPeer::get_unique_id not yet implemented");
	// ERR_FAIL_COND_V_MSG(!_is_active(), 0, "The multiplayer instance isn't currently active.");
	return unique_id;
}

SteamMultiplayerPeer::ConnectionStatus SteamMultiplayerPeer::get_connection_status() const {
	ERR_PRINT("ERROR:: SteamMultiplayerPeer::get_connection_status not yet implemented");
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
