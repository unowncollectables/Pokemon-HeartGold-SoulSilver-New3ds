<p align="center">
  <img src="assets/whirl-islands-title.webp" alt="Pokémon SoulSilver New 3DS Edition emblem over the Whirl Islands" width="900">
</p>

<h1 align="center">Pokémon HeartGold &amp; SoulSilver for New 3DS</h1>

<p align="center">
  Return to Johto in widescreen, with stereoscopic 3D and the Pokégear on the touch screen.
</p>

<p align="center">
  <a href="#setup"><img alt="New 3DS / New 2DS XL" src="https://img.shields.io/badge/New_3DS_%2F_New_2DS_XL-1e345b?style=flat-square"></a>
  &nbsp;
  <a href="LICENSE"><img alt="License: GPL v3" src="https://img.shields.io/badge/license_GPL_v3-4e679c?style=flat-square"></a>
  &nbsp;
  <a href="#status"><img alt="Status: in active development" src="https://img.shields.io/badge/status%3A_in_active_development-8a6232?style=flat-square"></a>
  &nbsp;
  <a href="#heartgold-experimental"><img alt="HeartGold: experimental" src="https://img.shields.io/badge/HeartGold-experimental-8a6232?style=flat-square"></a>
  &nbsp;
  <a href="#see-it-in-play"><img alt="Wide View + stereo 3D" src="https://img.shields.io/badge/Wide_View_%2B_stereo_3D-3d5a80?style=flat-square"></a>
</p>

<p align="center">
  <a href="https://github.com/unowncollectables/patch-builder/releases/latest"><strong>Download the Windows builder</strong></a>
  &nbsp;·&nbsp;
  <a href="#setup"><strong>Setup guide</strong></a>
  &nbsp;·&nbsp;
  <a href="#02--build-your-copy"><strong>Build instructions</strong></a>
</p>

## Contents

<table>
  <tr>
    <td width="33%"><a href="#features">Features</a></td>
    <td width="33%"><a href="#about">About</a></td>
    <td width="33%"><a href="#see-it-in-play">See it in play</a></td>
  </tr>
  <tr>
    <td><a href="#setup">Setup</a></td>
    <td><a href="#heartgold-experimental">HeartGold</a></td>
    <td><a href="#settings">Settings</a></td>
  </tr>
  <tr>
    <td><a href="#wireless--online">Wireless &amp; online</a></td>
    <td><a href="#status">Status</a></td>
    <td><a href="#for-developers">For developers</a></td>
  </tr>
</table>

