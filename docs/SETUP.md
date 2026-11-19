# Project setup

For playing, start with the [README setup guide](../README.md#setup). Windows users can use the [Pokémon 3DS Builder](https://github.com/unowncollectables/patch-builder/releases/latest). Manual Linux/WSL commands are in [BUILDING.md](BUILDING.md).

## Local layout

Keep a source checkout beside a folder containing your own ROM dumps:

```text
~/3ds/
├── pokemon-soulsilver-new3ds/
└── roms/
    ├── SoulSilver.nds
    └── HeartGold.nds
```

The build scripts fetch the pinned pspoke source and apply this repository's patches. You do not need to prepare a separate decompilation checkout or manually extract a game-data folder. Generated trees and outputs are ignored by Git.

Manual builds need devkitPro `3ds-dev`, Python 3 with `ndspy`, Git, make, patch, rsync and curl. Keep the generated work tree with the checkout that created it; its compiler commands contain absolute paths.

## Console and saves

The supported target is New 3DS, New 3DS XL or New 2DS XL with custom firmware. Copy your US dump to the game's SD folder and launch a `.3dsx`, or install a built `.cia` with FBI. Exact paths are in the [README](../README.md#setup).

Before replacing a build or bringing in a save, keep a copy of the save off the SD card. Each game's `.sav` is a raw 512 KiB cartridge-compatible image. SoulSilver and HeartGold have separate folders and saves.

Current defaults are built in. Player options are under L+R+Y; old `config.txt` values still take priority. The [Settings section](../README.md#settings) explains how to reset an older setup.
