/* ss3ds 0298: Black 2 / White 2 animated battle sprites (Settings > BATTLE "B2 sprites", key b2_sprites, default OFF).
 *
 * The pack sdmc:/3ds/soulsilver3ds/b2sprites.bin is made on the PC from the user's own Black 2 / White 2 ROM by
 * tools/b2sprites/build_pack.sh (AnimaEngine, MIT). It is never part of the build or the repository. Format: b2pack.py.
 *
 * Game side (hook 0298): pokemon.c tells us what each battle sprite template shows (SS3DS_B2Note), pokepic.c asks for
 * the Black 2 palette when a battler's palette loads (SS3DS_B2Palette) and for the current frame when it draws
 * (SS3DS_B2Pic). Each of the four battler slots gets its own platform texture (g3soft.cpp P3DS_B2TexPixels), so big
 * Pokemon are drawn at full size. Any "no" here makes the game draw its own sprite, unchanged.
 * The toggle is read once at the start of each battle, so a battle never mixes palettes. Never touches the save. */
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int P3DS_InBattle;
extern int P3DS_CfgInt(const char *, int);
extern void P3DS_CfgWrite(const char *, int);
extern void PSPNativeMemLog(const char *, ...);
extern u32 *P3DS_B2TexPixels(int slot, unsigned w, unsigned h);
extern void P3DS_B2TexFree(int slot);
extern void P3DS_Use(const char *);

#define B2_PATH "sdmc:/3ds/soulsilver3ds/b2sprites.bin"
#define B2_TEX_OFF 0x7FF80 /* + 8 * slot; port-side textures, see g3soft.cpp b2Lookup */
#define B2_PLTT 0x1FE80
#define SPECIES_SPINDA 327

typedef struct { u16 species; u8 form, flags; u32 off, size, pad; } B2Dir;

static int b2Cfg = -1;
int P3DS_B2Cfg(void) { if (b2Cfg < 0) { b2Cfg = P3DS_CfgInt("b2_sprites", 0) != 0; PSPNativeMemLog("[B2] b2_sprites=%d (Black 2 battle sprites, hook 0298, default OFF)", b2Cfg); } return b2Cfg; }
void P3DS_B2SetCfg(int on) { b2Cfg = on != 0; P3DS_CfgWrite("b2_sprites", b2Cfg); PSPNativeMemLog("[B2] b2_sprites=%d from settings (next battle)", b2Cfg); }

/* ---- pack directory ---- */
static int packState = -1; /* -1 not tried, 0 missing/bad, 1 ok */
static B2Dir *packDir;
static unsigned packN;
static int packOpen(void) {
    if (packState >= 0) return packState;
    packState = 0;
    FILE *f = fopen(B2_PATH, "rb");
    if (!f) { PSPNativeMemLog("[B2] %s missing: battles use the game's own sprites (make it with the builder's Black 2 sprites step)", B2_PATH); return 0; }
    u32 h[4];
    if (fread(h, 4, 4, f) == 4 && h[0] == 0x50533242u /* 'B2SP' */ && h[1] == 1 && h[2] && h[2] < 8192) {
        packDir = (B2Dir *)malloc(h[2] * sizeof(B2Dir));
        if (packDir && fread(packDir, sizeof(B2Dir), h[2], f) == h[2]) { packN = h[2]; packState = 1; }
        else { free(packDir); packDir = NULL; }
    }
    fclose(f);
    PSPNativeMemLog("[B2] b2sprites.bin %s: %u sprites", packState ? "ok" : "BAD (rebuild it with the builder)", packN);
    return packState;
}
static int packFind(int species, int form, int back) {
    int lo = 0, hi = (int)packN - 1;
    const u32 key = ((u32)species << 16) | ((u32)form << 8) | (u32)(back ? 1 : 0);
    while (lo <= hi) {
        const int mid = (lo + hi) / 2;
        const u32 k = ((u32)packDir[mid].species << 16) | ((u32)packDir[mid].form << 8) | packDir[mid].flags;
        if (k == key) return mid;
        if (k < key) lo = mid + 1; else hi = mid - 1;
    }
    return -1;
}

