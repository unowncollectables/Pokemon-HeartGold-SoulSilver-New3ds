/* Schedule check for fps60_pipe. Not a GPU timing run.
   Legacy wide path: each C3D_Sync* breaks the 3D command list.
   In-between, strip pack on: centre + two side packs + readback = 4.
   In-between, no strip pack: centre + one full readback = 2.
   Real picture: centre + readback = 2.
   Pipe path: one break after both in-between scenes, one after both real scenes. */
#include <stdio.h>

static int legacy_kicks(int eyes, int strip_pack) {
    const int mid = strip_pack ? 4 : 2;
    return eyes * (mid + 2);
}

enum { FILL, DRAW, SPLIT, DMA };

static int before(const int *op, const int *eye, int n, int want, int wantEye, int after, int afterEye) {
    int seenAfter = 0;
    for (int i = 0; i < n; i++) {
        if (op[i] == after && eye[i] == afterEye) seenAfter = 1;
        if (op[i] == want && eye[i] == wantEye) return seenAfter;
    }
    return 0;
}

/* Pipe queue for two eyes. Fills of a target are before its draw, the in-between
   DMA is before that target is cleared for the real scene, and the real DMA is last. */
static int pipe_order_ok(void) {
    const int op[] = {
        FILL, DRAW, FILL, DRAW, SPLIT, DMA, DMA, FILL, DRAW, FILL, DRAW, SPLIT, DMA, DMA
    };
    const int eye[] = {
        0, 0, 1, 1, -1, 0, 1, 0, 0, 1, 1, -1, 0, 1
    };
    const int n = (int)(sizeof op / sizeof op[0]);
    int bad = 0;
    for (int e = 0; e < 2; e++) {
        bad += !before(op, eye, n, DRAW, e, FILL, e);
        /* First DMA of this eye is the in-between copy; the real FILL is the second FILL. */
        int fills = 0, dma = 0, drawReal = 0, dmaAfterReal = 0, midDma = 0;
        for (int i = 0; i < n; i++) {
            if (eye[i] != e) continue;
            if (op[i] == FILL) fills++;
            if (op[i] == DRAW && fills == 2) drawReal = 1;
            if (op[i] == DMA) {
                dma++;
                if (!drawReal) {
                    if (fills != 1) bad++; /* in-between copy sits between the two clears */
                    midDma++;
                } else dmaAfterReal++;
            }
        }
        if (dma != 2 || midDma != 1 || dmaAfterReal != 1) bad++;
    }
    int splits = 0;
    for (int i = 0; i < n; i++) if (op[i] == SPLIT) splits++;
    if (splits != 2) bad++;
    return bad == 0;
}

int main(void) {
    int bad = 0;
    if (legacy_kicks(2, 1) != 12) bad++;
    if (legacy_kicks(1, 1) != 6) bad++;
    if (legacy_kicks(2, 0) != 8) bad++;
    if (legacy_kicks(1, 0) != 4) bad++;
    /* Centre extract starts 4 tiles in. A tile is 8x8 RGBA8 = 256 bytes. */
    if (4 * 256 != 4 * 8 * 8 * 4) bad++;
    if (8192 * 32 != 256 * 256 * 4) bad++;
    /* Two 32-pixel sides, 8 rows per tile, 256 rows: 64 * 256 * 4. */
    if (2 * (32 * 8 * 4) * (256 / 8) != 64 * 256 * 4) bad++;
    if (!pipe_order_ok()) bad++;
    if (bad) {
        fprintf(stderr, "fps60 pipe schedule failed (%d)\n", bad);
        return 1;
    }
    puts("fps60 pipe schedule ok");
    return 0;
}
