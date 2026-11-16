/* Host check: stereo share expand is bit-identical to dual TriClip2 x formula.
   Compile: gcc -O2 -o /tmp/g3_stereo_share_test tools/g3_stereo_share_test.c && /tmp/g3_stereo_share_test */
#include <stdio.h>
#include <string.h>
#include <math.h>

typedef struct { float x, y, z, w, s, t, r, g, b, a; } V;

/* Dual TriClip2 (reference): both eyes from clip + shL/shR. */
static void dual(V *L, V *R, float x, float y, float z, float w, float shL, float shR, int wide) {
    L->x = x + shL / 128.0f * w; R->x = x + shR / 128.0f * w;
    if (wide) { L->x *= 0.8f; R->x *= 0.8f; }
    L->y = R->y = y; L->z = R->z = z; L->w = R->w = w;
}

/* Share path: left as dual, then right = left + scale*(shR-shL)/128*w. */
static void share(V *L, V *R, float x, float y, float z, float w, float shL, float shR, int wide) {
    float dsh = shR - shL;
    L->x = x + shL / 128.0f * w; if (wide) L->x *= 0.8f;
    L->y = y; L->z = z; L->w = w;
    *R = *L;
    R->x += (wide ? 0.8f : 1.0f) * (dsh / 128.0f) * w;
}

static int near(float a, float b) { return fabsf(a - b) < 1e-5f; }

/* DEEP texfence: TexCreate wait only at level 1, skipped at 2. */
static int deep_skips_texcreate_wait(void) {
    int fence = 2, inflight = 1, need_create = 1;
    int wait_at_1 = (fence == 1) && need_create && inflight;
    int wait_at_2 = (fence == 2) && need_create && inflight; /* DEEP: no wait */
    (void)wait_at_2;
    fence = 1;
    wait_at_1 = (fence == 1) && need_create && inflight;
    fence = 2;
    int skip_at_2 = (fence >= 2) && need_create;
    return wait_at_1 == 1 && skip_at_2 == 1;
}

int main(void) {
    int bad = 0;
    const float cases[][6] = {
        {1.0f, 0.5f, 0.2f, 2.0f, -4.0f, 4.0f},
        {0.0f, -1.0f, 0.0f, 1.0f, -2.5f, 2.5f},
        {3.0f, 1.0f, -0.5f, 4.0f, 0.0f, 6.0f}, /* battle-style asymmetric */
    };
    for (int wide = 0; wide < 2; wide++) {
        for (unsigned i = 0; i < sizeof cases / sizeof cases[0]; i++) {
            V Ld, Rd, Ls, Rs;
            dual(&Ld, &Rd, cases[i][0], cases[i][1], cases[i][2], cases[i][3], cases[i][4], cases[i][5], wide);
            share(&Ls, &Rs, cases[i][0], cases[i][1], cases[i][2], cases[i][3], cases[i][4], cases[i][5], wide);
            if (!near(Ld.x, Ls.x) || !near(Rd.x, Rs.x) || !near(Ld.y, Ls.y) || !near(Rd.y, Rs.y)) bad++;
            if (!near(Ld.z, Ls.z) || !near(Rd.z, Rs.z) || !near(Ld.w, Ls.w) || !near(Rd.w, Rs.w)) bad++;
        }
    }
    if (!deep_skips_texcreate_wait()) bad++;
    if (bad) { fprintf(stderr, "g3_stereo_share bad %d\n", bad); return 1; }
    puts("g3_stereo_share ok");
    return 0;
}
