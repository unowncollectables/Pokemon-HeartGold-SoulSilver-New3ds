#!/usr/bin/env bash
# build_pack.sh <Black 2 / White 2 / Black / White .nds> <out b2sprites.bin> [work dir]
# Makes the animated battle sprite pack for Settings > Battle > "B2 sprites" from the user's own ROM.
# Uses AnimaEngine (MIT, KillDaWill/AnimaEngine) at a pinned commit plus animaengine-rawdump.patch, which
# only adds an indexed frame dump next to its GIF export. Nothing extracted here is ever committed: the pack
# goes onto the user's SD card (sdmc:/3ds/soulsilver3ds/b2sprites.bin).
set -euo pipefail
ROM="$(realpath "$1")"; OUT="$(realpath -m "$2")"; WORK="${3:-${TMPDIR:-/tmp}/ss3ds-b2sprites}"
HERE="$(cd "$(dirname "$0")" && pwd)"
AE_URL=https://github.com/KillDaWill/AnimaEngine.git
AE_COMMIT=34c559873af3a604bc431377a897e22d8c28d535
mkdir -p "$WORK"
if [ ! -x "$WORK/ae/AnimaEngine" ]; then
  rm -rf "$WORK/ae"; git clone -q "$AE_URL" "$WORK/ae"; git -C "$WORK/ae" checkout -q "$AE_COMMIT"
  git -C "$WORK/ae" apply "$HERE/animaengine-rawdump.patch"
  make -s -C "$WORK/ae" AnimaEngine >/dev/null 2>&1 || make -C "$WORK/ae" AnimaEngine
fi
AE="$WORK/ae/AnimaEngine"; EXP="$WORK/export"; rm -rf "$EXP"; mkdir -p "$EXP"
# Female-only art is not exported yet (AnimaEngine's batch export ignores --gender), so both genders use one sprite.
# Gen 4 species with forms in Black 2 / White 2 (form index = the game's own form number).
declare -A FORMS=([201]=28 [351]=4 [386]=4 [412]=3 [413]=3 [422]=2 [423]=2 [421]=2 [479]=6 [487]=2 [492]=2 [493]=17)
one() { # species form gender
  local d="$EXP/$(printf '%03d_%02d_%d' "$1" "$2" "$3")"
  local g=(); [ "$3" = 1 ] && g=(--gender 1)
  ANIMA_RAW_DUMP=1 "$AE" "$ROM" "$1" "$d" --gif-side both --gif-palette normal --gif-scale 1 --form "$2" "${g[@]}" >/dev/null 2>&1 || { echo "b2pack: species $1 form $2 gender $3 failed (skipped)"; rm -rf "$d"; return 0; }
  find "$d" -type f ! -name '*.raw' ! -name palettes.json -delete
}
for sp in $(seq 1 493); do
  n=${FORMS[$sp]:-1}
  for f in $(seq 0 $((n - 1))); do one "$sp" "$f" 0; done
  [ $((sp % 50)) = 0 ] && echo "b2pack: $sp / 493"
done
python3 "$HERE/b2pack.py" "$EXP" "$OUT"
