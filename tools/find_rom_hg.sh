#!/usr/bin/env bash
# find_rom_hg.sh [--list] : print the path of the user's US HeartGold .nds (first hit wins), or nothing.
# The HeartGold twin of find_rom.sh; it never returns a SoulSilver path. The ROM is only read, never copied.
# Order: HGROM / P3DS_HEARTGOLD (env), the path the last hg_build.sh was given (build-hg/rom_path), the repo's roms/
# folder, ~/3ds/roms/, ~/roms/, a sibling roms/ folder, then the SD copy patch-builder installs.
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cands=()
[ -n "${HGROM:-}" ] && cands+=("$HGROM")
[ -n "${P3DS_HEARTGOLD:-}" ] && cands+=("$P3DS_HEARTGOLD")
[ -f "$ROOT/build-hg/rom_path" ] && cands+=("$(head -n1 "$ROOT/build-hg/rom_path")")
for d in "$ROOT/roms" "$HOME/3ds/roms" "$HOME/roms" "$ROOT/../roms" "$ROOT/../3ds/roms"; do
  cands+=("$d/HeartGold.nds" "$d/heartgold.nds")
done
[ -n "${P3DS_SD:-}" ] && cands+=("$P3DS_SD/3ds/heartgold3ds/heartgold.nds")
if [ "${1:-}" = --list ]; then printf '%s\n' "${cands[@]}"; exit 0; fi
for c in "${cands[@]}"; do [ -f "$c" ] && { echo "$c"; exit 0; }; done
exit 0
