/* Host check for the in-between Wide strip crop.
   DS row y is GPU row 255-y. The visible 192 rows are GPU 64..255, tile rows 8..31.
   OFF copies tile row 0, 32 rows, height 256. */
#include <stdio.h>
static void span(int on, unsigned *row0, unsigned *rows, unsigned *height) {
    *row0 = on ? 8u : 0u;
    *rows = on ? 24u : 32u;
    *height = *rows * 8u;
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
    if (bad) { fprintf(stderr, "fps60 strips bad %d\n", bad); return 1; }
    puts("fps60 strips ok");
    return 0;
}
