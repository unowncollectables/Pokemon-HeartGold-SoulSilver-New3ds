"""Fit a title-screen model into the Platinum banner's structure (HeartGold's Ho-Oh). Standard library only.

SoulSilver's Lugia already has that shape (3 materials, 2 textures, few joints) and does not use this. Ho-Oh has 15
materials, 14 small textures and 32 joints, far past what the banner checks allow (to_cgfx.py: 5 materials, 4 textures,
11 meshes, 44 bones), so before it goes in the scene:

  prepare()  joints that never move against each other during the wing beat become one joint (32 -> 13 for Ho-Oh), and
             the thin parts drawn without back-face culling (wings, tail) get their back faces as real triangles;
  finish()   all textures are packed into one picture (an atlas) used by one alpha-tested material. A part whose texture
             repeats gets the repeats drawn into its atlas cell, and every cell has a border of its own wrapped texels,
             so nothing bleeds in from the neighbouring cell.

scene.build() calls prepare(), cuts the triangles to the budget, then calls finish().
"""
import math

import linalg

GUTTER = 2          # texels of wrapped border around each atlas cell
# The atlas: the banner checks (cgfx_fingerprint.py) allow about 82,000 texels in all, which is what the Platinum banner
# has; the portal and logo cards use 40,960, so the model gets one 256 x 128 picture.
ATLAS_W, ATLAS_H = 256, 128
ATLAS_NAME = "atlas"


def prepare(model, inst, tris, frames, flap_frames, two_sided):
    """-> (inst, tris, frames) with rigidly-linked joints merged and back faces added for the `two_sided` materials."""
    beat = frames[:flap_frames] or [inst]
    n = len(inst)
    lead, rel = list(range(n)), [None] * n
    leaders = []
    for j in range(n):
        for a in leaders:
            r0 = linalg.mul(linalg.inverse(beat[0][a]), beat[0][j])
            if all(linalg.close(linalg.mul(linalg.inverse(f[a]), f[j]), r0, atol=2e-3) for f in beat):
                lead[j], rel[j] = a, r0
                break
        else:
            leaders.append(j)
    new = {a: i for i, a in enumerate(leaders)}

    def move(v):
        j = v[0]
        if rel[j] is None:
            return (new[j],) + tuple(v[1:])
        pos = linalg.point(rel[j], v[1])
        nrm = linalg.normalize(linalg.apply(linalg.upper3(rel[j]), list(v[4])))
        return (new[lead[j]], pos, v[2], v[3], nrm)

    names = [m["name"] for m in model["materials"]]
    out = []
    for mat, t in tris:
        t = [move(v) for v in t]
        out.append((mat, t))
        if names[mat] in two_sided:
            flip = lambda v: (v[0], v[1], v[2], v[3], [-x for x in v[4]])
            out.append((mat, [flip(t[0]), flip(t[2]), flip(t[1])]))
    return [inst[a] for a in leaders], out, [[f[a] for a in leaders] for f in frames]


def _pack(cells):
    """Shelf packing, tallest first, into ATLAS_W x ATLAS_H. cells: {key: (w, h)} -> {key: (x, y)}, or None if too big."""
    pos, x, y, shelf = {}, 0, 0, 0
    for k in sorted(cells, key=lambda k: (-cells[k][1], -cells[k][0], str(k))):
        w, h = cells[k]
        if w > ATLAS_W:
            return None
        if x + w > ATLAS_W:
            x, y, shelf = 0, y + shelf, 0
        pos[k] = (x, y)
        x += w
        shelf = max(shelf, h)
    return pos if y + shelf <= ATLAS_H else None


def _halve(w, h, rgba, across):
    """The picture at half width (across) or half height: opaque texels averaged, cut-outs kept sharp."""
    nw, nh = (w // 2, h) if across else (w, h // 2)
    out = bytearray(nw * nh * 4)
    for y in range(nh):
        for x in range(nw):
            a = (y * w + 2 * x) * 4 if across else (2 * y * w + x) * 4
            b = a + 4 if across else a + w * 4
            pa, pb = rgba[a:a + 4], rgba[b:b + 4]
            if pa[3] >= 128 and pb[3] >= 128:
                px = bytes(((pa[i] + pb[i] + 1) // 2 for i in range(3))) + b"\xff"
            elif pa[3] >= 128 or pb[3] >= 128:
                px = bytes((pa if pa[3] >= 128 else pb)[:3]) + b"\xff"
            else:
                px = bytes(pa[:3]) + b"\x00"
            out[(y * nw + x) * 4:(y * nw + x) * 4 + 4] = px
    return nw, nh, bytes(out)


def finish(model, tris, images, keep):
    """-> (model, tris, images, keep) drawing every kept material from one atlas texture with one material."""
    mats = model["materials"]
    eps = 1e-6
    shifted, need = [], {}
    for mat, t in tris:
        m = mats[mat]
        if m["name"] not in keep or not m["texture"]:
            raise ValueError(f"material {m['name']} has no texture to pack")
        su = math.floor(min(v[2][0] for v in t) + eps)
        sv = math.floor(min(v[2][1] for v in t) + eps)
        uv = [(v[2][0] - su, v[2][1] - sv) for v in t]
        nu = max(1, math.ceil(max(u for u, _ in uv) - eps))
        nv = max(1, math.ceil(max(v for _, v in uv) - eps))
        tex = m["texture"]
        need[tex] = (max(need.get(tex, (1, 1))[0], nu), max(need.get(tex, (1, 1))[1], nv))
        shifted.append((tex, t, uv))
    images = dict(images)
    while True:
        cells = {tex: (images[tex][0] * nu + 2 * GUTTER, images[tex][1] * nv + 2 * GUTTER) for tex, (nu, nv) in need.items()}
        pos = _pack(cells)
        if pos:
            break
        # too big for the atlas: the cell with the most texels loses half of its longer side, until everything fits
        big = [k for k in sorted(cells, key=lambda k: (-cells[k][0] * cells[k][1], str(k))) if max(images[k][:2]) > 4]
        if not big:
            raise ValueError(f"the model's textures do not fit in one {ATLAS_W} x {ATLAS_H} picture")
        w, h, rgba = images[big[0]]
        images[big[0]] = _halve(w, h, rgba, across=cells[big[0]][0] >= cells[big[0]][1])
    aw, ah = ATLAS_W, ATLAS_H
    atlas = bytearray(b"\x80\x80\x80\xff" * (aw * ah))
    for tex, (cw, ch) in cells.items():
        w, h, rgba = images[tex]
        x0, y0 = pos[tex]
        for y in range(ch):
            sy = (y - GUTTER) % h
            row = bytearray()
            for x in range(cw):
                sx = (x - GUTTER) % w
                row += rgba[(sy * w + sx) * 4:(sy * w + sx) * 4 + 4]
            o = ((y0 + y) * aw + x0) * 4
            atlas[o:o + len(row)] = row
    out = []
    for tex, t, uv in shifted:
        w, h, _ = images[tex]
        x0, y0 = pos[tex]
        out.append((0, [(v[0], v[1], ((x0 + GUTTER + u * w) / aw, (y0 + GUTTER + s * h) / ah), v[3], v[4])
                        for v, (u, s) in zip(t, uv)]))
    name = "packed"
    return {"materials": [{"name": name, "texture": ATLAS_NAME}]}, out, {ATLAS_NAME: (aw, ah, bytes(atlas))}, (name,)
