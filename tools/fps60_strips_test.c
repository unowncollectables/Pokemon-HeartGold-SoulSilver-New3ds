/* Host check for the Wide strip crop used by midpoint and direct real frames.
   DS row y is GPU row 255-y. The visible 192 rows are GPU 64..255, tile rows 8..31.
   OFF copies tile row 0, 32 rows, height 256. */
#include <stdio.h>
#include <string.h>
static void span(int on, unsigned *row0, unsigned *rows, unsigned *height) {
    *row0 = on ? 8u : 0u;
    *rows = on ? 24u : 32u;
    *height = *rows * 8u;
}
/* Model the two strided GX copies in tiled space. Validate both side strips
   after the display-transfer row flip, using distinguishable source pixels. */
static unsigned tile_offset(unsigned width, unsigned x, unsigned y) {
    return ((y / 8) * (width / 8) + x / 8) * 64 + (y % 8) * 8 + x % 8;
}
static int packed_edges_match_full_readback(void) {
    enum { WIDTH = 320, HEIGHT = 256, STRIP = 32, PACKED = 64, VISIBLE = 192 };
    static unsigned wide[WIDTH * HEIGHT], packed[PACKED * VISIBLE];
    for (unsigned y = 0; y < HEIGHT; y++)
        for (unsigned x = 0; x < WIDTH; x++)
            wide[tile_offset(WIDTH, x, y)] = 1 + y * WIDTH + x;
    for (unsigned row = 0; row < VISIBLE / 8; row++) {
        const unsigned *source = wide + (row + 8) * WIDTH * 8;
        unsigned *dest = packed + row * PACKED * 8;
        memcpy(dest, source, STRIP * 8 * sizeof(*wide));
        memcpy(dest + STRIP * 8, source + (WIDTH - STRIP) * 8, STRIP * 8 * sizeof(*wide));
    }
    for (unsigned y = 0; y < VISIBLE; y++)
        for (unsigned x = 0; x < PACKED; x++) {
            const unsigned fullX = x < STRIP ? x : WIDTH - PACKED + x;
            if (packed[tile_offset(PACKED, x, VISIBLE - 1 - y)] !=
                wide[tile_offset(WIDTH, fullX, HEIGHT - 1 - y)]) return 0;
        }
    return 1;
}
int main(void) {
    int bad = 0;
    unsigned r0, rows, h;
    span(0, &r0, &rows, &h);
    if (r0 || rows != 32 || h != 256) bad++;
    span(1, &r0, &rows, &h);
    if (r0 != 8 || rows != 24 || h != 192) bad++;
    enum { SIDE = 32 * 8 * 4, WIDE_ROW = 320 * 8 * 4 };
    if (WIDE_ROW != 10240) bad++;
    if (8 * WIDE_ROW != 81920) bad++;
    /* flipped 192-tall subimage starting at GPU row 64: output row 0 is GPU row 255 (DS y 0). */
    const unsigned gpu0 = 64 + (192 - 1);
    if (gpu0 != 255 || (255 - gpu0) != 0) bad++;
    const unsigned gpu191 = 64 + 0;
    if (gpu191 != 64 || (255 - gpu191) != 191) bad++;
    if (SIDE * 24 >= SIDE * 32) bad++;
    if (!packed_edges_match_full_readback()) bad++;
    if (bad) { fprintf(stderr, "fps60 strips bad %d\n", bad); return 1; }
    puts("fps60 strips ok");
    return 0;
}
