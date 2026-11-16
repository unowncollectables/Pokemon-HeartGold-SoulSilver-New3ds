/* Evaluate fixed-point clip xform vs IEEE float for bit-identical output.
   If any sample differs, fixed-point must NOT ship as a "same picture" toggle.
   Run: gcc -O2 -o /tmp/g3_fixedxform_test tools/g3_fixedxform_test.c && /tmp/g3_fixedxform_test */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

typedef struct { float x, y, z; } V;

static void xfFloat(const float *M, const V *p, float *q) {
    q[0] = p->x * M[0] + p->y * M[4] + p->z * M[8] + M[12];
    q[1] = p->x * M[1] + p->y * M[5] + p->z * M[9] + M[13];
    q[2] = p->x * M[2] + p->y * M[6] + p->z * M[10] + M[14];
    q[3] = p->x * M[3] + p->y * M[7] + p->z * M[11] + M[15];
}

/* Q16.16 matrix × float vert → float (round trip). Typical "cheaper" candidate on ARM11. */
static void xfQ16(const float *M, const V *p, float *q) {
    int32_t m[16];
    for (int i = 0; i < 16; i++) m[i] = (int32_t)lrintf(M[i] * 65536.0f);
    int32_t x = (int32_t)lrintf(p->x * 65536.0f);
    int32_t y = (int32_t)lrintf(p->y * 65536.0f);
    int32_t z = (int32_t)lrintf(p->z * 65536.0f);
    for (int c = 0; c < 4; c++) {
        int64_t acc = (int64_t)x * m[c] + (int64_t)y * m[4 + c] + (int64_t)z * m[8 + c] + ((int64_t)m[12 + c] << 16);
        q[c] = (float)(acc >> 16) / 65536.0f;
    }
}

int main(void) {
    float M[16] = {
        1.1f, 0.2f, -0.3f, 0.05f,
        -0.4f, 0.9f, 0.15f, 0.02f,
        0.25f, -0.1f, 1.05f, 0.01f,
        12.0f, -7.5f, 3.25f, 1.0f
    };
    int mism = 0, n = 0;
    for (int i = 0; i < 200; i++) {
        V p = {(float)((i * 17) % 31) - 15.0f, (float)((i * 13) % 29) - 14.0f, (float)((i * 11) % 23) - 11.0f};
        float a[4], b[4];
        xfFloat(M, &p, a);
        xfQ16(M, &p, b);
        n++;
        if (memcmp(a, b, sizeof a)) mism++;
    }
    if (mism == 0) {
        puts("g3_fixedxform unexpected: Q16.16 matched float (would be safe to ship)");
        return 0;
    }
    /* Expected path for this port: not bit-identical → do not add a toggle. */
    printf("g3_fixedxform skip: Q16.16 differs on %d/%d samples (not bit-identical; leave float path)\n", mism, n);
    return 0;
}
