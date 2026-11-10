#!/usr/bin/env bash
# hg_build.sh /path/to/HeartGold.nds : the HeartGold (US) game compile. EXPERIMENTAL.
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
ROM="$(cd "$(dirname "${1:?usage: hg_build.sh /path/to/HeartGold.nds}")" && pwd)/$(basename "$1")"
[ -f "$ROM" ] || { echo "no ROM at $ROM" >&2; exit 1; }
export DEVKITPRO="${DEVKITPRO:-/opt/devkitpro}" DEVKITARM="${DEVKITARM:-${DEVKITPRO:-/opt/devkitpro}/devkitARM}"
[ -x "$DEVKITARM/bin/arm-none-eabi-gcc" ] || { echo "devkitARM not found at $DEVKITARM (install devkitPro 3ds-dev)" >&2; exit 1; }
WANT=$(sed -n 's/^heartgold_sha1=//p' "$ROOT/upstream.lock")
GOT=$( (command -v sha1sum >/dev/null && sha1sum "$ROM" || shasum -a 1 "$ROM") | cut -d' ' -f1)
[ "$GOT" = "$WANT" ] || { echo "error: this ROM is not the HeartGold (US) dump the port expects." >&2; echo "  ROM SHA1:      $GOT" >&2; echo "  expected SHA1: $WANT" >&2; echo "  Build stopped. Use a clean HeartGold (US) .nds (SoulSilver is built with ss3ds_build.sh)." >&2; exit 1; }
mkdir -p "$ROOT/build-hg" && echo "$ROM" > "$ROOT/build-hg/rom_path"   # lets heartgold3ds/full `make cia` find the same ROM
HG="$ROOT/pspoke-hg"
# The name filter edits tracked files of pspoke-hg in place. Put them back first, so bootstrap applies new patches to
# exactly what SoulSilver's pspoke has, then filter again (the filter is idempotent; build inputs are compared by content).
if [ -d "$HG/.git" ]; then git -C "$HG" am --abort >/dev/null 2>&1 || true; git -C "$HG" checkout -q -- .; fi
SS3DS_PSPOKE_DIR=pspoke-hg bash "$ROOT/scripts/bootstrap.sh"
python3 "$ROOT/scripts/hg_rename.py" tree "$HG"
# PSPDEV points pspoke at our stand-in SDK so it never downloads the PSP toolchain.
export SS3DS_GAME=HEARTGOLD
export PSPDEV="$ROOT/toolchain-3ds" PATH="$ROOT/toolchain-3ds/bin:$DEVKITARM/bin:$DEVKITPRO/tools/bin:$PATH"
echo "[HG] building HeartGold (experimental) in $HG; SoulSilver's tree ($ROOT/pspoke) is not used"
cd "$HG" && bash scripts/ss3ds_game.sh "$ROM"
