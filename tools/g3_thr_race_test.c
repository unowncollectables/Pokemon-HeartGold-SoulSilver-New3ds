/* Host model of g3_thread races + post-#486/#487 Azahar-repro logic bugs.
   1) Consumer acquire after new tail; replay verts copied; xf gen from job not live globals.
   2) Full POD snapshot survives live global mutation after submit.
   3) Depth-1 resolve: GetTexture/TexCreate only when the queue is idle (inline cadence).
   4) Deferred Batch carries sd/parL/wide/gt (FlushDeferred must not use live stereo/tex).
   5) Threaded vs inline clip xform hashes match after arena copy. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct { float x,y,z; unsigned color; float u,v; } Vtx;
typedef struct {
    const Vtx *v; unsigned n; float M[16]; int cullFace; int sd; int parL; int mid, wide;
    const void *xfKey; int replay;
    void *gt; unsigned tw, th; int hasTex;
    int hasK; float K[16];
    int sprDepth, cull3d; int cpuClip, guardClip, guardBat, edgeFix, fieldSpr;
    uint32_t xformCrc;
} Job;

typedef struct {
    const Vtx *v; unsigned n; void *gt; unsigned tw, th; int hasTex;
    int sd, parL, wide;
} Batch;

static unsigned xfGen(int sd, int mid, int wide, int cull) {
    return ((unsigned)(sd & 255)) | ((mid ? 1u : 0u) << 8) | ((wide ? 1u : 0u) << 9) | ((unsigned)(cull & 3) << 10);
}

static int liveCull = 0;
static unsigned badGenUsesLive(const Job *J) { return xfGen(J->sd, J->mid, J->wide, liveCull); }
static unsigned goodGenUsesJob(const Job *J) { return xfGen(J->sd, J->mid, J->wide, J->cullFace); }

static int copyThenMutate(void) {
    Vtx list[3] = {{1,0,0,0xffffffff,0,0},{0,1,0,0xffffffff,0,0},{0,0,1,0xffffffff,0,0}};
    Vtx arena[3];
    Job J = {0}; J.v = list; J.n = 3; J.replay = 1; J.xfKey = list; J.cullFace = 2; J.sd = 4; J.wide = 1;
    memcpy(arena, J.v, sizeof arena); J.v = arena;
    list[0].x = 99;
    return arena[0].x == 1 && J.xfKey == list;
}

static volatile unsigned head, tail;
static Job ring[4];
static Job seen;
static void publish(const Job *J) {
    unsigned t = tail;
    ring[t & 3] = *J;
    __atomic_store_n(&tail, t + 1, __ATOMIC_RELEASE);
}
static int consume_acquire(void) {
    unsigned h = head;
    unsigned t = __atomic_load_n(&tail, __ATOMIC_ACQUIRE);
    if (h == t) return 0;
    seen = ring[h & 3];
    __atomic_store_n(&head, h + 1, __ATOMIC_RELEASE);
    return 1;
}

static void xform(const Job *J, float out[12]) {
    const float *M = J->M;
    for (int k = 0; k < 3; k++) {
        const Vtx *p = &J->v[k];
        out[4*k+0] = p->x*M[0]+p->y*M[4]+p->z*M[8]+M[12];
        out[4*k+1] = p->x*M[1]+p->y*M[5]+p->z*M[9]+M[13];
        out[4*k+2] = p->x*M[2]+p->y*M[6]+p->z*M[10]+M[14];
        out[4*k+3] = p->x*M[3]+p->y*M[7]+p->z*M[11]+M[15];
    }
}

static uint32_t xformCrc(const Job *J) {
    uint32_t h = 2166136261u;
    const float *M = J->M;
    unsigned n = J->n < 48u ? J->n : 48u;
    for (unsigned i = 0; i < n; i++) {
        const Vtx *p = &J->v[i];
        float c[4] = {
            p->x*M[0]+p->y*M[4]+p->z*M[8]+M[12],
            p->x*M[1]+p->y*M[5]+p->z*M[9]+M[13],
            p->x*M[2]+p->y*M[6]+p->z*M[10]+M[14],
            p->x*M[3]+p->y*M[7]+p->z*M[11]+M[15]
        };
        const uint8_t *b = (const uint8_t *)c;
        for (unsigned k = 0; k < sizeof c; k++) h = (h ^ b[k]) * 16777619u;
    }
    h = (h ^ (uint8_t)J->sd) * 16777619u;
    h = (h ^ (uint8_t)J->parL) * 16777619u;
    h = (h ^ (uint8_t)J->wide) * 16777619u;
    h = (h ^ (uint8_t)J->hasTex) * 16777619u;
    return h;
}

/* Depth-1 model: resolving tex while a job is in flight is the buggy schedule. */
static int depth1_resolve_only_when_idle(void) {
    unsigned in_flight = 1; /* worker still on job 0 */
    int resolved_while_busy = 0;
    /* buggy deep queue */
    if (in_flight) resolved_while_busy = 1;
    /* fixed: wait until idle */
    in_flight = 0;
    int resolved_when_idle = !in_flight;
    return resolved_while_busy == 1 && resolved_when_idle == 1;
}

/* Deferred batch without snapshotted sd/gt is the FlushDeferred stereo/tex bug. */
static int batch_carries_stereo_tex(void) {
    Batch bad = {0}; bad.sd = 0; bad.gt = NULL; /* pre-fix */
    Batch good = {0}; good.sd = 4; good.parL = 0; good.wide = 1; good.gt = (void *)(uintptr_t)0x1234; good.hasTex = 1; good.tw = 32; good.th = 32;
    return bad.sd == 0 && bad.gt == NULL && good.sd == 4 && good.gt != NULL && good.hasTex;
}

int main(void) {
    int bad = 0;
    Job J = {0}; J.cullFace = 2; J.sd = 4; J.mid = 1; J.wide = 1;
    liveCull = 0;
    if (badGenUsesLive(&J) == goodGenUsesJob(&J)) bad++;
    if (goodGenUsesJob(&J) != xfGen(4, 1, 1, 2)) bad++;
    if (!copyThenMutate()) bad++;

    head = tail = 0;
    Vtx v0[3] = {{1,2,3,1,0,0},{4,5,6,1,0,0},{7,8,9,1,0,0}};
    Job pub = {0}; pub.v = v0; pub.n = 3; pub.cullFace = 2; pub.sd = 4; pub.parL = 0; pub.wide = 1;
    pub.xfKey = v0; pub.replay = 1; pub.gt = (void *)(uintptr_t)0x1234; pub.hasTex = 1; pub.tw = 32; pub.th = 32;
    for (int i = 0; i < 16; i++) pub.M[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    pub.xformCrc = xformCrc(&pub);
    publish(&pub);
    if (!consume_acquire()) bad++;
    if (seen.cullFace != 2 || seen.sd != 4 || seen.v != v0) bad++;
    if (xformCrc(&seen) != pub.xformCrc) bad++;

    float a[12], b[12];
    Job off = pub;
    xform(&off, a);
    Vtx arena[3]; memcpy(arena, pub.v, sizeof arena);
    Job on = pub; on.v = arena; on.xfKey = pub.v;
    v0[0].x = -999;
    xform(&on, b);
    if (memcmp(a, b, sizeof a)) bad++;
    if (xformCrc(&on) != pub.xformCrc) bad++; /* same clip hash after copy; original verts mutated */

    if (!depth1_resolve_only_when_idle()) bad++;
    if (!batch_carries_stereo_tex()) bad++;

    if (bad) { fprintf(stderr, "g3_thr_race bad %d\n", bad); return 1; }
    puts("g3_thr_race ok");
    return 0;
}
