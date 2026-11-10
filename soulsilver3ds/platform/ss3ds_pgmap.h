/* ss3ds run 10: the Pokegear's Johto/Kanto map in the JOHTO bottom layout (bottom_layout=1).
   The art is the user's own ROM data, read on the game thread by pspoke hook 0081 (pgmap_gra NARC members 10 NCGR 8bpp
   tiles, 11 NSCR 64x64 tile sheet whose top-left 47x20 tiles are the whole map, 20 NCLR palette) and decoded here; nothing
   from the ROM is ever written to the SD card or committed. Locations are the game's own sLocationSpecs (x/y/size in map
   tiles). Visited locations are kept in sdmc:/3ds/soulsilver3ds/visited.bin ("VB01", trainer ID, 128-bit set), never in
   the save; a different trainer ID (new game) starts empty. Pure pixel/state code except the file I/O. */
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ss3ds_settings_ui.h"
#include "p3ds_speciesnames.h"
#include "tracker.h"

extern "C" {
struct SS3DS_PgSpec { uint16_t mapId; uint8_t x, y; uint16_t bits; uint8_t flavorText, block, u8_, u9, uA, uB, srcX, srcY, dw, dh; };
extern const SS3DS_PgSpec sLocationSpecs[100];
extern volatile int P3DS_FlyReq, P3DS_FlyRes, P3DS_FlyN, P3DS_FlyHere; extern volatile unsigned short P3DS_FlyList[32]; /* hook 0228, ss3ds_glue.c */
volatile int P3DS_PgReady, P3DS_PgLoc = -1, P3DS_PgShoes = -1; volatile uint32_t P3DS_PgTid;
char P3DS_PgName[100][20]; char P3DS_PgText[100][100];
/* buffers the game thread fills (hook 0081). Keys: pgmap_gra member (0..63), 100 + a/0/1/4 member, 200–203 = registered
   item icon chars/palette (fixed 16 KB slots, refilled in place when the registered item changes). */
static uint8_t *pgRaw[256]; static uint32_t pgRawSz[256];
volatile int P3DS_PgGender = -1, P3DS_PgItem = -1, P3DS_PgItem2 = -1, P3DS_PgItemReq;
/* run 13: progress per Pokegear location from pspoke hook 0083 (read-only flag checks on the game thread). Row i < 100:
   [0] species available, [1] caught, [2] seen, [3] trainers, [4] beaten, [5] visible items, [6] taken, [7] hidden items,
   [8] found. Row 100 = whole map (species counted once). */
/* run 16: hook 0083's ROM tables, kept here because the field overlay's .bss is cleared on every reload (each battle) */
uint16_t P3DS_PsKId[2400]; uint8_t P3DS_PsKLoc[2400], P3DS_PsKKind[2400], P3DS_PsKSp[100][64]; int P3DS_PsKN, P3DS_PsKBuilt;
volatile int P3DS_PsReady; volatile uint32_t P3DS_PsTick; uint16_t P3DS_PsStat[101][9];
/* run 16: hook 0084 publishes the wild species set per location (bit = species 1..493) and which of them are caught */
uint8_t P3DS_PsSp[100][64], P3DS_PsCg[100][64];
/* run 146 / hook 0209 (pg_find): when a species is found per location, [t]: 0 morning, 1 day, 2 night (land), 3 any time (water, rock smash); kept here like P3DS_PsK* */
uint8_t P3DS_PsTm[100][4][64];
void *P3DS_PgAlloc(int key, uint32_t size) {
  if (key < 0 || key > 255 || size < 16 || size > 64 * 1024) return 0;
  if (key >= 200 && key <= 203) { if (size > 0x4000) return 0; if (!pgRaw[key]) pgRaw[key] = (uint8_t *)malloc(0x4000); pgRawSz[key] = pgRaw[key] ? size : 0; return pgRaw[key]; }
  free(pgRaw[key]); pgRaw[key] = (uint8_t *)malloc(size); pgRawSz[key] = pgRaw[key] ? size : 0; return pgRaw[key]; }
}
/* compatibility for the run 10 decode (members 10/11/20) */
#define pgRawM(m) pgRaw[m]
#define pgRawSzM(m) pgRawSz[m]

enum { kPgW = 47 * 8, kPgH = 20 * 8, kPgN = 100 };
struct SPg {
  uint32_t img[kPgW * kPgH]; uint8_t lit[47 * 20]; bool decoded, failed;
  uint8_t visited[16]; uint32_t tid; bool tidKnown, dirty;
  int sel = -1; int viewX = -1; int lastLoc = -2;
  int zoom = 1; int viewY = -1; int panAcc = 0; /* zoom 1 = the normal map; viewY is the map row at the top when zoom is 2 */
  int zmX0 = 0, zmY0 = 0, zmX1 = 0, zmY1 = 0, zpX0 = 0, zpY0 = 0, zpX1 = 0, zpY1 = 0; /* minus / plus, DS px; x1==x0 means hidden */
  int markX, markY; bool markOk, gameMarker; /* run 11: marker position (DS px) for the native overlay, and whether the game's head is used */
  bool ov; int ovScroll; int ovRows[kPgN]; int ovN, ovListY0, ovListY1; /* run 13: progress overview (menu-dots button) */
  struct { short x, y; uint8_t k; } ico[48]; int nIco; bool gameIco; /* small stat icons drawn by the native overlay */
  int dotsX0, dotsY0, dotsX1, dotsY1;
  bool achOpen; int achScroll, achX0, achY0, achX1, achY1; /* achievements list (achievements.c), button left of the dots */
  int achUpX0, achUpY0, achUpX1, achUpY1, achDnX0, achDnY0, achDnX1, achDnY1, achPage, achMax; /* UP/DN page the list; achMax is the clamp */
  int flyX0, flyY0, flyX1, flyY1; /* hook 0228 (pg_flytap): the FLY chip on the open info card, 0 size = none (screen px) */
  int find, findTm; /* run 146 pg_find: species being looked for (0 none) and the time filter (0 any, 1 morning, 2 day, 3 night) */
  int page; /* run 16: info card page, 0 = info, 1 = wild Pokemon list (tap the card to flip) */
  bool glide, moving; /* run 16: view slides to the player's new location instead of jumping; moving = still sliding (redraw) */
  int kantoX = -2, kantoAlways = -1; bool kantoOpen; uint8_t hide[47 * 20]; /* run 16: Kanto hidden until a Kanto place is visited */
};
static inline bool pgVis(const SPg &p, int i) { return i >= 0 && i < kPgN && (p.visited[i >> 3] >> (i & 7)) & 1; }
static inline void pgSpecRect(int i, int *x, int *y, int *w, int *h) {
  const SS3DS_PgSpec &l = sLocationSpecs[i]; *x = l.x; *y = l.y; *w = l.bits & 15; *h = (l.bits >> 4) & 15; if (*w < 1) *w = 1; if (*h < 1) *h = 1; }
/* Screen pixel whose centre samples DS pixel ds. The bottom picture is nearest-doubled into a 512-wide image
   (DS pixel ds fills texels [2*ds, 2*ds+2)) and then drawn linearly across 320 px. Truncating ds*5/4 matches
   that when ds%4 is 0 or 1. When ds%4 is 2 the sample sits on the texel edge, and when it is 3 the sample lands
   a full screen pixel into the previous pixel, so the head crawls against the town while the view recentres.
   n = round((2*ds+1)*5/8 - 1/2) = (2*ds+1)*5/8. */
static int pgDsToNative(int ds) { int n = (2 * ds + 1) * 5 / 8; return n < 0 ? 0 : n; }
static int pgMapZoom(); /* 1 or 2 */
static bool pgRoamSpr();
static int pgMapLimit(const SPg &p, int visW) { return !p.kantoOpen && p.kantoX > 0 && p.kantoX * 8 > visW ? p.kantoX * 8 : kPgW; }
static int pgClampLeft(const SPg &p, int left, int visW) { const int lim = pgMapLimit(p, visW); if (left > lim - visW) left = lim - visW; if (left < 0) left = 0; return left; }
/* panel interior pgDraw actually paints (the old tap math was 1 or 2 px off this, and the sign flipped with the name strip) */
static void pgInner(int *X0, int *Y0, int *X1, int *Y1) {
  const int nh = blNameH(); *X0 = kBlPanX0 + 2; *Y0 = kBlPanY0 + 15 + nh + 2; *X1 = kBlPanX1 - 2; *Y1 = kBlPanY1 - 2; }
static void pgTouchMap(const SPg &p, int tx, int ty, int *mx, int *my) {
  int X0, Y0, X1, Y1; pgInner(&X0, &Y0, &X1, &Y1); const int z = p.zoom < 2 ? 1 : 2; const int ah = Y1 - Y0;
  if (z <= 1) { const int oy = Y0 + (ah - kPgH) / 2; *mx = tx - X0 + p.viewX; *my = ty - oy; }
  else { *mx = p.viewX + (tx - X0) / z; *my = p.viewY + (ty - Y0) / z; } }
/* point marker: at 2x the +z/2 lands on the centre of that map pixel's block. At 1x it is the old DS pixel. */
static void pgToDs(const SPg &p, int X0, int Y0, int oy, int mx, int my, int *sx, int *sy) {
  const int z = p.zoom < 2 ? 1 : 2;
  if (z <= 1) { *sx = X0 + mx - p.viewX; *sy = oy + my; }
  else { *sx = X0 + (mx - p.viewX) * z + z / 2; *sy = Y0 + (my - p.viewY) * z + z / 2; } }
static void pgToDsCorner(const SPg &p, int X0, int Y0, int oy, int mx, int my, int *sx, int *sy) {
  const int z = p.zoom < 2 ? 1 : 2;
  if (z <= 1) { *sx = X0 + mx - p.viewX; *sy = oy + my; }
  else { *sx = X0 + (mx - p.viewX) * z; *sy = Y0 + (my - p.viewY) * z; } }
static void pgCenterOn(SPg &p, int mapCx, int mapCy, int aw, int ah) {
  p.zoom = pgMapZoom(); const int z = p.zoom < 2 ? 1 : 2; int visW = aw / z; if (visW < 1) visW = 1;
  p.viewX = pgClampLeft(p, mapCx - visW / 2, visW); p.glide = false;
  if (z > 1) { int visH = ah / z; if (visH < 1) visH = 1; int top = mapCy - visH / 2; if (top > kPgH - visH) top = kPgH - visH; if (top < 0) top = 0; p.viewY = top; } }

/* find a Nitro section by its 4-byte magic (stored reversed in the file) */
static const uint8_t *pgSect(const uint8_t *d, uint32_t n, const char *m) {
  for (uint32_t i = 16; i + 8 <= n; i += 4) if (!memcmp(d + i, m, 4)) return d + i; return 0; }
