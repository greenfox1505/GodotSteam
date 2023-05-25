# `SteamMultiplayerPeer` Blog

`SteamMultiplayerPeer` is at Minimum Viable Product (MVP). 

You can create a lobby, invite people, they can join you, and use Godot's High Level Multiplayer features (`rpc`, etc).

It doesn't fully duplicate all of HL Multiplayer's features, such as independent channels.

The next step is either to merge back into the GodotSteam or make this it's own Godot Module.

## GodotSteam is "un-opinionated" 

GodotSteam is a un-opinionated binding SteamworksSDK functions to Godot. It doesn't try to dictate design. It's a best effort to expose Steam's functionality as directly to Godot as possible, without getting in the user's way as much as possible. It uses built-in objects like arrays and dictionaries. It abstracts Steamworks callbacks into Godot signals. It's trying to let Godot programmers use Steamworks without learning C++. Which, IMHO, is the "correct" way to do what it needs to do. (although, at some point it might make sense to implement )

## SteamMultiplayer is Opinionated

`SteamMultiplayerPeer` is opinionated. My goal was to try to adapt Godot's ENetMultiplayerPeer architecture as closely as possible and trying to make it as easy a drop-in replacement from ENet as possible. Obviously, setting up or joining a Steam server is going to requires a different UI/UX, but mostly if you're using ENet, you should be able to use `SteamMultiplayerPeer` with minimal effort. (There is some ENet functionality still missing, but it's MVP)

Doing this required me to make a lot of choices in how I was going to abstract out Steamworks APIs. For example, Lobbies are an important part of how Steam Multiplayer works, but in `SteamMultiplayerPeer` there is no interface for Lobbies. Joining a "server" implies you are in the same lobby. On Steam, that's not necessarily true; you can often be in multiple lobbies and servers at once. For example, in a game like Dota2, you're in a party, on a team, and in a server against another team at the same time.

Someday, I, or someone else, might (probably will) make a different `SteamMultiplayerPeer`. One that handles lobbies differently. One that handles being in multiple lobbies properly. One that maybe emulates ENet's "mesh" networking features. Maybe one that lets you be in multiple "lobbies" at once. Maybe one that allows you to run dedicated servers. 

`SteamMultiplayerPeer` is not "correct" in the sense that it is the right way to do this. It works very well for some games and not at all for others.

`SteamMultiplayerPeer` is also WAY less stable than GodotSteam. It might get renamed to `SteamLobbyDirectMultiplayerPeer` or something like that.

## Next Steps

So, going forward, in my humble opinion, marrying `SteamMultiplayerPeer` to GodotSteam isn't a good idea because it nudges out other really good ideas and options. So I think it makes sense to make `SteamMultiplayerPeer` a separate module, one that you install along side GodotSteam. Perhaps it makes sense to put it in GodotSteam's pre-compiled binaries.

I would very much like to hear form ANYONE who is using `SteamMultiplayerPeer` or would like to use it about what they think the best next step is.