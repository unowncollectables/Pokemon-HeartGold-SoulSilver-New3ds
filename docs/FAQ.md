# FAQ

## Is this a full playable game?

Mostly. The game boots and plays on a New 3DS (overworld, battles, menus, saving, Pokégear, local wireless trading), but it is a homebrew port in active development, not a finished retail build, and some features are still untested on hardware. See [Status in the README](../README.md#status).

## Do I need a ROM?

Yes. Supply your own US SoulSilver cartridge dump, or your own US HeartGold dump for the experimental HeartGold build. The repository does not include ROM data. The Windows builder and manual build instructions are in the [README](../README.md#setup).

## Is it legal to distribute this project?

The repository is designed to stay within the BYO-ROM rule and avoid bundling copyrighted assets. You still need to comply with local laws and Nintendo's rights when building, testing, and sharing work.

## Can I contribute?

Yes. Contributions are welcome, especially documentation, build fixes, testing, compatibility notes, or feature work. Keep changes focused and do not add copyrighted content.

## Where is the game source?

The repo holds the 3DS platform layer, patches and tooling; the game code is built from pspoke and your own ROM at build time (`scripts/bootstrap.sh`), and no game data is stored here.

## What hardware does this target?

New Nintendo 3DS, New Nintendo 3DS XL, and New Nintendo 2DS XL with custom firmware. Stereoscopic 3D requires a model with a 3D screen. Older 3DS models are not the supported target.

## What is the goal of the project?

The goal is to bring Pokémon SoulSilver to original New 3DS hardware with native rendering, stereo 3D, and a more maintainable codebase built from decompilation work.

## Can my save go back to a real DS cart or an R4?

Yes. `sdmc:/3ds/soulsilver3ds/soulsilver.sav` is the raw 512 KiB image written by the game's own save code (two slots at
0x00000 and 0x40000, each with footers and CRC16), the same layout a cart or R4 `.sav` has. Copy it out to export; copy a
cart or R4 `.sav` over it to bring one in. Check any `.sav` with `python3 tools/ss_savecheck.py file.sav` (read only).
Always back up before swapping saves.

## Can I trade with another player?

Yes. Set Settings > Setup > Wireless to **LOCAL** on both consoles and meet in the Union Room. A full trade between two consoles running the port was confirmed on hardware. A DS cartridge cannot join that local 3DS session. Union Room battles still need a two-console test.

**ONLINE** uses community services for GTS and Mystery Gift. The original GTS path and WFC Mystery Gift have worked on SoulSilver hardware; the newer StarMe path still needs live deposit/pickup and hardware re-entry coverage. See the [wireless guide](WIRELESS.md).

## Does it really run at 60 fps?

The SoulSilver overworld is about 60 fps in mono and close to it in stereo 3D on New 3DS, based on the hardware performance pass. Heavy scenes, transitions, first-time map loads and battles can dip. This is not a claim that every screen or animation runs at 60 fps; the title Lugia remains at 30 fps. The tuned performance setup is now built in.

## Do I need to change settings after updating?

A fresh setup uses the current tuned defaults. Existing `config.txt` values still win. To reset an older setup, close the game, back up `config.txt`, remove it from that game's folder, and relaunch. Keep the `.sav` in place. Change player options through L+R+Y afterward.

## Is HeartGold tested as thoroughly as SoulSilver?

No. HeartGold compiles and packages as its own install, with a separate folder, save and HOME Menu title. It remains experimental and needs its own runtime and hardware checks. SoulSilver test results do not automatically validate HeartGold.
