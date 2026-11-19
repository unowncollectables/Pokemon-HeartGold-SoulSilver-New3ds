/* Renders the battle Y screen pages (ss3ds_catchui.h) as the 3DS shows them: DS 256x192 picture stretched to 320x240 with the sharp
   native text layer on top. Font: member 1 of the ROM's a/0/1/6 (pret/pokeheartgold files/graphic/font/font_00000001.bin), argv[1].
   gcc -c -O1 -w -I soulsilver3ds/platform tools/sharp_text_font_host.c -o /tmp/font_host.o
   g++ -O1 -w -I soulsilver3ds/platform tools/catch_ui_preview.cpp /tmp/font_host.o -o /tmp/catch_ui_preview
   ./catch_ui_preview font_00000001.bin out_prefix  -> out_prefix_<page>.ppm (320x240) */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" { int P3DS_GameFontGlyph(int, int, unsigned char (*)[16]); void host_set_font(uint8_t *, uint32_t); }
#include "ss3ds_sharptext.h"
#include "ss3ds_catchui.h"
#include "p3ds_abilnames.h"
#include "p3ds_movenames.h"
static SBtxRec gRec;
static int nDraw(uint32_t *b, int x, int y, const char *t, uint32_t fg, uint32_t sh) { return btxText(gRec, b, x, y, t, fg, sh, false); }
static int nW(const char *t) { return btxDsW(t, false); }
static int nDrawS(uint32_t *b, int x, int y, const char *t, uint32_t fg, uint32_t) { return btxText(gRec, b, x, y, t, fg, 0, true); }
static int nWS(const char *t) { return btxDsW(t, true); }
static const SUiText kNew = { nDraw, nW, nDrawS, nWS };
static void stretch(const uint32_t *ds, uint32_t *out) {
  for (int v = 0; v < 240; v++) for (int u = 0; u < 320; u++) {
    const float sx = (u + 0.5f) * 512.0f / 320.0f - 0.5f, sy = (v + 0.5f) * 384.0f / 240.0f - 0.5f;
    int x0 = (int)sx, y0 = (int)sy; const float fx = sx - x0, fy = sy - y0; int x1 = x0 + 1 > 511 ? 511 : x0 + 1, y1 = y0 + 1 > 383 ? 383 : y0 + 1;
    auto at = [&](int x, int y) { return ds[(y / 2) * 256 + x / 2]; };
    uint32_t o = 0; for (int s = 0; s < 24; s += 8) {
      const float a = (at(x0, y0) >> s & 255) * (1 - fx) + (at(x1, y0) >> s & 255) * fx, c = (at(x0, y1) >> s & 255) * (1 - fx) + (at(x1, y1) >> s & 255) * fx;
      o |= (uint32_t)(a * (1 - fy) + c * fy + 0.5f) << s; }
    out[v * 320 + u] = o; } }
static void over(uint32_t *out, const uint32_t *a) { for (int y = 0; y < 240; y++) for (int x = 0; x < 320; x++) { const uint32_t w = a[y * 512 + x]; if (w & 255) out[y * 320 + x] = (w >> 24) | ((w >> 16) & 255) << 8 | ((w >> 8) & 255) << 16; } }
static void ppm(const char *path, const uint32_t *b) { FILE *f = fopen(path, "wb"); if (!f) return; fprintf(f, "P6\n320 240\n255\n");
  for (int i = 0; i < 320 * 240; i++) { unsigned char p[3] = { (unsigned char)(b[i] & 255), (unsigned char)(b[i] >> 8), (unsigned char)(b[i] >> 16) }; fwrite(p, 1, 3, f); } fclose(f); }
static int mult(int a, int d) { /* tiny chart: enough for the preview (x2 units) */
  if (a == 13 && d == 11) return 4; if (a == 13 && d == 12) return 1; if (a == 13 && d == 4) return 0; if (a == 10 && d == 12) return 4; if (a == 10 && d == 11) return 1; if (a == 1 && d == 0) return 4; return 2; }
