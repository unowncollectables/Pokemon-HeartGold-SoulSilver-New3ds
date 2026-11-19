# Building HeartGold and SoulSilver for New 3DS

Build from your own US cartridge dump. The repository contains source, platform code and patches; the game data comes from your local ROM.

## Windows builder

Download [Pokémon 3DS Builder](https://github.com/unowncollectables/patch-builder/releases/latest), open it and select your US SoulSilver or HeartGold `.nds`. The builder sets up the toolchain, builds current main and can copy the result to your SD card. Both games can be built in one run.

Launch paths and save layout are in the [README setup guide](../README.md#setup).

## Linux or WSL

Install devkitPro's `3ds-dev` toolchain, Python 3 with `ndspy`, Git, make, patch, rsync and curl. The default toolchain location is `/opt/devkitpro`; `DEVKITPRO` and `DEVKITARM` can override it. Allow roughly 3 GB for the build tree, plus your ROM and outputs.

Keep your dump outside the repository, for example `~/3ds/roms/SoulSilver.nds`.

### SoulSilver

From the repository root:

```sh
bash scripts/bootstrap.sh
bash scripts/ss3ds_build.sh ~/3ds/roms/SoulSilver.nds
make -C soulsilver3ds/full check
make -C soulsilver3ds/full
make -C soulsilver3ds/full cia  # Optional HOME Menu package
```

Outputs are `soulsilver3ds/full/soulsilver3ds.3dsx` and, with the last command, `soulsilver3ds/full/soulsilver3ds.cia`. The game build checks the US ROM SHA1 against `upstream.lock` and stops on a mismatch. The dump is read in place.

### HeartGold (experimental)

```sh
bash scripts/hg_build.sh ~/3ds/roms/HeartGold.nds
make -C heartgold3ds/full
make -C heartgold3ds/full cia
```

HeartGold uses its own `pspoke-hg/` and `build-hg/` trees. Outputs, SD folder, save names and HOME Menu title are separate from SoulSilver. Its ROM check accepts the supported US dump or the US variant described by the file hashes in `upstream.lock`. Build/package success is not a HeartGold runtime test.

## Updating an existing checkout

Pull the current source and run the game-build script again before linking. New numbered patches can add game hooks; a platform-only link cannot add those hooks to old game objects. `bootstrap.sh` handles a leftover interrupted patch before applying the next one.

Keep generated caches with the checkout that created them. Copied or relocated `pspoke/.work` trees can retain absolute paths in `compile-command.json` and `rebuild_game_object.sh`, causing hooks to rebuild a different checkout. Use a fresh generated tree when moving a build, or verify and regenerate those paths before trusting an incremental result.

Existing player settings override current defaults. See [Settings](../README.md#settings) for resetting an older configuration.

## Troubleshooting

- Check that devkitARM and the devkitPro tools are on the script's PATH.
- Read `pspoke/.work/tree/logs/` and the first failing compiler or hook message. A guarded hook failure deliberately stops the build.
- If a new interface says it needs a hook, confirm that the full game-build step ran and that its generated paths point to this checkout.
- A CIA uses the game's model from your ROM for its banner when possible; a failed 3D-banner check falls back to the flat banner.

ROMs, saves, extracted game data and built binaries stay out of commits. Developer workflow and device-test requirements are in [AGENTS.md](../AGENTS.md).
