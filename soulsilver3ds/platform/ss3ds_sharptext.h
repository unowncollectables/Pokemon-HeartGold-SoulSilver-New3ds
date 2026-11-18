/* ss3ds: sharp bottom-screen text (2026-11-18, Nathan: "all the text in our bottom screen menus is gross looking ... except the
   new menus"). The Bottom menu panel and the L+R+Y settings app are drawn into a 256x192 DS picture that the GPU stretches to the
   320x240 touch screen (x1.25, gpuscale.c). Every 4th text pixel row and column came out doubled and the edges blurred, while the
   side column and tab row (ss3ds_blnative.h) are drawn at 320x240 and look sharp. Here the game font's text is drawn at 320x240
   too: 1 font pixel = 1 screen pixel (the same size the DS itself shows it), into a native text layer the GPU draws over the
   stretched picture (gpuscale.c txDraw).

   How a repaint works (render3ds.cpp, kSuiText):
   1. btxBegin(rec, dsPicture): text drawn into that picture is recorded. Each string is also drawn into the DS picture at 4/5
      size (the same size the native text has after the stretch) and every pixel it writes is remembered with the colour under it.
   2. btxEnd(rec, list): a string whose DS pixels were all left alone by later drawing (nothing drawn on top of it) is taken out
      of the DS picture again (pixels restored) and goes into `list`; the native layer draws it. A string that something covered
      stays in the DS picture (4/5 size, stretched) and is not in the list, so a popup over text never shows text through it and
      no string is ever shown twice.
   3. btxRender(list, atlas): the native layer, 320x240 words R<<24|G<<16|B<<8|A, stride 512, transparent where there is no text.
   Layout code asks for widths through the same SUiText, which returns the native width in DS pixels (x4/5), so centring, wrapping
   and right alignment match what is drawn. Pure pixel code, no 3DS calls (tools/sharp_text_preview.cpp renders it on a PC). */
#pragma once
#include <stdint.h>
#include <string.h>
#include "ss3ds_settings_ui.h" /* suiClipX0..suiClipY1: recorded strings keep the clip that was set when they were drawn */

/* game font glyph access (p3ds_gamefont.c P3DS_GameFontGlyph): width in pixels, v[16][16] = 0 none, 1 text, 2 shadow, 3 background;
   -1 = no glyph data */
typedef int (*BtxGlyphFn)(int font, int ch, unsigned char v[16][16]);

enum { kBtxCmds = 400, kBtxStr = 12288, kBtxPx = 40960, kBtxCanvasW = 320 };

struct SBtxCmd { int16_t x, y, cx0, cy0, cx1, cy1; uint8_t small, kept; uint32_t fg, sh; uint16_t s; uint32_t p0, p1; };
struct SBtxPix { uint16_t pos; uint8_t kind, pad; uint32_t old; };
/* the native strings of one DS picture: what btxRender draws */
struct SBtxList { int n, sn; uint32_t gen; struct { int16_t x, y, cx0, cy0, cx1, cy1; uint8_t small; uint32_t fg, sh; uint16_t s; } c[kBtxCmds]; char str[kBtxStr]; };
/* recording scratch, one is enough (repaints do not nest) */
struct SBtxRec { uint32_t *dst; bool on; int n, sn, pn, full, covered; SBtxCmd c[kBtxCmds]; char str[kBtxStr]; SBtxPix px[kBtxPx]; };

static BtxGlyphFn gBtxGlyph;
/* glyphs are decoded once per character code (the font never changes while running): width (-1 none) and ink rows */
static int16_t gBtxGW[256]; static uint8_t gBtxGOk[256]; static uint8_t gBtxGV[256][16][16];
static inline int btxGlyph(unsigned char ch) {
  if (!gBtxGOk[ch]) { gBtxGOk[ch] = 1; unsigned char v[16][16]; const int w = gBtxGlyph ? gBtxGlyph(1, ch, v) : -1; gBtxGW[ch] = (int16_t)(w > 16 ? 16 : w);
    if (w >= 0) for (int y = 0; y < 16; y++) for (int x = 0; x < 16; x++) gBtxGV[ch][y][x] = (v[y][x] == 1 || v[y][x] == 2) ? v[y][x] : 0; }
  return gBtxGW[ch]; }