static inline uint32_t pgU32(const uint8_t *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }
static inline uint16_t pgU16(const uint8_t *p) { return (uint16_t)(p[0] | p[1] << 8); }

/* decode once the hook has delivered the three members; returns true when the picture is ready */
static bool pgDecode(SPg &p) {
  if (p.decoded) return true; if (p.failed || !P3DS_PgReady) return false;
  const uint8_t *chr = pgSect(pgRaw[10], pgRawSz[10], "RAHC"), *scr = pgSect(pgRaw[11], pgRawSz[11], "NRCS"), *pal = pgSect(pgRaw[20], pgRawSz[20], "TTLP");
  if (!chr || !scr || !pal) { p.failed = true; return false; }
  const uint8_t *C = chr + 8 + pgU32(chr + 28); const uint32_t cn = pgU32(chr + 24);
  const int sw = pgU16(scr + 8) / 8; const uint8_t *S = scr + 20;
  const uint8_t *P = pal + 8 + pgU32(pal + 20); const uint32_t pn = pgU32(pal + 16) / 2;
  if (sw < 47 || cn < 64 || pn < 16 || C + cn > pgRaw[10] + pgRawSz[10]) { p.failed = true; return false; }
  for (int ty = 0; ty < 20; ty++) for (int tx = 0; tx < 47; tx++) {
    const uint16_t e = pgU16(S + 2 * (ty * sw + tx)); const uint32_t t = e & 1023; const bool fx = e >> 10 & 1, fy = e >> 11 & 1;
    for (int y = 0; y < 8; y++) for (int x = 0; x < 8; x++) {
      const int sx = fx ? 7 - x : x, sy = fy ? 7 - y : y; const uint32_t o = t * 64 + sy * 8 + sx; const int v = o < cn ? C[o] : 0;
      uint32_t c = SUI_RGB(0, 60, 46); /* index 0 = clear: the Pokegear's dark backdrop */
      if (v && (uint32_t)v < pn) { const uint16_t k = pgU16(P + 2 * v); c = SUI_RGB((k & 31) * 255 / 31, (k >> 5 & 31) * 255 / 31, (k >> 10 & 31) * 255 / 31); }
      p.img[(ty * 8 + y) * kPgW + tx * 8 + x] = c; } }
  p.decoded = true; return true; }

static void pgRelight(SPg &p) {
  memset(p.lit, 0, sizeof p.lit);
  for (int i = 0; i < kPgN; i++) if (pgVis(p, i)) { int x, y, w, h; pgSpecRect(i, &x, &y, &w, &h);
    for (int yy = y; yy < y + h && yy < 20; yy++) for (int xx = x; xx < x + w && xx < 47; xx++) if (xx >= 0 && yy >= 0) p.lit[yy * 47 + xx] = 1; } }

static void pgLoadVisited(SPg &p, uint32_t tid, const char *path) {
  memset(p.visited, 0, sizeof p.visited); p.tid = tid; p.tidKnown = true;
  FILE *f = fopen(path, "rb"); uint8_t b[24];
  if (f) { if (fread(b, 1, 24, f) == 24 && !memcmp(b, "VB01", 4) && pgU32(b + 4) == tid) memcpy(p.visited, b + 8, 16); fclose(f); }
  pgRelight(p); }
static bool pgSaveVisited(SPg &p, const char *path) {
  FILE *f = fopen(path, "wb"); if (!f) return false; uint8_t b[24]; memcpy(b, "VB01", 4);
  for (int i = 0; i < 4; i++) b[4 + i] = (uint8_t)(p.tid >> (8 * i)); memcpy(b + 8, p.visited, 16);
  const bool ok = fwrite(b, 1, 24, f) == 24; fclose(f); if (ok) p.dirty = false; return ok; }
/* returns 1 when a location was lit for the first time */
static int pgMark(SPg &p, int loc) {
  if (loc < 0 || loc >= kPgN || pgVis(p, loc)) return 0; p.visited[loc >> 3] |= 1 << (loc & 7); p.dirty = true; pgRelight(p); return 1; }
static int pgCount(const SPg &p) { int n = 0; for (int i = 0; i < kPgN; i++) n += pgVis(p, i); return n; }

/* location under map pixel (mx,my), smallest rect wins */
static int pgHitLoc(int mx, int my) {
  int best = -1, ba = 999; const int tx = mx / 8, ty = my / 8;
  for (int i = 0; i < kPgN; i++) { int x, y, w, h; pgSpecRect(i, &x, &y, &w, &h);
    if (tx >= x && tx < x + w && ty >= y && ty < y + h && w * h < ba) { ba = w * h; best = i; } }
  return best; }

/* simple word wrap into lines of at most `room` pixels */
static int pgWrap(const SUiText &T, const char *s, int room, char out[][64], int maxLines) {
  int n = 0; char line[64] = ""; const char *p = s;
  while (*p && n < maxLines) { char word[40]; int k = 0; while (*p == ' ') p++; while (*p && *p != ' ' && k < 39) word[k++] = *p++; word[k] = 0; if (!k) break;
    char tryl[100]; snprintf(tryl, sizeof tryl, "%s%s%s", line, line[0] ? " " : "", word);
    if (T.width(tryl) <= room || !line[0]) snprintf(line, sizeof line, "%s", tryl);
    else { snprintf(out[n++], 64, "%s", line); snprintf(line, sizeof line, "%s", word); } }
  if (line[0] && n < maxLines) snprintf(out[n++], 64, "%s", line); return n; }
/* shorten `s` into `out` so it fits maxW; a cut string ends with ".." */
static void pgFit(const SUiText &T, char *out, int sz, const char *s, int maxW) {
  if (sz < 1) return;
  snprintf(out, (size_t)sz, "%s", s ? s : "");
  if (maxW <= 0) { out[0] = 0; return; }
  if (T.width(out) <= maxW) return;
  int k = (int)strlen(out);
  while (k > 0) {
    k--;
    if (k + 3 <= sz) { out[k] = '.'; out[k + 1] = '.'; out[k + 2] = 0; }
    else out[k] = 0;
    if (out[0] && T.width(out) <= maxW) return;
    out[k] = 0;
  }
}

/* run 14: labelled stats ("Caught 2/9", "Trainers 1/3", "Items 1/2", "Hidden 0/2", optional "Seen"), two per line in
   columns at x and x + col; only stats the place has. Returns the number of lines drawn. */
static int pgStats(uint32_t *b, const SUiText &T, SPg &p, int x, int y, int loc, uint32_t fg, uint32_t sh, int col, bool seen = false) {
  const uint16_t *s = P3DS_PsStat[loc]; struct { const char *l; int a, n; } it[5]; int k = 0;
  if (s[0]) it[k++] = { "Caught", s[1], s[0] }; if (seen && s[0]) it[k++] = { "Seen", s[2], s[0] };
  if (s[3]) it[k++] = { "Trainers", s[4], s[3] }; if (s[5]) it[k++] = { "Items", s[6], s[5] }; if (s[7]) it[k++] = { "Hidden", s[8], s[7] };
  if (!k) { T.draw(b, x, y, "Nothing to collect here", SUI_RGB(120, 134, 138), sh); return 1; }
  for (int i = 0; i < k; i++) { char t[32]; snprintf(t, sizeof t, "%s %d/%d", it[i].l, it[i].a, it[i].n);
    T.draw(b, x + (i & 1) * col, y + (i / 2) * 10, t, it[i].a >= it[i].n ? SUI_RGB(40, 120, 64) : fg, sh); }
  return (k + 1) / 2; }
#include "achievements.h"
static void pgDots(uint32_t *b, SPg &p, int X1, int Y0) {
  p.dotsX0 = X1 - 20; p.dotsY0 = Y0 + 2; p.dotsX1 = X1 - 2; p.dotsY1 = Y0 + 12;
  p.achX1 = p.dotsX0 - 4; p.achX0 = p.achX1 - 16; p.achY0 = p.dotsY0; p.achY1 = p.dotsY1;
  for (int y = p.achY0; y < p.achY1; y++) for (int x = p.achX0; x < p.achX1; x++) {
    const bool edge = y == p.achY0 || y == p.achY1 - 1 || x == p.achX0 || x == p.achX1 - 1;
    suiPx(b, x, y, edge ? kBlNavy : SUI_RGB(246, 214, 96)); }
  if (P3DS_AchUnviewed()) for (int y = p.achY0 - 1; y < p.achY0 + 2; y++) for (int x = p.achX1 - 3; x < p.achX1 + 0; x++) suiPx(b, x, y, SUI_RGB(220, 40, 40));
  for (int y = p.dotsY0; y < p.dotsY1; y++) for (int x = p.dotsX0; x < p.dotsX1; x++) {
    const bool edge = y == p.dotsY0 || y == p.dotsY1 - 1 || x == p.dotsX0 || x == p.dotsX1 - 1;
    suiPx(b, x, y, edge ? kBlNavy : suiMix(SUI_RGB(248, 248, 252), SUI_RGB(200, 230, 218), y - p.dotsY0, 10)); }
  for (int d = 0; d < 3; d++) for (int yy = 0; yy < 2; yy++) for (int xx = 0; xx < 2; xx++) suiPx(b, p.dotsX0 + 4 + d * 4 + xx, p.dotsY0 + 4 + yy, SUI_RGB(7, 33, 54)); }
/* contest_helper (hook 0229): Bug-Catching Contest card over the top of the panel body while a contest runs. Time is the game's own whole-minute counter
   (the contest ends when 20 minutes are used), so "min left" can be up to a minute generous. Drawn only for blMode < 0 and no Pokegear pane. */
