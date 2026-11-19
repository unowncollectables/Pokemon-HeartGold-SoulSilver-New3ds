/* ss3ds sharp game text, stage 1 (2026-11-18, Nathan: "can we do this for all text boxes and everything in the entire game").
   The game prints every string through CopyGlyphToWindow (bg_window.c) into a window's pixel buffer, copies that buffer to BG VRAM,
   and the DS picture (256x192) is stretched 1.25x to the screen, so letters come out uneven and soft. Here (patch 0286 hooks the
   window code) the glyphs of a text window are RECORDED, taken out of the copy that goes to VRAM (the game's own pixel buffer is
   never touched, so every game routine that reads it still sees the text), and drawn at 1 font pixel = 1 screen pixel in a native
   top-screen layer (gpuscale.c topTxDraw, both eyes, after the stretched picture).
   Stage 1 = text windows on the main engine while the game is in the overworld and the main engine is the top screen: dialogue, the
   Yes/No box, field list menus. Everything else keeps its normal text (the hooks do nothing for it).
   Safety rules (nothing doubled, nothing lost):
   - A window is only sharpened after it was filled with one colour (FillWindowPixelBuffer); only recorded glyph ink pixels are
     erased (back to that colour), so anything else drawn into the window (the blinking down arrow, pictures) reaches VRAM as is.
     A rectangle drawn over glyphs (Blit / FillRect) makes the window forget those glyphs (P3DS_GtRect). A scroll with another
     colour or sideways marks the window "dirty" until the next full fill.
   - Per upload decision: when sharp text is off, not possible, or the window is dirty, the FULL buffer goes to VRAM and the native
     layer draws nothing for that window.
   - The native layer draws a window's glyph pixel only where the BG tile map in VRAM still shows that window's tile there, so a
     box that closed, or one drawn over part of it, hides exactly its own part. BG off, master brightness and BLDCNT fades are applied.
   - The registry is cleared whenever the overworld is left. Log: [GTEXT]. */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
typedef uint8_t u8; typedef uint16_t u16; typedef uint32_t u32;

extern int P3DS_InField, P3DS_InBattle;
extern unsigned char PSPNative_GfxRegisters[];
extern unsigned char s_HW_BG_PLTT[];
extern const unsigned char *P3DS_GtVramABG(void);
extern int P3DS_SharpGameTextCfg(void);
extern int P3DS_ScaleMode(void);
extern int P3DS_TopTxReady(void);
extern void PSPNativeMemLog(const char *, ...);

/* what the game side passes about a window (same struct in the patched bg_window.c) */
typedef struct { const void *buf; int bg, left, top, wT, hT, pal, base, is4bpp; } P3DS_GtDesc;

enum { kWins = 16, kGlyphs = 720, kScratch = 20480 };
typedef struct { const void *buf; u8 used, fillKnown, dirty, gave, shown; u8 bg, left, top, wT, hT, pal; u16 base; u8 fill; u16 nGlyph; } GtWin;
typedef struct { int16_t x, y; u8 win, used, x0, x1, y0, y1; u8 px[256]; } GtGlyph; /* x0..x1, y0..y1: ink box inside the 16x32 cell */ /* px: 16 x 32 tile layout (2 tiles wide), 4 bit, as the game's own glyph copy writes it */
static GtWin gWin[kWins];
static GtGlyph gGl[kGlyphs];
static u8 gScratch[kScratch] __attribute__((aligned(4)));
static unsigned gGen = 1, gNShown, gErased, gPlain, gRecorded, gFull;
static int gLastField;

static unsigned reg16(unsigned o) { return *(volatile u16 *)(PSPNative_GfxRegisters + o); }
static unsigned reg32(unsigned o) { return *(volatile u32 *)(PSPNative_GfxRegisters + o); }