/* string -> native 1x canvas (values 0/1/2), returns width; rows 0..15. Advance rules are P3DS_GameFontDraw's. */
static unsigned char gBtxCanvas[16][kBtxCanvasW];
static int btxRaster(const char *t, bool fill) {
  int x = 0; if (fill) memset(gBtxCanvas, 0, sizeof gBtxCanvas);
  for (; *t; t++) { const unsigned char ch = (unsigned char)*t; const int w = btxGlyph(ch);
    if (w < 0) { if (ch == ' ') x += 3; continue; }
    if (ch == ' ') { x += w > 0 ? w : 3; continue; } /* space advances only */
    if (fill) for (int py = 0; py < 16; py++) { const uint8_t *src = gBtxGV[ch][py]; unsigned char *dst = gBtxCanvas[py];
      for (int px = 0; px < w && x + px < kBtxCanvasW; px++) if (src[px]) dst[x + px] = src[px]; }
    x += w; }
  return x; }
/* small text (SUiText drawS, 3/4 of the DS size before) uses the same font pixels without the shadow: at 320x240 a 3/4 font breaks letters
   apart, and the 1x font is only 7% wider than the old small text after the stretch */
/* native width of a string as drawn; DS width = native x 4/5 rounded up */
static int btxNativeW(const char *t, bool) { return btxRaster(t, false); } /* cached widths, no pixels touched */
static int btxDsW(const char *t, bool small) { return (btxNativeW(t, small) * 4 + 4) / 5; }
/* native top-left of a string drawn at DS (x, y): ink rows 3..14 of the 16 px cell sit where the stretched DS text's ink was centred */
static inline int btxNX(int x) { return (x * 5 + 2) / 4; }
static inline int btxNY(int y, bool small) { return (y * 5 + 2) / 4 + (small ? -1 : 2); }
static inline uint32_t btxWord(uint32_t c) { return (c & 255u) << 24 | ((c >> 8) & 255u) << 16 | ((c >> 16) & 255u) << 8 | 0xFFu; }

static void btxBegin(SBtxRec &r, uint32_t *dst) { r.dst = dst; r.on = dst != 0; r.n = r.sn = r.pn = r.full = r.covered = 0; }
/* the DS-picture copy of a string at 4/5 size, into gBtxDs (n entries: DS position, 1 text / 2 shadow). DS pixel X covers native
   [1.25X, 1.25X+1.25), in quarter-native units [5X, 5X+5); canvas pixel c covers [4(n0+c), 4(n0+c)+4), so it touches one or two DS
   pixels per axis. Text where >= half the DS pixel's area (25 units) is text, else shadow where text+shadow >= half. Honours suiClip. */