static void ctCardDraw(uint32_t *b, const SUiText &T, int X0, int Y0, int X1) {
  if (!P3DS_ContestOn() || !P3DS_Ct.active) return;
  const P3DS_ContestInfo &c = P3DS_Ct; char t[64]; const int left = 20 - c.elapsed, x0 = X0 + 2, x1 = X1 - 2, y0 = Y0 + 2;
  mmFill(b, x0, y0, x1, y0 + 38, SUI_RGB(236, 244, 226)); mmFill(b, x0, y0, x1, y0 + 11, kBlNavy); mmFrame(b, x0, y0, x1, y0 + 38, kBlNavy);
  suiSDraw(T, b, x0 + 4, y0 + 1, "BUG-CATCHING CONTEST", kBlSilverHi, kBlNavy);
  const uint32_t ink = SUI_RGB(20, 40, 24), sh = SUI_RGB(236, 244, 226), warn = SUI_RGB(176, 40, 24);
  if (left > 0) snprintf(t, sizeof t, "TIME  %d min left", left); else snprintf(t, sizeof t, "TIME UP");
  suiSDraw(T, b, x0 + 4, y0 + 13, t, left <= 3 ? warn : ink, sh);
  snprintf(t, sizeof t, "BALLS  %d / 20", c.balls); suiSDraw(T, b, x0 + 4, y0 + 24, t, c.balls <= 3 ? warn : ink, sh);
  if (c.species > 0) { snprintf(t, sizeof t, "%s Lv%d", kP3dsSpecies[c.species], c.level); suiSDraw(T, b, x0 + 96, y0 + 24, t, ink, sh); }
  else suiSDraw(T, b, x0 + 96, y0 + 24, "none yet", SUI_RGB(100, 110, 100), sh);
}
/* run 45: tracker block (roaming Pokemon + Day-Care) at the top of the overview; returns the height used, 0 when off */
static const uint32_t kTrkCol[4] = { SUI_RGB(232, 184, 16), SUI_RGB(224, 72, 40), SUI_RGB(240, 96, 144), SUI_RGB(46, 208, 155) };
static int pgTracker(uint32_t *b, const SUiText &T, int X0, int Y, int X1, uint32_t ink, uint32_t fg, uint32_t sh) {
  if (!P3DS_TrkOn()) return 0; int y = Y;
  T.draw(b, X0 + 4, y, "ROAMING NOW", ink, sh); y += 10;
  if (!P3DS_TrkReady) { T.draw(b, X0 + 4, y, "Take a step to look.", fg, sh); return y + 12 - Y; }
  int shown = 0;
  for (int i = 0; i < 4; i++) { const P3DS_TrkRoamer &r = P3DS_TrkRoam[i]; if (!r.active || !r.species) continue; char t[64];
    const char *l = r.loc >= 0 && r.loc < kPgN && P3DS_PgName[r.loc][0] ? P3DS_PgName[r.loc] : "somewhere";
    for (int yy = 0; yy < 7; yy++) for (int xx = 0; xx < 7; xx++) suiPx(b, X0 + 5 + xx, y + 1 + yy, xx == 0 || yy == 0 || xx == 6 || yy == 6 ? SUI_RGB(7, 33, 54) : kTrkCol[i]);
    snprintf(t, sizeof t, "%s Lv%d: %s", kP3dsSpecies[r.species], r.level, l); T.draw(b, X0 + 15, y, t, fg, sh); y += 10; shown++; }
  if (!shown) { T.draw(b, X0 + 4, y, "None out yet.", fg, sh); y += 10; }
  y += 2; char t[64];
  T.draw(b, X0 + 4, y, "DAY-CARE", ink, sh); y += 10; const P3DS_TrkDaycare &d = P3DS_TrkDc;
  if (d.n == 0) { T.draw(b, X0 + 4, y, "Empty.", fg, sh); y += 10; }
  else if (d.n == 1) { T.draw(b, X0 + 4, y, "One Pokemon: needs a partner.", fg, sh); y += 10; }
  else {
    snprintf(t, sizeof t, "%s + %s", kP3dsSpecies[d.sp[0]], kP3dsSpecies[d.sp[1]]); T.draw(b, X0 + 4, y, t, fg, sh); y += 10;
    if (d.compat > 0) snprintf(t, sizeof t, "Egg chance %d%% per 256 steps", d.compat); else snprintf(t, sizeof t, "These two cannot breed.");
    T.draw(b, X0 + 4, y, t, d.compat > 0 ? fg : SUI_RGB(140, 100, 60), sh); y += 10;
    if (d.compat > 0) {
      if (d.egg) T.draw(b, X0 + 4, y, "An egg is waiting!", SUI_RGB(40, 120, 64), sh);
      else { snprintf(t, sizeof t, "Next egg roll in %d steps", d.toCheck < 0 ? 0 : d.toCheck); T.draw(b, X0 + 4, y, t, fg, sh); }
      y += 10;
      T.draw(b, X0 + 4, y, d.masuda ? "Masuda: YES, shiny 5/8192 (1/1638)" : "Masuda: no, shiny 1/8192", d.masuda ? SUI_RGB(40, 120, 64) : fg, sh); y += 10; } }
  return y + 2 - Y; }
/* the overview: whole-map totals, then every visited location with its stats (drag scrolls, tap opens its card) */
static void pgOverview(uint32_t *b, const SUiText &T, SPg &p, int X0, int Y0, int X1, int Y1) {
  const uint32_t ink = SUI_RGB(7, 33, 54), sh = SUI_RGB(236, 240, 248);
  for (int y = Y0; y < Y1; y++) for (int x = X0; x < X1; x++) suiPx(b, x, y, suiMix(SUI_RGB(244, 246, 252), SUI_RGB(212, 232, 224), y - Y0, Y1 - Y0));
  char t[64]; int n = 0; for (int i = 0; i < kPgN; i++) n += pgVis(p, i);
  T.draw(b, X0 + 4, Y0 + 2, "PROGRESS", ink, sh); snprintf(t, sizeof t, "%d/%d VISITED", n, kPgN); T.draw(b, X1 - 24 - T.width(t), Y0 + 2, t, SUI_RGB(18, 26, 38), sh);
  if (!P3DS_PsReady) { T.draw(b, X0 + 4, Y0 + 16, "Take a step to count.", SUI_RGB(18, 26, 38), sh); p.ovN = 0; return; }
  const int dy = pgTracker(b, T, X0, Y0 + 13, X1, ink, SUI_RGB(18, 26, 38), sh);
  T.draw(b, X0 + 4, Y0 + 13 + dy, "Whole map:", SUI_RGB(18, 26, 38), sh);
  const int tl = pgStats(b, T, p, X0 + 4, Y0 + 24 + dy, 100, ink, sh, (X1 - X0) / 2, true); const int hy = Y0 + 27 + dy + tl * 10;
  for (int x = X0; x < X1; x++) suiPx(b, x, hy, SUI_RGB(70, 150, 120));
  p.ovN = 0; for (int i = 0; i < kPgN; i++) if (pgVis(p, i)) { bool dup = false; for (int j = 0; j < p.ovN; j++) if (!strcmp(P3DS_PgName[p.ovRows[j]], P3DS_PgName[i])) dup = true; if (!dup) p.ovRows[p.ovN++] = i; }
  p.ovListY0 = hy + 2; p.ovListY1 = Y1; const int rowH = 32, maxS = p.ovN * rowH - (Y1 - p.ovListY0); if (p.ovScroll > maxS) p.ovScroll = maxS; if (p.ovScroll < 0) p.ovScroll = 0;
  if (!p.ovN) T.draw(b, X0 + 4, p.ovListY0 + 4, "Visit places to list them here.", SUI_RGB(18, 26, 38), sh);
  for (int r = 0; r < p.ovN; r++) { const int y = p.ovListY0 + r * rowH - p.ovScroll; if (y < p.ovListY0 || y + rowH > Y1) continue; const int l = p.ovRows[r];
    if (r & 1) for (int yy = y; yy < y + rowH; yy++) for (int x = X0; x < X1; x++) suiPx(b, x, yy, suiMix(b[yy * 256 + x], SUI_RGB(255, 255, 255), 1, 2));
    T.draw(b, X0 + 4, y + 1, P3DS_PgName[l][0] ? P3DS_PgName[l] : "?", ink, sh); pgStats(b, T, p, X0 + 10, y + 11, l, SUI_RGB(18, 26, 38), sh, (X1 - X0 - 10) / 2); }
  if (maxS > 0) { const int h = Y1 - p.ovListY0, bh = h * h / (h + maxS), by = p.ovListY0 + (h - bh) * p.ovScroll / maxS; for (int y = by; y < by + bh; y++) { suiPx(b, X1 - 3, y, SUI_RGB(70, 150, 120)); suiPx(b, X1 - 2, y, SUI_RGB(70, 150, 120)); } } }
/* overview row under DS y, -1 none */
static int pgOvHit(const SPg &p, int y) { if (!p.ov || y < p.ovListY0 || y >= p.ovListY1) return -1; const int r = (y - p.ovListY0 + p.ovScroll) / 32; return r >= 0 && r < p.ovN ? p.ovRows[r] : -1; }

/* ---- run 16: Kanto on the map only after you have been there (the real Pokegear shows it after the S.S. Aqua).
   Kanto places are recognised by their names (from the ROM via hook 0081); the hidden area starts at the westmost Kanto
   town's column, but places that are not Kanto (Route 26/27, Victory Road, Indigo Plateau, Mt. Silver...) stay visible even
   when they lie further east. The gate opens for good when a Kanto place is in visited.bin or you are standing in one. */
static bool pgIsKanto(int i) {
  if (i < 0 || i >= kPgN) return false; const char *n = P3DS_PgName[i]; char u[24]; int k = 0;
  for (; n[k] && k < 23; k++) u[k] = (char)(n[k] >= 'a' && n[k] <= 'z' ? n[k] - 32 : n[k]); u[k] = 0;
  static const char *const town[] = { "PALLET", "VIRIDIAN", "PEWTER", "CERULEAN", "VERMILION", "LAVENDER", "CELADON", "FUCHSIA", "SAFFRON", "CINNABAR",
    "SEAFOAM", "DIGLETT", "MT. MOON", "MT.MOON", "ROCK TUNNEL", "POWER PLANT" };
  for (const char *t : town) if (!strncmp(u, t, strlen(t))) return true;
  if (!strncmp(u, "ROUTE ", 6)) { const int r = atoi(u + 6); return r >= 1 && r <= 25; }
  return false; }
static bool pgIsKantoTown(int i) { return pgIsKanto(i) && strncmp(P3DS_PgName[i], "Route", 5) && strncmp(P3DS_PgName[i], "ROUTE", 5); }
/* recompute the gate; returns true when it changed (the caller logs and redraws) */
static bool pgKantoUpdate(SPg &p, int always) {
  if (p.kantoX == -2) { p.kantoX = -1; for (int i = 0; i < kPgN; i++) if (pgIsKantoTown(i)) { int x, y, w, h; pgSpecRect(i, &x, &y, &w, &h); if (p.kantoX < 0 || x < p.kantoX) p.kantoX = x; } }
  bool open = always || p.kantoX < 0 || pgIsKanto(P3DS_PgLoc); for (int i = 0; i < kPgN && !open; i++) if (pgIsKanto(i) && pgVis(p, i)) open = true;
  const bool changed = open != p.kantoOpen || always != p.kantoAlways || p.kantoX == -1; p.kantoOpen = open; p.kantoAlways = always;
  if (!changed) return false;
  memset(p.hide, 0, sizeof p.hide);
  if (!open) { for (int ty = 0; ty < 20; ty++) for (int tx = p.kantoX; tx < 47; tx++) p.hide[ty * 47 + tx] = 1;
    for (int pass = 0; pass < 2; pass++) for (int i = 0; i < kPgN; i++) { if (pgIsKanto(i) != (pass == 0)) continue; int x, y, w, h; pgSpecRect(i, &x, &y, &w, &h); /* Kanto rects hidden first, then non-Kanto rects shown */
      for (int yy = y; yy < y + h && yy < 20; yy++) for (int xx = x; xx < x + w && xx < 47; xx++) if (xx >= 0 && yy >= 0) p.hide[yy * 47 + xx] = pass == 0; } }
  if (p.kantoX == -1) p.kantoX = -3; /* "no Kanto town found" reported once */
  return true; }
