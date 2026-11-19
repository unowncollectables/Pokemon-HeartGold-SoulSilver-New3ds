# Changelog

Short summary of what the port can do. The full per-run development log (every fix and experiment, 2026-10-25 to 2026-11-18) is kept in [archive/CHANGELOG-dev-log-2026-11-18.md](archive/CHANGELOG-dev-log-2026-11-18.md). Current state: [HANDOFF.md](../HANDOFF.md).

## Unreleased

- Sharp game text (patch 0289, untested on hardware): the game's text windows on both screens, in every scene (dialogue, menus, battle messages, battle move names and HP box names, party, bag, summary, title menu), draw their text at the 3DS resolution (1 font pixel = 1 screen pixel) instead of DS text stretched 1.25x. The text pixels are taken out of the copy that goes to VRAM (the game's own window buffer is untouched) and drawn by native layers on each screen; where a screen cannot show the layer the text goes straight back into the picture. Settings > Display "Sharp game text", default ON, applies at once. Log: [GTEXT].
- Sharp bottom-screen text: the Bottom menu panel and the L+R+Y settings menu draw their text at the 3DS resolution (320x240) instead of stretching DS-size text 1.25x. Settings > Display "Sharp text", default ON. Platform only.
- Soft-reset / exit teardown (run 934, platform-only): once-guard `P3DS_SoftReset`; join engine B / c1, g3thr and par workers before freeing game heap or GPU state. Save sync/backup order unchanged. See HANDOFF.md.

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