enum { kBtxDsMax = 260 * 16 };
struct SBtxDsPx { uint16_t pos; uint8_t kind; };
static SBtxDsPx gBtxDs[kBtxDsMax]; static int gBtxDsN;
static uint16_t gBtxAccI[16][264], gBtxAccS[16][264];
static inline int btxFd5(int v) { return v >= 0 ? v / 5 : -((-v + 4) / 5); } /* floor(v / 5): strings may start left of or above the screen */
static void btxDsPixels(int x, int y, int nw, bool small, bool shadow) {
  gBtxDsN = 0; const int n0x = btxNX(x), n0y = btxNY(y, small); if (nw > kBtxCanvasW) nw = kBtxCanvasW;
  const int X0 = btxFd5(4 * n0x), Y0 = btxFd5(4 * n0y), W = btxFd5(4 * (n0x + nw) - 1) - X0 + 1, H = btxFd5(4 * (n0y + 16) - 1) - Y0 + 1;
  if (nw <= 0 || W > 264 || H > 16) return;
  for (int r = 0; r < H; r++) { memset(gBtxAccI[r], 0, W * 2); memset(gBtxAccS[r], 0, W * 2); }
  for (int cy = 0; cy < 16; cy++) { const int q0 = 4 * (n0y + cy), ya = btxFd5(q0) - Y0, yb = btxFd5(q0 + 3) - Y0, wya = ya == yb ? 4 : 5 * (ya + Y0 + 1) - q0, wyb = 4 - wya;
    const unsigned char *row = gBtxCanvas[cy];
    for (int cx = 0; cx < nw; cx++) { const int k = row[cx]; if (!k) continue;
      const int p0 = 4 * (n0x + cx), xa = btxFd5(p0) - X0, xb = btxFd5(p0 + 3) - X0, wxa = xa == xb ? 4 : 5 * (xa + X0 + 1) - p0, wxb = 4 - wxa;
      uint16_t (*acc)[264] = k == 1 ? gBtxAccI : gBtxAccS;
      acc[ya][xa] += wya * wxa; if (wxb) acc[ya][xb] += wya * wxb; if (wyb) { acc[yb][xa] += wyb * wxa; if (wxb) acc[yb][xb] += wyb * wxb; } } }
  for (int r = 0; r < H; r++) { const int Y = Y0 + r; if (Y < 0 || Y >= 192 || Y < suiClipY0 || Y >= suiClipY1) continue;
    for (int c = 0; c < W; c++) { const int X = X0 + c; if (X < 0 || X >= 256 || X < suiClipX0 || X >= suiClipX1) continue;
      const int ink = gBtxAccI[r][c], shd = gBtxAccS[r][c]; const uint8_t k = ink * 2 >= 25 ? 1 : (shadow && shd && (ink + shd) * 2 >= 25) ? 2 : 0;
      if (k && gBtxDsN < kBtxDsMax) { gBtxDs[gBtxDsN].pos = (uint16_t)(Y * 256 + X); gBtxDs[gBtxDsN].kind = k; gBtxDsN++; } } } }
/* a string drawn through the recorder; returns its DS width */
static int btxText(SBtxRec &r, uint32_t *b, int x, int y, const char *t, uint32_t fg, uint32_t sh, bool small) {
  const int nw = btxRaster(t, true); btxDsPixels(x, y, nw, small, sh != 0);
  const bool rec = r.on && b == r.dst; const int len = (int)strlen(t);
  const bool fits = rec && r.n < kBtxCmds && r.sn + len + 1 <= kBtxStr && r.pn + gBtxDsN <= kBtxPx;
  if (rec && !fits) r.full++;
  const uint32_t p0 = (uint32_t)r.pn;
  for (int i = 0; i < gBtxDsN; i++) { uint32_t &d = b[gBtxDs[i].pos]; const int k = gBtxDs[i].kind;
    if (fits) { SBtxPix &p = r.px[r.pn++]; p.pos = gBtxDs[i].pos; p.kind = (uint8_t)k; p.pad = 0; p.old = d; }
    d = k == 1 ? fg : sh; }
  if (fits) { SBtxCmd &c = r.c[r.n++]; c.x = (int16_t)x; c.y = (int16_t)y; c.cx0 = (int16_t)suiClipX0; c.cy0 = (int16_t)suiClipY0; c.cx1 = (int16_t)suiClipX1; c.cy1 = (int16_t)suiClipY1; c.small = small; c.kept = 0; c.fg = fg; c.sh = sh; c.s = (uint16_t)r.sn; c.p0 = p0; c.p1 = (uint32_t)r.pn;
    memcpy(r.str + r.sn, t, len + 1); r.sn += len + 1; }
  return (nw * 4 + 4) / 5; }
