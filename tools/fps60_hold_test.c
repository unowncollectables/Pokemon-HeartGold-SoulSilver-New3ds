/* fps60_hold repeats the real frame only when the in-between would match it.
   Camera and 3D motion are a matrix change (mov) or an unpaired draw (mat != tot):
   a new particle, a model that just appeared. Sprites, scroll and tiles are
   sceneMoved. Water and a walking NPC also move sampled vertices (vertsMoved). */
#include <stdio.h>

static int hold(int toggle, unsigned tot, unsigned mat, unsigned mov, int scene, int verts) {
    if (!toggle || !tot || mov || mat != tot || scene || verts) return 0;
    return 1;
}

static int expect(const char *name, int got, int want) {
    if (got == want) return 0;
    fprintf(stderr, "%s: got %d want %d\n", name, got, want);
    return 1;
}

int main(void) {
    int bad = 0;
    bad += expect("off", hold(0, 10, 10, 0, 0, 0), 0);
    bad += expect("still", hold(1, 10, 10, 0, 0, 0), 1);
    bad += expect("camera", hold(1, 10, 10, 1, 0, 0), 0);
    bad += expect("particle unpaired", hold(1, 10, 9, 0, 0, 0), 0);
    bad += expect("npc sprite", hold(1, 10, 10, 0, 1, 0), 0);
    bad += expect("water verts", hold(1, 10, 10, 0, 0, 1), 0);
    bad += expect("no 3d", hold(1, 0, 0, 0, 0, 0), 0);
    if (bad) return 1;
    puts("fps60 hold ok");
    return 0;
}