static int gtActive(void) {
    if (!P3DS_SharpGameTextCfg()) return 0;
    if (!P3DS_InField || P3DS_InBattle) return 0;
    if (P3DS_ScaleMode() == 0) return 0;            /* the 1:1 present has no native layer */
    if (!(reg16(0x304) & 0x8000)) return 0;         /* POWCNT: main engine on the top screen */
    if (reg32(0) & (1u << 30)) return 0;            /* BG extended palettes: not read here */
    return P3DS_TopTxReady();
}
static void gtReset(void) {
    memset(gWin, 0, sizeof gWin); memset(gGl, 0, sizeof gGl); gGen++; gNShown = 0;
}
static GtWin *gtFind(const P3DS_GtDesc *d, int create) {
    GtWin *w = 0, *fr = 0;
    for (int i = 0; i < kWins; i++) { if (gWin[i].used && gWin[i].buf == d->buf) { w = &gWin[i]; break; } if (!gWin[i].used && !fr) fr = &gWin[i]; }
    if (!w) { if (!create || !fr) return 0; w = fr; memset(w, 0, sizeof *w); w->used = 1; w->buf = d->buf; }
    w->bg = (u8)d->bg; w->left = (u8)d->left; w->top = (u8)d->top; w->wT = (u8)d->wT; w->hT = (u8)d->hT; w->pal = (u8)d->pal; w->base = (u16)d->base;
    return w;
}
static void gtDropGlyphs(int wi) { for (int i = 0; i < kGlyphs; i++) if (gGl[i].used && gGl[i].win == wi) gGl[i].used = 0; gWin[wi].nGlyph = 0; gGen++; }
static int gtIdx(const GtWin *w) { return (int)(w - gWin); }

/* ---- calls from the patched bg_window.c (game thread) ---- */
/* should the next glyph into this window be recorded (and the glyph copy done for the record)? */
int P3DS_GtWant(const P3DS_GtDesc *d) {
    if (!d->is4bpp || d->bg < 0 || d->bg > 3 || d->wT > 32 || d->hT > 24) return 0;
    GtWin *w = gtFind(d, 0);
    if (!w || !w->fillKnown || w->dirty) return 0;
    if (!gtActive()) { if (w->nGlyph) { w->gave = 1; gtDropGlyphs(gtIdx(w)); } return 0; }
    if (w->gave) return 0;
    return 1;
}
void P3DS_GtGlyph(const P3DS_GtDesc *d, int x, int y, const u8 *tile16x32, int hasInk) {
    GtWin *w = gtFind(d, 0); if (!w) return;
    if (!hasInk) return;                            /* blank glyph (space): nothing to draw or erase */
    int slot = -1; for (int i = 0; i < kGlyphs; i++) if (!gGl[i].used) { slot = i; break; }
    if (slot < 0) { w->gave = 1; gFull++; gtDropGlyphs(gtIdx(w)); return; }
    GtGlyph *g = &gGl[slot]; g->used = 1; g->win = (u8)gtIdx(w); g->x = (int16_t)x; g->y = (int16_t)y; memcpy(g->px, tile16x32, 256);
    g->x0 = 15; g->x1 = 0; g->y0 = 31; g->y1 = 0;
    for (int py = 0; py < 32; py++) for (int px = 0; px < 16; px++)
        if ((g->px[(((py >> 3) * 2 + (px >> 3)) << 5) + ((py & 7) << 2) + ((px & 7) >> 1)] >> ((px & 1) * 4)) & 15) {
            if (px < g->x0) g->x0 = (u8)px; if (px > g->x1) g->x1 = (u8)px; if (py < g->y0) g->y0 = (u8)py; if (py > g->y1) g->y1 = (u8)py; }
    w->nGlyph++; gGen++; gRecorded++;
}
void P3DS_GtFill(const P3DS_GtDesc *d, int fillValue) {
    GtWin *w = gtFind(d, 1); if (!w) return;
    if (w->nGlyph) gtDropGlyphs(gtIdx(w));
    w->fillKnown = d->is4bpp ? 1 : 0; w->dirty = 0; w->gave = 0; w->fill = (u8)(fillValue & 15); w->shown = 0; gGen++;
}
/* something other than text (the blinking down arrow, a cleared rectangle, a picture) is drawn into a rectangle of the window: the
   glyphs it touches were overwritten, so they are forgotten (never erased again, never drawn natively); the rest stays sharp.
   Only glyph pixels are ever erased, so whatever else is drawn into the window always reaches VRAM unchanged. */