static inline bool pgHiddenAt(const SPg &p, int mx, int my) { return mx >= 0 && my >= 0 && mx < kPgW && my < kPgH && p.hide[(my / 8) * 47 + mx / 8]; }
/* hook 0228: is this location a Fly point the player has unlocked (the game's own flag, published by the game thread)? */
static bool pgFlyUnlocked(int loc) { if (loc < 0 || loc >= kPgN) return false; const int m = sLocationSpecs[loc].mapId, n = P3DS_FlyN < 32 ? P3DS_FlyN : 32;
  for (int i = 0; i < n; i++) if (P3DS_FlyList[i] == m) return true; return false; }
static inline int pgCardH(const SPg &p) { return p.page ? 110 : 66; }
/* species in a location's wild set; fills `out` (up to max), returns the count, `*caught` = how many are caught */
static int pgWild(int loc, int *out, int max, int *caught) {
  int n = 0; *caught = 0; if (loc < 0 || loc >= kPgN) return 0;
  for (int sp = 1; sp <= 493; sp++) if (P3DS_PsSp[loc][sp >> 3] >> (sp & 7) & 1) { if (n < max) out[n] = sp; n++; if (P3DS_PsCg[loc][sp >> 3] >> (sp & 7) & 1) (*caught)++; }
  return n; }

/* ---- run 146: Pokedex encounter finder (pg_find). Tap a name on the wild list and the map pins every place that species lives, with a morning/day/night filter ---- */
static bool pgFindCfg(); /* render3ds.cpp */
static bool pgFlyCfg(); /* render3ds.cpp (hook 0228) */
static const char *const kPgFindTm[4] = { "ANY", "MORN", "DAY", "NITE" };
static int pgTimeNow() { const time_t now = time(0); struct tm tmv; const struct tm *lt = localtime_r(&now, &tmv); const int h = lt ? lt->tm_hour : 12; return h >= 4 && h < 10 ? 1 : h >= 10 && h < 20 ? 2 : 3; }
static bool pgTmHave() { for (int l = 0; l < 100; l++) for (int t = 0; t < 4; t++) for (int i = 0; i < 64; i++) if (P3DS_PsTm[l][t][i]) return true; return false; }
static bool pgFindAt(int loc, int sp, int tm, bool tmOk) {
  const uint8_t m = (uint8_t)(1 << (sp & 7)); const int o = sp >> 3;
  if (tm == 0 || !tmOk) return (P3DS_PsSp[loc][o] & m) != 0;
  return ((P3DS_PsTm[loc][tm - 1][o] | P3DS_PsTm[loc][3][o]) & m) != 0; }
static int pgFindCount(const SPg &p) { if (p.find <= 0 || p.find > 493) return 0; const bool ok = pgTmHave(); int n = 0; for (int l = 0; l < kPgN && l < 100; l++) n += pgFindAt(l, p.find, p.findTm, ok); return n; }
static void pgWildLayout(int aw, int ch, int wn, int *cw, int *rows, int *shown) { *cw = (aw - 8) / 3; *rows = (ch - 18) / 10; const int slots = *rows * 3; *shown = wn > slots ? slots - 1 : wn; }
/* species under a tap on the wild list page (card spans the panel's bottom), 0 = none */
static int pgWildHit(const SPg &p, int x, int y) {
  if (p.sel < 0 || !p.page) return 0; const int X0 = kBlPanX0 + 2, aw = (kBlPanX1 - 2) - X0, ch = pgCardH(p), cy0 = (kBlPanY1 - 2) - ch;
  int wl[64], cg = 0; const int wn = pgWild(p.sel, wl, 64, &cg); if (!wn) return 0; int cw, rows, shown; pgWildLayout(aw, ch, wn, &cw, &rows, &shown);
  const int col = (x - (X0 + 4)) / cw, row = (y - (cy0 + 16)) / 10; if (x < X0 + 4 || y < cy0 + 16 || col < 0 || col > 2 || row < 0 || row >= rows) return 0;
  const int i = col * rows + row; return i < shown ? wl[i] : 0; }
/* finder strip hit: 1 = clear (X), 2..5 = time filter 0..3 */
static int pgFindHit(const SPg &p, int Y0, int x, int y) {
  if (p.find <= 0) return 0; const int X0 = kBlPanX0 + 2, X1 = kBlPanX1 - 2, tx = X0 + 42, cw = (X1 - tx - 2) / 4;
  if (x >= X1 - 17 && x < X1 && y >= Y0 && y < Y0 + 16) return 1;
  if (y >= Y0 + 26 && y < Y0 + 39) { const int i = (x - tx) / cw; if (x >= tx && i >= 0 && i < 4) return 2 + i; }
  return 0; }
static void pgFindPins(uint32_t *b, const SPg &p, int X0, int Y0, int X1, int oy) {
  const bool ok = pgTmHave();
  for (int l = 0; l < 100 && l < kPgN; l++) { if (!pgFindAt(l, p.find, p.findTm, ok)) continue; int x, y, w, h; pgSpecRect(l, &x, &y, &w, &h);
    const int mx = x * 8 + w * 4, my = y * 8 + h * 4; if (pgHiddenAt(p, mx, my)) continue; int px, py; pgToDs(p, X0, Y0, oy, mx, my, &px, &py); if (px < X0 + 4 || px >= X1 - 4) continue;
    for (int yy = -4; yy <= 4; yy++) for (int xx = -4; xx <= 4; xx++) { const int d = abs(xx) + abs(yy); if (d <= 4) suiPx(b, px + xx, py + yy, d == 4 ? SUI_RGB(255, 255, 255) : d == 3 ? SUI_RGB(120, 16, 16) : SUI_RGB(236, 56, 48)); } } }
/* HGSS menu icons use tiled 4bpp graphics and a separate bank of 16-colour palettes. */
extern "C" int P3DS_RomNarcMember(const char *path, int member, unsigned char *dst, unsigned cap);
extern "C" uint32_t GetMonIconNaixEx(uint32_t species, int isEgg, uint32_t form);
extern "C" uint8_t GetMonIconPaletteEx(uint32_t species, uint32_t form, uint32_t isEgg);

static bool pgMonIcon(int sp, uint32_t *dst, bool speciesColours = false) {
  memset(dst, 0, 32 * 32 * sizeof(*dst));
  if (sp <= 0 || sp > 493) return false;

  static uint8_t graphics[2048], palettes[2048];
  const int member = GetMonIconNaixEx(sp, 0, 0);
  const unsigned bank = speciesColours ? GetMonIconPaletteEx(sp, 0, 0) : 0;
  const int graphicsSize = P3DS_RomNarcMember("a/0/2/0", member, graphics, sizeof graphics);
  const int paletteSize = P3DS_RomNarcMember("a/0/2/0", 0, palettes, sizeof palettes);
  if (graphicsSize <= 0 || paletteSize <= 0) return false;

  const uint8_t *chr = pgSect(graphics, graphicsSize, "RAHC");
  const uint8_t *pal = pgSect(palettes, paletteSize, "TTLP");
  if (!chr || !pal || graphics + graphicsSize - chr < 32 || palettes + paletteSize - pal < 24)
    return false;

  const uint64_t pixelsOffset = (chr - graphics) + 8ull + pgU32(chr + 28);
  const unsigned bankOffset = bank * 32;
  const uint64_t coloursOffset = (pal - palettes) + 8ull + pgU32(pal + 20) + bankOffset;
  if (pgU32(chr + 24) < 512 || pixelsOffset + 512 > (unsigned)graphicsSize ||
      bankOffset + 32 > pgU32(pal + 16) || coloursOffset + 32 > (unsigned)paletteSize)
    return false;

  uint32_t colours[16] = {};
  for (int i = 1; i < 16; i++) {
    const uint16_t colour = pgU16(palettes + coloursOffset + i * 2);
    const unsigned r = colour & 31, g = (colour >> 5) & 31, b = (colour >> 10) & 31;
    if (speciesColours) {
      colours[i] = SUI_RGB((r << 3) | (r >> 2), (g << 3) | (g >> 2), (b << 3) | (b >> 2));
    } else {
      const unsigned brightness = (r * 30 + g * 59 + b * 11) / 100;
      colours[i] = suiMix(SUI_RGB(0, 70, 54), SUI_RGB(250, 250, 255), brightness, 31);
    }
    colours[i] |= 0xFF000000u;
  }

  const uint8_t *pixels = graphics + pixelsOffset;
  for (int tile = 0; tile < 16; tile++) {
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        const unsigned index = (pixels[tile * 32 + y * 4 + x / 2] >> (4 * (x & 1))) & 15;
        dst[((tile / 4) * 8 + y) * 32 + (tile % 4) * 8 + x] = colours[index];
      }
    }
  }
  return true;
}
static uint32_t pgFindIco[32 * 32]; static int pgFindIcoSp = -1; static bool pgFindIcoOk;
static void pgFindIconLoad(int sp) {
  if (pgFindIcoSp == sp) return; pgFindIcoSp = sp; pgFindIcoOk = pgMonIcon(sp, pgFindIco);
  PSPNativeMemLog("[FIND] icon sp=%d -> %s", sp, pgFindIcoOk ? "ok" : "missing (card has no sprite)"); }
/* one 16x16 roamer icon, cached per slot. False = caller draws the letter. Loaded only when the species changes. */
static uint32_t pgRoamPx[4][16 * 16]; static int pgRoamSp[4] = { -1, -1, -1, -1 }; static int pgRoamOk[4];
static bool pgRoamBlit(uint32_t *b, int slot, int sp, int cx, int cy, int X0, int Y0, int X1, int Y1) {
  if (slot < 0 || slot > 3 || sp <= 0 || sp > 493) return false;
  if (pgRoamSp[slot] != sp) { pgRoamSp[slot] = sp; uint32_t big[32 * 32]; pgRoamOk[slot] = pgMonIcon(sp, big, true); memset(pgRoamPx[slot], 0, sizeof pgRoamPx[slot]);
    if (pgRoamOk[slot]) for (int y = 0; y < 16; y++) for (int x = 0; x < 16; x++) pgRoamPx[slot][y * 16 + x] = big[(y * 2) * 32 + x * 2];
    PSPNativeMemLog("[TRACK] roamer sprite slot %d species %d %s", slot, sp, pgRoamOk[slot] ? "ok" : "missing, letter kept"); }
  if (!pgRoamOk[slot]) return false;
  for (int y = 0; y < 16; y++) for (int x = 0; x < 16; x++) { const uint32_t c = pgRoamPx[slot][y * 16 + x]; if (!(c >> 24)) continue;
    const int px = cx - 8 + x, py = cy - 8 + y; if (px < X0 || py < Y0 || px >= X1 || py >= Y1) continue; suiPx(b, px, py, c & 0xFFFFFF); }
  return true; }
