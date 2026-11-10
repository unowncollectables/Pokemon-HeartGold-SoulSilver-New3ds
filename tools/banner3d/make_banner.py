#!/usr/bin/env python3
"""The SoulSilver HOME Menu 3D banner. The portal, sparkles and logo are this repo's own art (art/); with --rom the title
screen's Lugia (model, textures and wing beat) is read from your own ROM at build time and flies in the middle. The
banner made then contains the game's model, so it is never committed or shared. Without --rom, or if Lugia cannot be
read from the ROM, the banner is the art-only one. Layout and the CGFX fixes follow the Platinum 3DS port's
tools/banner3d (GabeSud0/pokeplatinum-3ds, GPL-3.0); see README.md.

    python3 tools/banner3d/make_banner.py [--rom SoulSilver.nds] --bannertool BANNERTOOL --chime banner.wav -o build/cia/banner3d.bnr

The scene is turned into CGFX by skyfloogle/pycgfx, fetched once at a pinned commit into build/pycgfx (needs git),
with the fixes in to_cgfx.py. Needs Python 3.12 or later; standard library only.
"""
import argparse
import hashlib
import os
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)

PYCGFX_REPO = "https://github.com/skyfloogle/pycgfx.git"
TITLE_NARC = "a/0/4/6"                       # the title screen's models (pokeheartgold: title_screen graphics)
LUGIA_MODEL_NAME = "lugia01_10t"             # the NSBMD's own name; its wing-beat NSBCA is the next member
# --game: which title-screen Pokemon, from which ROM, with which of this repo's art. Both models are in both ROMs'
# title archive; each game shows its own. keep = the model's materials that are the Pokemon (the rest is sea, sky,
# clouds); flap = frames in one wing beat; two_sided: see scene.build(). None = scene.py's defaults (Lugia), so
# SoulSilver's banner is unchanged.
GAMES = {
    "soulsilver": {"code": b"IPGE", "title": "SoulSilver", "pokemon": "Lugia", "model": LUGIA_MODEL_NAME, "art": "art",
                   "keep": None, "flap": None, "two_sided": None},
    "heartgold": {"code": b"IPKE", "title": "HeartGold", "pokemon": "Ho-Oh", "model": "houou_test", "art": "art-hg",
                  "keep": ("h_body_r", "h_body_w", "h_eye", "h_eye_line", "h_kuchi", "h_leg", "h_mouth", "h_mouth02", "h_neck",
                           "h_nose", "h_tail", "h_tosaka", "h_tsume", "h_wing01", "h_wing02"), "flap": 60,
                  "two_sided": ("h_tail", "h_wing01", "h_wing02"),
                  "pose": {"width": 14.0, "pos": (0.0, 1.4, 0.0), "turn": (12.0, -50.0)}},      # packed by fit_parts.py; these are drawn from both sides
}


def lugia_from_rom(rom_path, game=GAMES["soulsilver"]):
    """(nsbmd, nsbca) of the title-screen Lugia from a SoulSilver ROM (or the game's own Pokemon from its own ROM);
    ValueError if it is not where expected."""
    import nitrofs
    import narc
    data = open(rom_path, "rb").read()
    if data[0xC:0x10] != game["code"]:
        raise ValueError(f"{rom_path} is not a US {game['title']} ROM (game code {data[0xC:0x10]!r}, wanted {game['code']!r})")
    entry = next((e for e in nitrofs.parse_nitrofs(data) if e["path"] == TITLE_NARC), None)
    if entry is None:
        raise ValueError(f"{TITLE_NARC} isn't in this ROM")
    archive = data[entry["offset"]:entry["offset"] + entry["size"]]
    index = 0
    while True:
        try:
            member = narc.member(archive, index)
        except ValueError:
            break                                          # past the last member
        if member[:4] == b"BMD0" and game["model"].encode() in member[:0x100]:
            nsbca = narc.member(archive, index + 1)
            if nsbca[:4] != b"BCA0":
                raise ValueError(f"the title {game['pokemon']}'s animation isn't the next member")
            return member, nsbca
        index += 1
    raise ValueError(f"no {game['model']} model in {TITLE_NARC}")


def fetch_pycgfx(dest, commit):
    if os.path.isfile(os.path.join(dest, "main.py")):
        head = subprocess.run(["git", "-C", dest, "rev-parse", "HEAD"], capture_output=True, text=True).stdout.strip()
        if head == commit:
            return
    print(f"Fetching pycgfx ({commit[:7]}) into {dest}")
    os.makedirs(dest, exist_ok=True)
    git = lambda *a: subprocess.run(["git", "-C", dest, *a], check=True, stdout=subprocess.DEVNULL)
    if not os.path.isdir(os.path.join(dest, ".git")):
        git("init", "-q")
    git("fetch", "-q", "--depth", "1", PYCGFX_REPO, commit)
    git("checkout", "-q", "--force", "FETCH_HEAD")