/* ---- slots (defined below) ---- */
typedef struct Slot Slot;
static void slotsFree(void);

/* ---- per battle latch ---- */
static int latched, wasBattle;
static int battleOn(void) {
    if (P3DS_InBattle && !wasBattle) {
        slotsFree(); /* last battle's sprites */
        latched = P3DS_B2Cfg() && packOpen() == 1;
        if (latched) { P3DS_Use("b2_sprites"); PSPNativeMemLog("[B2] battle: Black 2 sprites ON"); }
    }
    wasBattle = P3DS_InBattle != 0;
    return P3DS_InBattle && latched;
}
int SS3DS_B2Want(void) { return battleOn(); }

/* ---- what each template shows (filled from pokemon.c) ---- */
typedef struct { u16 narc, chr, pal; u16 species; u8 gender, facing, shiny, form, height, used; } Note;
static Note notes[32];
static unsigned noteNext;
void SS3DS_B2Note(int narc, int chr, int pal, int species, int gender, int facing, int shiny, int form, int height) {
    for (unsigned i = 0; i < 32; i++) {
        Note *n = &notes[i];
        if (n->used && n->narc == narc && n->chr == chr && n->pal == pal) {
            n->species = (u16)species; n->gender = (u8)gender; n->facing = (u8)facing; n->shiny = (u8)(shiny != 0); n->form = (u8)form; n->height = (u8)height;
            return;
        }
    }
    Note *n = &notes[noteNext++ & 31];
    n->used = 1; n->narc = (u16)narc; n->chr = (u16)chr; n->pal = (u16)pal; n->species = (u16)species; n->gender = (u8)gender;
    n->facing = (u8)facing; n->shiny = (u8)(shiny != 0); n->form = (u8)form; n->height = (u8)height;
}
static const Note *noteFind(int narc, int chr, int pal) {
    for (unsigned i = 0; i < 32; i++) if (notes[i].used && notes[i].narc == narc && notes[i].chr == chr && notes[i].pal == pal) return &notes[i];
    return NULL;
}

/* ---- slots ---- */
struct Slot {
    int dir;           /* pack entry index, -1 none */
    u8 *blob;          /* whole entry */
    unsigned w, h, tw, th, nUniq, nSeq, delayCs, bw;
    int ox, oy;
    const u16 *palN, *palS, *seq;
    const u8 *frames;
    u64 t0;
    int lastFrame;
    u16 lastPal[16];
};
static Slot slots[4] = { { .dir = -1 }, { .dir = -1 }, { .dir = -1 }, { .dir = -1 } };
static void slotsFree(void) { for (int i = 0; i < 4; i++) { free(slots[i].blob); slots[i].blob = NULL; slots[i].dir = -1; P3DS_B2TexFree(i); } }
static unsigned loads;