/* card: sprite box left, name + count, X top right, time chips below (hit: pgFindHit) */
enum { kPgFindH = 40 };
static void pgFindBanner(uint32_t *b, const SUiText &T, const SPg &p, int X0, int Y0, int X1) {
  const uint32_t ink = SUI_RGB(255, 255, 255), bar = SUI_RGB(7, 33, 54), edge = SUI_RGB(190, 214, 206), chip = SUI_RGB(40, 130, 100), gold = SUI_RGB(246, 214, 96);
  for (int y = Y0; y < Y0 + kPgFindH; y++) for (int x = X0; x < X1; x++) suiPx(b, x, y, y == Y0 + kPgFindH - 1 || y == Y0 ? edge : suiMix(SUI_RGB(14, 120, 90), bar, y - Y0, kPgFindH));
  pgFindIconLoad(p.find);
  for (int y = 0; y < 34; y++) for (int x = 0; x < 34; x++) suiPx(b, X0 + 3 + x, Y0 + 3 + y, x == 0 || y == 0 || x == 33 || y == 33 ? edge : SUI_RGB(0, 60, 46));
  if (pgFindIcoOk) for (int y = 0; y < 32; y++) for (int x = 0; x < 32; x++) { const uint32_t c = pgFindIco[y * 32 + x]; if (c >> 24) suiPx(b, X0 + 4 + x, Y0 + 4 + y, c & 0xFFFFFF); }
  const int tx = X0 + 42; const int n = pgFindCount(p);
  for (int y = Y0 + 3; y < Y0 + 14; y++) for (int x = X1 - 15; x < X1 - 3; x++) suiPx(b, x, y, SUI_RGB(200, 60, 52));
  T.draw(b, X1 - 12, Y0 + 4, "X", ink, SUI_RGB(200, 60, 52));
  char t[48]; snprintf(t, sizeof t, "%s", kP3dsSpecies[p.find]); int k = (int)strlen(t); const int room = X1 - 18 - tx; while (k > 3 && T.width(t) > room) t[--k] = 0;
  T.draw(b, tx, Y0 + 4, t, ink, SUI_RGB(14, 120, 90));
  char cn[32]; snprintf(cn, sizeof cn, n == 1 ? "1 place" : "%d places", n); T.draw(b, tx, Y0 + 15, cn, n ? gold : SUI_RGB(200, 160, 160), SUI_RGB(12, 110, 84));
  const int now = pgTimeNow(), have = pgTmHave(), cw = (X1 - tx - 2) / 4;
  for (int i = 0; i < 4; i++) { const int x0 = tx + i * cw; const bool on = p.findTm == i, dim = i > 0 && !have;
    for (int y = Y0 + 27; y < Y0 + 38; y++) for (int x = x0; x < x0 + cw - 2; x++) suiPx(b, x, y, on ? gold : chip);
    T.draw(b, x0 + 3, Y0 + 28, kPgFindTm[i], on ? bar : dim ? SUI_RGB(140, 170, 160) : ink, on ? gold : chip);
    if (i == now) for (int x = x0 + 2; x < x0 + cw - 4; x++) suiPx(b, x, Y0 + 37, SUI_RGB(120, 230, 140)); } }

/* draw the map into the panel interior (X0,Y0)-(X1,Y1); the view follows the player's location unless panned */
static void pxMapPins(uint32_t *b, const SPg &p, int X0, int Y0, int X1, int oy); /* ss3ds_pgextra.h: phone contact pins */
static bool pxCardText(const SUiText &T, char *out, int sz, int loc, int maxW);
/* achievements list. The trophy and the dots buttons sit on the title row (pgDots, drawn after this), so the point total
   has its own band under them. Each entry is name + points, then a wrapped description, then the earned date on a smaller line.
   Scroll is in pixels. A line that crosses the list edge is still drawn: pixels outside the list are clipped, so a tall
   row cannot vanish and leave a blank gap before the next one. */
enum { kPgAchCtrl = 42, kPgAchLine = 16, kPgAchDate = 16 }; /* date is drawn with the 3/4 font; the slot stays a full line so the fallback font cannot run into the next row */
/* One sample of a finger that went down on the map panel. *drag is 0 until the finger has moved more than 16px
   (the tap gate). *prevY stays at the press y until that happens, so the sample that crosses the gate applies the
   whole stroke, not only the last step. Return value is added to achScroll: finger up (y smaller) is positive. */
static int pgAchDragDelta(int *drag, int *prevY, int y, int dist) {
  if (!*drag) { if (dist <= 16) return 0; *drag = 1; }
  int dy = y - *prevY; *prevY = y; return -dy; }
/* 1 = UP (toward the start of the list), 2 = DN (later rows), 0 = not on those chips */
static int pgAchBtnHit(const SPg &p, int x, int y) {
  if (!p.achOpen) return 0;
  if (p.achUpX1 > p.achUpX0 && x >= p.achUpX0 - 2 && x < p.achUpX1 + 2 && y >= p.achUpY0 - 2 && y < p.achUpY1 + 2) return 1;
  if (p.achDnX1 > p.achDnX0 && x >= p.achDnX0 - 2 && x < p.achDnX1 + 2 && y >= p.achDnY0 - 2 && y < p.achDnY1 + 2) return 2;
  return 0; }
static int pgAchWrap(const SUiText &T, const char *s, int room, char out[][64]) {
  int n = pgWrap(T, s, room, out, 2); if (n < 1) { out[0][0] = 0; return 0; }
  char probe[3][64]; if (n >= 2 && pgWrap(T, s, room, probe, 3) > 2) { char cut[68]; snprintf(cut, sizeof cut, "%s..", out[1]); pgFit(T, out[1], 64, cut, room); }
  for (int i = 0; i < n; i++) { char cut[64]; snprintf(cut, sizeof cut, "%s", out[i]); pgFit(T, out[i], 64, cut, room); }
  return n; }
