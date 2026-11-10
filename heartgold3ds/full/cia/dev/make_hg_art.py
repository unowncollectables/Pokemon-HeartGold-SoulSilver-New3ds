#!/usr/bin/env python3
"""Remakes heartgold3ds/full/cia/art/icon.png (48x48) and banner.png (256x128): original art, no game assets.
Gold lettering on a warm dark backdrop, the twin of SoulSilver's silver-blue art. Needs Pillow and the Inter fonts.

    python3 heartgold3ds/full/cia/dev/make_hg_art.py heartgold3ds/full/cia/art
"""
import sys
from PIL import Image, ImageDraw, ImageFont, ImageFilter, ImageChops

S = 4   # drawn 4x, scaled down
BOLD = "/usr/share/fonts/opentype/inter/Inter-ExtraBold.otf"
SEMI = "/usr/share/fonts/opentype/inter/Inter-Bold.otf"


def backdrop(w, h):
    im = Image.new("RGB", (w, h))
    px = im.load()
    cx, cy = w / 2, h * 0.52
    for y in range(h):
        for x in range(w):
            d = min(1.0, (((x - cx) / (w * 0.62)) ** 2 + ((y - cy) / (h * 0.75)) ** 2) ** 0.5)
            t = (1 - d) ** 1.6
            px[x, y] = (int(22 + 105 * t), int(8 + 34 * t), int(6 + 10 * t))
    return im


def gold_text(base, text, font, cy, spacing=0):
    """Vertical gold gradient lettering with a dark outline and a soft glow, centred on cy."""
    w, h = base.size
    d = ImageDraw.Draw(base)
    widths = [d.textlength(c, font=font) for c in text]
    total = sum(widths) + spacing * (len(text) - 1)
    mask = Image.new("L", (w, h), 0)
    md = ImageDraw.Draw(mask)
    x = (w - total) / 2
    for c, cw in zip(text, widths):
        md.text((x, cy), c, font=font, fill=255, anchor="lm")
        x += cw + spacing
    box = mask.getbbox()
    grad = Image.new("RGB", (w, h))
    gp = grad.load()
    for y in range(h):
        t = 0 if not box else min(1, max(0, (y - box[1]) / max(1, box[3] - box[1])))
        col = (255, int(246 - 96 * t), int(190 - 160 * t)) if t < 0.55 else (int(255 - 60 * (t - 0.55) / 0.45), int(193 - 70 * (t - 0.55) / 0.45), int(40 - 20 * (t - 0.55) / 0.45))
        for xx in range(w):
            gp[xx, y] = col
    glow = mask.filter(ImageFilter.GaussianBlur(6 * S // 4))
    base.paste(Image.new("RGB", (w, h), (255, 150, 40)), mask=glow.point(lambda v: v // 3))
    outline = mask.filter(ImageFilter.MaxFilter(2 * (S // 2) + 3))
    base.paste(Image.new("RGB", (w, h), (60, 20, 6)), mask=ImageChops.subtract(outline, mask))
    base.paste(grad, mask=mask)


def banner(path):
    w, h = 256 * S, 128 * S
    im = backdrop(w, h)
    gold_text(im, "POKÉMON", ImageFont.truetype(SEMI, 13 * S), 26 * S, spacing=3 * S)
    gold_text(im, "HEARTGOLD", ImageFont.truetype(BOLD, 34 * S), 58 * S, spacing=1 * S)
    d = ImageDraw.Draw(im)
    c = (w // 2, 80 * S)
    d.polygon([(c[0], c[1] - 3 * S), (c[0] + 3 * S, c[1]), (c[0], c[1] + 3 * S), (c[0] - 3 * S, c[1])], fill=(255, 205, 80))
    f = ImageFont.truetype(SEMI, 9 * S)
    text = "NEW 3DS EDITION"
    sp = 3 * S
    total = sum(d.textlength(ch, font=f) for ch in text) + sp * (len(text) - 1)
    x = (w - total) / 2
    for ch in text:
        d.text((x, 92 * S), ch, font=f, fill=(250, 232, 190), anchor="lm")
        x += d.textlength(ch, font=f) + sp
    im.resize((256, 128), Image.LANCZOS).save(path, optimize=True)


def icon(path):
    w = 48 * S
    im = backdrop(w, w)
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([S, S, w - S - 1, w - S - 1], radius=7 * S, outline=(205, 140, 40), width=S)
    gold_text(im, "HG", ImageFont.truetype(BOLD, 25 * S), 22 * S)
    c = (w // 2, 38 * S)
    d.polygon([(c[0], c[1] - 3 * S), (c[0] + 3 * S, c[1]), (c[0], c[1] + 3 * S), (c[0] - 3 * S, c[1])], fill=(255, 205, 80))
    im.resize((48, 48), Image.LANCZOS).save(path, optimize=True)


if __name__ == "__main__":
    out = sys.argv[1]
    banner(out + "/banner.png")
    icon(out + "/icon.png")
