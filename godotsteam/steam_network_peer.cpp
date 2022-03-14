#include "steam_network_peer.h"

void SteamNetworkPeer::_bind_methods() {
}
SteamNetworkPeer::SteamNetworkPeer() {
}
SteamNetworkPeer::~SteamNetworkPeer() {
}

Error SteamNetworkPeer::get_packet(const uint8_t **r_buffer, int &r_buffer_size) {
	return ERR_PRINTER_ON_FIRE;
}

Error SteamNetworkPeer::put_packet(const uint8_t *p_buffer, int p_buffer_size) {
	return ERR_PRINTER_ON_FIRE;
}

int SteamNetworkPeer::get_max_packet_size() const {
	return ERR_PRINTER_ON_FIRE;
}

int SteamNetworkPeer::get_available_packet_count() const {
	return ERR_PRINTER_ON_FIRE;
}

void SteamNetworkPeer::set_transfer_mode(TransferMode p_mode) {
}
NetworkedMultiplayerPeer::TransferMode SteamNetworkPeer::get_transfer_mode() const {
	return NetworkedMultiplayerPeer::TransferMode::TRANSFER_MODE_UNRELIABLE;
}
void SteamNetworkPeer::set_target_peer(int p_peer_id) {
}

int SteamNetworkPeer::get_packet_peer() const {
	return -1;
}

bool SteamNetworkPeer::is_server() const {
	return false;
}

void SteamNetworkPeer::poll() {
}

int SteamNetworkPeer::get_unique_id() const {
	return -1;
}

void SteamNetworkPeer::set_refuse_new_connections(bool p_enable) {
}
bool SteamNetworkPeer::is_refusing_new_connections() const {
	return false;
}

NetworkedMultiplayerPeer::ConnectionStatus SteamNetworkPeer::get_connection_status() const {
	return NetworkedMultiplayerPeer::ConnectionStatus::CONNECTION_CONNECTED;
}