static void pgAchPanel(uint32_t *b, const SUiText &T, SPg &p, int X0, int Y0, int X1, int Y1) {
  const uint32_t ink = SUI_RGB(7, 33, 54), sh = SUI_RGB(236, 240, 248), dim = SUI_RGB(100, 116, 112), gold = SUI_RGB(226, 168, 36), green = SUI_RGB(60, 170, 80);
  const uint32_t wh = SUI_RGB(255, 255, 255);
  for (int y = Y0; y < Y1; y++) for (int x = X0; x < X1; x++) suiPx(b, x, y, suiMix(SUI_RGB(252, 250, 240), SUI_RGB(226, 218, 190), y - Y0, Y1 - Y0));
  int mx = 0; const int tot = P3DS_AchTotal(-1, &mx); char t[64], nm[64];
  /* title row: words stop before the corner buttons. The glyph is 16px, so the totals band starts under it. */
  for (int y = Y0; y < Y0 + 17; y++) for (int x = X0; x < X1; x++) suiPx(b, x, y, suiMix(SUI_RGB(40, 170, 128), SUI_RGB(20, 140, 104), y - Y0, 17));
  pgFit(T, nm, sizeof nm, "ACHIEVEMENTS", (X1 - kPgAchCtrl) - (X0 + 4) - 4);
  T.draw(b, X0 + 4, Y0 + 1, nm, wh, SUI_RGB(7, 33, 54));
  /* totals band, full width, under the buttons */
  const int band0 = Y0 + 18, band1 = Y0 + 38;
  for (int y = band0; y < band1 && y < Y1; y++) for (int x = X0; x < X1; x++) suiPx(b, x, y, y == band0 ? SUI_RGB(12, 90, 70) : SUI_RGB(232, 244, 226));
  snprintf(t, sizeof t, "%d / %d G", tot, mx);
  p.achUpX0 = p.achUpX1 = p.achDnX0 = p.achDnX1 = 0;
  { const int bw = suiSWidth(T, "DN") + 10, gap = 3, dnX1 = X1 - 3, dnX0 = dnX1 - bw, upX1 = dnX0 - gap, upX0 = upX1 - bw;
    const int by0 = band0 + 2, by1 = band0 + 16;
    pgFit(T, nm, sizeof nm, t, upX0 - (X0 + 4) - 6);
    if (upX0 > X0 + 24 && by1 <= band1) {
      p.achUpX0 = upX0; p.achUpY0 = by0; p.achUpX1 = upX1; p.achUpY1 = by1;
      p.achDnX0 = dnX0; p.achDnY0 = by0; p.achDnX1 = dnX1; p.achDnY1 = by1;
      for (int y = by0; y < by1; y++) for (int x = upX0; x < upX1; x++) suiPx(b, x, y, (x == upX0 || x == upX1 - 1 || y == by0 || y == by1 - 1) ? kBlNavy : kBlPress);
      for (int y = by0; y < by1; y++) for (int x = dnX0; x < dnX1; x++) suiPx(b, x, y, (x == dnX0 || x == dnX1 - 1 || y == by0 || y == by1 - 1) ? kBlNavy : kBlPress);
      suiSDraw(T, b, upX0 + 5, by0 + 1, "UP", wh, kBlSelO); suiSDraw(T, b, dnX0 + 5, by0 + 1, "DN", wh, kBlSelO); } }
  if (band0 + 1 + kPgAchLine <= Y1) T.draw(b, X0 + 4, band0 + 1, nm, ink, SUI_RGB(232, 244, 226));
  { const int bx = X0 + 4, bw = X1 - X0 - 8, by = band0 + 17, fill = mx > 0 ? bw * tot / mx : 0;
    if (by + 2 < Y1) for (int x = 0; x < bw; x++) for (int yy = 0; yy < 3; yy++) suiPx(b, bx + x, by + yy, x < fill ? green : SUI_RGB(190, 204, 186)); }
  const int top = band1 + 1; int off = 0; const int textX = X0 + 22, textR = X1 - 6;
  const int clipX0 = suiClipX0, clipY0 = suiClipY0, clipX1 = suiClipX1, clipY1 = suiClipY1;
  suiClipX0 = X0; suiClipX1 = X1; suiClipY0 = top; suiClipY1 = Y1;
  for (int c = 0; c < ACH_CATS; c++) {
    int cm = 0; const int cs = P3DS_AchTotal(c, &cm); int cnt = 0, got = 0;
    for (int id = 0; id < ACH_COUNT; id++) if (P3DS_AchGet(id)->cat == c) { cnt++; if (P3DS_AchDate(id)) got++; }
    char left[48], right[32]; snprintf(left, sizeof left, "%s  %d/%d", P3DS_AchCatName(c), got, cnt); snprintf(right, sizeof right, "%dG of %dG", cs, cm);
    const int hy = top + off - p.achScroll; off += kPgAchLine;
    if (hy + kPgAchLine > top && hy < Y1) {
      for (int y = hy; y < hy + kPgAchLine; y++) for (int x = X0 + 2; x < X1 - 2; x++) suiPx(b, x, y, suiMix(SUI_RGB(218, 240, 232), SUI_RGB(190, 226, 212), y - hy, kPgAchLine));
      const int rw = T.width(right), room = textR - (X0 + 5) - 8;
      if (rw + 8 < room) { pgFit(T, nm, sizeof nm, left, room - rw - 8); T.draw(b, X0 + 5, hy, nm, ink, SUI_RGB(218, 240, 232)); T.draw(b, textR - rw, hy, right, dim, SUI_RGB(206, 232, 222)); }
      else { pgFit(T, nm, sizeof nm, left, room); T.draw(b, X0 + 5, hy, nm, ink, SUI_RGB(218, 240, 232)); } }
    for (int id = 0; id < ACH_COUNT; id++) { const P3DS_AchInfo *a = P3DS_AchGet(id); if (a->cat != c) continue;
      const int d = P3DS_AchDate(id); unsigned cu = 0, go = 0; const int prog = !d && P3DS_AchProgress(id, &cu, &go) && go > 1;
      char desc[2][64]; const int dl = pgAchWrap(T, a->desc, textR - textX, desc); const int lines = dl > 0 ? dl : 1;
      const int extra = (d || prog) ? kPgAchDate + 1 : 0;
      const int rh = 2 + kPgAchLine + lines * kPgAchLine + extra + 2;
      const int y = top + off - p.achScroll; off += rh;
      if (y + rh <= top || y >= Y1) continue;
      auto glyph = [&](int gy, int gh) { return gy + gh > top && gy < Y1; };
      if (glyph(y + 2, 12)) {
        const int ix = X0 + 5, iy = y + 2;
        for (int dy = 0; dy < 12; dy++) for (int dx = 0; dx < 12; dx++) { const int ex = dx - 6, ey = dy - 6; if (ex * ex + ey * ey > 33) continue;
          suiPx(b, ix + dx, iy + dy, d ? (ex * ex + ey * ey > 22 ? SUI_RGB(190, 138, 24) : (dy < 6 ? SUI_RGB(250, 214, 90) : gold)) : (ex * ex + ey * ey > 22 ? SUI_RGB(150, 150, 160) : SUI_RGB(190, 190, 198))); }
        if (d) { const int ck[7][2] = { {3,6},{4,7},{5,8},{6,7},{7,6},{8,5},{9,4} }; for (auto &q : ck) if (glyph(iy, 12)) { suiPx(b, ix + q[0] - 1, iy + q[1] - 1, SUI_RGB(70, 50, 8)); suiPx(b, ix + q[0] - 1, iy + q[1], SUI_RGB(70, 50, 8)); } }
        else { for (int dy = 5; dy < 9; dy++) for (int dx = 3; dx < 9; dx++) suiPx(b, ix + dx, iy + dy, SUI_RGB(110, 112, 124)); suiPx(b, ix + 4, iy + 4, SUI_RGB(110, 112, 124)); suiPx(b, ix + 7, iy + 4, SUI_RGB(110, 112, 124)); suiPx(b, ix + 5, iy + 3, SUI_RGB(110, 112, 124)); suiPx(b, ix + 6, iy + 3, SUI_RGB(110, 112, 124)); } }
      snprintf(t, sizeof t, "%dG", a->pts); const int pw = T.width(t), nameRoom = (textR - pw - 6) - textX;
      if (glyph(y + 1, kPgAchLine)) { pgFit(T, nm, sizeof nm, a->name, nameRoom > 8 ? nameRoom : 8); T.draw(b, textX, y + 1, nm, d ? ink : dim, sh); T.draw(b, textR - pw, y + 1, t, d ? gold : dim, sh); }
      for (int i = 0; i < lines; i++) { const int ly = y + 1 + kPgAchLine + i * kPgAchLine; if (glyph(ly, kPgAchLine) && desc[i][0]) T.draw(b, textX, ly, desc[i], dim, sh); }
      if (d && glyph(y + 1 + kPgAchLine + lines * kPgAchLine, kPgAchDate)) {
        snprintf(t, sizeof t, "%04d-%02d-%02d", d / 10000, d / 100 % 100, d % 100);
        suiSDraw(T, b, textX, y + 1 + kPgAchLine + lines * kPgAchLine, t, SUI_RGB(90, 110, 104), sh); }
      else if (prog && glyph(y + 1 + kPgAchLine + lines * kPgAchLine, kPgAchDate)) {
        snprintf(t, sizeof t, "%u/%u", cu > go ? go : cu, go); const int lw = suiSWidth(T, t), ly = y + 1 + kPgAchLine + lines * kPgAchLine;
        const int bw2 = 36, bx2 = textR - lw - bw2 - 4, f2 = (int)((unsigned long long)bw2 * (cu > go ? go : cu) / go);
        if (bx2 > textX) for (int x = 0; x < bw2; x++) for (int yy = 0; yy < 3; yy++) suiPx(b, bx2 + x, ly + 3 + yy, x < f2 ? green : SUI_RGB(200, 192, 168));
        suiSDraw(T, b, textR - lw, ly, t, dim, sh); } }
  }
  suiClipX0 = clipX0; suiClipY0 = clipY0; suiClipX1 = clipX1; suiClipY1 = clipY1;
  const int view = Y1 - top; int maxS = off - view; if (maxS < 0) maxS = 0;
  p.achMax = maxS; p.achPage = view > 24 ? view - 8 : 48;
  if (p.achScroll > maxS) p.achScroll = maxS; if (p.achScroll < 0) p.achScroll = 0; }

static void pgZoomChip(uint32_t *b, int x0, int y0, bool plus) {
  const int s = 12;
  for (int y = 0; y < s; y++) for (int x = 0; x < s; x++) { const bool edge = !x || !y || x == s - 1 || y == s - 1; suiPx(b, x0 + x, y0 + y, edge ? SUI_RGB(7, 33, 54) : SUI_RGB(232, 244, 226)); }
  for (int x = 3; x <= 8; x++) suiPx(b, x0 + x, y0 + 6, SUI_RGB(7, 33, 54));
  if (plus) for (int y = 3; y <= 9; y++) suiPx(b, x0 + 5, y0 + y, SUI_RGB(7, 33, 54)); }
