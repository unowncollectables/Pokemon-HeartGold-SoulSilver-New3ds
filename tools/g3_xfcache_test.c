/* Host check for g3_xfcache hit rules and sticky RGB565 scale-target fallback.
   Mirrors the predicates in g3soft.cpp / gpuscale.c without the 3DS headers. */
#include <stdio.h>
#include <string.h>

/* xfcache: same key + n + gen + cullFace + identical M => hit */
typedef struct { const void *key; unsigned n, gen, cullFace; float M[16]; int valid; } Slot;
static int xfHit(const Slot *s, const void *key, unsigned n, unsigned gen, unsigned cullFace, const float *M) {
    if (!s->valid || s->key != key || s->n != n || s->gen != gen || s->cullFace != cullFace) return 0;
    return memcmp(s->M, M, 16 * sizeof(float)) == 0;
}
static unsigned xfGen(int stereoSd, int mid, int wide, int cullFace) {
    return ((unsigned)(stereoSd & 255)) | ((mid ? 1u : 0u) << 8) | ((wide ? 1u : 0u) << 9) | ((unsigned)(cullFace & 3) << 10);
}

/* sticky RGB565: after RGBA8 fail, UpAlloc(0) becomes fmt16=1 until free > 3 MiB */
static int stickFmt(int want, int failed, unsigned long freeAfterUpFree) {
    if (!want && failed) {
        if (freeAfterUpFree > 3u * 1024u * 1024u) return 0; /* retry RGBA8 */
        return 1; /* stick RGB565 */
    }
    return want;
}

int main(void) {
    int bad = 0;
    float M[16] = {0}; M[0] = M[5] = M[10] = M[15] = 1.0f;
    float M2[16]; memcpy(M2, M, sizeof M); M2[12] = 1.0f;
    const char verts[] = "replay";
    Slot s = {0}; s.key = verts; s.n = 9; s.gen = xfGen(4, 1, 1, 2); s.cullFace = 2; memcpy(s.M, M, sizeof M); s.valid = 1;
    if (!xfHit(&s, verts, 9, xfGen(4, 1, 1, 2), 2, M)) bad++;
    if (xfHit(&s, verts, 9, xfGen(4, 1, 1, 2), 2, M2)) bad++;          /* matrix moved */
    if (xfHit(&s, verts, 9, xfGen(0, 1, 1, 2), 2, M)) bad++;           /* stereo changed */
    if (xfHit(&s, verts, 12, xfGen(4, 1, 1, 2), 2, M)) bad++;          /* count changed */
    if (xfHit(&s, verts + 1, 9, xfGen(4, 1, 1, 2), 2, M)) bad++;       /* different list */
    if (xfGen(4, 0, 1, 0) == xfGen(4, 1, 1, 0)) bad++;                 /* mid bit must differ */

    if (stickFmt(0, 1, 839168ul) != 1) bad++;   /* run 870 free after fail */
    if (stickFmt(0, 1, 4000000ul) != 0) bad++;  /* room again -> retry RGBA8 */
    if (stickFmt(0, 0, 839168ul) != 0) bad++;   /* never failed: still try RGBA8 */
    if (stickFmt(1, 1, 839168ul) != 1) bad++;   /* explicit RGB565 unchanged */

    if (bad) { fprintf(stderr, "g3_xfcache bad %d\n", bad); return 1; }
    puts("g3_xfcache ok");
    return 0;
}
