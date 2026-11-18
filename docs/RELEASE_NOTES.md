# Release notes

Current build (main). Not an official release; see the [README](../README.md) for install and known issues. Older per-run notes are in [archive/RELEASE_NOTES-dev-2026-11-18.md](archive/RELEASE_NOTES-dev-2026-11-18.md).

## What is in it
- Native New 3DS port of SoulSilver (and HeartGold as a second game): boot, title, overworld, battles and menus on the pspoke base, with stereoscopic 3D, Wide view, GPU 3D and a GPU 2D compositor.
- About 60 fps in mono and close to it in 3D. The tuned speed setup is the default; nothing to configure.
- Settings app on L+R+Y with Display, 3D, Input, Battle, Play and Setup tabs. The old Debug and Render tabs are gone. Everything new is a setting here, and `config.txt` is never needed.
- Touch-screen Pokégear panel: Johto map, party, bag, radio, roamer and breeding trackers, Contest helper, Tap to Fly.
- GTS (including an optional StarMe server), Mystery Gift, Union Room trades and the Pokéwalker over the infrared port.
- Hunt stats, damage check, achievements (never written to the save), fast text, reusable TMs, seasons and ambient weather.
- Zone prefetch and ROM cache for fewer load hitches; three rolling save backups at every launch. The save stays a plain 512 KiB `.sav`, compatible with DS cartridges and flashcarts.

## Known issues
- Poké Ball opening in battle can show a double image with the 3D slider up.
- Wide FX is expensive (keep it off).
- Some screens still use the old DS fill in Wide view (Pokéathlon, Photo Album, Hall of Fame, evolution, Battle Frontier). Wi-Fi Club minigames still stop and the Wi-Fi Plaza shows a closed message.
- Back up your save before using any build.
