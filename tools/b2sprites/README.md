# Black 2 / White 2 battle sprites pack

`build_pack.sh <your Black 2, White 2, Black or White .nds> <out/b2sprites.bin>` makes the pack that
Settings > BATTLE > **Black 2 sprites** (hook 0298, `soulsilver3ds/platform/b2sprites.c`) reads from
`sdmc:/3ds/soulsilver3ds/b2sprites.bin`. The builder (patch-builder) runs it for you when a Black 2 / White 2 ROM is picked.

- Uses [AnimaEngine](https://github.com/KillDaWill/AnimaEngine) (MIT) at a pinned commit, plus `animaengine-rawdump.patch`,
  which only adds an indexed-frame dump (`ANIMA_RAW_DUMP=1`) next to its GIF export. Needs gcc, make, libpng-dev, git, python3.
- `b2pack.py` turns those dumps into the pack: idle animation per Pokemon 1-493, front and back, every Gen 4 form,
  normal + shiny 16-colour palettes, frames de-duplicated. About 37 MB, about 4 minutes.
- Female-only art is not exported yet (AnimaEngine's batch export ignores `--gender`); both genders use one sprite.
- The pack is made from the user's own ROM and goes onto their SD card only. Never commit it, the ROM or any extracted file.
