/* Before/after check for sharp bottom-screen text (ss3ds_sharptext.h). Draws the settings app (Display tab) and the Johto map PROGRESS
   page with the game's message font, then shows them the way the 3DS does: OLD = DS 256x192 picture stretched to 320x240 (nearest 2x,
   then linear down, like gpuscale.c); NEW = the same with the text in the native layer. Font: member 1 of the ROM's a/0/1/6 (in
   pret/pokeheartgold as files/graphic/font/font_00000001.bin), passed as argv[1]; nothing from the ROM is stored in the repo.
   gcc -c -O1 -w -I soulsilver3ds/platform tools/sharp_text_font_host.c -o /tmp/font_host.o
   g++ -O1 -w -I soulsilver3ds/platform tools/sharp_text_preview.cpp /tmp/font_host.o -o /tmp/sharp_text_preview
   ./sharp_text_preview font_00000001.bin out_prefix   -> out_prefix_{settings,progress,popup}_{old,new}.ppm, 320x240 */
#include <stdint.h>
#include <ctime>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BLINV do {} while (0)
extern "C" {
void PSPNativeMemLog(const char *, ...);
int P3DS_CfgInt(const char *, int);
int P3DS_GameFontReady(void);
int P3DS_GameFontWidth(int, const char *);
int P3DS_GameFontDraw(uint32_t *, int, int, int, int, int, const char *, uint32_t, uint32_t, int, int);
int P3DS_GameFontWidth34(int, const char *);
int P3DS_GameFontDraw34(uint32_t *, int, int, int, int, int, const char *, uint32_t, int);
int P3DS_GameFontGlyph(int, int, unsigned char (*)[16]);
void host_set_font(uint8_t *, uint32_t);
void P3DS_CfgWrite(const char *, int) {}
void P3DS_HuntChime(void) {}
int P3DS_FpsTweaks(void) { return 0; }
void P3DS_Toast(const char *) {}
volatile int P3DS_PanelPartyWant, P3DS_PanelPartyOn;
}
#include "tracker.h"
extern "C" { volatile int P3DS_TrkReady = 1; P3DS_TrkRoamer P3DS_TrkRoam[4] = { { 1, 243, 40, 100, 0, 3 }, { 1, 244, 40, 100, 0, 5 } }; P3DS_TrkDaycare P3DS_TrkDc = { 1, { 25, 0 }, 0, 0, 0, -1 }; int P3DS_TrkOn(void) { return 1; } }
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
int P3DS_MobilePcChipOn(void) { return 0; }
void P3DS_MobilePcOnChipTap(void) {}
void P3DS_MobilePcNotePartyVisible(void) {}
int P3DS_MobilePcAnimFrame(void) { return -1; }
}
#include "ss3ds_minimap.h"
#include "ss3ds_pgmap.h"
#include "ss3ds_pgextra.h"
#include "ss3ds_sharptext.h"
extern "C" const SS3DS_PgSpec sLocationSpecs[100] = { { 1, 1, 1, 0x11 } };
/* the two SUiText the port has: old = DS-size game font; new = through the recorder */
static int oDraw(uint32_t *b, int x, int y, const char *t, uint32_t fg, uint32_t sh) { return P3DS_GameFontDraw(b, 256, 256, 192, x, y, t, fg, sh, 1, 1); }
static int oW(const char *t) { return P3DS_GameFontWidth(1, t); }
static int oDrawS(uint32_t *b, int x, int y, const char *t, uint32_t fg, uint32_t) { return P3DS_GameFontDraw34(b, 256, 256, 192, x, y, t, fg, 1); }
static int oWS(const char *t) { return P3DS_GameFontWidth34(1, t); }
static SBtxRec gRec;
static int nDraw(uint32_t *b, int x, int y, const char *t, uint32_t fg, uint32_t sh) { return btxText(gRec, b, x, y, t, fg, sh, false); }
static int nW(const char *t) { return btxDsW(t, false); }
static int nDrawS(uint32_t *b, int x, int y, const char *t, uint32_t fg, uint32_t) { return btxText(gRec, b, x, y, t, fg, 0, true); }
static int nWS(const char *t) { return btxDsW(t, true); }
static const SUiText kOld = { oDraw, oW, oDrawS, oWS }, kNew = { nDraw, nW, nDrawS, nWS };

