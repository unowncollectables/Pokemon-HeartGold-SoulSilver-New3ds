#!/usr/bin/env python3
"""hg_rename.py : the HeartGold build is the SoulSilver source through this name filter.

HeartGold shares every line of the port with SoulSilver. What must differ is where it lives: its own SD folder
(sdmc:/3ds/heartgold3ds/), its own save and ROM names (heartgold.sav, heartgold.nds), its own program names
(heartgold3ds.3dsx/.cia) and its own HOME Menu title. Nothing in the SoulSilver source changes for that: the HeartGold
build makes filtered copies (never committed) and compiles those.

  hg_rename.py tree DIR        in place, on the HeartGold copy of pspoke (pspoke-hg): SD paths + the ROM game code check
  hg_rename.py copy SRC DST    filtered copy of a file or folder (platform layer, link Makefile, CIA files);
                               files are rewritten only when their content changes, so make stays incremental
  hg_rename.py check FILE...   stop (exit 1) if a built file still names a SoulSilver SD path or the SoulSilver title

The check is the safety net for the save: a HeartGold program that could open sdmc:/3ds/soulsilver3ds/ is never linked.
"""
import re
import sys
from pathlib import Path

SS_UNIQUE_ID, HG_UNIQUE_ID = '0xF7D40', '0xF7D41'   # HOME Menu title: SoulSilver 000400000F7D4000, HeartGold 000400000F7D4100
SS_PRODUCT, HG_PRODUCT = 'CTR-H-SS3D', 'CTR-H-HG3D'

# Names of files and folders on the SD card and of the built programs. "soulsilver-native-*" (pspoke's folders) and
# "patches/soulsilver" are source folders shared by both games and stay as they are.
PATHS = [
    (re.compile(r'soulsilver3ds'), 'heartgold3ds'),
    (re.compile(r'soulsilver(?=\.(?:nds|sav)\b|_%s\.sav|_YYYYMMDD)'), 'heartgold'),
]
# What the player reads (boot screen, crash screen, log header, HOME Menu name).
WORDS = [
    (re.compile(r'SoulSilver 3DS'), 'HeartGold 3DS'),
    (re.compile(r'Pokemon SoulSilver'), 'Pokemon HeartGold'),
]
# Per file extras for the link Makefile and the CIA settings.
MAKEFILE = [
    (re.compile(r'\$\(ROOT\)/pspoke/'), '$(ROOT)/pspoke-hg/'),
    (re.compile(r'-DSS3DS -ffunction-sections'), '-DSS3DS -DSS3DS_HEARTGOLD -ffunction-sections'),
    (re.compile(r'tools/find_rom\.sh'), 'tools/find_rom_hg.sh'),
    (re.compile(r'SoulSilver\.nds'), 'HeartGold.nds'),
    (re.compile(r'scripts/ss3ds_build\.sh'), 'scripts/hg_build.sh'),
]
RSF = [
    (re.compile(re.escape(SS_UNIQUE_ID) + r'\b'), HG_UNIQUE_ID),
    (re.compile(r'000400000F7D4000'), '000400000F7D4100'),
    (re.compile(re.escape(SS_PRODUCT)), HG_PRODUCT),
]
# pspoke's ROM header check (ss_rom_header.c): US HeartGold is IPKE, US SoulSilver is IPGE.
ROMHEADER = [
    (re.compile(r'"IPGE"'), '"IPKE"'),
    (re.compile(r'actual IPGE header'), 'actual IPKE header'),
    (re.compile(r'SoulSilver ROM header'), 'HeartGold ROM header'),
]
FORBIDDEN = [b'soulsilver3ds', b'soulsilver.sav', b'soulsilver.nds', SS_PRODUCT.encode()]
TEXT_EXT = {'.c', '.h', '.cpp', '.hpp', '.inc', '.py', '.sh', '.json', '.txt', '.patch', '.pica', '.rsf', '.mk', '.s', '.S', '.md', ''}


