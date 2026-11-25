/* Layout check for the collapsible bottom panels (2026-11-25): the button column slid away, the tab row rolled up, both, and a
   mid-slide frame, composed like the 3DS does it (DS picture stretched x5/4, then the native atlas pieces on top). Block font,
   no ROM (the map panel shows the explored-cell fallback). Also checks the touch targets. Not a runtime screenshot.
   gcc -c -O1 -DACH_HOST_TEST -DACH_DIR=\"/tmp/ss3ds-ui-polish/\" -I soulsilver3ds/platform soulsilver3ds/platform/achievements.c -o /tmp/ach_host.o
   g++ -O1 -I soulsilver3ds/platform tools/panel_handles_preview.cpp /tmp/ach_host.o -o /tmp/panel_handles
   /tmp/panel_handles /tmp/panel_handles   (writes /tmp/panel_handles_<state>.ppm)
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctime>
#include <sys/stat.h>
#define BLINV do {} while (0)
extern "C" {
void PSPNativeMemLog(const char *, ...);
int P3DS_CfgInt(const char *, int);
void P3DS_CfgWrite(const char *, int);
void P3DS_HuntChime(void);
int P3DS_FpsTweaks(void);
void P3DS_Toast(const char *);
}
static bool pxOn() { return true; }
static bool pxNativeGear(int) { return false; }
static bool brCfg() { return false; }
static bool panelPartyCfg() { return false; }
static bool panelLiveCfg() { return false; }
static bool pgFindCfg() { return false; }
static bool pgFlyCfg() { return false; }
static int pgMapZoom() { return 1; }
static bool pgRoamSpr() { return false; }
extern "C" {
volatile int P3DS_PanelPartyWant;
volatile int P3DS_PanelPartyOn;
}
extern "C" {
int P3DS_MobilePcChipOn(void) { return 0; }
void P3DS_MobilePcOnChipTap(void) {}
void P3DS_MobilePcNotePartyVisible(void) {}
int P3DS_MobilePcAnimFrame(void) { return -1; }
}
#include "ss3ds_minimap.h"
#include "ss3ds_pgmap.h"
#include "ss3ds_pgextra.h"
#include "ss3ds_blnative.h"
extern "C" const SS3DS_PgSpec sLocationSpecs[100] = { { 1, 1, 1, 0x11 }, { 2, 4, 1, 0x11 } };
extern "C" {
void PSPNativeMemLog(const char *, ...) {}
int P3DS_CfgInt(const char *, int d) { return d; }
void P3DS_CfgWrite(const char *, int) {}
void P3DS_HuntChime(void) {}
int P3DS_FpsTweaks(void) { return 0; }
void P3DS_Toast(const char *) {}
}
static bool blPgDraw(uint32_t *, const SUiText &, int, int, int, int, bool) { return false; }
static int fails;
static void expect(int cond, const char *msg) { if (!cond) { printf("FAIL %s\n", msg); fails++; } else printf("ok   %s\n", msg); }
static int dr(uint32_t *b, int x, int y, const char *t, uint32_t fg, uint32_t) { int n = 0; for (; t[n]; n++) if (t[n] != ' ') for (int yy = 1; yy <= 10; yy++) for (int xx = 0; xx < 5; xx++) suiPx(b, x + n * 6 + xx, y + yy, fg); return n * 6; }
static int wd(const char *t) { return (int)strlen(t) * 6; }
static int drS(uint32_t *b, int x, int y, const char *t, uint32_t fg, uint32_t) { int n = 0; for (; t[n]; n++) if (t[n] != ' ') for (int yy = 1; yy <= 8; yy++) for (int xx = 0; xx < 4; xx++) suiPx(b, x + n * 5 + xx, y + yy, fg); return n * 5; }
static int wdS(const char *t) { return (int)strlen(t) * 5; }
static uint32_t atlas[512 * 256], ds[256 * 192], nat[320 * 240];
static void blit(int dx, int dy, int w, int h, int sx, int sy) { /* the RC clip of render3ds.cpp, then the GPU's alpha blend */
  if (dx < 0) { sx -= dx; w += dx; dx = 0; } if (dy < 0) { sy -= dy; h += dy; dy = 0; } if (dx + w > 320) w = 320 - dx; if (dy + h > 240) h = 240 - dy;
  for (int y = 0; y < h; y++) for (int x = 0; x < w; x++) { const uint32_t c = atlas[(sy + y) * 512 + sx + x]; if (c & 255) nat[(dy + y) * 320 + dx + x] = (c >> 24) | ((c >> 16) & 255) << 8 | ((c >> 8) & 255) << 16; } }
