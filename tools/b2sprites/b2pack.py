#!/usr/bin/env python3
"""Pack Black 2 / White 2 battle sprite animations into b2sprites.bin for the SoulSilver 3DS port.

Input: a folder of AnimaEngine exports made with ANIMA_RAW_DUMP=1 (see build_pack.sh), laid out as
  <in>/<key>/pokedexNNN_Name/{front,back}_idle_normal.raw + reconstruction_json/palettes.json
where <key> is "SSS_FF_G" (species, form, 0 male / 1 female). Output: one b2sprites.bin.
The pack holds data from the user's own ROM: it goes to their SD card only, never into a repository.

Format (little endian), version 1:
  header   'B2SP' u32 version u32 count u32 0
  dir      count x { u16 species, u8 form, u8 flags (bit0 back, bit1 female), u32 offset, u32 size, u32 0 }  sorted by key
  entry    u16 w, u16 h, s16 ox, s16 oy (world origin = feet point, inside the box), u16 nUnique, u16 nSeq,
           u16 delayCs, u16 0, u16 palNormal[16], u16 palShiny[16] (BGR555), u16 seq[nSeq], pad to 4,
           nUnique frames of ((w+1)//2)*h bytes, 4 bpp, low nibble = left pixel, index 0 = transparent.
"""
import json, os, struct, sys

def load_raw(path):
    d = open(path, 'rb').read()
    w, h, minx, miny, margin, n, delay, _ = struct.unpack_from('<8i', d)
    px = d[32:]
    if len(px) < w * h * n:
        raise ValueError('short raw ' + path)
    return w, h, minx, miny, margin, n, delay, [px[i * w * h:(i + 1) * w * h] for i in range(n)]

def pal_list(pj, name):
    for p in pj['palettes']:
        if p['name'] == name:
            c = [x['bgr555'] for x in sorted(p['colors'], key=lambda x: x['index'])][:16]
            return c + [0] * (16 - len(c))
    return None

def build_entry(raw, pj):
    w, h, minx, miny, margin, n, delay, frames = raw
    x0, y0, x1, y1 = w, h, -1, -1
    for f in frames:
        for y in range(h):
            row = f[y * w:(y + 1) * w]
            if any(row):
                if y < y0: y0 = y
                if y > y1: y1 = y
                l = next(i for i, v in enumerate(row) if v); r = w - 1 - next(i for i, v in enumerate(reversed(row)) if v)
                if l < x0: x0 = l
                if r > x1: x1 = r
    if x1 < 0:
        return None
    cw, ch = x1 - x0 + 1, y1 - y0 + 1
    ox, oy = (margin - minx) - x0, (margin - miny) - y0
    uniq, seq, blobs = {}, [], []
    for f in frames:
        b = bytearray(((cw + 1) // 2) * ch); bw = (cw + 1) // 2
        for y in range(ch):
            src = f[(y + y0) * w + x0:(y + y0) * w + x0 + cw]
            for x in range(cw):
                v = src[x] & 15
                if v:
                    b[y * bw + (x >> 1)] |= v << ((x & 1) * 4)
        b = bytes(b)
        if b not in uniq:
            uniq[b] = len(blobs); blobs.append(b)
        seq.append(uniq[b])
    pn, ps = pal_list(pj, 'normal'), pal_list(pj, 'shiny') or pal_list(pj, 'normal')
    out = struct.pack('<HHhhHHHH', cw, ch, ox, oy, len(blobs), len(seq), delay, 0)
    out += struct.pack('<16H', *pn) + struct.pack('<16H', *ps) + struct.pack('<%dH' % len(seq), *seq)
    out += b'\0' * (-len(out) % 4)
    return out + b''.join(blobs)

def main(indir, outpath):
    entries = []
    for key in sorted(os.listdir(indir)):
        try:
            sp, fo, fe = (int(v) for v in key.split('_'))
        except ValueError:
            continue
        sub = [d for d in os.listdir(os.path.join(indir, key)) if d.startswith('pokedex')]
        if not sub:
            continue
        base = os.path.join(indir, key, sub[0])
        pj = json.load(open(os.path.join(base, 'reconstruction_json', 'palettes.json')))
        for back, side in ((0, 'front'), (1, 'back')):
            p = os.path.join(base, side + '_idle_normal.raw')
            if not os.path.exists(p):
                continue
            e = build_entry(load_raw(p), pj)
            if e:
                entries.append(((sp, fo, back | (fe << 1)), e))
    # a female entry identical to the male one is dropped (the game falls back to the male key)
    m = {k: e for k, e in entries}
    entries = [(k, e) for k, e in entries if not (k[2] & 2 and m.get((k[0], k[1], k[2] & 1)) == e)]
    entries.sort(key=lambda t: t[0])
    hdr = struct.pack('<4sIII', b'B2SP', 1, len(entries), 0)
    off = len(hdr) + 16 * len(entries)
    dirb, body = b'', b''
    for (sp, fo, fl), e in entries:
        dirb += struct.pack('<HBBIII', sp, fo, fl, off + len(body), len(e), 0)
        body += e + b'\0' * (-len(e) % 4)
    open(outpath, 'wb').write(hdr + dirb + body)
    print('b2pack: %d sprites, %.1f MB -> %s' % (len(entries), (len(hdr) + len(dirb) + len(body)) / 1e6, outpath))

if __name__ == '__main__':
    if len(sys.argv) != 3:
        sys.exit('usage: b2pack.py <export folder> <b2sprites.bin>')
    main(sys.argv[1], sys.argv[2])