/* gpuscale.c: nearest 2x, then linear to 320x240 */
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
static void ppm(const char *path, const uint32_t *b) {
  FILE *f = fopen(path, "wb"); if (!f) return; fprintf(f, "P6\n320 240\n255\n");
  for (int i = 0; i < 320 * 240; i++) { unsigned char p[3] = { (unsigned char)(b[i] & 255), (unsigned char)(b[i] >> 8), (unsigned char)(b[i] >> 16) }; fwrite(p, 1, 3, f); }
  fclose(f); }

static void settings(uint32_t *b, const SUiText &T) {
  static const char *const tabs[] = { "Display", "3D", "Input", "Battle", "Play", "Setup" };
  suiBackground(b); suiHeader(b, T, tabs, 6, 0, nullptr);
  const SUiCell c[] = { { "Wide view", "ON", false, false }, { "Wide layouts", "OFF", false, false }, { "Camera zoom", "0", false, false },
    { "Title layout", "WIDE", false, false }, { "Ambient weather", "ON", true, true }, { "Seasonal trees", "ON", false, false }, { "Sharp text", "ON", false, false } };
  suiList(b, T, c, 7, 0); suiTip(b, T, "Weekly seasons bring showers, snow and mist. Tap WX on the Johto map for the seasonal forecast."); }
/* a popup over part of the list after the text was drawn: the covered strings must stay in the DS picture, not show through */
static void popup(uint32_t *b, const SUiText &T) { settings(b, T); suiPanel(b, 40, 60, 216, 110, kSuiCell, kSuiCellLo, kSuiSelEdge, true); }
static SPg gP;
static void progress(uint32_t *b, const SUiText &T) {
  for (int i = 0; i < 256 * 192; i++) b[i] = SUI_RGB(200, 208, 211);
  pxBar(b, T, kBlPanX0, kBlPanY0, kBlPanX1, "Route 37");
  gP.ov = true; gP.ovScroll = 0; pgOverview(b, T, gP, kBlPanX0, kBlPanY0 + 17, kBlPanX1, kBlPanY1); }

int main(int argc, char **argv) {
  if (argc < 3) return 1;
  FILE *f = fopen(argv[1], "rb"); if (!f) return 1; static uint8_t font[65536]; const uint32_t n = (uint32_t)fread(font, 1, sizeof font, f); fclose(f);
  host_set_font(font, n); gBtxGlyph = P3DS_GameFontGlyph;
  P3DS_PsReady = 1; uint16_t *t = P3DS_PsStat[100]; t[0] = 128; t[1] = 15; t[2] = 73; t[3] = 362; t[4] = 87; t[5] = 231; t[6] = 35; t[7] = 204; t[8] = 5;
  for (int i = 0; i < 6; i++) { snprintf(P3DS_PgName[i], sizeof P3DS_PgName[i], "%s", i ? "Route 3x" : "Sprout Tower"); P3DS_PsStat[i][0] = 2; P3DS_PsStat[i][1] = 1; P3DS_PsStat[i][3] = 6; P3DS_PsStat[i][4] = 5; gP.visited[i / 8] |= 1 << (i % 8); }
  static uint32_t ds[256 * 192], out[320 * 240], atlas[512 * 256]; static SBtxList list; char path[512];
  void (*const scr[3])(uint32_t *, const SUiText &) = { settings, progress, popup }; const char *const nm[3] = { "settings", "progress", "popup" };
  for (int s = 0; s < 3; s++) {
    scr[s](ds, kOld); stretch(ds, out); snprintf(path, sizeof path, "%s_%s_old.ppm", argv[2], nm[s]); ppm(path, out);
    btxBegin(gRec, ds); scr[s](ds, kNew); btxEnd(gRec, list); btxRender(list, atlas);
    stretch(ds, out); over(out, atlas); snprintf(path, sizeof path, "%s_%s_new.ppm", argv[2], nm[s]); ppm(path, out);
    printf("%s: %d strings native, %d left in the DS picture (covered), %d not recorded (full)\n", nm[s], list.n, gRec.covered, gRec.full); }
  return 0; }