static void compose(const char *path, const SUiText &T, SMm &m, int side, int top, int sideN, int topN) {
  gBlGrabOn = 1; gBlSideHid = side; gBlTopHid = top; gBlSideN = sideN; gBlTopN = topN;
  const char *lb[kBlButtonCount] = { "POKEDEX", "POKEMON", "BAG", "POKEGEAR", "TRAINER", "SAVE", "OPTIONS", "MAP", "1", "RUN", "2" };
  bool on[kBlButtonCount]; for (bool &o : on) o = true;
  blDraw(ds, T, m, 1, false, lb, lb, on, -1);
  for (int y = 0; y < 240; y++) for (int x = 0; x < 320; x++) nat[y * 320 + x] = ds[(y * 4 / 5) * 256 + x * 4 / 5];
  memset(atlas, 0, sizeof atlas); int bx[kBlButtonCount], by[kBlButtonCount];
  bnFrame(atlas, lb, lb, on, -1, 0, bx, by); bnGrabPills(atlas);
  int cx0[kPxMaxTab], cx1[kPxMaxTab], cy0, cy1; pxChips(T, kBlPanX1, kBlPanY0, cx0, cx1, &cy0, &cy1);
  SBnTabDst d[kPxMaxTab]; const int nd = bnTabStrip(atlas, pxPane(), pxNTab(), cx0, cx1, cy0, cy1, kPxTabIcon, kPxTabs, kPxTabShort, d, kPxMaxTab);
  const int so = gBlSideN;
  if (so > 0) { blit(235, 0, so, 4, 100, 160); blit(235, 236, so, 4, 100, 164); }
  blit(235 + so, 0, 85, 240, 256, 0); blit(0, 0, 235, 4, 0, 160); blit(0, 236, 235, 4, 0, 164); blit(0, 4, 4, 232, 344, 0);
  blit(237 + so, 4, kBnGrabW, kBnGrabH, kBnGrabAX + (gBlTopHid ? 36 : 24), kBnGrabAY); blit(237 + so, 110, kBnGrabW, kBnGrabH, kBnGrabAX + (gBlSideHid ? 12 : 0), kBnGrabAY);
  if (gBlTopN < kBlTopSlideN) for (int i = 0; i < nd; i++) blit(d[i].dx, d[i].dy - gBlTopN, d[i].w, d[i].h, d[i].sx, d[i].sy);
  FILE *f = fopen(path, "wb"); if (!f) return; fprintf(f, "P6\n320 240\n255\n");
  for (int i = 0; i < 320 * 240; i++) { const uint32_t c = nat[i]; unsigned char p[3] = { (unsigned char)(c & 255), (unsigned char)(c >> 8), (unsigned char)(c >> 16) }; fwrite(p, 1, 3, f); }
  fclose(f); }
int main(int argc, char **argv) {
  const char *base = argc > 1 ? argv[1] : "/tmp/panel_handles"; char path[256];
  const SUiText T = { dr, wd, drS, wdS }; gBlHd = 1;
  static SMm m; mmUpdate(m, 1, 10, 10, 5, 5, 1, 1, "Violet City", 1); for (int i = 0; i < 12; i++) mmUpdate(m, 1, 10, 10, 2 + i, 4 + i % 3, 1 + i % 3, 1, "Violet City", 2 + i);
  struct { const char *nm; int side, top, sn, tn; } st[] = { { "open", 0, 0, 0, 0 }, { "side", 1, 0, kBlSideShiftN, 0 }, { "top", 0, 1, 0, kBlTopSlideN },
    { "both", 1, 1, kBlSideShiftN, kBlTopSlideN }, { "midslide", 1, 0, kBlSideShiftN / 2, 0 }, { "midroll", 0, 1, 0, kBlTopSlideN / 2 } };
  for (auto &s : st) { snprintf(path, sizeof path, "%s_%s.ppm", base, s.nm); compose(path, T, m, s.side, s.top, s.sn, s.tn); }
  /* touch targets (DS px): the strip toggles, the column answers only while it is shown, the tab row only while it is down */
  gBlGrabOn = 1; gBlSideHid = 0; gBlTopHid = 0;
  expect(blHit(193, 100) == 13 && blHit(193, 10) == 13, "open: the gap left of the column is the handle strip");
  expect(blHit(220, 10) == 0, "open: the first column button still answers");
  expect(blHit(40, 8) == 11 && blHit(40, 40) == 10, "open: tabs and panel unchanged");
  gBlSideHid = 1;
  expect(kBlPanX1 == 243, "side hidden: panel reaches DS x 243");
  expect(blHit(250, 100) == 13 && blHit(220, 100) == 10, "side hidden: strip at the screen edge, the old column area is panel");
  gBlTopHid = 1;
  expect(blHit(40, 8) == 10, "tabs rolled up: the top rows are panel");
  gBlGrabOn = 0; gBlSideHid = 0; gBlTopHid = 0;
  expect(blHit(193, 100) == -1, "handles off: the gap is dead space as before");
  printf(fails ? "RESULT FAIL %d\n" : "RESULT PASS\n", fails);
  return fails ? 1 : 0; }