[Credits](#credits) · [Legal](#legal)

---

## Features

SoulSilver is the build played on hardware. HeartGold installs beside it.

<table>
  <tr>
    <td width="50%" valign="top">
      <strong>3D slider</strong><br>
      Stereoscopic depth in the field and in battles, on a console with a 3D slider.
    </td>
    <td width="50%" valign="top">
      <strong>Wide View</strong><br>
      Towns, routes, and battles in widescreen.
    </td>
  </tr>
  <tr>
    <td valign="top">
      <strong>Smoother frame rate</strong><br>
      GPU shortcuts for a steadier picture. Steady 30 fps in every scene is still the target.
    </td>
    <td valign="top">
      <strong>Touch-screen Pokégear</strong><br>
      A live map, party, and bag, plus phone and radio, without leaving the field.
    </td>
  </tr>
  <tr>
    <td valign="top">
      <strong>Trading</strong><br>
      Union Room trades between two consoles running the port. Confirmed on hardware.
    </td>
    <td valign="top">
      <strong>GTS</strong><br>
      Community-server Global Trade Station. The original path has worked on SoulSilver hardware.
    </td>
  </tr>
  <tr>
    <td valign="top">
      <strong>Mystery Gift</strong><br>
      Wonder Cards through Get via Nintendo WFC. The WFC path has worked on SoulSilver hardware.
    </td>
    <td valign="top">
      <strong>Pokéwalker</strong><br>
      Send and return over the New 3DS infrared port. Both have worked on a New 3DS.
    </td>
  </tr>
  <tr>
    <td valign="top">
      <strong>Seasonal scenery</strong><br>
      Petals, seeds, leaves, and flurries on clear outdoor maps when Ambient Weather is on.
    </td>
    <td valign="top">
      <strong>HeartGold</strong><br>
      The same port as its own install. Experimental, and not yet played on a console.
    </td>
  </tr>
</table>

The map can also pin encounters, offer Fly, and show egg progress, a seasonal forecast, and Bug-Catching Contest stats. New behaviour is a toggle in Settings (<kbd>L</kbd> + <kbd>R</kbd> + <kbd>Y</kbd>). Open checks are under [Status](#status).

## About

**Pokémon HeartGold & SoulSilver for New 3DS** is a fan-made, native New Nintendo 3DS port of Pokémon SoulSilver and Pokémon HeartGold (HGSS), built from the [pret/pokeheartgold](https://github.com/pret/pokeheartgold) decompilation. It is not a DS emulator and not a ROM hack: the game is compiled to run directly on the 3DS, and you install it as a `.cia` from the HOME Menu (or launch the `.3dsx` from the Homebrew Launcher) on a New 3DS, New 3DS XL, or New 2DS XL with custom firmware (Luma3DS).

It adds things the original DS game can't do: stereoscopic 3D with the 3D slider, a widescreen Wide View, a smoother frame rate, and a touch-screen Pokégear with a live map, party, and bag. Trading, the GTS, Mystery Gift, and the Pokéwalker (send and return over the 3DS infrared port) still work. Everything new is a toggle in the in-game settings.

This repository holds only source code and patches. It contains no ROMs, game assets, or saves. You build your own copy from a legally dumped HeartGold or SoulSilver ROM you own.

## See it in play

### Goldenrod above. Johto below.

<table>
  <tr>
    <td align="center" width="33%">
      <img src="assets/screens/field-goldenrod.png" alt="Goldenrod City with visible wild Pokémon on the top screen and the Johto map with roamer sprites below" width="260"><br>
      <sub>Goldenrod with visible wild Pokémon and roamer sprites on the map.</sub>
    </td>
    <td align="center" width="33%">
      <img src="assets/screens/map-2x.png" alt="The bottom-screen Johto map zoomed to 2X" width="260"><br>
      <sub>Bottom map at 2X zoom.</sub>
    </td>
    <td align="center" width="33%">
      <img src="assets/screens/route-35.png" alt="Route 35 grass and trees north of Goldenrod" width="260"><br>
      <sub>Route 35, just north of Goldenrod.</sub>
    </td>
  </tr>
  <tr>
    <td align="center">
      <img src="assets/screens/panel-party.png" alt="Party panel on the touch screen" width="260"><br>
      <sub>Party panel without leaving the field.</sub>
    </td>
    <td align="center">
      <img src="assets/screens/panel-bag.png" alt="Bag panel on the touch screen" width="260"><br>
      <sub>Bag panel.</sub>
    </td>
    <td align="center">
      <img src="assets/screens/panel-radio.png" alt="Pokégear Radio panel on the touch screen" width="260"><br>
      <sub>Radio panel. Tap-to-tune still needs more hardware testing.</sub>
    </td>
  </tr>
</table>

### Around Johto

<table>
  <tr>
    <td align="center" width="50%">
      <img src="assets/screens/olivine.png" alt="Olivine City Pokémon Center and seaside boardwalk" width="340"><br>
      <sub>Olivine City.</sub>
    </td>
    <td align="center" width="50%">
      <img src="assets/screens/ecruteak.png" alt="Ecruteak City streets" width="340"><br>
      <sub>Ecruteak City.</sub>
    </td>
  </tr>
  <tr>
    <td align="center">
      <img src="assets/screens/mahogany.png" alt="Mahogany Town among the pines" width="340"><br>
      <sub>Mahogany Town.</sub>
    </td>
    <td align="center">
      <img src="assets/screens/blackthorn.png" alt="Blackthorn City on its rocky plateau" width="340"><br>
      <sub>Blackthorn City, with Tap to Fly on the bottom map.</sub>
    </td>
  </tr>
</table>

<sub>All screenshots captured in the Azahar emulator from a personal SoulSilver build.</sub>

- **Top screen.** Walk through towns and routes in widescreen, with optional stereoscopic depth in the field and in battles on consoles with a 3D slider.
- **Touch screen.** Check your party, bag, map, phone, or radio without leaving the field. Optional tools surface encounter locations, Fly destinations, egg progress, a seasonal forecast, and Bug-Catching Contest stats.
- **Your save.** Bring over a standard 512 KiB cartridge or flashcart save by copying it onto the save file. The port keeps rolling backups and can export a copy from Settings. Shiny hunt stats live in `hunt.txt`, and a StarMe profile lives in its own files.
- **Your pace.** Customize battle speed, text speed, move effectiveness hints, and visible wild encounters. With Ambient Weather on, clear outdoor maps can show the current season.

<p align="center">
  <img src="assets/johto-waystation.webp" alt="A compass and folded chart on a window ledge overlooking islands at dawn" width="900">
</p>

<a id="getting-started"></a>

## Setup

Build from your own US cartridge dump. The repository has no ROM in it. If you already play this port, copy your save off the SD card before replacing the build.

### 1. Check your kit

| You need | |
| --- | --- |
| Console | New Nintendo 3DS, New 3DS XL, or New 2DS XL with custom firmware. See the [3DS Hacks Guide](https://3ds.hacks.guide/). |
| Game | Your own dump of Pokémon SoulSilver (USA). HeartGold uses its own dump. See [HeartGold](#heartgold-experimental). |
| SD card | The card you use with the console. |

<a id="02--build-your-copy"></a>

### 2. Build your copy

On **Windows 10/11**, open [Pokémon 3DS Builder](https://github.com/unowncollectables/patch-builder) and choose your US `.nds` dump. SoulSilver and HeartGold can be built in the same run. The builder sets up the toolchain and can copy the finished build to the SD card. A `.cia` lands in the card's `cias` folder.

<details>
<summary><strong>Build SoulSilver by hand on Linux or WSL</strong></summary>

Install devkitPro's `3ds-dev` toolchain, Python 3 with `ndspy`, Git, make, patch, rsync, and curl. Keep the ROM outside the clone.

```sh
bash scripts/bootstrap.sh
bash scripts/ss3ds_build.sh ~/roms/SoulSilver.nds
cd soulsilver3ds/full && make check && make
make cia  # Optional HOME Menu package
```

See the [build guide](docs/BUILDING.md) and [toolchain setup](docs/SETUP.md). HeartGold has its own commands under [HeartGold](#heartgold-experimental).

</details>

### 3. Launch on the console

Put your dump at `sdmc:/3ds/soulsilver3ds/soulsilver.nds`.

| Launch | Where the program goes |
| --- | --- |
| Homebrew Launcher | Copy `soulsilver3ds.3dsx` into `sdmc:/3ds/`. |
| HOME Menu | Copy `soulsilver3ds.cia` to `sdmc:/cias/` and install it with FBI. |

In [New3DS FBI](https://github.com/unowncollectables/New3DS-FBI), **Update my Pokemon ports** (the progress screen is titled "Update my ports") lists that file and, after you confirm, installs title `000400000F7D4000`.

When bringing over an existing save, use a copy and keep the original off the SD card.

<details>
<summary><strong>Save files and SD card layout</strong></summary>

SoulSilver uses these files in `sdmc:/3ds/soulsilver3ds/`. HeartGold uses the same names under `sdmc:/3ds/heartgold3ds/`, with `heartgold.nds` and `heartgold.sav`.

| File | Role |
| --- | --- |
| `soulsilver.nds` | Your US cartridge dump. The game only reads it. |
| `soulsilver.sav` | The 512 KiB save. The game creates it on first launch if needed. |
| `soulsilver.sav.bak1` … `bak8` | Rolling backups. |
| `config.txt` | Settings. Delete a line to return that key to its built-in default. |
| `hunt.txt` | Hunt stats. Separate from the save. |
| `starme-wfc-*.bin` | StarMe WFC profile for one trainer, when that server is selected. Separate from the save. |
| `log.txt` | The current run log. `log_crash.txt` keeps a log from an unclean exit. |

To bring in a cart, flashcart, or emulator save, copy it over `soulsilver.sav` (keep a copy of the old one first). There is no import step.

</details>

## HeartGold (experimental)

HeartGold is the same port, built as its own game from your own US HeartGold dump. It installs beside SoulSilver. SoulSilver's folder, save, and HOME Menu title stay as they are.

The first full HeartGold build compiled, linked, and packaged, including the Ho-Oh banner. A console playtest and an emulator playtest are both still ahead. Every SoulSilver hardware result on this page still needs its own HeartGold check, online included.

| | SoulSilver | HeartGold |
| --- | --- | --- |
| SD folder | `sdmc:/3ds/soulsilver3ds/` | `sdmc:/3ds/heartgold3ds/` |
| ROM and save | `soulsilver.nds`, `soulsilver.sav` | `heartgold.nds`, `heartgold.sav` |
| Programs | `soulsilver3ds.3dsx`, `soulsilver3ds.cia` | `heartgold3ds.3dsx`, `heartgold3ds.cia` |
| HOME Menu title | `000400000F7D4000` | `000400000F7D4100` (`CTR-H-HG3D`) |
| HOME Menu banner | Lugia, taken from your SoulSilver ROM at build time | Ho-Oh, taken from your HeartGold ROM at build time |
| Settings | `config.txt` in the SoulSilver folder | Its own `config.txt`. The menu and the missing-key defaults are the same. |

A line in one `config.txt` stays in that game. Hunt stats go to that game's `hunt.txt`.

<details>
<summary><strong>Build and install HeartGold</strong></summary>

`scripts/hg_build.sh` compiles HeartGold in `pspoke-hg/`, and `scripts/hg_rename.py` gives that build HeartGold names.

On Windows, choose your US HeartGold dump in [Pokémon 3DS Builder](https://github.com/unowncollectables/patch-builder). The builder treats HeartGold as experimental and gives it its own output folder.

By hand, from this repository:

```sh
bash scripts/hg_build.sh /path/to/HeartGold.nds   # compiles the game (long the first time)
make -C heartgold3ds/full                         # heartgold3ds.3dsx
make -C heartgold3ds/full cia                     # heartgold3ds.cia, Ho-Oh on the banner
```

The ROM is only read. Nothing from it is stored in this repository. If the Ho-Oh banner check fails, the package falls back to the flat banner. The link stops if a HeartGold program still names a SoulSilver file.

Install it beside SoulSilver:

1. Put your dump at `sdmc:/3ds/heartgold3ds/heartgold.nds`.
2. Homebrew: copy `heartgold3ds.3dsx` to `sdmc:/3ds/`.
3. HOME Menu: copy `heartgold3ds.cia` to `sdmc:/cias/`. In New3DS FBI, open **Update my Pokemon ports**. That screen looks in `sdmc:/cias/` (and `sdmc:/fbi/ports/`) for title `000400000F7D4100` and installs it beside SoulSilver after you confirm.

</details>

## Settings

Open Settings with <kbd>L</kbd> + <kbd>R</kbd> + <kbd>Y</kbd>, or hold the top-left corner of the touch screen.

| | |
| --- | --- |
| Browse | <kbd>L</kbd> and <kbd>R</kbd>, or tap a tab. <kbd>SELECT</kbd> reveals Debug. |
| Change | <kbd>←</kbd> <kbd>→</kbd> or <kbd>A</kbd> |
| Apply and save | Close with <kbd>B</kbd>, <kbd>START</kbd>, or <kbd>L</kbd> + <kbd>R</kbd> + <kbd>Y</kbd>. Changes are written to `config.txt`. |

<details>
<summary><strong>Built-in defaults</strong></summary>

A missing `config.txt` line uses `kCfgDefaults` in `soulsilver3ds/platform/psp_ctru.c`. A line already in the file wins, including OFF. HeartGold reads that same table. The [settings reference](soulsilver3ds/full/README_settings.txt) lists keys; where it and the table disagree, the table is what the game reads.

| If this line is missing | It starts as |
| --- | --- |
| Title layout | WIDE |
| Command shortcut, Bounds shortcut, Affine object GPU, Second screen fades | ON |
| Second screen GPU | ON (`gpu_compose_b=1`; CHECK is 2) |
| Sharp 3D | FAST AUTO |
| Ambient weather | ON |
| Seasonal trees | OFF |
| Alive wild Pokémon | ON + TOWNS |
| Pokédex finder, Tap to Fly, Roamer sprites, Contest helper, Breeding tab, Reusable TMs, Event Pokémon without the event flag, Hunt stats, Damage check, Pokéwalker | ON |
| Map zoom | 1X (2X is the closer map; `-` / `+` on the map match it) |
| GTS server picker | OFF |
| Wireless | ONLINE |
| FPS counter | ON |
| Log detail | FULL |

Debug rows follow the same table when their line is missing. Speed tweaks starts at its highest step (`fps_tweaks=7`). The Debug tab stays behind SELECT. Try a copy of your save before relying on an experimental option.

</details>

## Wireless &amp; online

| Path | In short |
| --- | --- |
| Local | Settings → Setup → Wireless → **LOCAL** on both consoles, then the Union Room. Trading between two consoles running the port is confirmed on hardware. |
| Online | With no `wireless` line, Wireless starts at **ONLINE**: community-server GTS, and Mystery Gift → Get via Nintendo WFC. Both paths have worked on SoulSilver hardware. |
| Pokéwalker | On by default. Send a Pokémon to a Pokéwalker from the PC and bring it back, over the New 3DS infrared port. Send and return have both worked on a New 3DS. Repeat runs and the latest menu fix still need more hardware checks. |

<details>
<summary><strong>Wireless, GTS, and Pokéwalker notes</strong></summary>

**Local.** Discovery can take a moment. A DS cartridge can't join this 3DS session. The other Union Room activities are still unverified.

**Online.** GTS and Mystery Gift use separate services, and availability depends on the community hosts. Online mode requires every `cheat_*` setting off. `wifi_dns` overrides the community DNS used by Poké Classic. See the [wireless guide](docs/WIRELESS.md).

**Server picker.** Turn on **GTS server picker** in Setup, then reopen the title menu. Online Settings offers Poké Classic (the DNS path above), StarMe, and Wi-Fi setup instructions. Turning the picker off uses the DNS path again, including after StarMe was selected. StarMe's WFC identity is a file in the port folder. Poké Classic keeps using the profile stored in the save. Leaving the GTS runs the WFC cleanup that exit used to skip. Live deposit, pickup, and a hardware re-entry are still open.

**Pokéwalker.** On by default. Send a Pokémon to a Pokéwalker from the PC and bring it back, over the New 3DS infrared port. Send and return have both worked on a New 3DS. Repeat runs and the latest menu fix still need more hardware checks.

**Frontier and the plaza.** The Battle Frontier and the Safari Zone area screens are in the tree and still need a device visit. The Wi-Fi Plaza attendant answers `WI-FI PLAZA IS CLOSED`. The Wi-Fi Club minigames still stop.

</details>

## Status

**In active development.** On SoulSilver hardware, the core game, stereoscopic 3D, save round-trips, and a local Union Room trade have been exercised. Steady 30 fps across scenes is still the performance target. Fades, some battles, and the first visit to a map can hitch.

When [reporting an issue](https://github.com/unowncollectables/Pokemon-HeartGold-SoulSilver-New3ds/issues), include your console model and `log.txt` or `log_crash.txt` from `sdmc:/3ds/soulsilver3ds/` (or `sdmc:/3ds/heartgold3ds/` for that build).

<details>
<summary><strong>What's new</strong></summary>

> [!NOTE]
> Highlights merged after the previous README pass. A 3DS result is listed only where the [changelog](docs/CHANGELOG.md), [handoff](docs/HANDOFF.md), or [release notes](docs/RELEASE_NOTES.md) record one.

| Topic | In the tree | Checked |
| --- | --- | --- |
| Starter Poké Balls | In Elm's lab the three Poké Balls and the turntable draw on the touch screen again. The port's rotation matrices kept a garbage translation, so the balls sat off screen. Platform-only. | Azahar, from a copied save. A New 3DS look is still open. |
| Seasons | With Ambient Weather on, petals, seeds, leaves, and flurries spread across the field (still six at a time). Clear spring has more petals by Goldenrod's Flower Shop and wooded routes; clear autumn has more leaves around Ecruteak, those routes, and Kanto Route 2. **Seasonal trees** (Display, off until you turn it on) recolors supported outdoor canopies the next time the area loads. Summer keeps the original greens. Trunks, models, and shadows stay original. | Visibility and frame time are still open on a New 3DS, in both games. |
| Fresh `config.txt` | A missing line uses the built-in table. Alive wild Pokémon starts at ON + TOWNS, Ambient weather and Pokéwalker start on, and the other tuned keys under [Settings](#settings) follow. A line already in the file is kept. HeartGold uses the same table. The save file is untouched. | Defaults only. No new device pass. |
| GTS | **GTS server picker** (Setup, off until you turn it on) adds Online Settings to the title menu: Poké Classic, StarMe, or Wi-Fi setup. The row can show the last deposit seen for that trainer on that server. StarMe keeps its WFC profile in its own files, separate from the cartridge save. Poké Classic still uses the profile in the save. Leaving the GTS shuts that WFC session down. | The menu, a StarMe profile, and Mystery Gift were checked in Azahar on a copied save. Live deposit, pickup, and a hardware re-entry are still open. |
| Frontier, Safari, Plaza | The Battle Frontier and the Safari Zone area customizer and decoration screens are seeded. Talking to the Wi-Fi Plaza attendant shows `WI-FI PLAZA IS CLOSED`. The Wi-Fi Club minigames still stop. | A SoulSilver package was produced from this work. Those screens have not been opened on a 3DS. |
| Saves | Bring a save in by copying it over `soulsilver.sav` (or `heartgold.sav`). There is no import step. Rolling backups and Export save stay. The GTS preview and a StarMe profile replace the previous file; a damaged StarMe profile is kept as `.bad` and a new one is made. | A device look at a second GTS preview and the boot backup line is still open. |

</details>

<details>
<summary><strong>Experimental features and known issues</strong></summary>

- **HeartGold.** Compiles and packages. A console or emulator playtest has not been done.
- **Starter Poké Balls.** Elm's lab balls and turntable draw in Azahar. A New 3DS look is still open. A Poké Ball *opening in battle* can still show a double image with the 3D slider up.
- **Seasons.** Ambient scenery, the Johto WX radar, and Seasonal trees are awaiting on-device checks.
- **Frontier, Safari, Plaza.** Seeded in the tree. The Plaza shows a closed toast. None of these screens have been opened on a 3DS. The Club minigames still stop.
- **GTS picker and StarMe.** Checked in Azahar on a copied save. Live GTS and hardware re-entry are still open.
- **Pokéwalker.** Send and return transfers completed on a New 3DS. Repeatability and the latest menu rendering fix still need hardware verification.
- **Touch-screen panel.** The two registered item shortcuts are awaiting on-device checks.
- **3D and widescreen.** Wide FX is expensive. The wide title presentation still needs a dedicated hardware check.
- **Other checks.** Radio tuning, the roamer tracker, Contest helper, Tap to Fly, Alive wild Pokémon, and achievements still need more device coverage.

See the [handoff notes](docs/HANDOFF.md) for test results, the [changelog](docs/CHANGELOG.md) for recent changes, and the [roadmap](docs/ROADMAP.md) for what's planned.

</details>

## For developers

The port compiles the [pret/pokeheartgold](https://github.com/pret/pokeheartgold) decompilation through [pspoke](https://github.com/IbrahimIrfan/pspoke), retargeted to devkitARM. Platform code lives in `soulsilver3ds/`. Guides live in [`docs/`](docs/).

<details>
<summary><strong>Layout, patches, and doc index</strong></summary>

The native platform layer in `soulsilver3ds/` handles 3DS rendering, audio, input, and settings. Game-side changes are maintained as numbered patches in `pspoke-patches/`. HeartGold is a filtered build of that same source (`scripts/hg_rename.py`, `heartgold3ds/full`); the generated HeartGold copies are not committed.

A stopped build can leave a half-applied patch. `scripts/bootstrap.sh` aborts that leftover before the next patch. The Battle Frontier patch applies on a fresh checkout and on an existing tree.

Conduct, contributing, and security live in [`.github/`](.github/).

- [Architecture](docs/ARCHITECTURE.md) and [development guide](docs/DEVELOPMENT.md)
- [Build guide](docs/BUILDING.md), [setup](docs/SETUP.md), and [FAQ](docs/FAQ.md)
- [Contributing](.github/CONTRIBUTING.md), [project conventions](AGENTS.md), and [handoff notes](docs/HANDOFF.md)
- [Release notes](docs/RELEASE_NOTES.md), [changelog](docs/CHANGELOG.md), and [security policy](.github/SECURITY.md)

</details>

## Credits

- [unowncollectables](https://github.com/unowncollectables) leads the project and tests on hardware.
- [pret/pokeheartgold](https://github.com/pret/pokeheartgold), [pret/pokeplatinum](https://github.com/pret/pokeplatinum), and [IbrahimIrfan/pspoke](https://github.com/IbrahimIrfan/pspoke) provide the decompilation and native build foundations.
- [3ds-platinum](https://github.com/unowncollectables/3ds-platinum) and [GabeSud0/pokeplatinum-3ds](https://github.com/GabeSud0/pokeplatinum-3ds) contributed platform code, patches, and interface work.
- [devkitPro](https://devkitpro.org/), libctru, citro3d, and [Azahar](https://azahar-emu.org/) provide tooling for development and testing. Claude and Grok Bot contributed code, tooling, and documentation.
- New3DS FBI is this project's installer fork of [FBI by Steveice10](https://github.com/Steveice10/FBI).
- Thanks to everyone who tests builds and shares logs. See [third-party notices](docs/THIRD_PARTY.md) for full attributions.

## Legal

This unofficial, personal-use fan project is licensed under the [GNU GPL v3](LICENSE). You supply the original game data by dumping your own US cartridge (SoulSilver, or HeartGold for that build). The repository contains source code and documentation, including screenshots for illustration. Game assets and trademarks belong to their respective owners. Nintendo, Game Freak, and The Pokémon Company have no affiliation with this project.
