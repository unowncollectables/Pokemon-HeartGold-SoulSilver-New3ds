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
#include "ss3ds_minimap.h"
#include "ss3ds_pgmap.h"
#include "ss3ds_pgextra.h"
#include "ss3ds_blnative.h"
extern "C" const SS3DS_PgSpec sLocationSpecs[100] = {
  { 1, 1, 1, 0x11 }, { 2, 4, 1, 0x11 }
};
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
  for (int hd = 0; hd <= 1; hd++) {
    gBlHd = hd;
    int x0, y0, x1, y1, a0, b0, a1, b1, runX0, runY0, runX1, runY1;
    blRect(kBlItem1, &x0, &y0, &x1, &y1);
    blRect(kBlItem2, &a0, &b0, &a1, &b1);
    blRect(kBlRun, &runX0, &runY0, &runX1, &runY1);
    expect(x0 == a0 && x1 == a1 && y1 <= b0 && a1 <= runX0 &&
           blHit((x0 + x1) / 2, (y0 + y1) / 2) == kBlItem1 &&
           blHit((a0 + a1) / 2, (b0 + b1) / 2) == kBlItem2Hit &&
           blHit((runX0 + runX1) / 2, (runY0 + runY1) / 2) == kBlRun,
           hd ? "native item 1, item 2 and Run have separate hit targets" :
                "DS-size item 1, item 2 and Run have separate hit targets");
  }
  gBlHd = 0;
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
  /* Scroll uses the same subtraction as the bottom-menu touch path: achScroll -= (new DS y - previous DS y).
     Finger up => DS y falls => scroll grows => later rows move up into the list. DS y is touchToDS:
     scale 0 is py-24 (1:1, centered); any other scale is py*192/240. Wide view does not retarget the bottom. */
  { const int dsHi = 160 * 192 / 240, dsLo = 120 * 192 / 240;
    expect(dsHi - dsLo == 32, "40 native px is 32 DS px on the scaled bottom");
    expect((dsHi - dsLo) * 5 / 4 == 40, "32 DS px of list motion displays as those same 40 px");
    expect((160 - 24) - (120 - 24) == 40, "1:1 touch moves 1:1 after the centered inset"); }
  { const int top = Y0 + 39, textX = X0 + 22, textW = (X1 - 6) - textX;
    int nameAt[16]; int gymEnd = kPgAchLine;
    for (int id = 0; id < 16; id++) {
      nameAt[id] = gymEnd + 1;
      char desc[2][64]; const int dl = pgAchWrap(T, P3DS_AchGet(id)->desc, textW, desc);
      const int lines = dl > 0 ? dl : 1, d = P3DS_AchDate(id); unsigned cu = 0, go = 0;
      const int prog = !d && P3DS_AchProgress(id, &cu, &go) && go > 1;
      const int extra = (d || prog) ? kPgAchDate + 1 : 0;
      gymEnd += 2 + kPgAchLine + lines * kPgAchLine + extra + 2; }
    auto mark = [&](int y) { if (y < top || y >= Y1) return 0; for (int x = textX; x < textX + 10; x++) {
      const uint32_t c = ds[y * 256 + x]; if (c == ink || c == dim) return 1; } return 0; };
    /* third gym, only the lower part of its name across the list edge (the case that used to vanish) */
    p.achScroll = nameAt[2] + 6; pgAchPanel(ds, T, p, X0, Y0, X1, Y1);
    expect(p.achScroll == nameAt[2] + 6, "that scroll is inside the list range");
    int partial = 0; for (int y = top; y < top + 8; y++) partial += mark(y);
    expect(partial > 0, "a name that only partly enters the list is still drawn");
    int leaked = 0; for (int x = textX; x < textX + 40; x++) if (ds[(top - 1) * 256 + x] == ink || ds[(top - 1) * 256 + x] == dim) leaked++;
    expect(leaked == 0, "the clipped name does not paint the header");
    /* continuous upward strokes, same 16px drag gate as the menu, each longer than the viewport */
    const int view = Y1 - top;
    auto gesture = [&](int fromDs, int toDs) {
      const int step = toDs > fromDs ? 1 : -1; int tY0 = fromDs, tYp = fromDs, drag = 0;
      for (int tY = fromDs; tY != toDs; tY += step) {
        if (abs(tY - tY0) > 16) drag = 1;
        if (drag && tY != tYp) p.achScroll -= tY - tYp;
        tYp = tY; } };
    p.achScroll = 0; int strokes = 0;
    while (p.achScroll < view * 2 && strokes < 12) { gesture(150, 150 - (view + 30)); strokes++; }
    pgAchPanel(ds, T, p, X0, Y0, X1, Y1);
    expect(p.achScroll > view, "repeated upward drags pass a full viewport");
    p.achScroll = nameAt[8] > 8 ? nameAt[8] - 8 : 0;
    pgAchPanel(ds, T, p, X0, Y0, X1, Y1);
    const int yLater = top + nameAt[8] - p.achScroll; int sawLater = 0;
    for (int y = yLater; y < yLater + 12; y++) sawLater += mark(y);
    expect(sawLater > 0, "gym row 9 is drawn when the list is scrolled onto it");
    expect(top + nameAt[0] + 16 - p.achScroll <= top, "the first gym has left the list");
    while (p.achScroll > 0 && strokes < 40) { gesture(40, 40 + view + 30); strokes++; }
    if (p.achScroll < 0) p.achScroll = 0;
    pgAchPanel(ds, T, p, X0, Y0, X1, Y1);
    expect(p.achScroll < 40, "downward drags return to the start");
    int sawFirst = 0; const int yFirst = top + nameAt[0] - p.achScroll;
    for (int y = yFirst; y < yFirst + 12; y++) sawFirst += mark(y);
    expect(sawFirst > 0, "the first gym is visible again");
    p.achScroll = -80; pgAchPanel(ds, T, p, X0, Y0, X1, Y1);
    expect(p.achScroll == 0, "scroll clamps at the top");
    p.achScroll = 1000000; pgAchPanel(ds, T, p, X0, Y0, X1, Y1);
    const int maxS = p.achScroll;
    expect(maxS >= nameAt[15], "scroll can reach the 16th gym, not only the first screen");
    p.achScroll = nameAt[15] > 4 ? nameAt[15] - 4 : 0;
    pgAchPanel(ds, T, p, X0, Y0, X1, Y1);
    const int yLast = top + nameAt[15] - p.achScroll; int sawLast = 0;
    for (int y = yLast; y < yLast + 16; y++) sawLast += mark(y);
    expect(sawLast > 0, "the 16th gym row is drawn when scrolled onto it");
    p.achScroll = maxS;
    int blank = 0;
    for (int s = 0; s <= maxS; s += 4) {
      p.achScroll = s; pgAchPanel(ds, T, p, X0, Y0, X1, Y1);
      int marks = 0;
      for (int y = top; y < Y1; y += 2) if (mark(y)) marks++;
      if (!marks) blank++; }
    expect(blank == 0, "no scroll step leaves the list blank");
    p.achScroll = 0; pgAchPanel(ds, T, p, X0, Y0, X1, Y1);
    /* The runtime only scrolls from a held touch that started on the panel. A move of 10px stays a tap.
       The sample that passes 16px applies the whole stroke, then DN/UP are ordinary taps. */
    int drag = 0, prev = 100;
    expect(pgAchDragDelta(&drag, &prev, 90, 10) == 0 && !drag && prev == 100, "10px of movement does not scroll");
    expect(pgAchDragDelta(&drag, &prev, 70, 30) == 30 && drag && prev == 70, "passing 16px scrolls the whole stroke");
    expect(pgAchDragDelta(&drag, &prev, 50, 50) == 20, "later samples keep scrolling");
    const int medal0 = firstY(ds, 256, 192, SUI_RGB(250, 214, 90));
    expect(pgAchBtnHit(p, (p.achDnX0 + p.achDnX1) / 2, (p.achDnY0 + p.achDnY1) / 2) == 2, "DN is a tap target on the totals band");
    expect(pgAchBtnHit(p, (p.achUpX0 + p.achUpX1) / 2, (p.achUpY0 + p.achUpY1) / 2) == 1, "UP is a tap target on the totals band");
    expect(p.achPage > 16, "one tap moves by most of the visible list");
    p.achScroll = p.achPage; pgAchPanel(ds, T, p, X0, Y0, X1, Y1);
    int still = 0; for (int x = 0; x < 40; x++) if (ds[medal0 * 256 + x] == SUI_RGB(250, 214, 90)) still++;
    expect(medal0 > 0 && still == 0, "a DN tap moves the top medal off that row");
    p.achScroll = 0; pgAchPanel(ds, T, p, X0, Y0, X1, Y1); }
  pgDots(ds, p, X1, Y0);
  ppm(argv[1], ds, 256, 192, 256);
  /* native tab strip: same box colours as a column button */
  static uint32_t atlas[512 * 256]; memset(atlas, 0, sizeof atlas);
  int cx0[6], cx1[6], cy0, cy1; pxChips(T, kBlPanX1, kBlPanY0, cx0, cx1, &cy0, &cy1);
  SBnTabDst dst[6]; const int nd = bnTabStrip(atlas, 0, 5, cx0, cx1, cy0, cy1, kPxTabIcon, kPxTabs, kPxTabShort, dst, 6);
  expect(nd == 5, "five native tab buttons");
  { int c6x0[6], c6x1[6]; /* breed_tab adds a sixth button; same height, still inside the atlas row */
    const int L = kBlPanX0 + 2, R = kBlPanX1 - 2, N = 6, W = R - L - (N - 1) * kPxTabGap;
    for (int i = 0; i < N; i++) { c6x0[i] = L + i * kPxTabGap + W * i / N; c6x1[i] = L + i * kPxTabGap + W * (i + 1) / N; }
    SBnTabDst d6[6]; const int n6 = bnTabStrip(atlas, 5, 6, c6x0, c6x1, cy0, cy1, kPxTabIcon, kPxTabs, kPxTabShort, d6, 6);
    int hok = n6 == 6; for (int i = 0; i < n6; i++) { if (d6[i].h != d6[0].h || d6[i].w < 16) hok = 0; if (i && d6[i - 1].dx + d6[i - 1].w > d6[i].dx) hok = 0; }
    expect(hok, "six tabs (breed) stay the same height, do not overlap, and fit the atlas row");
    bool matchingDetails = n6 == 6;
    for (int i = 0; i < n6; i++) {
      const auto &tab = d6[i];
      const auto *pixels = atlas + (tab.sy + 4) * 512 + tab.sx;
      matchingDetails &= pixels[tab.w - 2] == bnRGB(212, 59, 60);
      matchingDetails &= pixels[13] == (i == 5 ? bnRGB(26, 108, 95) : bnRGB(164, 176, 182));
    }
    expect(matchingDetails, "all six native tabs use the column divider and red accent");

    bnTabStrip(atlas, 0, 5, cx0, cx1, cy0, cy1, kPxTabIcon, kPxTabs, kPxTabShort, dst, 6); }
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
  // Render the tallest Day-Care state and verify that both cards in the last row fit.
  P3DS_BrReady = 1;
  P3DS_BrD[0] = 2;
  P3DS_BrD[1] = 25;
  P3DS_BrD[2] = 132;
  P3DS_BrD[7] = 70;
  P3DS_BrD[9] = 1;
  P3DS_BrD[12] = 2;
  P3DS_BrD[13] = P3DS_BrD[32] = 6;
  memset(ds, 0, sizeof ds);
  const int panelY = kBlPanY0 + 17;
  pxBreed(ds, T, kBlPanX0, panelY, kBlPanX1, kBlPanY1);
  const int lastRow = panelY + kPxSubH + 4 + 66 + 4 + 15 + 44;
  expect(ds[lastRow * 256 + 15] == kBlNavy && ds[lastRow * 256 + 125] == kBlNavy,
         "all six egg cards fit alongside a full Day-Care");

  P3DS_PhRing = 2;
  pxRingFrame();
  expect(pxRingShown() == 2, "ring is visible on its first frame");
  P3DS_PhRing = 0;
  bool held = true;
  for (int frame = 0; frame < 19; frame++) {
    pxRingFrame();
    for (int draw = 0; draw < 5; draw++) held &= pxRingShown() == 2;
  }
  expect(held, "reading the ring does not shorten its hold");
  pxRingFrame();
  expect(pxRingShown() == 0, "ring banner clears after 20 quiet frames");
  snprintf(P3DS_PgName[1], sizeof P3DS_PgName[1], "Pallet Town");
  P3DS_PgWeather[0] = 1;
  P3DS_PgWeather[1] = 2;
  P3DS_PgWeatherTick++;
  pgWeatherRefresh(false);
  expect(pgWeatherMask[10 * kPgW + 10] == ((5 << 2) | 1) &&
         pgWeatherMask[10 * kPgW + 6] == ((3 << 2) | 1),
         "rain is anchored to its map cell and feathers at the edge");
  expect(pgWeatherMask[10 * kPgW + 34] == 0, "Kanto weather stays hidden until the map opens");
  pgWeatherRefresh(true);
  expect(pgWeatherMask[10 * kPgW + 34] == ((5 << 2) | 2), "Kanto snow appears after the map opens");
  P3DS_PgWeather[0] = 0;
  P3DS_PgWeatherTick++;
  pgWeatherRefresh(true);
  expect(pgWeatherMask[10 * kPgW + 10] == 0, "a clear forecast removes the old rain");
  printf(fails ? "RESULT FAIL %d\n" : "RESULT PASS\n", fails);
  return fails ? 1 : 0;
}
