/* Layout check for the JOHTO tab strip and the achievement list. Block font, no ROM, no save.
   Synthetic achievement state is written under /tmp only. This is not a runtime screenshot.
   gcc -c -O1 -DACH_HOST_TEST -DACH_DIR=\"/tmp/ss3ds-ui-polish/\" -I soulsilver3ds/platform \
       soulsilver3ds/platform/achievements.c -o /tmp/ach_host.o
   g++ -O1 -I soulsilver3ds/platform tools/ui_polish_preview.cpp /tmp/ach_host.o -o /tmp/ui_polish
   mkdir -p /tmp/ss3ds-ui-polish && /tmp/ui_polish /tmp/ui_polish_ds.ppm /tmp/ui_polish_tabs.ppm
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
static bool brCfg() { return false; }
static bool panelLiveCfg() { return false; }
static bool pgFindCfg() { return false; }
static bool pgFlyCfg() { return false; }
extern "C" {
volatile int P3DS_PanelPartyWant;
volatile int P3DS_PanelPartyOn;
}
#include "ss3ds_minimap.h"
#include "ss3ds_pgmap.h"
#include "ss3ds_pgextra.h"
#include "ss3ds_blnative.h"
extern "C" {
void PSPNativeMemLog(const char *, ...) {}
int P3DS_CfgInt(const char *, int d) { return d; }
void P3DS_CfgWrite(const char *, int) {}
void P3DS_HuntChime(void) {}
int P3DS_FpsTweaks(void) { return 0; }
void P3DS_Toast(const char *) {}
}
static int fails;
static void expect(int cond, const char *msg) { if (!cond) { printf("FAIL %s\n", msg); fails++; } else printf("ok   %s\n", msg); }
/* 6px advance, ink in the top 10 rows of the 16px cell, so a 16px line pitch does not overlap */
static int dr(uint32_t *b, int x, int y, const char *t, uint32_t fg, uint32_t sh) {
  (void)sh; int n = 0; for (; t[n]; n++) if (t[n] != ' ') for (int yy = 1; yy <= 10; yy++) for (int xx = 0; xx < 5; xx++) suiPx(b, x + n * 6 + xx, y + yy, fg); return n * 6; }
static int wd(const char *t) { return (int)strlen(t) * 6; }
static int drS(uint32_t *b, int x, int y, const char *t, uint32_t fg, uint32_t sh) {
  (void)sh; int n = 0; for (; t[n]; n++) if (t[n] != ' ') for (int yy = 1; yy <= 8; yy++) for (int xx = 0; xx < 4; xx++) suiPx(b, x + n * 5 + xx, y + yy, fg); return n * 5; }
static int wdS(const char *t) { return (int)strlen(t) * 5; }
static int firstY(const uint32_t *b, int w, int h, uint32_t c) {
  for (int y = 0; y < h; y++) for (int x = 0; x < w; x++) if (b[y * w + x] == c) return y;
  return -1; }
static void ppm(const char *path, const uint32_t *b, int w, int h, int stride) {
  FILE *f = fopen(path, "wb"); if (!f) return; fprintf(f, "P6\n%d %d\n255\n", w, h);
  for (int y = 0; y < h; y++) for (int x = 0; x < w; x++) { uint32_t c = b[y * stride + x]; unsigned char p[3] = { (unsigned char)(c & 255), (unsigned char)(c >> 8), (unsigned char)(c >> 16) }; fwrite(p, 1, 3, f); }
  fclose(f); }
