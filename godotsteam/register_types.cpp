#include "register_types.h"
#include "core/class_db.h"
#include "core/engine.h"
#include "godotsteam.h"

#include "steam_network_peer.h"

static Steam* SteamPtr = NULL;

void register_godotsteam_types(){
	ClassDB::register_class<Steam>();
	ClassDB::register_class<SteamNetworkPeer>();
	SteamPtr = memnew(Steam);
	Engine::get_singleton()->add_singleton(Engine::Singleton("Steam",Steam::get_singleton()));
}

void unregister_godotsteam_types(){
	memdelete(SteamPtr);
}
