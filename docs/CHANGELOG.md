# Changelog

Short summary of what the port can do. The full per-run development log (every fix and experiment, 2026-10-25 to 2026-11-18) is kept in [archive/CHANGELOG-dev-log-2026-11-18.md](archive/CHANGELOG-dev-log-2026-11-18.md). Current state: [HANDOFF.md](../HANDOFF.md).

## 2026-11 release state

- **Speed:** about 60 fps in mono and close to it in stereo 3D (#517-#540). GPU right eye, hidden-rows skip, 3D state cache and screen hand-off are always on.
- **Settings:** trimmed to the player menu (L+R+Y). The tuned setup is the default, so no `config.txt` editing is needed.
- **Online:** GTS (Poké Classic, optional StarMe server), Mystery Gift, Union Room trades and battles, Pokéwalker over the New 3DS infrared port.
- **Touch screen:** Pokégear panel with Johto map, party, bag, radio, roamer and breeding trackers, Contest helper and Tap to Fly.
- **Display:** stereoscopic 3D, Wide view, wide layouts, sharper title, living world with seasons and weather.
- **Extras:** hunt stats, damage check, achievements (never written to the save), Scratch-Off, Photo Album, Pal Park, Ball seals and Accessory Portrait screens.
- **HeartGold:** built from the same source as a second game (see README).
- **Saves:** plain 512 KiB `.sav`, compatible with DS cartridges and flashcarts, with rolling backups.

## 2026-10

- Audio and load-speed pass, faster battles and battle info (`pspoke-patches/0030`), first native boot on hardware, local wireless trade. Details in the archived dev log.