def filt(text, name):
    rules = list(PATHS) + list(WORDS)
    if name == 'Makefile' or name.endswith('.mk'):
        rules += MAKEFILE
    if name.endswith('.rsf'):
        rules += RSF
    if name == 'ss_rom_header.c':
        rules += ROMHEADER
    for rx, to in rules:
        text = rx.sub(to, text)
    return text


def read_text(p):
    if p.suffix not in TEXT_EXT:
        return None
    try:
        return p.read_text(encoding='utf-8')
    except (UnicodeDecodeError, OSError):
        return None


def write_if_changed(p, data):
    """data: str or bytes. Keeps the old file (and its time stamp) when nothing changed."""
    raw = data.encode('utf-8') if isinstance(data, str) else data
    if p.exists() and p.read_bytes() == raw:
        return False
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_bytes(raw)
    return True


def cmd_tree(root):
    """pspoke-hg: only the SD paths and the ROM header check. Everything else in pspoke is shared as is."""
    n = 0
    for sub in ('port', 'scripts', 'patches'):
        for p in sorted((root / sub).rglob('*')):
            if not p.is_file() or '.git' in p.parts:
                continue
            text = read_text(p)
            if text is None:
                continue
            new = text
            for rx, to in PATHS + (ROMHEADER if p.name == 'ss_rom_header.c' else []):
                new = rx.sub(to, new)
            if new != text:
                p.write_text(new, encoding='utf-8')
                n += 1
    hdr = root / 'port/soulsilver-native-core/nitromain-perf/ss_rom_header.c'
    if '"IPKE"' not in hdr.read_text():
        sys.exit('hg_rename: the ROM game code check was not found in %s (pspoke changed?); HeartGold would refuse its own ROM' % hdr)
    print('hg_rename: %d pspoke files now use the HeartGold names' % n)


def cmd_copy(src, dst):
    files = [src] if src.is_file() else [p for p in sorted(src.rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
    n = 0
    keep = set()
    for p in files:
        rel = Path(p.name) if src.is_file() else p.relative_to(src)
        out = dst if src.is_file() else dst / Path(filt(str(rel), ''))
        keep.add(out)
        text = read_text(p)
        n += write_if_changed(out, filt(text, out.name) if text is not None else p.read_bytes())
    if src.is_dir() and dst.is_dir():      # a file removed from the SoulSilver source goes from the copy too
        for p in sorted(dst.rglob('*')):
            if p.is_file() and p not in keep:
                p.unlink()
                n += 1
    if dst.suffix == '.rsf':
        t = dst.read_text()
        if HG_UNIQUE_ID not in t or re.search(re.escape(SS_UNIQUE_ID) + r'\b', t) or HG_PRODUCT not in t or SS_PRODUCT in t:
            dst.unlink()
            sys.exit('hg_rename: could not give HeartGold its own HOME Menu title in %s; stopped so it can never install over SoulSilver' % dst)
    print('hg_rename: %s -> %s (%d changed)' % (src, dst, n))


def cmd_check(paths):
    bad = 0
    for p in paths:
        data = Path(p).read_bytes()
        for word in FORBIDDEN:
            if word in data:
                at = data.index(word)
                print('hg_rename: %s still contains "%s" (near: %r)' % (p, word.decode(), data[max(0, at - 40):at + 40]), file=sys.stderr)
                bad += 1
    if bad:
        sys.exit('hg_rename: STOPPED. A HeartGold build must not name SoulSilver files; nothing was installed.')
    print('hg_rename: check ok (%d file(s) name no SoulSilver path or title)' % len(paths))


def main(a):
    if len(a) == 2 and a[0] == 'tree':
        cmd_tree(Path(a[1]))
    elif len(a) == 3 and a[0] == 'copy':
        cmd_copy(Path(a[1]), Path(a[2]))
    elif len(a) >= 2 and a[0] == 'check':
        cmd_check(a[1:])
    else:
        sys.exit(__doc__)


if __name__ == '__main__':
    main(sys.argv[1:])