def main():
    if sys.version_info < (3, 12):
        sys.exit("The 3D banner needs Python 3.12 or later (pycgfx uses it).")
    import scene
    import to_cgfx
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--fetch-pycgfx-only", action="store_true", help="just fetch pycgfx (for the tests)")
    ap.add_argument("--game", choices=sorted(GAMES), default="soulsilver", help="heartgold: Ho-Oh from a US HeartGold ROM, gold art (default: soulsilver)")
    ap.add_argument("--rom", help="your US SoulSilver ROM (.nds): the title-screen Lugia goes in the banner")
    ap.add_argument("--sparkles", action="store_true", help="EXPERIMENT: four drifting sparkle cards (not in the Platinum banner; fails its structure check)")
    ap.add_argument("--max-tris", type=int, default=scene.PLATINUM_TRIS, help="Lugia triangle budget (default: the Platinum banner's 1000; 0 = no limit)")
    ap.add_argument("--allow-mismatch", action="store_true", help="EXPERIMENT: build even if the banner differs from Platinum's (never used by the Makefile)")
    ap.add_argument("--fingerprint-out", help="write the banner's structural fingerprint (JSON, no game data) here")
    ap.add_argument("--bannertool", help="bannertool (tools/build_cia_tools.sh)")
    ap.add_argument("--chime", help="the banner's sound (.wav, at most 3 seconds)")
    ap.add_argument("--pycgfx", default=os.path.join(ROOT, "build", "pycgfx"), help="where pycgfx is fetched")
    ap.add_argument("-o", "--output", help="the .bnr to write")
    ap.add_argument("--keep", help="keep the glTF scene and CGFX in this folder")
    args = ap.parse_args()
    to_cgfx.ALLOW_MISMATCH = args.allow_mismatch
    to_cgfx.FINGERPRINT_OUT = args.fingerprint_out

    if args.fetch_pycgfx_only:
        fetch_pycgfx(args.pycgfx, to_cgfx.PYCGFX_COMMIT)
        return
    if not (args.bannertool and args.chime and args.output):
        ap.error("needs --bannertool, --chime and -o")

    game = GAMES[args.game]
    who = game["pokemon"]
    extra = {k: v for k, v in (("keep", game["keep"]), ("flap_frames", game["flap"]), ("two_sided", game["two_sided"])) if v is not None}
    extra.update(game.get("pose", {}), who=who)
    lugia = None
    if args.rom:
        try:
            lugia = lugia_from_rom(args.rom, game)
            print(f"{who}: {len(lugia[0])} + {len(lugia[1])} bytes from {args.rom}")
        except (OSError, ValueError) as ex:
            # BANNER=3d-lugia asked for Lugia: do not quietly make a different 3D banner, let the Makefile use the flat one
            sys.exit(f"[BANNER] FAILED: no {who} from the ROM ({ex}).")

    fetch_pycgfx(args.pycgfx, to_cgfx.PYCGFX_COMMIT)
    with tempfile.TemporaryDirectory() as tmp:
        work = args.keep or tmp
        try:
            gltf = scene.build(os.path.join(HERE, game["art"]), work, sparkles=args.sparkles, lugia=lugia, max_tris=args.max_tris, **extra)
            cgfx = to_cgfx.convert(args.pycgfx, gltf)
        except to_cgfx.BannerLimitError as ex:
            sys.exit(f"[BANNER] FAILED a HOME Menu safety limit: {ex}")
        except Exception as ex:                      # nothing odd may reach the HOME Menu: the Makefile falls back to flat
            sys.exit(f"[BANNER] FAILED: {ex!r}")
        cgfx_path = os.path.join(work, "banner.cgfx")
        with open(cgfx_path, "wb") as f:
            f.write(cgfx)
        digest = hashlib.sha1(cgfx).hexdigest()
        os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
        subprocess.run([args.bannertool, "makebanner", "-ci", cgfx_path, "-a", args.chime, "-o", args.output],
                       check=True, stdout=subprocess.DEVNULL)
    print(f"Made {args.output} (3D banner{' with ' + who if lugia else ', art only'}, CGFX {len(cgfx)} bytes, sha1 {digest[:12]}, passed the safety limits, UNTESTED on a real HOME Menu)")


if __name__ == "__main__":
    main()