int main(int argc, char **argv) {
  if (argc < 3) return 1; FILE *f = fopen(argv[1], "rb"); if (!f) return 1; static uint8_t font[65536]; const uint32_t n = (uint32_t)fread(font, 1, sizeof font, f); fclose(f);
  host_set_font(font, n); gBtxGlyph = P3DS_GameFontGlyph;
  cc::Wild w{}; int c[20] = { 1, 179, 16, 63, 63, 0, 9, 9, 0, 5, 13, 16, 1, 3, 24, 235, 1, 0, 0, 7 }; memcpy(w.c, c, sizeof c); w.name = "Mareep"; w.ability = "Static"; w.nature = "Lax"; w.met = 4; w.metOn = true; w.c[16] = 0;
  cc::Foe fo{}; int ff[40] = { 1, 0, 6, 3, 76, 44, 143, 143, 0, 5, 1, 99, 89, 14, 15, 24, 8, 16, 5, 0, 76, 143, 143, 130, 110, 120, 112, 90, 90, 143, 143, 143, 18, 0, 0, 0, 0, 0, 0, 0 }; memcpy(fo.f, ff, sizeof ff);
  fo.name = "Golem"; fo.ability = "Sturdy"; fo.moves = kP3dsMove; fo.nMoves = 468; fo.spec = kP3dsSpec; fo.nSpec = 494;
  cc::Dmg dm{}; int d[32] = { 1, 0, 44, 118, 90, 100, 0, 13, 13, 44, 80, 70, 40, 100, 11, 4, 0, 76, 85, 98, 93, 20, 13, 11, 1, 4, 90, 90, 60, 0 }; memcpy(dm.d, d, sizeof d);
  dm.foeName = "Golem"; dm.moves = kP3dsMove; dm.nMoves = 468; dm.mult = mult; dm.d[13] = 143;
  cc::Hunt hu{ "Mareep", 412, 412, 0, 0, 7200, 20261101, 0, 8192, 5678, 18868, 0.049, false };
  static uint32_t ds[256 * 192], out[320 * 240], atlas[512 * 256]; static SBtxList list; char path[512];
  for (int s = 0; s < 6; s++) {
    int tabs[4], nt; const char *nm;
    if (s < 4) { nt = 4; tabs[0] = cc::P_INFO; tabs[1] = cc::P_CATCH; tabs[2] = cc::P_HUNT; tabs[3] = cc::P_DMG; } else { nt = 3; tabs[0] = cc::P_FOE; tabs[1] = cc::P_PARTY; tabs[2] = cc::P_DMG; }
    btxBegin(gRec, ds);
    switch (s) { case 0: nm = "info"; cc::wildInfo(ds, kNew, w, nt, tabs, 0); break; case 1: nm = "catch"; cc::wildCatch(ds, kNew, w, nt, tabs, 1); break; case 2: nm = "hunt"; cc::huntPage(ds, kNew, hu, nt, tabs, 2); break;
      case 3: nm = "damage"; cc::dmgPage(ds, kNew, dm, nt, tabs, 3); break; case 4: nm = "foe"; cc::foePage(ds, kNew, fo, nt, tabs, 0); break; default: nm = "party"; cc::partyPage(ds, kNew, fo, nt, tabs, 1); }
    btxEnd(gRec, list); btxRender(list, atlas); stretch(ds, out); over(out, atlas); snprintf(path, sizeof path, "%s_%s.ppm", argv[2], nm); ppm(path, out);
    printf("%s: %d strings native, %d covered, %d full\n", nm, list.n, gRec.covered, gRec.full); }
  { /* FIGHT menu type hint pills (same drawing as hintSharpDraw in render3ds.cpp) on a fake FIGHT menu */
    for (int i = 0; i < 256 * 192; i++) ds[i] = SUI_RGB(120, 150, 200);
    static const char *mv[4] = { "Thunderbolt", "Surf", "Ice Beam", "Tackle" }; static const int ms[4] = { 8, 4, 2, 16 };
    btxBegin(gRec, ds);
    for (int k = 0; k < 4; k++) { const int x0 = k & 1 ? 132 : 4, y0 = k >> 1 ? 144 : 79, by = k >> 1 ? 100 : 28; cc::rrect(ds, x0, by, x0 + 120, by + 48, 6, SUI_RGB(250, 250, 250), SUI_RGB(210, 220, 235)); btxText(gRec, ds, x0 + 10, by + 6, mv[k], SUI_RGB(20, 20, 30), SUI_RGB(200, 200, 210), false);
      const int m = ms[k]; if (m == 4) continue; const char *t = m == 0 ? "No effect" : m == 1 ? "Resist x1/4" : m == 2 ? "Resist x1/2" : m == 8 ? "Super x2" : "Super x4";
      const uint32_t c = m == 0 ? SUI_RGB(110, 116, 132) : m < 4 ? SUI_RGB(208, 86, 62) : m == 8 ? SUI_RGB(48, 168, 84) : SUI_RGB(30, 192, 70);
      const int tw = btxDsW(t, false), pw = tw + 22, px = x0 + 120 - 4 - pw, py = y0 - 2, ph = 10; cc::pillBase(ds, px, py, px + pw, py + ph, c);
      if (m == 0) cc::cross(ds, px + 5, py + 2, SUI_RGB(255, 255, 255)); else if (m < 4) cc::triDn(ds, px + 5, py + 3, SUI_RGB(255, 255, 255)); else cc::triUp(ds, px + 5, py + 3, SUI_RGB(255, 255, 255));
      btxText(gRec, ds, px + 16, py - 1, t, SUI_RGB(255, 255, 255), cc::mixc(c, SUI_RGB(0, 0, 0), 150), false); }
    btxEnd(gRec, list); btxRender(list, atlas); stretch(ds, out); over(out, atlas); snprintf(path, sizeof path, "%s_hint.ppm", argv[2]); ppm(path, out); }
  return 0; }
