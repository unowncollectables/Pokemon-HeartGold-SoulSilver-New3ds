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
# Rebuild AnimaEngine whenever the pinned commit or our patch changes (a kept old build ignored the seam fix / female art).
AE_STAMP="$AE_COMMIT $(sha1sum < "$HERE/animaengine-rawdump.patch" | cut -c1-40)"
if [ ! -x "$WORK/ae/AnimaEngine" ] || [ "$(cat "$WORK/ae/.ss3ds-stamp" 2>/dev/null)" != "$AE_STAMP" ]; then
  echo "b2pack: building AnimaEngine with the current patch"
  rm -rf "$WORK/ae"; git clone -q "$AE_URL" "$WORK/ae"; git -C "$WORK/ae" checkout -q "$AE_COMMIT"
  git -C "$WORK/ae" apply "$HERE/animaengine-rawdump.patch"
  make -s -C "$WORK/ae" AnimaEngine >/dev/null 2>&1 || make -C "$WORK/ae" AnimaEngine
  echo "$AE_STAMP" > "$WORK/ae/.ss3ds-stamp"
fi
# AnimaEngine reads the whole ROM for every sprite (about 1300 runs): read it from a local copy, not a Windows drive.
if [ "$ROM" != "$WORK/rom.nds" ] && ! cmp -s "$ROM" "$WORK/rom.nds" 2>/dev/null; then cp "$ROM" "$WORK/rom.nds"; fi
ROM="$WORK/rom.nds"
AE="$WORK/ae/AnimaEngine"; EXP="$WORK/export"; rm -rf "$EXP"; mkdir -p "$EXP"
# Female art: the patched AnimaEngine (ANIMA_FEMALE=1) swaps in the female sheets and skips species/forms that have none;
# b2pack.py also drops a female entry identical to the male one, and the game falls back to the male sprite.
# Gen 4 species with forms in Black 2 / White 2 (form index = the game's own form number).
declare -A FORMS=([201]=28 [351]=4 [386]=4 [412]=3 [413]=3 [422]=2 [423]=2 [421]=2 [479]=6 [487]=2 [492]=2 [493]=17)
one() { # species form gender
  local d="$EXP/$(printf '%03d_%02d_%d' "$1" "$2" "$3")" fem=()
  [ "$3" = 1 ] && fem=(env ANIMA_FEMALE=1)
  if ! ANIMA_RAW_DUMP=1 "${fem[@]}" "$AE" "$ROM" "$1" "$d" --gif-side both --gif-palette normal --gif-scale 1 --form "$2" >/dev/null 2>&1; then
    [ "$3" = 1 ] || echo "b2pack: species $1 form $2 failed (skipped)"
    rm -rf "$d"; return 0
  fi
  find "$d" -type f ! -name '*.raw' ! -name palettes.json -delete
}
for sp in $(seq 1 493); do
  n=${FORMS[$sp]:-1}
  for f in $(seq 0 $((n - 1))); do one "$sp" "$f" 0; one "$sp" "$f" 1; done
  [ $((sp % 50)) = 0 ] && echo "b2pack: $sp / 493"
done
python3 "$HERE/b2pack.py" "$EXP" "$OUT"
rm -f "$WORK/rom.nds"
