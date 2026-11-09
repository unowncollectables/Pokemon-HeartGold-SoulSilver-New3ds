<p align="center">
  <img src="./assets/banner.svg" alt="Pokémon SoulSilver — New 3DS Edition" width="860" />
</p>

<p align="center">
  <strong>Back to Johto — natively on the New 3DS.</strong><br/>
  A personal-use, fan-made native port of Pokémon SoulSilver.<br/>
  Sister of the <a href="https://github.com/unowncollectables/3ds-platinum">Platinum 3DS</a> port · built from <em>your</em> legally dumped ROM.
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Platform-New_3DS%20%2F%20New_2DS_XL-1f6feb?style=for-the-badge&logo=nintendo3ds&logoColor=white" alt="Platform: New 3DS / New 2DS XL" />
  <img src="https://img.shields.io/badge/Status-WIP-f59e0b?style=for-the-badge" alt="Status: WIP" />
  <img src="https://img.shields.io/badge/Use-Personal_only-9466FF?style=for-the-badge" alt="Personal use only" />
  <a href="LICENSE"><img src="https://img.shields.io/badge/License-GPL--3.0-4B5563?style=for-the-badge" alt="GPL-3.0" /></a>
</p>

<p align="center">
  <a href="https://github.com/unowncollectables/patch-builder"><img src="https://img.shields.io/badge/Builder-One--window_installer-0ea5e9?style=flat-square&logo=windows&logoColor=white" alt="Pokemon 3DS Builder" /></a>
  <a href="https://github.com/unowncollectables/Pokemon-Soulsilver-New3ds/commits/main"><img src="https://img.shields.io/github/last-commit/unowncollectables/Pokemon-Soulsilver-New3ds?style=flat-square&logo=github" alt="Last commit" /></a>
  <a href="https://github.com/unowncollectables/Pokemon-Soulsilver-New3ds/issues"><img src="https://img.shields.io/github/issues/unowncollectables/Pokemon-Soulsilver-New3ds?style=flat-square" alt="Open issues" /></a>
  <img src="https://img.shields.io/badge/Native-not_an_emulator-22c55e?style=flat-square" alt="Native, not an emulator" />
</p>

<p align="center">
  <a href="#features">Features</a> ·
  <a href="#status">Status</a> ·
  <a href="#getting-started">Getting started</a> ·
  <a href="#settings-app">Settings</a> ·
  <a href="#wireless--online">Wireless</a> ·
  <a href="#for-developers">Developers</a> ·
  <a href="#credits">Credits</a> ·
  <a href="#legal">Legal</a>
</p>

<p align="center">
  <img src="./assets/screens/field-map.png" alt="Goldenrod City on the wide top screen, with the Johto map and tab row on the bottom screen" width="400" />
  <img src="./assets/screens/panel-radio.png" alt="The RADIO tab on the bottom-screen panel" width="400" />
  <br/><sub>Captured in the Azahar emulator: the whole game wide, and the bottom-screen panel (MAP and RADIO tabs).</sub>
</p>

---

## At a glance

