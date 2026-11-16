/* Host check for the area summary window.
   A line is due when the open window's zone or map changed, or it already
   holds 300 ticks. The tick that trips it starts the next window. */
#include <stdio.h>
enum { AREA_TICKS = 300 };
typedef struct { int open, zone, map; unsigned ticks, shown; } A;
static int step(A *a, int zone, int map, int shown) {
    const int due = a->open && (zone != a->zone || map != a->map || a->ticks >= AREA_TICKS);
    unsigned emitted = 0;
    if (due) { emitted = a->ticks; a->open = 0; }
    if (!a->open) { a->open = 1; a->zone = zone; a->map = map; a->ticks = a->shown = 0; }
    a->ticks++; if (shown) a->shown++;
    return (int)emitted;
}
int main(void) {
    int bad = 0;
    A a; a.open = 0;
    for (int i = 0; i < 300; i++) if (step(&a, 10, 33, 1)) bad++;
    if (a.ticks != 300) bad++;
    if (step(&a, 10, 33, 1) != 300) bad++;
    if (a.ticks != 1 || a.zone != 10) bad++;
    A b; b.open = 0;
    if (step(&b, 1, 101, 0) || step(&b, 1, 101, 0)) bad++;
    if (step(&b, 2, 33, 1) != 2) bad++;
    if (b.zone != 2 || b.map != 33 || b.ticks != 1) bad++;
    if (216 * 100 / 300 != 72) bad++;
    if (bad) { fprintf(stderr, "fps60 area bad %d\n", bad); return 1; }
    puts("fps60 area ok");
    return 0;
}
