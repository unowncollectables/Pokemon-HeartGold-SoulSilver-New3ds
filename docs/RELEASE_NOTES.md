# Release notes

Current build (main). Not an official release; see the [README](../README.md) for install and known issues. Older per-run notes are in [archive/RELEASE_NOTES-dev-2026-11-18.md](archive/RELEASE_NOTES-dev-2026-11-18.md).

## What is in it
- A [complete project wiki](https://github.com/unowncollectables/Pokemon-HeartGold-SoulSilver-New3ds/wiki) with player guides, developer references, tool documentation and labeled archives. [Maintenance](WIKI.md) describes refreshing it from main; no game rebuild is required.
- Native New 3DS port of SoulSilver (and HeartGold as a second game): boot, title, overworld, battles and menus on the pspoke base, with stereoscopic 3D, Wide view, GPU 3D and a GPU 2D compositor.
- Overworld: about 60 fps in mono and close to it in stereo 3D; busy scenes, battles and transitions can dip. The tuned speed setup is the default; nothing to configure.
- Settings app on L+R+Y with Display, 3D, Input, Battle, Play and Setup tabs. The old Debug and Render tabs are gone. Player options are here; the tested performance improvements are built in. Existing config values still override new defaults; see the README for resetting an older setup.
- Touch-screen Pokégear panel: Johto map, party, bag, radio, roamer and breeding trackers, Contest helper, Tap to Fly. Its text and the settings menu text are drawn at the 3DS resolution (Display > Sharp text).
- Sharp game text on both screens (dialogue, menus, battle messages and sprite-based move/Pokémon names), default ON. Broader hardware checks remain open.
- Encounter finder LIST with slot chances and time filters, true-colour icons, map sub-area tracking, smoother map panning and faster tab swaps.
- Reworked Y battle-info overlay: sharp pages, tabs and swipe; the battle keeps running underneath.
- Visible wild groups, NPC avoidance and post-battle scattering, plus Safari Zone visible encounters.
- GTS (including an optional StarMe server), Mystery Gift, Union Room trades and the Pokéwalker over the infrared port.
- Hunt stats, damage check, achievements (never written to the save), fast text, reusable TMs, seasons and ambient weather.
- Zone prefetch and ROM cache for fewer load hitches; one save backup (`soulsilver.sav.bak1`) at launch, after in-game saves and at exit; a failed backup write preserves the existing backup. The save stays a plain 512 KiB `.sav`, compatible with DS cartridges and flashcarts.

## Known issues
- Poké Ball opening in battle can show a double image with the 3D slider up.
- Wide FX is expensive (keep it off).
- Some screens still use the old DS fill in Wide view (Pokéathlon, Photo Album, Hall of Fame, evolution, Battle Frontier). Wi-Fi Club minigames still stop and the Wi-Fi Plaza shows a closed message.
- Back up your save before using any build.
