/* Host check for the RGB8 in-between clear.
   A 24-bit GX memory fill repeats the first three little-endian bytes of the
   fill word. GPU_RGB8 texels are B,G,R (Azahar DecodeRGB8). The compositor's
   backdrop word is R<<24|G<<16|B<<8|A. Passing that word straight through
   stores A,B,G, so an opaque backdrop is sampled as blue. */
#include <stdio.h>

static unsigned sample_rgb8(unsigned fill) {
    unsigned b0 = fill & 255u, b1 = (fill >> 8) & 255u, b2 = (fill >> 16) & 255u;
    return (b2 << 16) | (b1 << 8) | b0; /* R, G, B */
}

static unsigned backdrop(unsigned r, unsigned g, unsigned b) {
    return (r << 24) | (g << 16) | (b << 8) | 0xFFu;
}

static int expect(const char *name, unsigned got, unsigned want) {
    if (got == want) return 0;
    fprintf(stderr, "%s: got %06x want %06x\n", name, got, want);
    return 1;
}

int main(void) {
    int bad = 0;
    /* The bug: opaque black (the usual indoor backdrop) fills as pure blue. */
    bad += expect("bug black", sample_rgb8(backdrop(0, 0, 0)), 0x0000FFu);
    bad += expect("bug red", sample_rgb8(backdrop(0x11, 0x22, 0x33)), 0x2233FFu);
    /* The fix: shift the RGBA word down one byte before the 24-bit fill. */
    static const unsigned cols[][3] = {
        {0, 0, 0}, {255, 255, 255}, {0x11, 0x22, 0x33}, {8, 16, 200}, {240, 10, 20},
    };
    for (unsigned i = 0; i < sizeof cols / sizeof cols[0]; i++) {
        unsigned r = cols[i][0], g = cols[i][1], b = cols[i][2];
        unsigned got = sample_rgb8(backdrop(r, g, b) >> 8);
        bad += expect("fix", got, (r << 16) | (g << 8) | b);
    }
    /* RGBA8 targets keep the original word. Shifting them would be wrong. */
    bad += expect("rgba word", backdrop(1, 2, 3), 0x010203FFu);
    if (bad) return 1;
    puts("rgb8 clear packing ok");
    return 0;
}
