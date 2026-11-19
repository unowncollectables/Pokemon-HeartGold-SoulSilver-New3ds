#!/usr/bin/env python3
"""Export repository documentation as a GitHub wiki. Does not push or delete pages."""
import argparse
import re
import subprocess
from pathlib import Path
from urllib.parse import quote, unquote, urlsplit
import posixpath

ROOT = Path(__file__).resolve().parents[1]
REPO = 'https://github.com/unowncollectables/Pokemon-HeartGold-SoulSilver-New3ds'


def git(*args):
    return subprocess.check_output(['git', '-C', str(ROOT), *args], text=True).strip()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output', type=Path, help='Existing wiki clone or an empty output directory')
    args = parser.parse_args()
    out = args.output.resolve()
    if out == ROOT or ROOT in out.parents:
        parser.error('Output must be outside the source checkout')
    out.mkdir(parents=True, exist_ok=True)
    sha = git('rev-parse', 'HEAD')
    paths = git('ls-files').splitlines()
    sources = [p for p in paths if p.endswith('.md')]
    sources += ['soulsilver3ds/full/README_settings.txt', 'LICENSE']
    names = {
        'README.md': 'Project-Overview', 'AGENTS.md': 'Agent-and-Project-Conventions',
        'CLAUDE.md': 'Claude-Entry-Point', 'HANDOFF.md': 'Current-Handoff',
        'soulsilver3ds/full/README_settings.txt': 'Full-Settings-Reference',
        'LICENSE': 'License',
    }
    for source in sources:
        if source not in names:
            stem = source.removesuffix('.md').replace('/', '--').lstrip('.')
            names[source] = stem.replace('_', '-')
    if len(set(names.values())) != len(names):
        raise ValueError('Wiki page name collision')

    def page(source, label=None):
        return f'[{label or names[source]}]({names[source]})'

    def convert_url(url, source, image=False):
        parsed = urlsplit(url)
        if parsed.scheme or parsed.netloc or not parsed.path:
            return url
        path = posixpath.normpath(posixpath.join(posixpath.dirname(source), unquote(parsed.path)))
        fragment = ('#' + parsed.fragment) if parsed.fragment else ''
        if path in names and not image:
            return names[path] + fragment
        encoded = quote(path, safe='/')
        if image:
            return f'https://raw.githubusercontent.com/unowncollectables/Pokemon-HeartGold-SoulSilver-New3ds/{sha}/{encoded}' + fragment
        kind = 'tree' if (ROOT / path).is_dir() else 'blob'
        return f'{REPO}/{kind}/{sha}/{encoded}' + fragment

    for source in sources:
        body = (ROOT / source).read_text(encoding='utf-8')
        if source.endswith('.txt') or source == 'LICENSE':
            body = f'# {names[source].replace("-", " ")}\n\n```text\n{body.rstrip()}\n```\n'
        else:
            # Preserve code examples literally; rewrite navigation in prose only.
            chunks = re.split(r'(^```[^\n]*\n[\s\S]*?^```[^\n]*$)', body, flags=re.M)
            for index in range(0, len(chunks), 2):
                chunks[index] = re.sub(r'(!?\[[^\]\n]*\]\()([^\s)]+)([^)]*\))',
                    lambda m: m[1] + convert_url(m[2], source, m[1].startswith('!')) + m[3], chunks[index])
                chunks[index] = re.sub(r'((?:src|href)=\")([^\"]+)(\")',
                    lambda m: m[1] + convert_url(m[2], source, m[1].startswith('src')) + m[3], chunks[index])
            body = ''.join(chunks)
        archive = '/archive/' in source
        old_reference = source in ('docs/WIRELESS.md', 'soulsilver3ds/full/README_settings.txt', 'AGENTS.md')
        warning = ''
        if archive:
            warning = '> **Historical archive.** This is an older development record. Features, defaults, paths, and test status may have changed. Start with [Current status](Current-Status) for today’s guidance.\n\n'
        elif old_reference:
            warning = '> **Technical reference with historical sections.** Some older entries describe superseded menus, defaults, or plans. Use [Controls and settings](Controls-and-Settings), [Wireless and online](Wireless-and-Online), and [Current status](Current-Status) for current player guidance.\n\n'
        provenance = f'> From [`{source}`]({REPO}/blob/{sha}/{quote(source, safe="/")}) at `{sha[:7]}`.\n\n'
        (out / (names[source] + '.md')).write_text(warning + provenance + body, encoding='utf-8')

    custom = {
        'Home': f'''# Pokémon HeartGold & SoulSilver for New 3DS

Welcome to the complete project wiki: setup, player guides, technical documentation, tools, and development history.

This is a native New Nintendo 3DS homebrew port. Bring your own supported US cartridge dump. SoulSilver is played on hardware; HeartGold remains experimental. The project is in active development.

## Start here

1. [Getting started](Getting-Started) — requirements, Windows builder, installation, and updates.
2. [Controls and settings](Controls-and-Settings) — menus, map, battle tools, and current defaults.
3. [Saves and backups](Saves-and-Backups) — cart/flashcart saves, SD folders, and recovery.
4. [Wireless and online](Wireless-and-Online) — local trade, GTS, Mystery Gift, and Pokéwalker.
5. [HeartGold](HeartGold) — its separate install and testing limits.
6. [Troubleshooting](Troubleshooting) — fixes and useful bug reports.

## Explore the whole project

- [Current status and remaining tests](Current-Status)
- {page('README.md', 'Full feature tour and screenshot gallery')}
- {page('docs/FAQ.md', 'FAQ')} · {page('docs/BUILDING.md', 'Build instructions')}
- {page('docs/CHANGELOG.md', 'Changelog')} · {page('docs/RELEASE_NOTES.md', 'Release notes')} · {page('docs/ROADMAP.md', 'Roadmap')}
- [Documentation library](Documentation-Library) — every repository Markdown guide, tool README, policy, technical plan, and archive, plus the full settings reference and license.

## About this wiki

Published from main snapshot [`{sha[:7]}`]({REPO}/commit/{sha}), 19 November 2026. Source pages link to that exact revision. Archived material is labeled; historical plans are not promises of current support. See [Wiki maintenance](Wiki-Maintenance) for updating this wiki from a newer main.

[Source repository]({REPO}) · [Report a bug]({REPO}/issues/new/choose) · [Ask a question]({REPO}/discussions)
''',
        'Getting-Started': f'''# Getting started

## Requirements

- New Nintendo 3DS, New 3DS XL, or New 2DS XL with custom firmware. Older 3DS models are not the supported target. Stereo 3D needs a console with a 3D screen.
- Your own supported US SoulSilver dump, or US HeartGold dump for that build.
- Your console’s SD card. Copy any existing save off the card before installing or updating.

## Build on Windows

Download [Pokémon 3DS Builder](https://github.com/unowncollectables/patch-builder/releases/latest), open it, and choose your US `.nds` dump. SoulSilver and HeartGold can be built in the same run. The builder sets up the toolchain and can copy the results to the SD card. CIA files go in `cias/` for installation.

The repository provides source and tooling. It does not distribute game ROMs or prebuilt game packages.

## Install SoulSilver

1. Put your ROM at `sdmc:/3ds/soulsilver3ds/soulsilver.nds`.
2. For Homebrew Launcher, put `soulsilver3ds.3dsx` in `sdmc:/3ds/`.
3. For HOME Menu, put `soulsilver3ds.cia` in `sdmc:/cias/` and install it with FBI. In [New3DS FBI](https://github.com/unowncollectables/New3DS-FBI), use **Update my Pokemon ports** and confirm the install.
4. To bring an existing save, close the game and follow [Saves and backups](Saves-and-Backups).

SoulSilver’s HOME Menu title is `000400000F7D4000`. [HeartGold](HeartGold) has separate files and a separate title.

## Update an existing installation

Back up the game folder first, especially the `.sav`. Rebuild from current source using the builder and install the new program. Keep your ROM and save in place. Existing settings take priority over new defaults; see [Controls and settings](Controls-and-Settings).

For manual Linux/WSL builds, use {page('docs/BUILDING.md', 'the complete build guide')} and {page('docs/SETUP.md', 'toolchain setup')}. When updating a source checkout, rerun the game-build script before linking so new hooks are included.
''',
        'Controls-and-Settings': f'''# Controls and settings

## Settings menu

Open with **L + R + Y**, or hold the top-left corner of the touch screen.

- Browse tabs with L/R or touch. The player tabs are Display, 3D, Input, Battle, Play, and Setup.
- Select a row with up/down; change it with left/right or A.
- Close with B, START, or L + R + Y to apply and save to `config.txt`.

Debug and Render tabs were removed. The tested performance setup is built in.

## Map and battle tools

- The touch-screen Pokégear provides map, party, bag, phone, and radio panels.
- Pan the map; use `-` and `+` for zoom. Tap a species in a location’s wild list for encounter pins and time filters. LIST shows locations and encounter-slot chances. AREAS tracks recognized sub-area visits separately from the save.
- Press **Y in battle** for the tabbed information overlay. Tap or swipe between available info, catch, hunt, damage, and trainer-party pages. The battle keeps running.
- FIGHT offers move-effectiveness hints. Optional tools include Tap to Fly, roamer sprites, breeding progress, and the Contest helper.
- Press **ZR in the field** for Hunt Stats. The latest overlay has Hunt, Totals, Top, Recent, and Shiny pages. On Hunt, use up/down or swipe to choose a species. X pins or unpins it, Y lets you browse all species, and tapping Reset twice clears that species’ counters. Hunt stats are stored separately in `hunt.txt`. The latest overlay still needs a device check.

## Current fresh-setup defaults

Wide layouts, sharp panel text, sharp game text, seasonal trees, fast tab swaps, and 60 fps field are on. Alive wild Pokémon starts at ON + TOWNS. Finder, Tap to Fly, roamer sprites, Contest helper, breeding tab, reusable TMs, event Pokémon without the event flag, hunt stats, damage check, and Pokéwalker start on. Map zoom is 1X; 3D strength is 1X. Wireless is ONLINE and the GTS server picker is on. FPS counter is on; log detail is SHORT.

**Ambient Weather currently defaults OFF until it is fixed.** It is separate from seasonal trees. Some gallery captures enable it to show the effect.

Saved `config.txt` values override defaults. Normal player choices belong in the settings menu. To reset an older configuration, close the game, back up `config.txt`, remove it from that game’s folder, and relaunch. Keep the `.sav` in place.

{page('soulsilver3ds/full/README_settings.txt', 'Full technical settings reference')} includes legacy keys and historical notes; it is not a list of settings players need to edit. {page('README.md', 'The source README')} records the current default table.
''',
        'Saves-and-Backups': '''# Saves and backups

The save is a raw **512 KiB `.sav`**, compatible with a DS cartridge or R4-style flashcart. Save round-trips have been exercised on SoulSilver hardware. Always keep an independent copy off the SD card.

## Import or export

1. Close the game before copying files.
2. Copy your existing port save and the incoming save somewhere safe.
3. To import SoulSilver, copy a compatible raw save to `sdmc:/3ds/soulsilver3ds/soulsilver.sav`. There is no import menu or conversion step for a compatible raw save.
4. To export, copy that file off the SD card. Use your save manager’s normal process to restore it to a cartridge.

HeartGold uses `sdmc:/3ds/heartgold3ds/heartgold.sav`. Do not swap game names or folders accidentally. A save-check tool can read a copy: `python3 tools/ss_savecheck.py /path/to/copy.sav`.

## Files in the game folder

- `soulsilver.nds`: your US ROM, read by the game.
- `soulsilver.sav`: the live save.
- `soulsilver.sav.bak1`: the single automatic backup; current release notes describe backups at launch, after in-game saves, and at exit. A failed backup write preserves the existing backup.
- `config.txt`: settings.
- `hunt.txt`: separate hunt statistics.
- `starme-wfc-*.bin`: per-trainer StarMe network profiles, separate from the save.
- `log.txt`: current run log; `log_crash.txt`: preserved crash log.

HeartGold uses its own folder and `heartgold.nds` / `heartgold.sav` names.

## Recover from a backup

Close the game. Copy the entire game folder off the SD card before relaunching. Preserve the live save and `.bak1` under distinct names. Work on copies; inspect a backup before copying it over the live `.sav`. Keep the untouched originals so a recovery attempt cannot overwrite the only remaining copy.

Automatic backup files can be replaced by later sessions; they do not replace an independent backup. Older documents mention eight rotating backups; current main uses one `.bak1`.
''',
        'Wireless-and-Online': f'''# Wireless and online

## Local trading

Set **Settings → Setup → Wireless → LOCAL** on both consoles, then meet in the Union Room. A full trade between two consoles running the SoulSilver port has worked on hardware. Union Room battles still need two-console testing.

A DS cartridge cannot join this native 3DS local session. Do not treat older cross-port feasibility plans as completed compatibility tests.

## GTS and Mystery Gift

Set Wireless to **ONLINE**. The original community-server GTS path and **Mystery Gift → Receive → Get via Nintendo WFC** have worked on SoulSilver hardware. Online mode requires every `cheat_*` setting off. Community services can be unavailable independently of the port; GTS and Mystery Gift use separate services.

## Server picker

With **GTS server picker** enabled in Setup, reopen the title menu to access Online Settings. It offers Poké Classic, StarMe, and Wi-Fi setup instructions. The picker starts on in fresh configurations.

Poké Classic uses the original DNS path and save-backed WFC profile. StarMe uses its own per-trainer identity files in the port’s folder. Disabling the picker returns to the DNS path even after StarMe was selected. The new picker and StarMe work have Azahar coverage on copied saves; live deposit/pickup and hardware re-entry coverage remain open.

Do not erase or recreate a network profile just to troubleshoot without preserving a backup first.

## Pokéwalker

Pokéwalker starts on. Send from the PC and return through the New 3DS infrared port. Both send and return have worked on a New 3DS. Repeated transfers and the latest menu rendering fix still need more hardware checks.

## Remaining limits

Wi-Fi Club minigames still stop. Wi-Fi Plaza displays a closed message. HeartGold needs its own online and infrared checks; SoulSilver results do not validate it.

{page('docs/WIRELESS.md', 'Full wireless engineering history and feasibility plan')} contains older status sections; the player guidance above follows current main’s README and FAQ.
''',
        'HeartGold': f'''# HeartGold — experimental

HeartGold is built from your own supported US HeartGold dump as a separate game. Its first full build compiled, linked, and packaged with the Ho-Oh banner. **A console playtest and emulator playtest are still pending in the current documentation.** SoulSilver’s hardware results do not establish HeartGold compatibility.

## Separate installation

- Folder: `sdmc:/3ds/heartgold3ds/`.
- ROM: `heartgold.nds`; save: `heartgold.sav`.
- Program: `heartgold3ds.3dsx` or `heartgold3ds.cia`.
- HOME Menu title: `000400000F7D4100` (`CTR-H-HG3D`).
- Settings and hunt statistics: its own `config.txt` and `hunt.txt`.

SoulSilver stays in its own folder and uses title `000400000F7D4000`. Both can be installed together.

## Build and launch

On Windows select your HeartGold ROM in Pokémon 3DS Builder. For Linux or WSL, from the source checkout:

```sh
bash scripts/hg_build.sh /path/to/HeartGold.nds
make -C heartgold3ds/full
make -C heartgold3ds/full cia
```

Put the ROM in the folder above. For Homebrew Launcher put the `.3dsx` in `sdmc:/3ds/`. For HOME Menu put the `.cia` in `sdmc:/cias/` and install with FBI or New3DS FBI’s **Update my Pokemon ports**.

The banner takes Ho-Oh from your ROM at build time and falls back to a flat banner if its check fails. {page('docs/BUILDING.md', 'The full build guide')} covers supported ROM checks and separate build trees.
''',
        'Current-Status': f'''# Current status

This is an active homebrew project, not a finished retail release. This summary follows main `{sha[:7]}` on 19 November 2026.

## Exercised on SoulSilver hardware

Core gameplay, stereo 3D, save round-trips, local Union Room trading, the original GTS path, WFC Mystery Gift downloads, and Pokéwalker send/return have hardware results. Overworld performance is about 60 fps in mono and close to it in stereo 3D. Busy scenes, battles, fades, and first map loads can dip. Title Lugia remains at 30 fps.

## Recent features needing broader hardware coverage

Sharp game text and panel text, refreshed finder LIST layout, the tabbed Y battle companion, map navigation improvements, visible wild behavior changes, and newer StarMe flows have varying build/emulator checks. These are not blanket hardware certifications. The current handoff records the exact evidence and outstanding checks.

Main also includes the redesigned ZR Hunt Stats overlay, with species pinning, browsing and counter reset. The handoff records host test coverage for pinning. The updated overlay still needs a build and device test.

## Known limits and remaining work

- Poké Ball opening can show a double image with the 3D slider up.
- Wide FX is expensive; keep it off.
- Some screens retain older DS fill in Wide View: Pokéathlon, Photo Album, Hall of Fame, evolution, and Battle Frontier.
- Union Room battles need a two-console test.
- Wi-Fi Club minigames stop; Wi-Fi Plaza shows a closed message.
- HeartGold needs runtime and hardware validation of its own.
- StarMe live GTS deposit/pickup and hardware re-entry remain open.
- New upstream backports and reset teardown fixes have pending device checks documented in the handoff.

## Detailed records

- {page('HANDOFF.md', 'Current handoff and exact testing status')}
- {page('docs/CHANGELOG.md', 'Changelog')} and {page('docs/RELEASE_NOTES.md', 'release notes')}
- {page('docs/ROADMAP.md', 'Roadmap')}
- [Troubleshooting and reporting](Troubleshooting)

Historical research is under [Documentation library](Documentation-Library). Archived status statements describe their original dates.
''',
        'Troubleshooting': f'''# Troubleshooting

## The game will not start

Check the console model, installed program, supported US dump, filename, and game folder. SoulSilver expects `sdmc:/3ds/soulsilver3ds/soulsilver.nds`; HeartGold expects `sdmc:/3ds/heartgold3ds/heartgold.nds`. Install the CIA after copying it, or launch the `.3dsx` through Homebrew Launcher.

## Build stops or a feature says a hook is missing

Use {page('docs/BUILDING.md', 'the build guide')}. Read the first failing compiler/hook message. After updating source, run the game-build step before linking: new patches cannot appear in old game objects through a platform-only link. Copied generated trees may contain absolute paths to another checkout; regenerate them in the correct location.

## Text is missing or doubled

Turn **Settings → Display → Sharp game text** off and check whether the original text returns. Include the log and a screenshot in a report. Panel/menu sharp text is a separate setting.

## Settings or performance differ after updating

Old settings override fresh defaults. Close the game and back up `config.txt` before resetting it as described in [Controls and settings](Controls-and-Settings). Keep the save in place. The tuned performance setup is built in. A busy scene or first map visit can still dip.

## Save missing or damaged

Stop and preserve the game folder before relaunching. Follow [Saves and backups](Saves-and-Backups). Work on copies. Never upload a save just to make an ordinary bug report.

## Online or local connection fails

Check LOCAL versus ONLINE and the chosen server. Local mode connects ports using native 3DS wireless, not DS cartridges. Service availability can differ for GTS and Mystery Gift. See [Wireless and online](Wireless-and-Online) and preserve profile files before changing them.

## Report a bug

Use [the bug report form]({REPO}/issues/new/choose). Include the console model, SoulSilver or HeartGold, build/run ID from the log, exact steps, what happened, and any relevant screenshot. Copy `log.txt` or `log_crash.txt` from that game’s folder before launching again, since a new run replaces `log.txt`. Review logs for personal details before sharing. Do not attach ROMs, extracted assets, game packages, or private save files.

[Discussions]({REPO}/discussions) is available for general questions. {page('.github/SECURITY.md', 'The security policy')} explains reporting security issues.
''',
        'Wiki-Maintenance': f'''# Wiki maintenance

The wiki is an export of repository documentation plus player navigation guides. No game build, ROM, save, or additional GitHub token is needed to generate pages locally. Publishing requires existing Git push access to the initialized wiki.

From an up-to-date source checkout:

```sh
git clone {REPO}.wiki.git ../hgss-wiki
python3 scripts/build_wiki.py ../hgss-wiki
git -C ../hgss-wiki diff --stat
git -C ../hgss-wiki add -- '*.md'
git -C ../hgss-wiki commit -m "Refresh complete project wiki"
git -C ../hgss-wiki push origin HEAD
```

If the wiki clone already exists, pull its current branch before generating. Review the diff before committing. The exporter overwrites its generated pages but does not delete unrecognized pages. Review stale generated pages manually if source documents are removed or renamed. Custom edits to generated pages should go into the source document or exporter first, otherwise the next export replaces them.

Each reference page links to its exact source commit. Relative document links become wiki links; images and other source files link to that pinned revision. Historical archives receive a banner, and older mixed-status references warn readers to use the current player guides.

Before publishing, refresh the player summaries against the newest README, FAQ, handoff, and release notes. Their status statements require editorial review; importing a newer source page does not automatically rewrite the summaries.
''',
    }
    current = [p for p in sources if p.startswith('docs/') and '/archive/' not in p]
    archives = [p for p in sources if '/archive/' in p]
    other = [p for p in sources if p not in current and p not in archives]
    library = '# Complete documentation library\n\nEvery tracked repository Markdown document is included, plus the settings text reference and license. Reference pages are pinned to the source revision shown on each page.\n\n'
    for title, entries in [('Current documentation', current), ('Project, tools, policies, and references', other), ('Historical archive — older plans and test records', archives)]:
        library += f'## {title}\n\n' + '\n'.join(f'- {page(p, p)}' for p in entries) + '\n\n'
    custom['Documentation-Library'] = library
    custom['_Sidebar'] = '''**Player guide**

- [Home](Home)
- [Getting started](Getting-Started)
- [Controls and settings](Controls-and-Settings)
- [Saves and backups](Saves-and-Backups)
- [Wireless and online](Wireless-and-Online)
- [HeartGold](HeartGold)
- [Current status](Current-Status)
- [Troubleshooting](Troubleshooting)

**Complete reference**

- [Documentation library](Documentation-Library)
- [Feature tour and gallery](Project-Overview)
- [Full settings reference](Full-Settings-Reference)
- [Wiki maintenance](Wiki-Maintenance)
'''
    custom['_Footer'] = f'[Home](Home) · [All documentation](Documentation-Library) · [Source]({REPO}) · Snapshot `{sha[:7]}`. Historical pages may describe superseded behavior.'
    for name, body in custom.items():
        if name in names.values():
            raise ValueError(f'Custom page collision: {name}')
        (out / (name + '.md')).write_text(body, encoding='utf-8')
    expected = set(names.values()) | set(custom)
    # Check every relative Markdown page link before anything is published.
    broken = []
    for name in expected:
        body = (out / (name + '.md')).read_text(encoding='utf-8')
        body = re.sub(r'^```[^\n]*\n[\s\S]*?^```[^\n]*$', '', body, flags=re.M)
        for target in re.findall(r'\]\(([^\s)]+)', body):
            parsed = urlsplit(target)
            if not parsed.scheme and not parsed.netloc and parsed.path and unquote(parsed.path) not in expected:
                broken.append(f'{name}: {target}')
    if broken:
        raise ValueError('Broken wiki page links:\n' + '\n'.join(broken))
    print(f'Exported {len(expected)} pages from {sha[:7]}; {len(sources)} source documents; all local page links resolve.')


if __name__ == '__main__':
    main()