Looking for **Pokémon SoulSilver on Nintendo 3DS**? This is a native **Pokémon SoulSilver 3DS port** (a `.cia` / `.3dsx` homebrew, not an emulator) for the **New 3DS, New 3DS XL and New 2DS XL**. It is built from the [pret/pokeheartgold](https://github.com/pret/pokeheartgold) decompilation with devkitARM, runs the whole game wide on both screens with stereoscopic 3D, and adds a touch-screen Johto map panel, an in-game settings app, local wireless trading between two 3DS consoles, and online GTS and Mystery Gift through community servers.

**Quick install:** use the [Pokemon 3DS Builder](https://github.com/unowncollectables/patch-builder) (Windows) or the [build steps](#getting-started) with your own `SoulSilver.nds` dump (US version), then copy the `.cia` or `.3dsx` to your SD card. No ROM or game data is included here.

**Keywords:** Pokémon SoulSilver 3DS, SoulSilver New 3DS port, native 3DS homebrew, pokeheartgold 3DS, SoulSilver .cia, Johto on 3DS.

---

## What this is

**Pokémon SoulSilver New 3DS Edition** compiles the [pret/pokeheartgold](https://github.com/pret/pokeheartgold) decompilation for the New 3DS CPU. It is **not an emulator**: the game’s own C runs natively, drawn the way a 3DS title draws, with both screens filled and optional stereoscopic 3D.

It builds on [pspoke](https://github.com/IbrahimIrfan/pspoke) (retargeted from PSP → 3DS with devkitARM) and reuses the platform layer, patches, and lessons from the Platinum 3DS ports ([3ds-platinum](https://github.com/unowncollectables/3ds-platinum), [GabeSud0/pokeplatinum-3ds](https://github.com/GabeSud0/pokeplatinum-3ds)).

> [!IMPORTANT]
> **No game content ships in this repository.** No ROM, save, graphics, music, text, `.3dsx`, or `.cia` is included or distributed. You supply your own legal **US SoulSilver** cartridge dump; the game reads its data from it at runtime. Dump your own cart. Never commit a ROM or a save.

---

## Features

| | What you get |
| :--- | :--- |
| **Native, not emulated** | Decompiled game code compiled for the 3DS. Real 512 KiB `.sav` files, compatible with carts / flashcarts / PKHeX. |
| **Both screens used** | DS picture scaled 1.25× onto the 3DS screens (fixed, no scale option) and the **whole game wide**: title, intro, field, battles, menus, party, Pokédex and PC fill the full 400 px top screen. |
| **Stereoscopic 3D** | Depth from the 3D slider in the overworld, battles, and title. **SHARP 3D** / **FAST 3D** options for crisp mono or supersampled edges (see Settings). |
| **Johto map on the touch screen** | Bottom screen becomes the Johto map + icon column (Pokédex, Pokémon, Bag, Pokégear, Trainer, Save, Options). Steps aside for the game’s own touch UI (YES/NO, starter Poké Balls, Pokégear). **X** opens a highlight cursor over the tiles. |
| **Running Shoes button** | A dedicated **RUN** square on the bottom-screen column toggles running shoes (Sprint Button art read from *your* ROM once the shoes are unlocked). ZL run-toggle and Auto run are also available. |
| **Bottom-screen panel** | A tab row on the Johto map: **MAP / PHONE / RADIO / PARTY / BAG** (plus **EGGS**, see below). On the scaled bottom screen those tabs are drawn at native resolution, with the same button chrome and green pixel icons as the side menu. Real Pokégear map, tap a contact to call, tap a radio station, and read-only PARTY and BAG viewers that stay live while you walk. |
| **Pokédex finder** *(opt-in)* | Tap a place on the Johto map, tap its card, tap a Pokémon: the map pins every place it lives, with ANY / MORN / DAY / NITE time filters. |
| **Breeding tab** *(opt-in)* | EGGS tab: Day-Care Pokémon and levels gained, egg chance or EGG READY, Masuda, steps left on party eggs. Read-only. |
| **Contest helper** *(opt-in)* | During a Bug-Catching Contest, the bottom panel shows minutes left, Sport Balls left, and the Pokémon you are holding. Read-only. |
| **Tap to Fly** *(opt-in)* | On the Johto map, tap a town you can Fly to. The card’s FLY chip starts the game’s own flight when Fly works there. |
| **Hunt stats** *(opt-in)* | Press **ZR** in the overworld for read-only pages from `hunt.txt`: encounters, shinies, time, most met, timeline and best streaks. |
| **Damage check** *(opt-in)* | The **Y** foe screen gets a last page: rough damage range and % of the foe’s HP for each of your moves, plus who moves first. Singles only. |
| **Achievements** | Optional Xbox-style toasts and a progress list, including trade and counter achievements. **UP** and **DN** chips on the score band page the list. Never writes to your save. |
| **In-game Settings app** | Press **L+R+Y** (or hold the top-left of the touch screen) for Display, 3D, Input, Battle, Play, and Setup tabs — toggles apply on close and persist in `config.txt`. |
| **Battle QoL** | Optional SUPER EFFECTIVE / NOT VERY / NO EFFECT badges on move buttons; faster battles; catch info; encounter counter; shiny-hunt suite (opt-in). |
| **Local wireless trading** | Full Pokémon trade between two New 3DS consoles via the game’s Union Room over 3DS UDS — confirmed on hardware. |
| **Online GTS** | The game’s own Global Terminal over community revival servers (Wiimmfi / Poké Classic Network), plus Mystery Gift download. Both paths are confirmed on hardware. A revival GTS host was reported down on 8 Nov 2026 (server-side); Mystery Gift is a separate service. |
| **Visible & alive wild Pokémon** | Optional: wilds from the map’s encounter tables walk the field; walk into one to battle. **Alive wild Pokémon** (opt-in, untested on hardware) makes them solid and adds ambient flyers, naps, pairs and other small touches. |
| **Smoother and cleaner** | Work toward a steady 30 fps (off-screen skipping, faster wide battles, the 3D shortcuts below). One hardware sample with those shortcuts on sat mostly around 29–30 fps in the field and in battles; that is one run, and steady 30 fps everywhere is still the goal. Audio fixes: notes start on time and music no longer dips under loud sound effects. |
| **GPU rendering** | 3D scenes and 2D compositing on the 3DS GPU, including optional shortcuts for command packets, bounds, the second screen, rotating sprites, and fades (Settings → 3D). |
| **Safe saves & durable logs** | Rolling `.sav` backups; each save is **fsync’d** to the SD card. `log.txt` is fsync’d in batches (and on HOME / sleep / crash) so the last lines **survive reboots and forced closes**; unclean exits are kept as `log_crash.txt`. |
| **HOME Menu ready** | `.cia` with a 3D banner, or `.3dsx` from the Homebrew Launcher. |

---

## Status

| ✅ Confirmed on hardware | 🧪 Built / wired, still baking | 🛠️ Later / wishlist |
| :--- | :--- | :--- |
| Boot, title, overworld, battles, menus, saving; online **GTS** (Global Terminal) | Alive wild Pokémon | Steady 30 fps everywhere; 60 fps later |
| 3D depth, whole game wide, GPU 3D + 2D compose, FAST / SHARP 3D; the five 3D shortcuts on through field and battles, stereo included | Achievements (toast, list, UP/DN paging) | Wi-Fi Club / Plaza with a real DS |
| Johto map, X tile cursor, YES/NO & starter balls on touch | Pokégear RADIO tap-to-tune (needs Radio Card); native-style bottom tabs | Pokéathlon |
| Pokégear MAP / PHONE / PARTY / BAG tabs; battle move badges | Roamer tracker; Contest helper; Tap to Fly | |
| Hunt stats, damage check, breeding tab, Pokédex finder | Union Room Battle, Records, Draw, Greet, Spin | |
| Retail `.sav` round-trip; automatic backups; durable `log.txt`; online **Mystery Gift** (WFC download) | Speed tweaks / FAST AUTO 3D; wide title on the title screen | |
| HMs without the Pokémon, shortcuts, visible wilds, shiny suite | | |
| **Local wireless trade between two 3DS consoles** | | |

On a New 3DS run of `1fcccb2` (run 702) the five 3D shortcuts were on for field and battles, including with the 3D slider up. Samples in that run were mostly 29–30 fps, the exit was clean, and the log had no crash. Play of that build found those options working well. That pass does not cover every scene, and it is not a measured frames-per-second comparison with the previous build. Title sides were switched to **WIDE** later in the same run; the title screen was not opened again, so a hardware look at the wide title is still ahead. Fades and battle speed can still dip.

Details: [`docs/ROADMAP.md`](docs/ROADMAP.md) · history: [`CHANGELOG.md`](CHANGELOG.md) · live notes: [`HANDOFF.md`](HANDOFF.md).

---

## Wireless & online

**Local (3DS ↔ 3DS).** Settings → Play → **Local wireless** maps the game’s DS wireless code onto 3DS UDS. On a hardware run, two New 3DS consoles found each other in the Union Room and completed a full trade (animation included; both Pokémon arrived intact).

- Needs two consoles running this port with Local wireless ON. Discovery can take a moment.
- Untested: Union Room Battle / Records / Draw / Greet / Spin.
- A real DS cannot join locally — native 3DS mode cannot speak DS MP radio.

**Online.** An ONLINE wireless mode drives the game’s own NitroDWC / NitroWiFi stack over 3DS sockets toward community revival servers (Wiimmfi / Poké Classic Network). On hardware, **GTS / Global Terminal** and **Mystery Gift** (Receive → Get via Nintendo WFC) both work. On 8 Nov 2026 the revival server used for GTS was reported down; that report is about the server, and this port’s GTS support is unchanged. Mystery Gift is a separate download and was still working. Local wireless Mystery Gift parent search still cannot find a DS parent. Online mode is refused while any `cheat_*` option is on. Back up saves before linking. See [`docs/WIRELESS.md`](docs/WIRELESS.md).

---

## Getting started

### What you need

- A **New Nintendo 3DS / New 3DS XL / New 2DS XL** with custom firmware (Luma3DS) and the Homebrew Launcher (plus **FBI** for the HOME Menu `.cia`). Guide: <https://3ds.hacks.guide>
- Your own dump of **Pokémon SoulSilver (USA)** as `SoulSilver.nds` (dump the cart you own)
- A Windows, Linux, or **WSL Ubuntu** machine to build

### 1. Easiest — Pokemon 3DS Builder

Use the one-window [patch-builder / Pokemon 3DS Builder](https://github.com/unowncollectables/patch-builder) (Windows 10/11): download `Pokemon3DSBuilder.exe` from its Releases page, tick SoulSilver, point it at the ROM you dumped and your SD card, and press the green button. It sets up WSL + Ubuntu and the toolchain for you (the first build takes an hour or two, later ones minutes), pulls the latest version of this project, compiles, and copies the `.3dsx` (and optionally `.cia`) while backing up your save first. It writes a build log you can attach to an issue. Follow that repo’s README for the click-path.

### 2. Or build by hand (Linux / WSL Ubuntu)

Generic flow from the repo docs — keep the ROM **outside** the clone:

```sh
bash scripts/bootstrap.sh                          # pspoke @ upstream.lock + pspoke-patches/
bash scripts/ss3ds_build.sh ~/roms/SoulSilver.nds  # ROM is read in place, never copied in
cd soulsilver3ds/full && make check && make        # → soulsilver3ds.3dsx
make cia                                           # optional HOME Menu .cia
```

Needs: devkitPro `3ds-dev` (`DEVKITPRO=/opt/devkitpro`), Python 3 + `ndspy`, git, make, patch, rsync, curl, ~3 GB disk. Full detail: [`docs/BUILDING.md`](docs/BUILDING.md) · [`docs/SETUP.md`](docs/SETUP.md).

### 3. Copy to the SD card and play

1. Copy `soulsilver3ds.3dsx` → `sdmc:/3ds/` (Homebrew Launcher) **or** install `soulsilver3ds.cia` with FBI.
2. Place your ROM at `sdmc:/3ds/soulsilver3ds/soulsilver.nds`.
3. Launch. The game creates `soulsilver.sav` beside it if none exists.

<details>
<summary><strong>SD card layout</strong> (<code>sdmc:/3ds/soulsilver3ds/</code>)</summary>

| File | Purpose |
|---|---|
| `soulsilver.nds` | your ROM (required) |
| `soulsilver.sav` | 512 KiB raw save |
| `soulsilver.sav.bak1..8` | automatic copies at launch, after each in-game save, and at clean exit |
| `import.sav` | optional cart / R4 / DeSmuME save to import on next launch |
| `config.txt` | settings written by the in-game Settings app |
| `log.txt` / `log_crash.txt` | run log (fsync’d so tails survive reboots); unclean exits preserved |
| `hunt.txt` | encounter / shiny-hunt counters (never touches the save) |

</details>

**Saves.** Plain 512 KiB `.sav`, interchangeable with a real cart. **Back up off the SD card before every new build.** Rolling backups help; a copy you made yourself is safer.

---

## Settings app

Press **L+R+Y** (or hold the top-left of the touch screen ~0.3 s). L/R or tap switches tab; left/right/A changes a value; B / START / L+R+Y closes. Changes apply on close → `config.txt`. SELECT reveals a hidden Debug tab.

| Tab | Highlights |
|---|---|
| **Display** | Wide view, Camera zoom, Bottom menu HD, Title sides (`title_clean_sides`: LEGACY / CLASSIC / WIDE). The screen is always the 1.25× wide picture; there is no scale option. The intended title-sides baseline is **WIDE**. |
| **Wide** | Wide battles / party / Pokédex / PC / menus, Wide FX (slow — keep off), haze, weather strips, edge fill |
| **3D** | 3D strength, battle depth, Split top, Smooth title 3D, SHARP / FAST 3D, plus command packets (`g3_packets`), fast bounds (`g3_fast_bounds`), second-screen compose (`gpu_compose_b`: OFF / ON / CHECK), rotating sprites (`gpu_obj_affine`), and GPU fades (`gpu_b_fades`). The intended baseline is all five on, with `gpu_compose_b` at **ON** (CHECK is the pixel-compare mode). |
| **Input** | Bottom menu, Pokégear tabs, Pokédex finder, Contest helper, Tap to Fly, Auto run, Bottom layout (Johto map or 4×2 tiles), Kanto gate, Menu button, Circle Pad / C-Stick as D-pad, ZR = Y, **ZL run toggle**, IV/EV on summary (Y) |
| **Battle** | Faster battles, Catch info, Encounter counter, Hunt stats (ZR), Damage check, Alive wild Pokémon, SELECT = soft reset, Skip intro on reset |
| **Setup** | Volume, Menu music, Save backups, Export save, Log size / detail, Fast text, Hold B skips text, Auto-advance, FPS counter, Breeding tab |
| **Play** | Reusable TMs, Field moves w/o HM, Repel prompt, No wild battles, Visible wild Pokémon, Pokémon menu X/Y shortcuts, Roamer + breeding tracker, **Local / Online wireless** |
| **Debug** (SELECT) | Audio limiter / stall fade, Speed tweaks — testing only |

`soulsilver3ds/full/README_settings.txt` lists the `config.txt` keys it was written to cover. It does not yet name every key the menu can write, including `g3_packets`, `g3_fast_bounds`, `gpu_compose_b`, `gpu_obj_affine`, `gpu_b_fades`, and `title_clean_sides`. The baseline to play with is those five 3D shortcuts on (`gpu_compose_b` = ON) and title sides at WIDE. Each stays in the menu so it can be turned back. Putting that baseline into the built-in defaults is a separate change still to come; this page does not change what a new `config.txt` starts as, so set the rows (or the file) yourself until that change lands. Newer features (Pokédex finder, Breeding tab, Hunt stats, Damage check, Alive wild Pokémon, Contest helper, Tap to Fly) still start OFF until they have had more testing. Experimental toggles: try them on a **copy** of your save. Hidden `cheat_*` keys exist only in `config.txt`, default off, and are not shown in the menu (online mode refuses them).

---

## Known issues

- Poké Ball opening can double-image with the 3D slider up.
- Wide FX is very slow — leave it off.
- Steady ~30 fps (the game’s own rate) is the goal and still being tuned; 60 fps is a later experiment ([`docs/FPS60.md`](docs/FPS60.md)). With the 3D shortcuts on, fades and some battles can still dip.
- Wide title sides (**WIDE**) still need a hardware look at the title screen. The 8 Nov 2026 run switched the option on later and did not return to the title.
- Local wireless: trading works; discovery can be slow; other Union Room modes untested.
- First visit to a map can hitch while data loads from the SD card.
- Local wireless Mystery Gift (friend / distribution) still searches forever without a real DS parent; use **Get via Nintendo WFC** with Wireless = ONLINE instead.

Found a bug? Open an [issue](https://github.com/unowncollectables/Pokemon-Soulsilver-New3ds/issues) and attach `sdmc:/3ds/soulsilver3ds/log.txt` (or `log_crash.txt`) plus your console model.

---

## For developers

```
your SoulSilver.nds  (runtime only — never committed)

pret/pokeheartgold → pspoke @ upstream.lock → + pspoke-patches/ → + soulsilver3ds/ platform → .3dsx / .cia
```

- Game C is compiled with GCC for the 3DS. Changes to pspoke live as **numbered patches** in `pspoke-patches/` — never edit an applied patch in place; add a new number.
- `soulsilver3ds/` is the platform layer: GPU renderer / compose, audio, input, Settings app, bottom menu, logging.
- Experimental features get a Settings toggle, removed once proven.

Start here: [`AGENTS.md`](AGENTS.md) · [`HANDOFF.md`](HANDOFF.md) · [`CONTRIBUTING.md`](CONTRIBUTING.md)

<details>
<summary><strong>Doc map</strong></summary>

| Doc | Covers |
| :--- | :--- |
| [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) | how the port is put together |
| [`docs/BUILDING.md`](docs/BUILDING.md), [`docs/SETUP.md`](docs/SETUP.md) | toolchain & build |
| [`docs/DEVELOPMENT.md`](docs/DEVELOPMENT.md) | working on the code |
| [`docs/ROADMAP.md`](docs/ROADMAP.md), [`docs/FAQ.md`](docs/FAQ.md) | plans & common questions |
| [`docs/FPS60.md`](docs/FPS60.md), [`docs/SPEEDUPS.md`](docs/SPEEDUPS.md) | frame-rate work |
| [`docs/WIRELESS.md`](docs/WIRELESS.md) | local UDS + online plan |
| [`docs/PLATINUM_SYNC.md`](docs/PLATINUM_SYNC.md) | shared work with Platinum |
| [`RELEASE_NOTES.md`](RELEASE_NOTES.md), [`CHANGELOG.md`](CHANGELOG.md), [`SECURITY.md`](SECURITY.md), [`THIRD_PARTY.md`](THIRD_PARTY.md) | releases, history, security, licences |

</details>

---

## Credits

- [@unowncollectables](https://github.com/unowncollectables) — project lead, hardware testing, direction
- **[pret/pokeheartgold](https://github.com/pret/pokeheartgold)** (and [pret/pokeplatinum](https://github.com/pret/pokeplatinum)) — the decompilations this port stands on
- **[IbrahimIrfan/pspoke](https://github.com/IbrahimIrfan/pspoke)** (GPL-3.0) — native SoulSilver build retargeted here
- **[GabeSud0](https://github.com/GabeSud0)** / [pokeplatinum-3ds](https://github.com/GabeSud0/pokeplatinum-3ds) — Platinum 3DS port; icons, lower-menu frame drawing, and platform ideas reused (see [`THIRD_PARTY.md`](THIRD_PARTY.md))
- GPL / third-party code credited per file headers and [`THIRD_PARTY.md`](THIRD_PARTY.md) (pspoke, melonDS-derived 2D/3D cores, GabeSud0 pieces, and more)
- **Claude** (Anthropic) — AI pair programmer; much of the platform code, patches, tooling, and docs
- **Grok Bot** — AI contributor; patches, crash fixes, Mystery Gift seeding, README & tooling help
- [devkitPro](https://devkitpro.org), libctru, citro3d; [Azahar](https://azahar-emu.org) for emulator testing
- Everyone who tests, files issues, and shares `log.txt` — thank you. Want a name here? Open a PR.

---

## Legal

This project is licensed under **GPL-3.0** — see [`LICENSE`](LICENSE). pspoke and the pret decompilations carry their own licences.

**No ROMs, assets, saves, or builds are included or distributed.** You must dump your own cartridge. Game data and assets belong to their copyright holders and are read only from the dump you provide.

Pokémon is a trademark of Nintendo / Game Freak / The Pokémon Company. This is an unofficial, personal-use fan project and is **not affiliated with, endorsed by, or connected to** Nintendo, Game Freak, or The Pokémon Company.
