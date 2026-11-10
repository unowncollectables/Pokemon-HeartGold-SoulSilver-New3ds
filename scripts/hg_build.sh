#!/usr/bin/env bash
# hg_build.sh /path/to/HeartGold.nds : the HeartGold (US) game compile. EXPERIMENTAL.
#   hg_build.sh --prepare : only fetch pspoke-hg, apply new patches and the HeartGold names (used by patch-builder)
#   needs: the same tools as ss3ds_build.sh (devkitPro 3ds-dev, python3 + ndspy, git, make, patch, rsync, curl)
#
# HeartGold is the same port as SoulSilver, built as a separate game:
#   - its own copy of pspoke in ./pspoke-hg (own work tree and objects; ./pspoke, the SoulSilver tree, is never touched)
#   - the decomp compiled as HeartGold: SS3DS_GAME=HEARTGOLD makes toolchain-3ds/bin/psp-gcc pass -DHEARTGOLD
#   - its own names: scripts/hg_rename.py gives the copy the HeartGold SD folder and ROM check (sdmc:/3ds/heartgold3ds/,
#     heartgold.nds, heartgold.sav, game code IPKE)
# Then: make -C heartgold3ds/full   (links heartgold3ds.3dsx; `make cia` for heartgold3ds.cia)
# The ROM is only read in place; it is never copied into this repository.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
HG="$ROOT/pspoke-hg"
# prepare: fetch pspoke into pspoke-hg / apply new pspoke-patches, then give it the HeartGold names.
# The name filter edits tracked files of pspoke-hg in place. They are put back first, so bootstrap applies new patches to
# exactly what SoulSilver's pspoke has; then the filter runs again (it is idempotent; build inputs are compared by content).
prepare() {
  if [ -d "$HG/.git" ]; then git -C "$HG" am --abort >/dev/null 2>&1 || true; git -C "$HG" checkout -q -- .; fi
  SS3DS_PSPOKE_DIR=pspoke-hg bash "$ROOT/scripts/bootstrap.sh"
  python3 "$ROOT/scripts/hg_rename.py" tree "$HG"
}
if [ "${1:-}" = --prepare ]; then prepare; exit 0; fi   # patch-builder: patches + names only, no compile
ROM="$(cd "$(dirname "${1:?usage: hg_build.sh /path/to/HeartGold.nds   (or --prepare)}")" && pwd)/$(basename "$1")"
[ -f "$ROM" ] || { echo "no ROM at $ROM" >&2; exit 1; }
export DEVKITPRO="${DEVKITPRO:-/opt/devkitpro}" DEVKITARM="${DEVKITARM:-${DEVKITPRO:-/opt/devkitpro}/devkitARM}"
[ -x "$DEVKITARM/bin/arm-none-eabi-gcc" ] || { echo "devkitARM not found at $DEVKITARM (install devkitPro 3ds-dev)" >&2; exit 1; }
# Exact reference dump, or a US HeartGold whose every file matches it (see scripts/hg_romcheck.py). SoulSilver is refused.
python3 "$ROOT/scripts/hg_romcheck.py" "$ROM" || { echo "  Build stopped. Use your own clean HeartGold (US) .nds (SoulSilver is built with ss3ds_build.sh)." >&2; exit 1; }
mkdir -p "$ROOT/build-hg" && echo "$ROM" > "$ROOT/build-hg/rom_path"   # lets heartgold3ds/full `make cia` find the same ROM
prepare
# PSPDEV points pspoke at our stand-in SDK so it never downloads the PSP toolchain.
export SS3DS_GAME=HEARTGOLD
export PSPDEV="$ROOT/toolchain-3ds" PATH="$ROOT/toolchain-3ds/bin:$DEVKITARM/bin:$DEVKITPRO/tools/bin:$PATH"
echo "[HG] building HeartGold (experimental) in $HG; SoulSilver's tree ($ROOT/pspoke) is not used"
cd "$HG" && bash scripts/ss3ds_game.sh "$ROM"