void P3DS_GtRect(const P3DS_GtDesc *d, int x, int y, int wd, int ht) {
    GtWin *w = gtFind(d, 0); if (!w || !w->nGlyph) return;
    const int wi = gtIdx(w); unsigned dropped = 0;
    for (int i = 0; i < kGlyphs; i++) { GtGlyph *g = &gGl[i]; if (!g->used || g->win != wi) continue;
        if (g->x + g->x1 < x || g->x + g->x0 >= x + wd || g->y + g->y1 < y || g->y + g->y0 >= y + ht) continue;
        g->used = 0; if (w->nGlyph) w->nGlyph--; dropped++; }
    if (dropped) gGen++;
}
void P3DS_GtScroll(const P3DS_GtDesc *d, int dir, int amount, int fillValue) {
    GtWin *w = gtFind(d, 0); if (!w) return;
    if (dir > 1 || (fillValue & 15) != w->fill) { w->dirty = 1; gtDropGlyphs(gtIdx(w)); w->shown = 0; return; }   /* sideways / other colour: give up until the next fill */
    for (int i = 0; i < kGlyphs; i++) if (gGl[i].used && gGl[i].win == gtIdx(w)) {
        gGl[i].y = (int16_t)(gGl[i].y + (dir == 0 ? -amount : amount));
        if (gGl[i].y <= -32 || gGl[i].y >= w->hT * 8) { gGl[i].used = 0; if (w->nGlyph) w->nGlyph--; } }
    gGen++;
}
void P3DS_GtRemove(const void *buf) {
    for (int i = 0; i < kWins; i++) if (gWin[i].used && gWin[i].buf == buf) { gtDropGlyphs(i); gWin[i].used = 0; }
}
/* the buffer to copy to VRAM: the window's own, or a copy with the recorded glyphs taken out */
const void *P3DS_GtUpload(const P3DS_GtDesc *d, unsigned size) {
    GtWin *w = gtFind(d, 0);
    if (!w) return d->buf;
    if (!w->nGlyph || w->dirty || w->gave || !w->fillKnown || !d->is4bpp || size != (unsigned)(d->wT * d->hT * 32) || size > kScratch || !gtActive()) {
        if (w->shown) { w->shown = 0; gGen++; }
        if (w->nGlyph) gPlain++;
        return d->buf;
    }
    memcpy(gScratch, d->buf, size);
    const int wi = gtIdx(w), W = d->wT, wpx = d->wT * 8, hpx = d->hT * 8; const u8 fillByte = (u8)(w->fill | (w->fill << 4));
    for (int i = 0; i < kGlyphs; i++) { const GtGlyph *g = &gGl[i]; if (!g->used || g->win != wi) continue;
        for (int py = 0; py < 32; py++) { const int Y = g->y + py; if (Y < 0 || Y >= hpx) continue;
            for (int px = 0; px < 16; px++) { const int X = g->x + px; if (X < 0 || X >= wpx) continue;
                const u8 n = (g->px[(((py >> 3) * 2 + (px >> 3)) << 5) + ((py & 7) << 2) + ((px & 7) >> 1)] >> ((px & 1) * 4)) & 15;
                if (!n) continue;
                u8 *dst = gScratch + ((((Y >> 3) * W + (X >> 3)) << 5) + ((Y & 7) << 2) + ((X & 7) >> 1));
                *dst = (u8)((X & 1) ? ((*dst & 0x0F) | (fillByte & 0xF0)) : ((*dst & 0xF0) | (fillByte & 0x0F))); } } }
    if (!w->shown) { w->shown = 1; gGen++; }
    gErased++;
    { static unsigned n; if (n < 8) { n++; PSPNativeMemLog("[GTEXT] window bg%d %dx%d tiles at %d,%d: %u glyphs taken out of the VRAM copy (native layer draws them)", d->bg, d->wT, d->hT, d->left, d->top, (unsigned)w->nGlyph); } }
    return gScratch;
}