static void pgDraw(uint32_t *b, const SUiText &T, SPg &p, int X0, int Y0, int X1, int Y1, bool blink) {
  const int aw = X1 - X0, ah = Y1 - Y0; p.nIco = 0; p.zmX1 = p.zpX1 = 0; p.zoom = pgMapZoom();
  if (p.achOpen) { p.markOk = false; pgAchPanel(b, T, p, X0, Y0, X1, Y1); pgDots(b, p, X1, Y0); return; }
  if (p.ov) { p.markOk = false; pgOverview(b, T, p, X0, Y0, X1, Y1); pgDots(b, p, X1, Y0); return; }
  const int z = p.zoom < 2 ? 1 : 2; int visW = aw / z; if (visW < 1) visW = 1;
  int cx = kPgW / 2, cy = kPgH / 2; if (P3DS_PgLoc >= 0) { int x, y, w, h; pgSpecRect(P3DS_PgLoc, &x, &y, &w, &h); cx = x * 8 + w * 4; cy = y * 8 + h * 4; }
  { static int prevZ = 1; if (z != prevZ && p.viewX >= 0) { const int oldW = aw / prevZ < 1 ? 1 : aw / prevZ; p.viewX = pgClampLeft(p, p.viewX + oldW / 2 - visW / 2, visW); if (z > 1) p.viewY = -1; p.glide = false; prevZ = z; } else prevZ = z; }
  const int mapW = pgMapLimit(p, visW); /* Kanto hidden: do not scroll into the empty east */
  int goal = cx - visW / 2; if (goal > mapW - visW) goal = mapW - visW; if (goal < 0) goal = 0;
  if (p.viewX < 0) { p.viewX = goal; p.glide = false; }
  else if (p.glide) { const int d = goal - p.viewX; if (d >= -1 && d <= 1) { p.viewX = goal; p.glide = false; } else { const int q = P3DS_FpsTweaks() >= 5 ? 2 : 4; /* level 5: half the repaints, double the step */ p.viewX += d / q ? d / q : (d > 0 ? 1 : -1); } }
  p.moving = p.glide;
  if (p.viewX > mapW - visW) p.viewX = mapW - visW; if (p.viewX < 0) p.viewX = 0;
  if (z <= 1) p.panAcc = 0;
  const int oy = Y0 + (ah - kPgH) / 2;
  if (z > 1 && (p.glide || p.viewY < 0)) { int visH = ah / z; if (visH < 1) visH = 1; int top = cy - visH / 2; if (top > kPgH - visH) top = kPgH - visH; if (top < 0) top = 0; p.viewY = top; }
  const uint32_t dimTo = SUI_RGB(0, 44, 34);
  for (int y = Y0; y < Y1; y++) {
    for (int x = X0; x < X1; x++) {
      int mx, my; if (z <= 1) { mx = x - X0 + p.viewX; my = y - oy; }
      else { mx = p.viewX + (x - X0) / z; my = p.viewY + (y - Y0) / z; if (mx >= kPgW) mx = kPgW - 1; if (my >= kPgH) my = kPgH - 1; if (mx < 0) mx = 0; if (my < 0) my = 0; }
      if (my < 0 || my >= kPgH || mx < 0 || mx >= kPgW) { suiPx(b, x, y, SUI_RGB(0, 60, 46)); continue; }
      if (p.hide[(my / 8) * 47 + mx / 8]) { suiPx(b, x, y, SUI_RGB(0, 60, 46)); continue; }
      const uint32_t c = p.img[my * kPgW + mx]; suiPx(b, x, y, p.lit[(my / 8) * 47 + mx / 8] ? c : suiMix(c, dimTo, 5, 8)); } }
  pxMapPins(b, p, X0, Y0, X1, oy);
  if (p.find > 0 && pgFindCfg()) pgFindPins(b, p, X0, Y0, X1, oy);
  /* selected location outline, then the player marker */
  if (p.sel >= 0) { int x, y, w, h; pgSpecRect(p.sel, &x, &y, &w, &h); int sx, sy, ex, ey; pgToDsCorner(p, X0, Y0, oy, x * 8, y * 8, &sx, &sy); pgToDsCorner(p, X0, Y0, oy, x * 8 + w * 8, y * 8 + h * 8, &ex, &ey);
    for (int k = 0; k < 2; k++) for (int xx = sx - k; xx < ex + k; xx++) { if (xx >= X0 && xx < X1) { suiPx(b, xx, sy - 1 - k, SUI_RGB(255, 224, 64)); suiPx(b, xx, ey + k, SUI_RGB(255, 224, 64)); } }
    for (int k = 0; k < 2; k++) for (int yy = sy - k; yy < ey + k; yy++) { if (sx - 1 - k >= X0) suiPx(b, sx - 1 - k, yy, SUI_RGB(255, 224, 64)); if (ex + k < X1) suiPx(b, ex + k, yy, SUI_RGB(255, 224, 64)); } }
  if (P3DS_PgLoc >= 0) { int x, y, w, h; pgSpecRect(P3DS_PgLoc, &x, &y, &w, &h); int px, py; pgToDs(p, X0, Y0, oy, x * 8 + w * 4, y * 8 + h * 4, &px, &py);
    p.markX = px; p.markY = py; p.markOk = px >= X0 + 3 && px < X1 - 3 && (p.sel < 0 || py < Y1 - pgCardH(p) - 6);
    if (px >= X0 + 3 && px < X1 - 3 && !p.gameMarker) mmPlayer(b, px, py, blink); } else p.markOk = false;
  /* roamers: letter square, or the species icon from the ROM when roamer_sprites is on. Same place, same hide rules. */
  if (P3DS_TrkOn() && P3DS_TrkReady) for (int i = 0; i < 4; i++) { const P3DS_TrkRoamer &r = P3DS_TrkRoam[i]; if (!r.active || r.loc < 0 || r.loc >= kPgN) continue;
    int x, y, w, h; pgSpecRect(r.loc, &x, &y, &w, &h); const int mx = x * 8 + w * 4, my = y * 8 + h * 4; if (pgHiddenAt(p, mx, my)) continue;
    int rx, ry; pgToDs(p, X0, Y0, oy, mx, my, &rx, &ry); const int inset = pgRoamSpr() ? 9 : 5;
    if (rx < X0 + inset || rx >= X1 - inset || ry < Y0 + inset || ry >= Y1 - inset) continue;
    if (!(pgRoamSpr() && pgRoamBlit(b, i, r.species, rx, ry, X0, Y0, X1, Y1))) {
      for (int yy = -4; yy <= 4; yy++) for (int xx = -4; xx <= 4; xx++) suiPx(b, rx + xx, ry + yy, xx == -4 || xx == 4 || yy == -4 || yy == 4 ? SUI_RGB(7, 33, 54) : kTrkCol[i]);
      static const char *const kIni[4] = { "R", "E", "L", "L" }; const char *ini = kIni[i]; T.draw(b, rx - T.width(ini) / 2, ry - 4, ini, SUI_RGB(30, 30, 40), kTrkCol[i]); }
    else { for (int k = 0; k < 3; k++) { suiPx(b, rx - 8 + k, ry - 8, kTrkCol[i]); suiPx(b, rx - 8, ry - 8 + k, kTrkCol[i]); } } }
  /* info card for the tapped location */
  p.flyX0 = p.flyY0 = p.flyX1 = p.flyY1 = 0;
  if (p.sel >= 0) { const int ch = pgCardH(p), cy0 = Y1 - ch; char ln[4][64];
    for (int y = cy0; y < Y1; y++) for (int x = X0; x < X1; x++) suiPx(b, x, y, suiMix(SUI_RGB(236, 240, 248), SUI_RGB(196, 228, 214), y - cy0, ch));
    for (int x = X0; x < X1; x++) suiPx(b, x, cy0, SUI_RGB(7, 33, 54));
    T.draw(b, X0 + 4, cy0 + 3, P3DS_PgName[p.sel][0] ? P3DS_PgName[p.sel] : "?", SUI_RGB(7, 33, 54), SUI_RGB(236, 240, 248));
    const uint32_t sh = SUI_RGB(236, 240, 248), fg = SUI_RGB(18, 26, 38);
    int wl[64], cg = 0; const int wn = pgWild(p.sel, wl, 64, &cg); const bool hasWild = P3DS_PsReady && P3DS_PsStat[p.sel][0];
    if (p.page) { /* wild Pokemon list: three columns, caught ones in green */
      char hd[32]; if (wn) snprintf(hd, sizeof hd, "CAUGHT %d/%d", cg, wn); else snprintf(hd, sizeof hd, "WILD");
      if (wn && pgFindCfg()) { char h2[40]; snprintf(h2, sizeof h2, "TAP=FIND  %s", hd); if (T.width(h2) + T.width(P3DS_PgName[p.sel]) + 14 < aw) snprintf(hd, sizeof hd, "%s", h2); }
      T.draw(b, X1 - 4 - T.width(hd), cy0 + 3, hd, cg >= wn && wn ? SUI_RGB(40, 120, 64) : fg, sh);
      if (!P3DS_PsReady) T.draw(b, X0 + 4, cy0 + 18, "Take a step to count.", fg, sh);
      else if (!hasWild) T.draw(b, X0 + 4, cy0 + 18, "No wild Pokemon here.", fg, sh);
      else if (!wn) { T.draw(b, X0 + 4, cy0 + 18, "Wild list needs hook 0084,", fg, sh); T.draw(b, X0 + 4, cy0 + 28, "not in this build.", fg, sh); }
      else { int cw, rows, shown; pgWildLayout(aw, ch, wn, &cw, &rows, &shown);
        for (int i = 0; i < shown; i++) { const int sp = wl[i], cx0 = X0 + 4 + (i / rows) * cw, cy = cy0 + 16 + (i % rows) * 10; const bool c = P3DS_PsCg[p.sel][sp >> 3] >> (sp & 7) & 1;
          char nm[16]; snprintf(nm, sizeof nm, "%s", kP3dsSpecies[sp]); int k = (int)strlen(nm); while (k > 1 && T.width(nm) > cw - 3) nm[--k] = 0;
          T.draw(b, cx0, cy, nm, c ? SUI_RGB(40, 120, 64) : fg, sh); }
        if (shown < wn) { char m[16]; snprintf(m, sizeof m, "+%d more", wn - shown); T.draw(b, X0 + 4 + 2 * cw, cy0 + 16 + (rows - 1) * 10, m, SUI_RGB(120, 134, 138), sh); } } }
    else {
    const char *st = pgVis(p, p.sel) ? "VISITED" : "NOT YET"; T.draw(b, X1 - 4 - T.width(st), cy0 + 3, st, pgVis(p, p.sel) ? SUI_RGB(40, 120, 64) : SUI_RGB(140, 100, 60), sh);
    if (pgFlyCfg() && pgFlyUnlocked(p.sel)) { /* hook 0228: FLY chip, green when Fly works here now, grey otherwise (the tap then says why) */
      const int cw = T.width("FLY") + 10, cx1 = X1 - 4 - T.width(st) - 6, cx0 = cx1 - cw, cyy = cy0 + 1;
      if (cx0 > X0 + 8 + T.width(P3DS_PgName[p.sel])) { const bool rdy = P3DS_FlyHere == 0; const uint32_t bg = rdy ? SUI_RGB(40, 140, 90) : SUI_RGB(150, 160, 164);
        for (int y = cyy; y < cyy + 12; y++) for (int x = cx0; x < cx1; x++) suiPx(b, x, y, ((x == cx0 || x == cx1 - 1) && (y == cyy || y == cyy + 11)) ? sh : bg);
        T.draw(b, cx0 + 5, cyy + 1, "FLY", SUI_RGB(255, 255, 255), bg); p.flyX0 = cx0; p.flyY0 = cyy; p.flyX1 = cx1; p.flyY1 = cyy + 12; } }
    char pcl[64]; const int pcAvail = aw - 8 - (hasWild ? T.width("Tap card: wild Pokemon") + 6 : 0); const bool hasPc = pcAvail > 30 && pxCardText(T, pcl, sizeof pcl, p.sel, pcAvail);
    int ty = cy0 + 16; int fl = (hasWild || hasPc) ? 3 : 4; if (P3DS_PsReady) { const int k = pgStats(b, T, p, X0 + 4, ty, p.sel, fg, sh, aw / 2); ty += k * 10 + 1; fl -= k; }
    const int n = fl > 0 ? pgWrap(T, P3DS_PgText[p.sel], aw - 8, ln, fl) : 0; for (int i = 0; i < n; i++) T.draw(b, X0 + 4, ty + i * 10, ln[i], fg, sh);
    if (hasWild) { const char *h = "Tap card: wild Pokemon"; T.draw(b, X1 - 4 - T.width(h), Y1 - 11, h, SUI_RGB(24, 153, 115), sh); }
    if (hasPc) T.draw(b, X0 + 4, Y1 - 11, pcl, SUI_RGB(7, 33, 54), sh); } }
  if (p.find > 0 && pgFindCfg()) pgFindBanner(b, T, p, X0, Y0, X1);
  { int by = Y1 - 14; if (p.sel >= 0) { const int top = Y1 - pgCardH(p); if (by + 12 > top - 1) by = top - 14; }
    const int fy = (p.find > 0 && pgFindCfg()) ? Y0 + kPgFindH : Y0; if (by >= fy + 1 && by + 12 < Y1) {
      pgZoomChip(b, X0 + 2, by, false); pgZoomChip(b, X0 + 16, by, true);
      p.zmX0 = X0 + 2; p.zmY0 = by; p.zmX1 = X0 + 14; p.zmY1 = by + 12; p.zpX0 = X0 + 16; p.zpY0 = by; p.zpX1 = X0 + 28; p.zpY1 = by + 12; } }
  pgDots(b, p, X1, Y0); }
static int pgZoomHit(const SPg &p, int x, int y) {
  if (p.zmX1 > p.zmX0 && x >= p.zmX0 - 1 && x < p.zmX1 && y >= p.zmY0 - 2 && y < p.zmY1 + 2) return 1;
  if (p.zpX1 > p.zpX0 && x >= p.zpX0 && x < p.zpX1 + 1 && y >= p.zpY0 - 2 && y < p.zpY1 + 2) return 2;
  return 0; }