static unsigned pow2(unsigned v) { unsigned p = 8; while (p < v) p <<= 1; return p; }
static int slotLoad(Slot *s, int dir) {
    if (s->dir == dir) return 1;
    free(s->blob); s->blob = NULL; s->dir = -1;
    const B2Dir *d = &packDir[dir];
    if (d->size < 80 || d->size > 4u << 20) return 0;
    u8 *b = (u8 *)malloc(d->size);
    if (!b) { PSPNativeMemLog("[B2] out of memory for sprite %u (%u bytes)", (unsigned)dir, (unsigned)d->size); return 0; }
    FILE *f = fopen(B2_PATH, "rb");
    const int ok = f && !fseek(f, (long)d->off, SEEK_SET) && fread(b, 1, d->size, f) == d->size;
    if (f) fclose(f);
    const u16 *h16 = (const u16 *)b;
    const unsigned w = h16[0], hh = h16[1], nu = h16[4], ns = h16[5], dl = h16[6];
    const unsigned bw = (w + 1) / 2, hdr = 16 + 64 + ns * 2, hdrA = (hdr + 3) & ~3u;
    if (!ok || !w || !hh || w > 256 || hh > 256 || !nu || !ns || hdrA + nu * bw * hh > d->size) {
        free(b); PSPNativeMemLog("[B2] sprite %u unreadable (%s)", (unsigned)dir, ok ? "bad entry" : "read failed"); return 0;
    }
    s->blob = b; s->dir = dir; s->w = w; s->h = hh; s->tw = pow2(w); s->th = pow2(hh); s->nUniq = nu; s->nSeq = ns; s->delayCs = dl ? dl : 5; s->bw = bw;
    s->ox = (s16)h16[2]; s->oy = (s16)h16[3]; s->palN = h16 + 8; s->palS = h16 + 24; s->seq = h16 + 40; s->frames = b + hdrA;
    s->t0 = svcGetSystemTick(); s->lastFrame = -1;
    if (loads++ < 40) PSPNativeMemLog("[B2] slot %d: species %u form %u %s, %ux%u, %u frames (%u unique)", (int)(s - slots), d->species, d->form, (d->flags & 1) ? "back" : "front", w, hh, ns, nu);
    return 1;
}
static int slotFor(int slot, int narc, int chr, int pal, const Note **outNote) {
    if (slot < 0 || slot > 3 || !battleOn()) return 0;
    const Note *n = noteFind(narc, chr, pal);
    if (!n || n->species == SPECIES_SPINDA || !n->species) return 0;
    const int back = n->facing != 2;
    int dir = packFind(n->species, n->form, back);
    if (dir < 0 && n->form) dir = packFind(n->species, 0, back);
    if (dir < 0) return 0;
    if (!slotLoad(&slots[slot], dir)) return 0;
    *outNote = n;
    return 1;
}

int SS3DS_B2Palette(int slot, int narc, int chr, int pal, u16 *dst16) {
    const Note *n;
    if (!slotFor(slot, narc, chr, pal, &n)) return 0;
    memcpy(dst16, n->shiny ? slots[slot].palS : slots[slot].palN, 32);
    return 1;
}

typedef struct { int texOff, sizeS, sizeT, w, h, ox, oy, height, scale256, pltt; } SS3DSB2Pic;
int SS3DS_B2Pic(int slot, int narc, int chr, int pal, const u16 *livePltt, SS3DSB2Pic *o) {
    const Note *n;
    if (!slotFor(slot, narc, chr, pal, &n)) return 0;
    Slot *s = &slots[slot];
    (void)n;
    const u64 el = svcGetSystemTick() - s->t0;
    const unsigned step = (unsigned)(el / (CPU_TICKS_PER_MSEC * 10ull * s->delayCs));
    const int fr = s->seq[step % s->nSeq] < s->nUniq ? s->seq[step % s->nSeq] : 0;
    if (fr != s->lastFrame || memcmp(livePltt, s->lastPal, 32)) {
        u32 *px = P3DS_B2TexPixels(slot, s->tw, s->th);
        if (!px) return 0;
        u32 c[16];
        for (int i = 0; i < 16; i++) { const unsigned v = livePltt[i], r = v & 31, g = (v >> 5) & 31, b = (v >> 10) & 31; c[i] = (r << 3 | r >> 2) | ((g << 3 | g >> 2) << 8) | ((b << 3 | b >> 2) << 16) | 0xFF000000u; }
        c[0] = 0;
        memset(px, 0, s->tw * s->th * 4);
        const u8 *src = s->frames + (size_t)fr * s->bw * s->h;
        for (unsigned y = 0; y < s->h; y++) {
            u32 *row = px + y * s->tw;
            const u8 *sr = src + y * s->bw;
            for (unsigned x = 0; x < s->w; x++) row[x] = c[(sr[x >> 1] >> ((x & 1) * 4)) & 15];
        }
        s->lastFrame = fr;
        memcpy(s->lastPal, livePltt, 32);
    }
    unsigned ls = 0, lt = 0;
    while ((8u << ls) < s->tw) ls++;
    while ((8u << lt) < s->th) lt++;
    o->texOff = B2_TEX_OFF + slot * 8; o->sizeS = (int)ls; o->sizeT = (int)lt; o->w = (int)s->w; o->h = (int)s->h; o->ox = s->ox; o->oy = s->oy;
    o->height = n->height; o->scale256 = 256; o->pltt = B2_PLTT;
    return 1;
}