/* ---- native top layer (called from gpuscale.c topTxDraw, render thread = game thread) ---- */
static unsigned gLastSig; static int gHave;
static u32 gfx555(u16 c, int mode, int ev) {
    int r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
    if (mode == 1) { r += (31 - r) * ev / 16; g += (31 - g) * ev / 16; b += (31 - b) * ev / 16; }
    else if (mode == 2) { r -= r * ev / 16; g -= g * ev / 16; b -= b * ev / 16; }
    r = (r << 3) | (r >> 2); g = (g << 3) | (g >> 2); b = (b << 3) | (b >> 2);
    return ((u32)r << 24) | ((u32)g << 16) | ((u32)b << 8) | 0xFFu;
}
static unsigned fnv(unsigned h, unsigned v) { return (h ^ v) * 16777619u; }
int P3DS_GtRender(u32 *buf, float fxf, float fyf, float fwf, float fhf, int *dirty) {
    *dirty = 0;
    if (P3DS_InField != gLastField) { gLastField = P3DS_InField; if (!P3DS_InField) { gtReset(); } }
    unsigned any = 0; for (int i = 0; i < kWins; i++) if (gWin[i].used && gWin[i].shown && gWin[i].nGlyph && !gWin[i].gave && !gWin[i].dirty) any++;
    if (!any || !gtActive()) { gHave = 0; return 0; }
    const u8 *vr = P3DS_GtVramABG(); if (!vr) return 0;
    const unsigned dc = reg32(0), bld = reg16(0x50), bldy = reg16(0x54) & 31, mb = reg16(0x6C);
    const int fx = (int)(fxf + 0.5f), fy = (int)(fyf + 0.5f), fw = (int)(fwf + 0.5f), fh = (int)(fhf + 0.5f);
    /* per window: BG on? tile visibility mask */
    static u8 vis[kWins][32 * 24]; unsigned sig = fnv(2166136261u, gGen); sig = fnv(sig, (unsigned)(fx * 4096 + fy * 64 + fw)); sig = fnv(sig, (unsigned)fh);
    sig = fnv(sig, dc & 0xF00); sig = fnv(sig, bld); sig = fnv(sig, bldy); sig = fnv(sig, mb);
    for (int i = 0; i < 256; i += 4) sig = fnv(sig, *(const u32 *)(s_HW_BG_PLTT + i));
    unsigned shownN = 0;
    for (int i = 0; i < kWins; i++) { GtWin *w = &gWin[i]; if (!(w->used && w->shown && w->nGlyph && !w->gave && !w->dirty)) continue;
        const unsigned cnt = reg16(0x08 + 2 * w->bg), sbase = ((dc >> 27) & 7) * 0x10000 + ((cnt >> 8) & 31) * 0x800, size = (cnt >> 14) & 3;
        sig = fnv(sig, reg16(0x10 + 4 * w->bg)); sig = fnv(sig, reg16(0x12 + 4 * w->bg)); sig = fnv(sig, cnt);
        for (int r = 0; r < w->hT; r++) for (int c = 0; c < w->wT; c++) {
            const int tx = w->left + c, ty = w->top + r;
            const unsigned blk = size == 1 ? (unsigned)(tx >> 5) : size == 2 ? (unsigned)(ty >> 5) : size == 3 ? (unsigned)((tx >> 5) + 2 * (ty >> 5)) : 0u;
            const unsigned a = sbase + blk * 0x800 + (unsigned)(((ty & 31) * 32 + (tx & 31)) * 2);
            const unsigned e = a + 1 < 0x80000 ? (vr[a] | (vr[a + 1] << 8)) : 0xFFFFu;
            const unsigned want = ((w->base + r * w->wT + c) & 0x3FFu) | ((unsigned)w->pal << 12);
            vis[i][r * w->wT + c] = (u8)((e & 0xF3FFu) == want);   /* flips are not ours */
            sig = fnv(sig, vis[i][r * w->wT + c]); }
        shownN++; }
    if (gHave && sig == gLastSig) return 1;
    gLastSig = sig; gHave = 1;
    for (int y = 0; y < 240; y++) memset(buf + y * 512, 0, 400 * 4);
    unsigned drawnPx = 0, nGl = 0;
    for (int i = 0; i < kWins; i++) { GtWin *w = &gWin[i]; if (!(w->used && w->shown && w->nGlyph && !w->gave && !w->dirty)) continue;
        if (!(dc & (0x100u << w->bg))) continue;     /* BG switched off */
        int bmode = 0, ev = 0;
        if (((bld >> 6) & 3) >= 2 && (bld & (1u << w->bg))) { bmode = ((bld >> 6) & 3) - 1; ev = bldy > 16 ? 16 : (int)bldy; }
        if (!bmode && (mb >> 14) >= 1 && (mb >> 14) <= 2) { bmode = (int)(mb >> 14); ev = (int)(mb & 31); if (ev > 16) ev = 16; }
        const int hofs = (int)(reg16(0x10 + 4 * w->bg) & 0x1FF), vofs = (int)(reg16(0x12 + 4 * w->bg) & 0x1FF);
        u32 col[16]; for (int n = 1; n < 16; n++) col[n] = gfx555(*(const u16 *)(s_HW_BG_PLTT + ((w->pal * 16 + n) & 255) * 2), bmode, ev);
        /* glyphs in print order; a run = same line, small gaps; the run's start is placed by the 1.25 scaled position, its letters
           follow at the font's own pixel advance (1 font pixel = 1 screen pixel) */
        const GtGlyph *prev = 0; int ox = 0, oy = 0, rx = 0, ry = 0;
        for (int gi = 0; gi < kGlyphs; gi++) { const GtGlyph *g = &gGl[gi]; if (!g->used || g->win != i) continue; nGl++;
            if (!prev || g->y != prev->y || g->x < prev->x || g->x - prev->x > 24) {
                const int dsx = w->left * 8 + g->x - hofs, dsy = w->top * 8 + g->y - vofs;
                ox = fx + (dsx * fw + (dsx >= 0 ? 128 : -128)) / 256; oy = fy + (dsy * fh + (dsy >= 0 ? 96 : -96)) / 192 + 2; rx = g->x; ry = g->y; }
            prev = g;
            for (int py = 0; py < 32; py++) { const int Y = g->y + py; if (Y < 0 || Y >= w->hT * 8) continue;
                for (int px = 0; px < 16; px++) { const int X = g->x + px; if (X < 0 || X >= w->wT * 8) continue;
                    const u8 n = (g->px[(((py >> 3) * 2 + (px >> 3)) << 5) + ((py & 7) << 2) + ((px & 7) >> 1)] >> ((px & 1) * 4)) & 15;
                    if (!n || n == w->fill || !vis[i][(Y >> 3) * w->wT + (X >> 3)]) continue; /* the fill colour is the box itself: never redrawn */
                    const int nx = ox + (g->x - rx) + px, ny = oy + (g->y - ry) + py;
                    if (nx < 0 || nx >= 400 || ny < 0 || ny >= 240) continue;
                    buf[ny * 512 + nx] = col[n]; drawnPx++; } } } }
    *dirty = 1;
    { static unsigned n; if (n < 12 || (n % 600) == 0) PSPNativeMemLog("[GTEXT] native layer: %u windows, %u glyphs, %u pixels (recorded %u, erased uploads %u, plain uploads %u, pool full %u)", shownN, nGl, drawnPx, gRecorded, gErased, gPlain, gFull); n++; }
    return 1;
}