/* ---- run 11: the game's own icons, decoded from the members above (nothing is written anywhere) ---- */
/* LZ10 (BIOS LZ77, first byte 0x10) into a malloc'd buffer; returns 0 if the data is not LZ10 or broken */
static uint8_t *pgLz10(const uint8_t *s, uint32_t n, uint32_t *outN) {
  if (n < 4 || s[0] != 0x10) return 0; const uint32_t len = s[1] | s[2] << 8 | s[3] << 16; if (!len || len > 256 * 1024) return 0;
  uint8_t *d = (uint8_t *)malloc(len); if (!d) return 0; uint32_t i = 4, o = 0;
  while (o < len && i < n) { const uint8_t f = s[i++];
    for (int b = 0; b < 8 && o < len; b++) {
      if (f & (0x80 >> b)) { if (i + 1 >= n) { free(d); return 0; } const int l = (s[i] >> 4) + 3, dist = ((s[i] & 15) << 8 | s[i + 1]) + 1; i += 2;
        if ((uint32_t)dist > o) { free(d); return 0; } for (int k = 0; k < l && o < len; k++, o++) d[o] = d[o - dist]; }
      else { if (i >= n) { free(d); return 0; } d[o++] = s[i++]; } } }
  *outN = len; return d; }
/* member as plain bytes (LZ10 unpacked); caller frees when *own */
static const uint8_t *pgPlain(int key, uint32_t *n, bool *own) {
  *own = false; if (!pgRaw[key]) return 0; if (pgRaw[key][0] == 0x10) { uint8_t *u = pgLz10(pgRaw[key], pgRawSz[key], n); *own = u != 0; return u; }
  *n = pgRawSz[key]; return pgRaw[key]; }
/* palette bank -> 16 colours as 0x00BBGGRR (index 0 = clear) */
static bool pgPal16(int key, int bank, uint32_t *out) {
  const uint8_t *t = pgSect(pgRaw[key], pgRawSz[key], "TTLP"); if (!t) return false; const uint8_t *P = t + 8 + pgU32(t + 20); const uint32_t pn = pgU32(t + 16) / 2;
  if ((uint32_t)(bank * 16 + 16) > pn) return false;
  for (int i = 0; i < 16; i++) { const uint16_t k = pgU16(P + 2 * (bank * 16 + i)); out[i] = SUI_RGB((k & 31) * 255 / 31, (k >> 5 & 31) * 255 / 31, (k >> 10 & 31) * 255 / 31); } return true; }
/* RGBA icon (alpha 0/255), 0x00BBGGRR | A<<24 */
struct SPgIcon { int w, h; uint32_t px[32 * 32]; bool ok; };
/* a 32x32 OBJ in 1D mapping (tiles 0..15 row-major 4x4) */
static bool pgIcon32(SPgIcon &ic, int chrKey, int palKey, int bank) {
  uint32_t n; bool own; const uint8_t *d = pgPlain(chrKey, &n, &own); uint32_t pal[16]; ic.ok = false;
  if (!d) return false; const uint8_t *c = pgSect(d, n, "RAHC");
  if (c && pgPal16(palKey, bank, pal)) { const uint8_t *C = c + 8 + pgU32(c + 28); const uint32_t cn = pgU32(c + 24);
    if (cn >= 16 * 32 && C + 16 * 32 <= d + n) { ic.w = ic.h = 32;
      for (int t = 0; t < 16; t++) for (int y = 0; y < 8; y++) for (int x = 0; x < 8; x++) { const int v = C[t * 32 + y * 4 + x / 2] >> (4 * (x & 1)) & 15;
        ic.px[((t / 4) * 8 + y) * 32 + (t % 4) * 8 + x] = v ? pal[v] | 0xFF000000u : 0; }
      ic.ok = true; } }
  if (own) free((void *)d); return ic.ok; }
/* run 20: one NCER cell with each OAM's own 16-colour bank of a 256-colour palette, drawn into a w x h RGBA box with the
   cell origin at (ox, oy); only opaque pixels are written. Used for the touch menu's Sprint Button (a/0/1/4 70/68/7). */
static bool pgCellBanks(uint32_t *out, int w, int h, int ox, int oy, int chrKey, int cellKey, int palKey, int cell) {
  uint32_t n, en; bool own, eown; const uint8_t *d = pgPlain(chrKey, &n, &own), *e = pgPlain(cellKey, &en, &eown); bool ok = false;
  const uint8_t *c = d ? pgSect(d, n, "RAHC") : 0, *k = e ? pgSect(e, en, "KBEC") : 0;
  if (c && k) {
    const uint8_t *C = c + 8 + pgU32(c + 28); const uint32_t cn = pgU32(c + 24); const int nc = pgU16(k + 8), ext = pgU16(k + 10) == 1, csz = ext ? 16 : 8;
    const uint32_t mapping = pgU32(k + 16); const uint8_t *base = k + 8 + pgU32(k + 12), *oam = base + nc * csz;
    static const uint8_t SW[3][4] = { { 8, 16, 32, 64 }, { 16, 32, 32, 64 }, { 8, 8, 16, 32 } }, SH[3][4] = { { 8, 16, 32, 64 }, { 8, 8, 16, 32 }, { 16, 32, 32, 64 } };
    if (cell < nc && oam < e + en) { ok = true;
      const int no = pgU16(base + cell * csz); const uint32_t oo = pgU32(base + cell * csz + 4);
      for (int q = 0; q < no && ok; q++) { const uint8_t *a = oam + oo + q * 6; if (a + 6 > e + en) break;
        const uint16_t a0 = pgU16(a), a1 = pgU16(a + 2), a2 = pgU16(a + 4); int y = a0 & 255; if (y > 127) y -= 256; int x = a1 & 511; if (x > 255) x -= 512;
        const int sh = a0 >> 14, sz = a1 >> 14; if (sh > 2) continue; const int ow = SW[sh][sz], oh = SH[sh][sz], hf = a1 >> 12 & 1, vf = a1 >> 13 & 1;
        const int t0 = (a2 & 1023) << (mapping < 16 ? (mapping & 3) : 0); uint32_t pal[16]; if (!pgPal16(palKey, a2 >> 12, pal)) { ok = false; break; }
        for (int ty = 0; ty < oh / 8; ty++) for (int tx = 0; tx < ow / 8; tx++) { const int t = t0 + ty * (ow / 8) + tx; if ((uint32_t)(t * 32 + 32) > cn) continue;
          for (int yy = 0; yy < 8; yy++) for (int xx = 0; xx < 8; xx++) { const int v = C[t * 32 + yy * 4 + xx / 2] >> (4 * (xx & 1)) & 15; if (!v) continue;
            int lx = tx * 8 + xx, ly = ty * 8 + yy; if (hf) lx = ow - 1 - lx; if (vf) ly = oh - 1 - ly; const int px = ox + x + lx, py = oy + y + ly;
            if (px >= 0 && px < w && py >= 0 && py < h) out[py * w + px] = pal[v] | 0xFF000000u; } } } } }
  if (own) free((void *)d); if (eown) free((void *)e); return ok; }
/* run 20: the Sprint Button cropped to our square RUN slot (31x32: cell x 0..30, y 3..34), off = cells 4+12, on = 5+8.
   Recolours (user-approved mockup): the tab's dark-grey edge, any dark pixel, and the uncovered corner -> the tab orange. */
static bool pgSprint(uint32_t *out, bool on) {
  enum { W = 31, H = 32 }; memset(out, 0, W * H * 4);
  if (!pgCellBanks(out, W, H, 0, -3, 170, 168, 107, on ? 5 : 4) || !pgCellBanks(out, W, H, 0, -3, 170, 168, 107, on ? 8 : 12)) return false;
  const uint32_t OR = SUI_RGB(184, 104, 16) | 0xFF000000u;
  for (int i = 0; i < W * H; i++) { const uint32_t c = out[i]; /* DS 5-bit -> 8-bit as c << 3 (184,104,16 is the tab orange) */
    const int r = ((c & 255) * 31 + 127) / 255 << 3, g = ((c >> 8 & 255) * 31 + 127) / 255 << 3, b = ((c >> 16 & 255) * 31 + 127) / 255 << 3; out[i] = SUI_RGB(r, g, b) | (c & 0xFF000000u);
    if (!(c >> 24) || r + g + b < 140 || (r == 72 && g == 72 && b == 72)) out[i] = OR; }
  return true; }
/* one NCER cell (pgmap player head) into a 32x32 box centred on the cell origin */
static bool pgCell(SPgIcon &ic, int chrKey, int cellKey, int palKey, int cell) {
  uint32_t n, en; bool own, eown; const uint8_t *d = pgPlain(chrKey, &n, &own), *e = pgPlain(cellKey, &en, &eown); uint32_t pal[16]; ic.ok = false;
  const uint8_t *c = d ? pgSect(d, n, "RAHC") : 0, *k = e ? pgSect(e, en, "KBEC") : 0;
  if (c && k && pgPal16(palKey, 0, pal)) {
    const uint8_t *C = c + 8 + pgU32(c + 28); const uint32_t cn = pgU32(c + 24); const int nc = pgU16(k + 8), ext = pgU16(k + 10) == 1, csz = ext ? 16 : 8;
    const uint32_t mapping = pgU32(k + 16); const uint8_t *base = k + 8 + pgU32(k + 12), *oam = base + nc * csz;
    static const uint8_t SW[3][4] = { { 8, 16, 32, 64 }, { 16, 32, 32, 64 }, { 8, 8, 16, 32 } }, SH[3][4] = { { 8, 16, 32, 64 }, { 8, 8, 16, 32 }, { 16, 32, 32, 64 } };
    if (cell < nc && oam < e + en) { ic.w = ic.h = 32; memset(ic.px, 0, sizeof ic.px);
      const int no = pgU16(base + cell * csz); const uint32_t oo = pgU32(base + cell * csz + 4);
      for (int q = 0; q < no; q++) { const uint8_t *a = oam + oo + q * 6; if (a + 6 > e + en) break;
        const uint16_t a0 = pgU16(a), a1 = pgU16(a + 2), a2 = pgU16(a + 4); int y = a0 & 255; if (y > 127) y -= 256; int x = a1 & 511; if (x > 255) x -= 512;
        const int sh = a0 >> 14, sz = a1 >> 14; if (sh > 2) continue; const int w = SW[sh][sz], h = SH[sh][sz]; const int t0 = (a2 & 1023) << (mapping < 16 ? (mapping & 3) : 0);
        for (int ty = 0; ty < h / 8; ty++) for (int tx = 0; tx < w / 8; tx++) { const int t = t0 + ty * (w / 8) + tx; if ((uint32_t)(t * 32 + 32) > cn) continue;
          for (int yy = 0; yy < 8; yy++) for (int xx = 0; xx < 8; xx++) { const int v = C[t * 32 + yy * 4 + xx / 2] >> (4 * (xx & 1)) & 15; const int px = 16 + x + tx * 8 + xx, py = 16 + y + ty * 8 + yy;
            if (v && px >= 0 && px < 32 && py >= 0 && py < 32) ic.px[py * 32 + px] = pal[v] | 0xFF000000u; } } }
      ic.ok = true; } }
  if (own) free((void *)d); if (eown) free((void *)e); return ic.ok; }
