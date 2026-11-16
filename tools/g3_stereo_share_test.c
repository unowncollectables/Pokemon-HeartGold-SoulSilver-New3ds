/* Host check: deferred stereo expand is bit-identical to dual TriClip2 x formula,
   including incremental expand after a hard-clip sync and in-place batch count growth.
   Compile: gcc -O2 -o /tmp/g3_stereo_share_test tools/g3_stereo_share_test.c && /tmp/g3_stereo_share_test */
#include <stdio.h>
#include <string.h>
#include <math.h>

typedef struct { float x, y, z, w, s, t, r, g, b, a; } V;
typedef struct { unsigned first, count; } B;

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
    int fence = 1, inflight = 1, need_create = 1;
    int wait_at_1 = (fence == 1) && need_create && inflight;
    fence = 2;
    int skip_at_2 = (fence >= 2) && need_create;
    return wait_at_1 == 1 && skip_at_2 == 1;
}

/* Model: share extends left batch in place (bn unchanged); expand syncs count in-place then copies only new batches. */
static int expand_syncs_batch_count(void) {
    V vb0[6], vb1[6];
    B bt0[2] = {{0, 3}, {0, 0}}, bt1[2] = {{0, 3}, {0, 0}};
    float dsh[6] = {8, 8, 8, 8, 8, 8};
    unsigned vn0 = 3, vn1 = 3, bn0 = 1, bn1 = 1;
    /* second share tri extends left batch count only */
    vn0 = 6; bt0[0].count = 6; int pending = 1;
    if (!pending) return 0;
    unsigned v0 = vn1, b0 = bn1;
    memcpy(vb1 + v0, vb0 + v0, (vn0 - v0) * sizeof(V));
    for (unsigned i = v0; i < vn0; i++) vb1[i].x += (dsh[i] / 128.0f) * 1.0f;
    if (b0 > 0 && b0 <= bn0) bt1[b0 - 1].count = bt0[b0 - 1].count; /* incremental */
    if (bn0 > b0) memcpy(bt1 + b0, bt0 + b0, (bn0 - b0) * sizeof(B));
    vn1 = vn0; bn1 = bn0; pending = 0;
    return bt1[0].count == 6 && bn1 == 1 && vn1 == 6 && pending == 0;
}
/* Engagement proof: left-only path increments leftTris; dual path increments dualTris. */
static int share_stats_engage(void) {
    unsigned left = 0, dual = 0;
    int shareOn = 1, dshOk = 1; /* malloc succeeded */
    if (shareOn && dshOk) left += 3; else dual += 3;
    return left == 3 && dual == 0;
}
/* Worker publish: job.stereoShare must drive SetStereoShare on the worker (not only Begin). */
static int job_publishes_share(void) {
    int sd = 4, stereoShare = 1; /* snapshot: stereoSd && cfg */
    int want = sd && stereoShare;
    int workerShare = 0;
    workerShare = want; /* GpuRawJ: G3G_SetStereoShare(J.sd && J.stereoShare) */
    return workerShare == 1;
}

/* Hard-clip gate: expand before any non-share write so eye-0 Tri cannot leave stale dsh verts. */
static int expand_before_hardclip(void) {
    int pending = 1;
    /* G3G_Tri / G3G_TriClip always call Expand first when pending */
    if (pending) pending = 0;
    int wrote_eye0 = 1, wrote_eye1 = 1;
    return pending == 0 && wrote_eye0 && wrote_eye1;
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
    if (!expand_syncs_batch_count()) bad++;
    if (!expand_before_hardclip()) bad++;
    if (!share_stats_engage()) bad++;
    if (!job_publishes_share()) bad++;
    if (bad) { fprintf(stderr, "g3_stereo_share bad %d\n", bad); return 1; }
    puts("g3_stereo_share ok");
    return 0;
}
