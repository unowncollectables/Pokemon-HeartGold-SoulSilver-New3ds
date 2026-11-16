/* Host check for g3_fastxform: hoisted clip-matrix locals must be bit-identical
   to indexing M[] each multiply (same IEEE mul/add order). Also a tiny timing
   probe so Azahar/hardware A/B has a host baseline for "same picture". */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

typedef struct { float x, y, z; } V;

static void xfIndex(const float *M, const V *p, float *q) {
    q[0] = p->x * M[0] + p->y * M[4] + p->z * M[8] + M[12];
    q[1] = p->x * M[1] + p->y * M[5] + p->z * M[9] + M[13];
    q[2] = p->x * M[2] + p->y * M[6] + p->z * M[10] + M[14];
    q[3] = p->x * M[3] + p->y * M[7] + p->z * M[11] + M[15];
}

static void xfHoist(const float *M, const V *p, float *q) {
    const float m0 = M[0], m1 = M[1], m2 = M[2], m3 = M[3], m4 = M[4], m5 = M[5], m6 = M[6], m7 = M[7];
    const float m8 = M[8], m9 = M[9], m10 = M[10], m11 = M[11], m12 = M[12], m13 = M[13], m14 = M[14], m15 = M[15];
    q[0] = p->x * m0 + p->y * m4 + p->z * m8 + m12;
    q[1] = p->x * m1 + p->y * m5 + p->z * m9 + m13;
    q[2] = p->x * m2 + p->y * m6 + p->z * m10 + m14;
    q[3] = p->x * m3 + p->y * m7 + p->z * m11 + m15;
}

static uint64_t nsec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

int main(void) {
    int bad = 0;
    float M[16] = {
        1.1f, 0.2f, -0.3f, 0.05f,
        -0.4f, 0.9f, 0.15f, 0.02f,
        0.25f, -0.1f, 1.05f, 0.01f,
        12.0f, -7.5f, 3.25f, 1.0f
    };
    V verts[66];
    for (int i = 0; i < 66; i++) {
        verts[i].x = (float)((i * 17) % 31) - 15.0f;
        verts[i].y = (float)((i * 13) % 29) - 14.0f;
        verts[i].z = (float)((i * 11) % 23) - 11.0f;
    }

    for (int i = 0; i < 66; i++) {
        float a[4], b[4];
        xfIndex(M, &verts[i], a);
        xfHoist(M, &verts[i], b);
        if (memcmp(a, b, sizeof a)) bad++;
    }

    /* Warm + time both paths over the same verts (host only; 3DS VFP savings differ). */
    float sink = 0.0f;
    const int reps = 200000;
    uint64_t t0 = nsec();
    for (int r = 0; r < reps; r++)
        for (int i = 0; i < 66; i++) { float q[4]; xfIndex(M, &verts[i], q); sink += q[0] + q[3]; }
    uint64_t tIndex = nsec() - t0;
    t0 = nsec();
    for (int r = 0; r < reps; r++)
        for (int i = 0; i < 66; i++) { float q[4]; xfHoist(M, &verts[i], q); sink += q[0] + q[3]; }
    uint64_t tHoist = nsec() - t0;

    if (bad) {
        fprintf(stderr, "g3_fastxform bad %d (sink=%g)\n", bad, (double)sink);
        return 1;
    }
    printf("g3_fastxform ok bit-identical; host index %llu ns hoist %llu ns over %d*%d verts (sink=%g)\n",
           (unsigned long long)tIndex, (unsigned long long)tHoist, reps, 66, (double)sink);
    return 0;
}
