/* Host check for g3soft PERF-gap helpers: empty FlushArray, RawMatrix fx32 skip,
   60-frame front = frame - idle - raw - flush - end, and g3_wake cadence. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

static int emptyFlush(const void *ptr, const void *base) { return ptr == base; }

static int rawSame(const int32_t *a, const int32_t *b) { return memcmp(a, b, 64) == 0; }

static unsigned frontUs(unsigned frame, unsigned idle, unsigned raw, unsigned flush, unsigned end) {
    unsigned known = idle + raw + flush + end;
    return frame > known ? frame - known : 0;
}

static unsigned wakeEvery(int g3_wake) { return g3_wake ? 1u : 6u; }

int main(void) {
    int bad = 0;
    char base[8], *ptr = base;
    if (!emptyFlush(ptr, base)) bad++;
    ptr++; if (emptyFlush(ptr, base)) bad++;

    int32_t M[16] = {0}, M2[16] = {0}; M[0] = 4096; M[5] = 4096; M[10] = 4096; M[15] = 4096;
    memcpy(M2, M, sizeof M);
    if (!rawSame(M, M2)) bad++;
    M2[12] = 1; if (rawSame(M, M2)) bad++;

    /* Hardware-ish: frame 21000, raw 7455, end 1277, flush small, idle small -> front ~12k */
    if (frontUs(21000, 500, 7455, 200, 1277) != 21000 - (500 + 7455 + 200 + 1277)) bad++;
    if (frontUs(7000, 100, 7455, 200, 1277) != 0) bad++; /* underflow clamp */

    if (wakeEvery(0) != 6) bad++;
    if (wakeEvery(1) != 1) bad++;

    if (bad) { fprintf(stderr, "g3_gap bad %d\n", bad); return 1; }
    puts("g3_gap ok");
    return 0;
}
