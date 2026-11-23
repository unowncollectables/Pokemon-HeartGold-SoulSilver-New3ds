# Changelog

Short summary of what the port can do. The full per-run development log (every fix and experiment, 2026-10-25 to 2026-11-18) is kept in [archive/CHANGELOG-dev-log-2026-11-18.md](archive/CHANGELOG-dev-log-2026-11-18.md). Current state: [HANDOFF.md](../HANDOFF.md).

## Unreleased

- Mobile PC (hook 0293): Settings > SYSTEM "Mobile PC" (default OFF) adds a BOXES button on the party screen footer, left of CANCEL, drawn like the official CANCEL button (same rounded shape, height, border and outlined white font) with a purple fill and a small box icon (SwSh/BDSP "To Boxes"), after the Hall of Fame. The JOHTO tab bar stays the original labeled 5/6 chips; the 7th PC tab was removed. Tapping BOXES plays a short zoom, closes the party if it is open, then opens the game's own box app (PCBox_LaunchApp mode 2, Move Pokemon) from a safe idle overworld only (not battles, menus, scripts, Safari, Bug Contest, Pal Park, GTS/Mystery Gift/Union Room Wi-Fi rooms). Closing boxes restores the field. No auto-save and no auto-reset. Unlock uses the item fanfare (SEQ_ME_ITEM) and a field message; the chip pop-in / NEW / glow plays the first time the party is opened after that. "Already shown" is a port sidecar keyed by trainer ID, never the .sav. DEBUG "Skip HoF gate" (default OFF) skips the Hall of Fame check at runtime only and never writes flags; DEBUG "Preview unlock" plays the ceremony without writing the sidecar. Field message + box launch follow pret/pokeheartgold (GPL-3.0) Task_PrintRegisteredKeyItemUseMessage and ScrCmd_158.

- Starter soft-reset (run 996): audio teardown frees ndsp wave buffers and exits the DSP once before relaunch; hook 0292 counts Elm starters on the hunt path; Select soft reset works on the ChooseStarter screen. Builds on #576. Unbuilt on hardware. Follow-up: `huntTick` no longer double-counts the last starter (Totodile) when leaving the field for ChooseStarter.

- Complete project wiki: player guides, settings, saves, wireless, HeartGold, troubleshooting, all repository Markdown guides and tool READMEs, full settings reference, policies, and labeled historical archives. `scripts/build_wiki.py` exports pinned source pages and validates local page links; see [wiki maintenance](WIKI.md).

- Finder readability (#566): count and time-filter rows, result separators and paging footer leave room for full glyphs. LIST hides map controls and markers that covered its first result. Built and checked in LOTUS/CITnLO; hardware validation remains open.

- Hunt stats (ZR): now a sharp tabbed overlay in the battle companion style while the field keeps running (it used to freeze the game). HUNT page shows chance so far, 50%/90% marks, encounters to go and pace; Up/Down flips species; X pins a species as your hunt, Y lets you pick any species, RESET (tap twice) clears one. TOTALS, TOP, RECENT and SHINY pages. Unbuilt.
- Battle companion: Y opens a sharp tabbed overlay while the battle keeps running. Tap or swipe through available info, catch, hunt, damage and trainer-party pages. FIGHT effectiveness hints and finder LIST rows have also been refreshed (PR #565); broader hardware coverage remains open.
- Exploration: finder LIST with true-colour species icons, encounter-slot chances and time filters; AREAS page for map sub-areas and separate visit tracking. Smoother map panning, consistent cards and faster tab swaps (PRs #555, #557, #559–#562).
- Visible wilds: groups, NPC avoidance, post-battle scattering and Safari Zone support (PRs #550, #552, #563). Shore encounters use the land-battle path; the water-battle intro has been repaired (#553–#554).

- Sharp game text (patch 0289, untested on hardware): the game's text windows on both screens, in every scene (dialogue, menus, battle messages, battle move names and HP box names, party, bag, summary, title menu), draw their text at the 3DS resolution (1 font pixel = 1 screen pixel) instead of DS text stretched 1.25x. The text pixels are taken out of the copy that goes to VRAM (the game's own window buffer is untouched) and drawn by native layers on each screen; where a screen cannot show the layer the text goes straight back into the picture. Settings > Display "Sharp game text", default ON, applies at once. Log: [GTEXT].
- Sharp bottom-screen text: the Bottom menu panel and the L+R+Y settings menu draw their text at the 3DS resolution (320x240) instead of stretching DS-size text 1.25x. Settings > Display "Sharp text", default ON. Platform only.
- Soft-reset / exit teardown (run 934, platform-only): once-guard `P3DS_SoftReset`; join engine B / c1, g3thr and par workers before freeing game heap or GPU state. Save sync/backup order unchanged. See HANDOFF.md.

## 2026-11 release state

- **Speed:** about 60 fps in mono and close to it in stereo 3D (#517-#540). GPU right eye, hidden-rows skip, 3D state cache and screen hand-off are always on.
- **Settings:** trimmed to the player menu (L+R+Y). The tuned setup is the default, so no `config.txt` editing is needed.
- **Online:** GTS (Poké Classic, optional StarMe server), Mystery Gift, Union Room trades (battles still need a two-console test), and Pokéwalker over the New 3DS infrared port.
- **Touch screen:** Pokégear panel with Johto map, party, bag, radio, roamer and breeding trackers, Contest helper and Tap to Fly.
- **Display:** stereoscopic 3D, Wide view, wide layouts, sharper title, living world with seasons and weather.
- **Extras:** hunt stats, damage check, achievements (never written to the save), Scratch-Off, Photo Album, Pal Park, Ball seals and Accessory Portrait screens.
- **HeartGold:** built from the same source as a second game (see README).
- **Saves:** plain 512 KiB `.sav`, compatible with DS cartridges and flashcarts, with rolling backups.

## 2026-10

- Audio and load-speed pass, faster battles and battle info (`pspoke-patches/0030`), first native boot on hardware, local wireless trade. Details in the archived dev log.
