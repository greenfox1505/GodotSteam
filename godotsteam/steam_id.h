#ifndef STEAM_ID_H
#define STEAM_ID_H

#include "core/object.h"
#include "steam/steam_api.h"

class SteamId : public Object {
private:
	GDCLASS(SteamId, Object);
	CSteamID steamId;
protected:
public:
	uint64_t getRaw();
	SteamId(){steamId = CSteamID();}
	SteamId(uint64_t raw){steamId = CSteamID(uint64(raw));};
	uint32_t getAccountId(){ return steamId.GetAccountID();}
	CSteamID getSteamId(){return steamId;};
};

#endif // STEAM_ID_H