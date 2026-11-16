/* Host check for the 60 fps VRAM set.
   Place 288000, 288000, 196608, 196608 into free holes, largest first.
   A miss restores every hole, so nothing already allocated is evicted and
   a failed attempt keeps none of the set. */
#include <stdio.h>
#include <string.h>
enum { N = 4 };
static const unsigned kSz[N] = { 288000u, 288000u, 196608u, 196608u };
static int pack(unsigned *holes, int n) {
    unsigned saved[8];
    if (n > 8) return 0;
    memcpy(saved, holes, (unsigned)n * sizeof holes[0]);
    for (int s = 0; s < N; s++) {
        int placed = 0;
        for (int i = 0; i < n; i++) if (holes[i] >= kSz[s]) { holes[i] -= kSz[s]; placed = 1; break; }
        if (!placed) { memcpy(holes, saved, (unsigned)n * sizeof holes[0]); return 0; }
    }
    return 1;
}
int main(void) {
    int bad = 0;
    unsigned fit[2] = { 497920u, 497920u };
    if (!pack(fit, 2) || fit[0] != 13312u || fit[1] != 13312u) bad++;
    unsigned tight[2] = { 409600u, 409600u };
    if (pack(tight, 2) || tight[0] != 409600u || tight[1] != 409600u) bad++;
    unsigned one[1] = { 995840u };
    if (!pack(one, 1) || one[0] != 995840u - (288000u * 2 + 196608u * 2)) bad++;
    unsigned uneven[2] = { 600000u, 395840u };
    if (!pack(uneven, 2)) bad++;
    unsigned shortb[2] = { 700000u, 200000u };
    if (pack(shortb, 2) || shortb[0] != 700000u || shortb[1] != 200000u) bad++;
    if (288000u * 2 + 196608u * 2 != 969216u) bad++;
    if (bad) { fprintf(stderr, "fps60 vram bad %d\n", bad); return 1; }
    puts("fps60 vram ok");
    return 0;
}
