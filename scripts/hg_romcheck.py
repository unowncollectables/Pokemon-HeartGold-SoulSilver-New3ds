#!/usr/bin/env python3
"""hg_romcheck.py ROM : is this a US HeartGold ROM the HeartGold build can use? Exit 0 = yes.

The exact reference dump (heartgold_sha1 in upstream.lock) passes at once. Another US HeartGold file also passes when
every file in it (all game data and all code overlays, heartgold_files_sha1) is identical to the reference: such a ROM
differs only in the boot code (arm9: a re-dump or an anti-piracy fix), which this port does not use, since the game
code is compiled from the decompilation. Anything else is refused. The ROM is only read.
"""
import hashlib
import struct
import sys
from pathlib import Path


def files_digest(d):
    """One SHA1 over the SHA1 of every file in the ROM's file table, in order (data files and overlays)."""
    fat, size = struct.unpack_from('<II', d, 0x48)
    if fat + size > len(d) or size % 8:
        return None
    h = hashlib.sha1()
    for i in range(size // 8):
        a, b = struct.unpack_from('<II', d, fat + i * 8)
        if not (0 <= a <= b <= len(d)):
            return None
        h.update(hashlib.sha1(d[a:b]).digest())
    return h.hexdigest()


def check(path, lock):
    """(ok, message)"""
    want = dict(l.strip().split('=', 1) for l in Path(lock).read_text().splitlines() if '=' in l and not l.startswith('#'))
    d = Path(path).read_bytes()
    code = d[12:16].decode('ascii', 'replace') if len(d) >= 0x200 else ''
    if code != 'IPKE':
        return False, 'not Pokemon HeartGold (US): game code %r, wanted IPKE' % code
    whole = hashlib.sha1(d).hexdigest()
    if whole == want.get('heartgold_sha1'):
        return True, 'HeartGold (US), the exact reference dump'
    if files_digest(d) == want.get('heartgold_files_sha1'):
        return True, ('HeartGold (US): all game data and overlays match the reference; only the boot code differs '
                      '(SHA1 %s), which this build does not use' % whole)
    return False, 'US HeartGold, but its game data differs from the reference dump (SHA1 %s). Use a clean dump.' % whole


if __name__ == '__main__':
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    ok, msg = check(sys.argv[1], Path(__file__).resolve().parent.parent / 'upstream.lock')
    print(('[HG] ROM ok: ' if ok else 'error: ') + msg, file=sys.stdout if ok else sys.stderr)
    sys.exit(0 if ok else 1)
