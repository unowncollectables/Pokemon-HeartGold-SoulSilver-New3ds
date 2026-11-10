#!/usr/bin/env python3
"""Remakes art-hg/logo.png and art-hg/portal.png, the HeartGold banner's own art (committed; not needed to build; needs
numpy and Pillow). It is the SoulSilver art of dev/make_logo.py and dev/make_art.py drawn again with the title
"HEARTGOLD" and the colours mirrored (red and blue swapped), which turns silver and steel blue into gold and copper.
No game or third-party art. sparkle.png is a copy of SoulSilver's (art/).

    python3 tools/banner3d/dev/make_hg_art.py tools/banner3d/art-hg [LOGO_PREVIEW.png]
"""
import os
import sys

from PIL import Image

sys.path.insert(0, os.path.dirname(__file__))
import make_art
import make_logo


def warm(im):
    r, g, b, a = im.convert("RGBA").split()
    return Image.merge("RGBA", (b, g, r, a))


def main(dst, preview=None):
    os.makedirs(dst, exist_ok=True)
    text, dither = make_logo.text_mask, make_logo.dither
    make_logo.text_mask = lambda txt, *a, **k: text("HEARTGOLD" if txt == "SOULSILVER" else txt, *a, **k)
    make_logo.dither = lambda im: dither(warm(im))
    make_logo.main(os.path.join(dst, "logo.png"), None)
    if preview:
        out = Image.open(os.path.join(dst, "logo.png")).convert("RGBA")
        bg = Image.new("RGBA", out.size, (60, 40, 40, 255)); bg.alpha_composite(out)
        bg.resize((out.width * 3, out.height * 3), Image.NEAREST).save(preview)
    p = os.path.join(dst, "portal.png")
    make_art.portal(p)
    warm(Image.open(p)).save(p)


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else None)
