#include "steam_multiplayer_peer.h"

int SteamMultiplayerPeer::get_available_packet_count() const {
    return -1;
}

Error SteamMultiplayerPeer::get_packet(const uint8_t **r_buffer, int &r_buffer_size) {
	return Error();
}

Error SteamMultiplayerPeer::put_packet(const uint8_t *p_buffer, int p_buffer_size) {
	return Error();
}

int SteamMultiplayerPeer::get_max_packet_size() const {
	return 0;
}

void SteamMultiplayerPeer::set_target_peer(int p_peer_id) {
}

int SteamMultiplayerPeer::get_packet_peer() const {
	return 0;
}

SteamMultiplayerPeer::TransferMode SteamMultiplayerPeer::get_packet_mode() const {
	return TransferMode::TRANSFER_MODE_RELIABLE;
}

int SteamMultiplayerPeer::get_packet_channel() const {
	return 0;
}

void SteamMultiplayerPeer::disconnect_peer(int p_peer, bool p_force) {
}

bool SteamMultiplayerPeer::is_server() const {
	return false;
}

void SteamMultiplayerPeer::poll() {
}

void SteamMultiplayerPeer::close() {
}

int SteamMultiplayerPeer::get_unique_id() const {
	return 0;
}

SteamMultiplayerPeer::ConnectionStatus SteamMultiplayerPeer::get_connection_status() const {
	return ConnectionStatus::CONNECTION_DISCONNECTED;
}