int main(int argc, char **argv) {
  if (argc < 3) return 1;
  mkdir("/tmp/ss3ds-ui-polish", 0777);
  const SUiText T = { dr, wd, drS, wdS };
  /* three gym badges earned, plus a partial catch counter so a locked row has progress. Game text only. */
  P3DS_AchPublish(0x7, 0, 0);
  unsigned short idx = 10; unsigned val = 20; P3DS_AchStats(&idx, &val, 1);
  static uint32_t ds[256 * 192];
  for (int i = 0; i < 256 * 192; i++) ds[i] = SUI_RGB(200, 208, 211);
  pxBar(ds, T, kBlPanX0, kBlPanY0, kBlPanX1, "Goldenrod City");
  pxNameStrip(ds, T, kBlPanX0, kBlPanY0, kBlPanX1, "Goldenrod City");
  /* hit band is unchanged: the tab row is still y 3..18, the panel below it */
  expect(blHit(40, 8) == 11, "tab band still hit 11");
  expect(blHit(40, 24) == 10, "below the tab band is the panel");
  expect(pxChipHit(T, kBlPanX1, kBlPanY0, 12) == 0, "left tab is MAP");
  expect(pxChipHit(T, kBlPanX1, kBlPanY0, 170) == 4, "right tab is BAG");
  SPg p{}; p.achOpen = true;
  const int X0 = 5, Y0 = 32, X1 = 185, Y1 = 187;
  pgAchPanel(ds, T, p, X0, Y0, X1, Y1);
  int whiteInCtrl = 0;
  for (int y = Y0; y < Y0 + 17; y++) for (int x = X1 - kPgAchCtrl; x < X1; x++) if (ds[y * 256 + x] == SUI_RGB(255, 255, 255)) whiteInCtrl++;
  expect(whiteInCtrl == 0, "title does not enter the corner-button rect");
  const uint32_t ink = SUI_RGB(7, 33, 54), gold = SUI_RGB(226, 168, 36), dim = SUI_RGB(100, 116, 112);
  int overlap = 0, descPast = 0, medals = 0;
  for (int y = Y0 + 39; y < Y1; y++) {
    int inkMax = -1, goldMin = 999, dimMax = -1, dimN = 0;
    for (int x = X0 + 22; x < X1; x++) { uint32_t c = ds[y * 256 + x];
      if (c == ink) if (x > inkMax) inkMax = x;
      if (c == gold && x > X0 + 40 && (goldMin > x)) goldMin = x;
      if (c == dim) { dimN++; if (x > dimMax) dimMax = x; } }
    if (inkMax >= 0 && goldMin < 999 && inkMax + 4 > goldMin) overlap++;
    if (dimN && dimMax > X1 - 6) descPast++;
  }
  expect(overlap == 0, "name ink and points do not share x");
  expect(descPast == 0, "description stays inside the right margin");
  /* a medal pixel: move the list 8px and it must move 8px */
  const int yA = firstY(ds, 256, 192, SUI_RGB(250, 214, 90));
  p.achScroll = 8; pgAchPanel(ds, T, p, X0, Y0, X1, Y1);
  const int yB = firstY(ds, 256, 192, SUI_RGB(250, 214, 90));
  expect(yA > 0 && yB == yA - 8, "8px drag moves the list 8px");
  p.achScroll = 0; pgAchPanel(ds, T, p, X0, Y0, X1, Y1);
  for (int y = 0; y < 192; y++) for (int x = 0; x < 20; x++) if (ds[y * 256 + x] == gold || ds[y * 256 + x] == SUI_RGB(250, 214, 90)) { medals++; break; }
  expect(medals > 0 && medals < 40, "fewer medal rows than the old cramped list");
  printf("info visible medal-rows (scan) %d, title/points gap checked\n", medals);
  /* every description line fits, and a long name leaves a gap before its points */
  int fitBad = 0;
  for (int id = 0; id < ACH_COUNT; id++) { const P3DS_AchInfo *a = P3DS_AchGet(id); char pts[16], nm[64], desc[2][64];
    snprintf(pts, sizeof pts, "%dG", a->pts); const int pw = T.width(pts), textX = X0 + 22, textR = X1 - 6;
    const int room = (textR - pw - 6) - textX; pgFit(T, nm, sizeof nm, a->name, room > 8 ? room : 8);
    if (textX + T.width(nm) + 6 > textR - pw) fitBad++;
    int n = pgAchWrap(T, a->desc, textR - textX, desc); for (int i = 0; i < n; i++) if (T.width(desc[i]) > textR - textX) fitBad++; }
  expect(fitBad == 0, "every name leaves 6px before its points, every desc line fits");
  /* location line keeps the place name to the right of the MAP chip */
  int city = 0, chip = 0;
  for (int y = 18; y < 32; y++) for (int x = 3; x < 187; x++) if (ds[y * 256 + x] == SUI_RGB(255, 255, 255)) { if (x < 40) chip++; else city++; }
  expect(chip > 0 && city > 0, "selected caption and place name are both on the line");
  pgDots(ds, p, X1, Y0);
  ppm(argv[1], ds, 256, 192, 256);
  /* native tab strip: same box colours as a column button */
  static uint32_t atlas[512 * 256]; memset(atlas, 0, sizeof atlas);
  int cx0[6], cx1[6], cy0, cy1; pxChips(T, kBlPanX1, kBlPanY0, cx0, cx1, &cy0, &cy1);
  SBnTabDst dst[6]; const int nd = bnTabStrip(atlas, 0, 5, cx0, cx1, cy0, cy1, kPxTabIcon, dst, 6);
  expect(nd == 5, "five native tab buttons");
  { int c6x0[6], c6x1[6]; /* breed_tab adds a sixth button; same height, still inside the atlas row */
    const int L = kBlPanX0 + 2, R = kBlPanX1 - 2, N = 6, W = R - L - (N - 1) * kPxTabGap;
    for (int i = 0; i < N; i++) { c6x0[i] = L + i * kPxTabGap + W * i / N; c6x1[i] = L + i * kPxTabGap + W * (i + 1) / N; }
    SBnTabDst d6[6]; const int n6 = bnTabStrip(atlas, 5, 6, c6x0, c6x1, cy0, cy1, kPxTabIcon, d6, 6);
    int hok = n6 == 6; for (int i = 0; i < n6; i++) { if (d6[i].h != d6[0].h || d6[i].w < 16) hok = 0; if (i && d6[i - 1].dx + d6[i - 1].w > d6[i].dx) hok = 0; }
    expect(hok, "six tabs (breed) stay the same height, do not overlap, and fit the atlas row");
    bnTabStrip(atlas, 0, 5, cx0, cx1, cy0, cy1, kPxTabIcon, dst, 6); }
  int sameH = 1, minW = 999, maxW = 0;
  for (int i = 0; i < nd; i++) { if (dst[i].h != dst[0].h) sameH = 0; if (dst[i].w < minW) minW = dst[i].w; if (dst[i].w > maxW) maxW = dst[i].w;
    printf("info tab %d native %d,%d %dx%d\n", i, dst[i].dx, dst[i].dy, dst[i].w, dst[i].h); }
  expect(sameH && maxW - minW <= 2, "tab buttons share a height and stay within 2px of width");
  const uint32_t SEL = bnRGB(46, 208, 155), BTN = bnRGB(244, 247, 254), NAVY = bnRGB(7, 33, 54), INK = bnRGB(0, 84, 64);
  int selPx = 0, idlePx = 0, navyPx = 0, inkPx = 0;
  for (int y = 0; y < dst[0].h; y++) for (int x = 0; x < dst[0].w; x++) { uint32_t c = atlas[(180 + y) * 512 + dst[0].sx + x]; if (c == SEL) selPx++; if (c == NAVY) navyPx++; if (c == INK) inkPx++; }
  for (int y = 0; y < dst[1].h; y++) for (int x = 0; x < dst[1].w; x++) if (atlas[(180 + y) * 512 + dst[1].sx + x] == BTN) idlePx++;
  expect(selPx > 20 && navyPx > 10 && inkPx > 10, "selected tab is the column green with a navy edge and a green icon");
  expect(idlePx > 20, "other tabs use the column's silver fill");
  /* a reference column button, same bnBox call, to sit beside the strip in the ppm */
  const SBnCanvas ref = { atlas, 0, 210, 0, 0, 67, 22 };
  bnRect(ref, 0, 0, 66, 21, bnRGB(200, 208, 211));
  bnBox(ref, 0, 0, 66, 21, NAVY, SEL, SEL, bnRGB(24, 153, 115), 2);
  bnIcon2x(ref, 4, 4, 0, INK, bnRGB(255, 255, 255));
  static uint32_t nat[320 * 48];
  for (int i = 0; i < 320 * 48; i++) nat[i] = 0x00C8D0D3u;
  for (int i = 0; i < nd; i++) for (int y = 0; y < dst[i].h && y < 40; y++) for (int x = 0; x < dst[i].w && dst[i].dx + x < 320; x++) {
    uint32_t c = atlas[(dst[i].sy + y) * 512 + dst[i].sx + x]; if (c) nat[y * 320 + dst[i].dx + x] = (c >> 24) | ((c >> 16) & 255) << 8 | ((c >> 8) & 255) << 16; }
  for (int y = 0; y < 22; y++) for (int x = 0; x < 67; x++) { uint32_t c = atlas[(210 + y) * 512 + x]; if (c) nat[(y + 24) * 320 + 8 + x] = (c >> 24) | ((c >> 16) & 255) << 8 | ((c >> 8) & 255) << 16; }
  ppm(argv[2], nat, 320, 48, 320);
  printf(fails ? "RESULT FAIL %d\n" : "RESULT PASS\n", fails);
  return fails ? 1 : 0;
}
