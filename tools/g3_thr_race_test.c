/* Host model of the g3_thread races that flashed overworld terrain on hardware (run 872).
   1) Consumer must acquire after seeing a new tail before reading the job (ARM11 weak order).
   2) Replay draws must copy vertices into the job arena; xfKey keeps list identity for xfcache.
   3) xf gen must use the job's snapshotted stereo/mid/wide/cull, not live globals. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct { float x,y,z; unsigned color; float u,v; } Vtx;
typedef struct {
    const Vtx *v; unsigned n; float M[16]; int cullFace; int sd; int mid, wide;
    const void *xfKey; int replay;
} Job;

static unsigned xfGen(int sd, int mid, int wide, int cull) {
    return ((unsigned)(sd & 255)) | ((mid ? 1u : 0u) << 8) | ((wide ? 1u : 0u) << 9) | ((unsigned)(cull & 3) << 10);
}

/* Simulate a stale global cullFace while the worker still processes job cullFace=2. */
static int liveCull = 0;
static unsigned badGenUsesLive(const Job *J) {
    return xfGen(J->sd, J->mid, J->wide, liveCull); /* buggy */
}
static unsigned goodGenUsesJob(const Job *J) {
    return xfGen(J->sd, J->mid, J->wide, J->cullFace);
}

/* Copy path: worker reads arena bytes, not the live list pointer. */
static int copyThenMutate(void) {
    Vtx list[3] = {{1,0,0,0xffffffff,0,0},{0,1,0,0xffffffff,0,0},{0,0,1,0xffffffff,0,0}};
    Vtx arena[3];
    Job J = {0}; J.v = list; J.n = 3; J.replay = 1; J.xfKey = list; J.cullFace = 2; J.sd = 4; J.mid = 0; J.wide = 1;
    memcpy(arena, J.v, sizeof arena); J.v = arena; /* publish copy */
    list[0].x = 99; /* game mutates the list-cache slot / reuses the pointer */
    return arena[0].x == 1 && J.xfKey == list;
}

/* Barrier model: without acquire, consumer may pair new tail with old payload. */
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

/* Bit-exact transform of one tri with job matrix (stand-in for GpuRawJ output). */
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

int main(void) {
    int bad = 0;
    Job J = {0}; J.cullFace = 2; J.sd = 4; J.mid = 1; J.wide = 1;
    liveCull = 0;
    if (badGenUsesLive(&J) == goodGenUsesJob(&J)) bad++; /* live cull must differ */
    if (goodGenUsesJob(&J) != xfGen(4, 1, 1, 2)) bad++;

    if (!copyThenMutate()) bad++;

    head = tail = 0;
    Vtx v0[3] = {{1,2,3,1,0,0},{4,5,6,1,0,0},{7,8,9,1,0,0}};
    Job pub = {0}; pub.v = v0; pub.n = 3; pub.cullFace = 2; pub.sd = 4; pub.xfKey = v0; pub.replay = 1;
    for (int i = 0; i < 16; i++) pub.M[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    publish(&pub);
    if (!consume_acquire()) bad++;
    if (seen.cullFace != 2 || seen.sd != 4 || seen.v != v0) bad++;

    /* Same command stream: "thread off" (direct) vs "thread on" (copy + snapshot) must match xform. */
    float a[12], b[12];
    Job off = pub;
    xform(&off, a);
    Vtx arena[3]; memcpy(arena, pub.v, sizeof arena);
    Job on = pub; on.v = arena; on.xfKey = pub.v;
    xform(&on, b);
    if (memcmp(a, b, sizeof a)) bad++;

    if (bad) { fprintf(stderr, "g3_thr_race bad %d\n", bad); return 1; }
    puts("g3_thr_race ok");
    return 0;
}