/* ends the recording: strings nothing was drawn over leave the DS picture and go to `out` (gen bumped when it changed) */
static void btxEnd(SBtxRec &r, SBtxList &out) {
  if (!r.on) return;
  r.on = false; uint32_t *b = r.dst;
  for (int i = r.n - 1; i >= 0; i--) { SBtxCmd &c = r.c[i]; bool ok = true;
    for (uint32_t k = c.p0; k < c.p1 && ok; k++) { const SBtxPix &p = r.px[k]; ok = b[p.pos] == (p.kind == 1 ? c.fg : c.sh); }
    c.kept = ok; if (!ok) { r.covered++; continue; }
    for (uint32_t k = c.p1; k-- > c.p0;) b[r.px[k].pos] = r.px[k].old; }
  static SBtxList nl; nl.n = nl.sn = 0; /* static: ~19 KB, kept off the thread stack */
  for (int i = 0; i < r.n; i++) { const SBtxCmd &c = r.c[i]; if (!c.kept) continue; const char *s = r.str + c.s; const int len = (int)strlen(s);
    if (nl.sn + len + 1 > kBtxStr) break;
    nl.c[nl.n].x = c.x; nl.c[nl.n].y = c.y; nl.c[nl.n].cx0 = c.cx0; nl.c[nl.n].cy0 = c.cy0; nl.c[nl.n].cx1 = c.cx1; nl.c[nl.n].cy1 = c.cy1; nl.c[nl.n].small = c.small; nl.c[nl.n].fg = c.fg; nl.c[nl.n].sh = c.sh; nl.c[nl.n].s = (uint16_t)nl.sn; nl.n++;
    memcpy(nl.str + nl.sn, s, len + 1); nl.sn += len + 1; }
  bool same = nl.n == out.n && nl.sn == out.sn && !memcmp(nl.str, out.str, nl.sn);
  same = same && !memcmp(nl.c, out.c, sizeof nl.c[0] * nl.n); /* entries are only ever filled field by field into the zeroed static nl, then copied whole */
  if (!same) { const uint32_t g = out.gen + 1; out.n = nl.n; out.sn = nl.sn; memcpy(out.c, nl.c, sizeof nl.c[0] * (nl.n ? nl.n : 1)); memcpy(out.str, nl.str, nl.sn); out.gen = g; } }
/* the native layer: a[y*512+x], 320x240 cleared to transparent, then every string at 1 font pixel = 1 screen pixel */
static void btxRender(const SBtxList &l, uint32_t *a) {
  for (int y = 0; y < 240; y++) memset(a + y * 512, 0, 320 * 4);
  for (int i = 0; i < l.n; i++) { const bool small = l.c[i].small != 0; const int nw = btxRaster(l.str + l.c[i].s, true);
    const int nh = 16, x0 = btxNX(l.c[i].x), y0 = btxNY(l.c[i].y, small); const uint32_t fg = btxWord(l.c[i].fg), sh = l.c[i].sh ? btxWord(l.c[i].sh) : 0;
    /* the DS clip in screen pixels: DS [c0, c1) is native [5c0/4, 5c1/4), the same edges the stretched picture has */
    const int nx0 = btxNX(l.c[i].cx0) < 0 ? 0 : btxNX(l.c[i].cx0), nx1 = btxNX(l.c[i].cx1) > 320 ? 320 : btxNX(l.c[i].cx1),
              ny0 = btxNX(l.c[i].cy0) < 0 ? 0 : btxNX(l.c[i].cy0), ny1 = btxNX(l.c[i].cy1) > 240 ? 240 : btxNX(l.c[i].cy1);
    for (int y = 0; y < nh; y++) { const int Y = y0 + y; if (Y < ny0 || Y >= ny1) continue;
      for (int x = 0; x < nw && x < kBtxCanvasW; x++) { const int X = x0 + x; if (X < nx0 || X >= nx1) continue; const int k = gBtxCanvas[y][x];
        if (k == 1) a[Y * 512 + X] = fg; else if (k == 2 && sh) a[Y * 512 + X] = sh; } } } }
