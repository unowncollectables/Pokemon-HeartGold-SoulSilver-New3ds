/* Slot 2: continue-menu GBA SLOT button + list + top view, plus a virtual
   CTRDG that serves a work COPY of the matching GBA .sav. Never writes the
   source .sav. Official migrate / Pal Park stay in overlay 74. */
#include "ss3ds_slot2.h"
#include "ss3ds_ctrdg.h"

#include <3ds.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

extern void PSPNativeMemLog(const char *, ...);
extern int P3DS_CfgInt(const char *, int);
extern void P3DS_CfgWrite(const char *, int);
extern int P3DS_GameBoxReady(void);
extern int P3DS_GameFontReady(void);
extern void P3DS_GameBoxDraw(unsigned int *img, int stride, int cw, int ch, int x, int y, int w, int h);
extern int P3DS_GameFontDraw(unsigned int *img, int stride, int cw, int ch, int x, int y, const char *t, unsigned int fg, unsigned int sh, int scale, int font);
extern int P3DS_GameFontWidth(int font, const char *t);
extern int P3DS_GameFontDraw34(unsigned int *img, int stride, int cw, int ch, int x, int y, const char *t, unsigned int fg, int font);
extern int P3DS_GameFontWidth34(int font, const char *t);
extern int P3DS_ScaleMode(void);

#define SLOT2_DIR  "sdmc:/3ds/soulsilver3ds/slot2"
#define SLOT2_WORK "sdmc:/3ds/soulsilver3ds/slot2-work"
#define SLOT2_LAST "sdmc:/3ds/soulsilver3ds/slot2-last.txt"
#define SLOT2_MAX 12
#define FLASH_SIZE 0x20000u
#define kFg 0x00505858u
#define kSh 0x00B8B8A8u
#define kEdge 0x00283C50u
#define kFill 0x00F4F4F4u
#define kFill2 0x00D8D8D8u
#define kSel 0x00F0D890u
#define kRed 0x003030E0u /* 0x00BBGGRR, like the backdrop below */

/* Official continue-menu arrow pads are 28x28 at (222,6) and (222,158). */
enum { kBtnX = 222, kBtnY = 40, kBtnS = 28 };

static int sOn = -1;
static int sInContinue;
static int sListOpen;
static int sInserted = -1; /* -1 = Empty */
static int sIgnoreHeld;    /* wait for the opening touch/A to be released */
static int sSel;
static int sN;
static int sScanned;
static int sNeedRedraw;
static int sPrepared;
static char sStatus[20]; /* top card, last line: INSERTED or why MIGRATE FROM is not shown */

typedef struct {
    char file[48];
    char name[16];
    char code[8];
    char note[12]; /* "" = can insert; else why not (shown in the list) */
} Slot2Cart;
static Slot2Cart sCarts[SLOT2_MAX];

static const struct { const char *code, *name; } kCodes[] = {
    { "BPEE", "EMERALD" }, { "BPEJ", "EMERALD" }, { "BPED", "EMERALD" },
    { "BPEF", "EMERALD" }, { "BPES", "EMERALD" }, { "BPEI", "EMERALD" },
    { "BPRE", "FIRERED" }, { "BPRJ", "FIRERED" }, { "BPRD", "FIRERED" },
    { "BPRF", "FIRERED" }, { "BPRS", "FIRERED" }, { "BPRI", "FIRERED" },
    { "BPGE", "LEAFGREEN" }, { "BPGJ", "LEAFGREEN" }, { "BPGD", "LEAFGREEN" },
    { "BPGF", "LEAFGREEN" }, { "BPGS", "LEAFGREEN" }, { "BPGI", "LEAFGREEN" },
    { "AXVE", "RUBY" }, { "AXVJ", "RUBY" }, { "AXVD", "RUBY" },
    { "AXVF", "RUBY" }, { "AXVS", "RUBY" }, { "AXVI", "RUBY" },
    { "AXPE", "SAPPHIRE" }, { "AXPJ", "SAPPHIRE" }, { "AXPD", "SAPPHIRE" },
    { "AXPF", "SAPPHIRE" }, { "AXPS", "SAPPHIRE" }, { "AXPI", "SAPPHIRE" },
};

int P3DS_Slot2Cfg(void)
{
    if (sOn < 0) {
        sOn = P3DS_CfgInt("slot2", 0) != 0;
        PSPNativeMemLog("[SLOT2] slot2=%d (visual GBA SLOT on the continue menu, default OFF)", sOn);
    }
    return sOn;
}

void P3DS_Slot2SetCfg(int on)
{
    sOn = on != 0;
    P3DS_CfgWrite("slot2", sOn);
    if (!sOn) {
        sListOpen = 0;
        sInContinue = 0;
        sInserted = -1;
        sPrepared = 0;
        P3DS_CtrdgEject();
    }
    PSPNativeMemLog("[SLOT2] slot2=%d from settings", sOn);
}

void P3DS_Slot2ContinueTick(void)
{
    if (!sInContinue)
        PSPNativeMemLog("[SLOT2] continue menu up (button %s)", P3DS_Slot2Cfg() ? "on" : "off");
    sInContinue = 1;
}

void P3DS_Slot2ContinueEnd(void)
{
    if (sInContinue)
        PSPNativeMemLog("[SLOT2] continue menu ended");
    sInContinue = 0;
    sListOpen = 0;
}

int P3DS_Slot2ListOpen(void) { return sListOpen && P3DS_Slot2Cfg(); }

int P3DS_Slot2BlockMenuInput(void) { return P3DS_Slot2Cfg() && (sListOpen || sIgnoreHeld); }

int P3DS_Slot2ButtonHit(int x, int y)
{
    if (!P3DS_Slot2Cfg() || !sInContinue || sListOpen) return 0;
    return x >= kBtnX && x < kBtnX + kBtnS && y >= kBtnY && y < kBtnY + kBtnS;
}

static const char *nameForCode(const char *code)
{
    unsigned i;
    for (i = 0; i < sizeof kCodes / sizeof kCodes[0]; i++)
        if (memcmp(code, kCodes[i].code, 4) == 0) return kCodes[i].name;
    return 0;
}

static void trimTitle(char *dst, const unsigned char *src, int n)
{
    int i, e = 0;
    for (i = 0; i < n && src[i]; i++) {
        unsigned char c = src[i];
        if (c >= 'a' && c <= 'z') c = (unsigned char)(c - 32);
        if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == ' ' || c == '-')
            dst[e++] = (char)c;
    }
    while (e > 0 && dst[e - 1] == ' ') e--;
    dst[e] = 0;
}

static int endsGba(const char *n)
{
    int L = (int)strlen(n);
    return L >= 4 && n[L - 4] == '.' &&
           (n[L - 3] == 'g' || n[L - 3] == 'G') &&
           (n[L - 2] == 'b' || n[L - 2] == 'B') &&
           (n[L - 1] == 'a' || n[L - 1] == 'A');
}

static void slot2Dirs(void)
{
    mkdir("sdmc:/3ds", 0755);
    mkdir("sdmc:/3ds/soulsilver3ds", 0755);
    mkdir(SLOT2_DIR, 0755);
    mkdir(SLOT2_WORK, 0755);
}

static void savName(char *dst, int n, const char *gbaFile)
{
    int L = (int)strlen(gbaFile);
    snprintf(dst, n, "%s", gbaFile);
    if (L >= 4) {
        dst[L - 4] = '.';
        dst[L - 3] = 's';
        dst[L - 2] = 'a';
        dst[L - 1] = 'v';
        if (n > L) dst[L] = 0;
    }
}

static void writeLast(const char *file)
{
    FILE *f;
    slot2Dirs();
    f = fopen(SLOT2_LAST, "w");
    if (!f) return;
    fprintf(f, "%s\n", file ? file : "EMPTY");
    fclose(f);
}

static int readLast(char *dst, int n)
{
    FILE *f = fopen(SLOT2_LAST, "r");
    int L;
    if (!f) return 0;
    if (!fgets(dst, n, f)) { fclose(f); return 0; }
    fclose(f);
    L = (int)strlen(dst);
    while (L > 0 && (dst[L - 1] == '\n' || dst[L - 1] == '\r' || dst[L - 1] == ' '))
        dst[--L] = 0;
    return L > 0;
}

/* Copy the user's source .sav into the work file if the work file is missing.
   32KB/64KB dumps are padded to 128KB with 0xFF. Never writes the source. */
static int ensureWork(const char *src, const char *work)
{
    struct stat st;
    FILE *in, *out;
    unsigned char buf[0x1000];
    size_t total = 0, n;
    slot2Dirs();
    if (stat(work, &st) == 0 && st.st_size >= 0x8000) return 1;
    in = fopen(src, "rb");
    if (!in) {
        PSPNativeMemLog("[SLOT2] no source save %s", src);
        return 0;
    }
    out = fopen(work, "wb");
    if (!out) { fclose(in); return 0; }
    while ((n = fread(buf, 1, sizeof buf, in)) > 0) {
        if (total + n > FLASH_SIZE) n = FLASH_SIZE - total;
        if (fwrite(buf, 1, n, out) != n) { fclose(in); fclose(out); return 0; }
        total += n;
        if (total >= FLASH_SIZE) break;
    }
    fclose(in);
    memset(buf, 0xFF, sizeof buf);
    while (total < FLASH_SIZE) {
        n = FLASH_SIZE - total;
        if (n > sizeof buf) n = sizeof buf;
        if (fwrite(buf, 1, n, out) != n) { fclose(out); return 0; }
        total += n;
    }
    fclose(out);
    PSPNativeMemLog("[SLOT2] made work copy %s (%u bytes, source left untouched)", work, (unsigned)FLASH_SIZE);
    return 1;
}

static int attachIndex(int idx)
{
    char gba[256], src[256], work[256], sav[64];
    if (idx < 0 || idx >= sN) {
        P3DS_CtrdgEject();
        return 0;
    }
    savName(sav, sizeof sav, sCarts[idx].file);
    snprintf(gba, sizeof gba, "%s/%s", SLOT2_DIR, sCarts[idx].file);
    snprintf(src, sizeof src, "%s/%s", SLOT2_DIR, sav);
    snprintf(work, sizeof work, "%s/%s", SLOT2_WORK, sav);
    if (!ensureWork(src, work)) return 0;
    if (!P3DS_CtrdgAttach(gba, work)) return 0;
    return 1;
}


/* A Gen 3 save has a 0x08012025 signature in the footer of its sectors (slot A at 0xFF8, slot B at 0xE000+0xFF8).
   A blank (all 0xFF) or missing dump has none, so the pak can never migrate anything. */
static int savHasData(const char *path)
{
    FILE *f = fopen(path, "rb");
    unsigned sig[2] = { 0, 0 };
    int ok = 0;
    if (!f) return 0;
    if (fseek(f, 0xFF8, SEEK_SET) == 0 && fread(&sig[0], 4, 1, f) == 1 && sig[0] == 0x08012025u) ok = 1;
    if (!ok && fseek(f, 0xE000 + 0xFF8, SEEK_SET) == 0 && fread(&sig[1], 4, 1, f) == 1 && sig[1] == 0x08012025u) ok = 1;
    fclose(f);
    return ok;
}

static void slot2Scan(void)
{
    DIR *d;
    struct dirent *e;
    sN = 0;
    sScanned = 1;
    slot2Dirs();
    d = opendir(SLOT2_DIR);
    if (!d) {
        PSPNativeMemLog("[SLOT2] no folder " SLOT2_DIR " (list is Empty only)");
        return;
    }
    while ((e = readdir(d)) != NULL && sN < SLOT2_MAX) {
        char path[192];
        unsigned char hdr[0xC0];
        FILE *f;
        const char *nm;
        if (e->d_name[0] == '.') continue;
        if (!endsGba(e->d_name)) continue;
        snprintf(path, sizeof path, "%s/%s", SLOT2_DIR, e->d_name);
        f = fopen(path, "rb");
        if (!f) continue;
        if (fread(hdr, 1, sizeof hdr, f) != sizeof hdr) { fclose(f); continue; }
        fclose(f);
        memset(&sCarts[sN], 0, sizeof sCarts[sN]);
        snprintf(sCarts[sN].file, sizeof sCarts[sN].file, "%s", e->d_name);
        memcpy(sCarts[sN].code, hdr + 0xAC, 4);
        sCarts[sN].code[4] = 0;
        nm = nameForCode(sCarts[sN].code);
        if (nm) snprintf(sCarts[sN].name, sizeof sCarts[sN].name, "%s", nm);
        else {
            trimTitle(sCarts[sN].name, hdr + 0xA0, 12);
            if (!sCarts[sN].name[0])
                snprintf(sCarts[sN].name, sizeof sCarts[sN].name, "%s", sCarts[sN].code[0] ? sCarts[sN].code : "PAK");
        }
        {
            /* Say up front why a pak cannot migrate, instead of a silent missing row. */
            char sav[64], src[256], work[256];
            struct stat st;
            savName(sav, sizeof sav, e->d_name);
            snprintf(src, sizeof src, "%s/%s", SLOT2_DIR, sav);
            snprintf(work, sizeof work, "%s/%s", SLOT2_WORK, sav);
            if (P3DS_CtrdgCheckHeader(hdr) != 0)
                snprintf(sCarts[sN].note, sizeof sCarts[sN].note, "BAD DUMP");
            else if (stat(src, &st) != 0 && stat(work, &st) != 0)
                snprintf(sCarts[sN].note, sizeof sCarts[sN].note, "NO .SAV");
            else if (!savHasData(stat(work, &st) == 0 ? work : src))
                snprintf(sCarts[sN].note, sizeof sCarts[sN].note, "EMPTY SAV");
            else if (!nm)
                snprintf(sCarts[sN].note, sizeof sCarts[sN].note, "NO MIGRATE");
        }
        PSPNativeMemLog("[SLOT2] found %s (%s / %s)%s%s", e->d_name, sCarts[sN].code, sCarts[sN].name,
                        sCarts[sN].note[0] ? " note " : "", sCarts[sN].note);
        sN++;
    }
    closedir(d);
    PSPNativeMemLog("[SLOT2] scanned %d pak(s)", sN);
}

void P3DS_Slot2OpenList(void)
{
    if (!P3DS_Slot2Cfg() || !sInContinue) return;
    if (!sScanned) slot2Scan();
    sListOpen = 1;
    sIgnoreHeld = 1;
    sSel = sInserted + 1;
    if (sSel < 0) sSel = 0;
    if (sSel > sN) sSel = 0;
    PSPNativeMemLog("[SLOT2] list opened (sel %d, %d paks, inserted %d)", sSel, sN, sInserted);
}

static void slot2Insert(int row)
{
    if (row <= 0) {
        sInserted = -1;
        P3DS_CtrdgEject();
        writeLast("EMPTY");
        PSPNativeMemLog("[SLOT2] inserted Empty (virtual CTRDG ejected)");
    } else if (row <= sN && sCarts[row - 1].note[0] && strcmp(sCarts[row - 1].note, "NO MIGRATE") != 0) {
        /* BAD DUMP / NO .SAV: nothing to attach. Keep the list up so the note stays visible. */
        PSPNativeMemLog("[SLOT2] %s not inserted: %s", sCarts[row - 1].file, sCarts[row - 1].note);
        sIgnoreHeld = 1;
        return;
    } else if (row <= sN) {
        if (!attachIndex(row - 1)) {
            sInserted = -1;
            P3DS_CtrdgEject();
            PSPNativeMemLog("[SLOT2] insert failed for %s (need a matching .sav next to the .gba)",
                            sCarts[row - 1].file);
        } else {
            sInserted = row - 1;
            writeLast(sCarts[sInserted].file);
            PSPNativeMemLog("[SLOT2] inserted %s (%s) via work copy (source .sav untouched)",
                            sCarts[sInserted].name, sCarts[sInserted].code);
        }
    }
    sStatus[0] = 0;
    sListOpen = 0;
    sIgnoreHeld = 1; /* the same tap must not confirm Continue on the menu underneath */
    sNeedRedraw = 1; /* official MIGRATE FROM row is built from DetectInsertedGBACart */
}

static void px(unsigned int *img, int x, int y, unsigned int c)
{
    if (x >= 0 && x < 256 && y >= 0 && y < 192) img[y * 256 + x] = c;
}

static void fill(unsigned int *img, int x, int y, int w, int h, unsigned int c)
{
    int yy, xx;
    for (yy = 0; yy < h; yy++)
        for (xx = 0; xx < w; xx++)
            px(img, x + xx, y + yy, c);
}

static void drawBtn(unsigned int *bot)
{
    int x, y;
    const int x0 = kBtnX, y0 = kBtnY, s = kBtnS;
    /* Compact beveled pad in the official arrow column (a GameBox is too thick).
       Light cream fill + 1px highlight / shadow to match the official pads. */
    for (y = 0; y < s; y++) for (x = 0; x < s; x++) {
        int edge = (x == 0 || x == s - 1 || y == 0 || y == s - 1);
        int corner = (x == 0 || x == s - 1) && (y == 0 || y == s - 1);
        unsigned int c;
        if (corner) continue;
        if (edge) c = kEdge;
        else if (y == 1 || x == 1) c = 0x00FFFFFFu;
        else if (y >= s - 3 || x >= s - 3) c = kFill2;
        else c = 0x00F8F8F8u;
        px(bot, x0 + x, y0 + y, c);
    }
    if (P3DS_GameFontReady()) {
        const char *t = "GBA";
        int tw = P3DS_GameFontWidth(1, t);
        int ty = y0 + (s - 12) / 2;
        if (tw <= s - 4)
            P3DS_GameFontDraw(bot, 256, 256, 192, x0 + (s - tw) / 2, ty, t, kEdge, kSh, 1, 1);
        else {
            tw = P3DS_GameFontWidth34(1, t);
            P3DS_GameFontDraw34(bot, 256, 256, 192, x0 + (s - tw) / 2, ty, t, kEdge, 1);
        }
    }
}

static void drawTop(unsigned int *top)
{
    const int x = 48, y = 40, w = 160, h = 80;
    const char *title = "GBA SLOT";
    int tw;
    if (!P3DS_GameBoxReady()) return;
    P3DS_GameBoxDraw(top, 256, 256, 192, x, y, w, h);
    if (!P3DS_GameFontReady()) return;
    tw = P3DS_GameFontWidth(1, title);
    P3DS_GameFontDraw(top, 256, 256, 192, x + (w - tw) / 2, y + 8, title, kFg, kSh, 1, 1);
    if (sInserted >= 0 && sInserted < sN) {
        tw = P3DS_GameFontWidth(1, sCarts[sInserted].name);
        P3DS_GameFontDraw(top, 256, 256, 192, x + (w - tw) / 2, y + 32, sCarts[sInserted].name, kFg, kSh, 1, 1);
        tw = P3DS_GameFontWidth(1, sCarts[sInserted].code);
        P3DS_GameFontDraw(top, 256, 256, 192, x + (w - tw) / 2, y + 48, sCarts[sInserted].code, kFg, kSh, 1, 1);
        {
            const char *st = sStatus[0] ? sStatus : "INSERTED";
            tw = P3DS_GameFontWidth(1, st);
            P3DS_GameFontDraw(top, 256, 256, 192, x + (w - tw) / 2, y + 64, st, sStatus[0] && strcmp(st, "MIGRATE READY") ? kRed : kFg, kSh, 1, 1);
        }
    } else {
        tw = P3DS_GameFontWidth(1, "EMPTY");
        P3DS_GameFontDraw(top, 256, 256, 192, x + (w - tw) / 2, y + 32, "EMPTY", kFg, kSh, 1, 1);
        tw = P3DS_GameFontWidth(1, "NO GAME PAK");
        P3DS_GameFontDraw(top, 256, 256, 192, x + (w - tw) / 2, y + 48, "NO GAME PAK", kFg, kSh, 1, 1);
    }
}

static const char *rowLabel(int i, char *buf, int n)
{
    if (i <= 0) return "EMPTY";
    if (i - 1 < sN) {
        snprintf(buf, n, "%s", sCarts[i - 1].name);
        return buf;
    }
    return "?";
}

static void drawList(unsigned int *bot)
{
    const int x = 16, y = 8, w = 224, h = 168;
    const int rows = sN + 1;
    /* Official continue backdrop (lavender) and message-box fill (cream).
       GameBoxDraw skips palette 0, so the interior would otherwise show the menu underneath. */
    const unsigned back = 0x00FB6161u; /* official continue backdrop RGB(97,97,251) */
    int i, tw;
    char lab[20];
    fill(bot, 0, 0, 256, 192, back);
    fill(bot, x, y, w, h, kFill);
    if (!P3DS_GameBoxReady()) return;
    P3DS_GameBoxDraw(bot, 256, 256, 192, x, y, w, h);
    if (!P3DS_GameFontReady()) return;
    tw = P3DS_GameFontWidth(1, "GBA SLOT");
    P3DS_GameFontDraw(bot, 256, 256, 192, x + (w - tw) / 2, y + 4, "GBA SLOT", kFg, kSh, 1, 1);
    tw = P3DS_GameFontWidth(1, "WHICH GAME PAK?");
    P3DS_GameFontDraw(bot, 256, 256, 192, x + (w - tw) / 2, y + 20, "WHICH GAME PAK?", kFg, kSh, 1, 1);
    for (i = 0; i < rows && i < 8; i++) {
        const int ry = y + 40 + i * 16;
        const char *labp = rowLabel(i, lab, sizeof lab);
        if (i == sSel) fill(bot, x + 8, ry, w - 16, 16, kSel);
        P3DS_GameFontDraw(bot, 256, 256, 192, x + 16, ry, labp, kFg, kSh, 1, 1);
        if (sInserted + 1 == i) {
            int iw = P3DS_GameFontWidth34(1, "IN");
            P3DS_GameFontDraw34(bot, 256, 256, 192, x + w - 16 - iw, ry + 2, "IN", kFg, 1);
        } else if (i > 0 && i - 1 < sN && sCarts[i - 1].note[0]) {
            int iw = P3DS_GameFontWidth34(1, sCarts[i - 1].note);
            P3DS_GameFontDraw34(bot, 256, 256, 192, x + w - 16 - iw, ry + 2, sCarts[i - 1].note, kRed, 1);
        }
    }
    tw = P3DS_GameFontWidth34(1, "A: INSERT   B: CLOSE");
    P3DS_GameFontDraw34(bot, 256, 256, 192, x + (w - tw) / 2, y + h - 18, "A: INSERT   B: CLOSE", kFg, 1);
}

static int touchToDS(int px, int py, int *x, int *y)
{
    int dx, dy;
    if (P3DS_ScaleMode() == 0) { dx = px - 32; dy = py - 24; }
    else { dx = px * 256 / 320; dy = py * 192 / 240; }
    if (dx < 0 || dx > 255 || dy < 0 || dy > 191) return 0;
    *x = dx; *y = dy; return 1;
}

static void listInput(void)
{
    static u32 prev;
    u32 kh, kd;
    int rows = sN + 1;
    hidScanInput();
    kh = hidKeysHeld();
    if (sIgnoreHeld) {
        /* The tap that opened this list is often still held and sits over Empty. */
        if (kh & (KEY_TOUCH | KEY_A)) { prev = kh; return; }
        sIgnoreHeld = 0;
    }
    kd = kh & ~prev;
    prev = kh;
    if (kd & KEY_DUP) { if (sSel > 0) sSel--; }
    if (kd & KEY_DDOWN) { if (sSel + 1 < rows) sSel++; }
    if (kd & KEY_A) { slot2Insert(sSel); prev = kh; return; }
    if (kd & KEY_B) { sListOpen = 0; PSPNativeMemLog("[SLOT2] list closed"); prev = kh; return; }
    if (kd & KEY_TOUCH) {
        touchPosition tp;
        int x, y;
        hidTouchRead(&tp);
        if (touchToDS(tp.px, tp.py, &x, &y)) {
            const int x0 = 16, y0 = 48, w = 224;
            if (x >= x0 && x < x0 + w && y >= y0 && y < y0 + rows * 16) {
                int row = (y - y0) / 16;
                if (row >= 0 && row < rows) slot2Insert(row);
            }
        }
    }
}

void P3DS_Slot2Present(unsigned *top, unsigned *bot)
{
    if (!P3DS_Slot2Cfg()) return;
    if (!sInContinue && !sListOpen) return;
    if (!top || !bot) return;
    if (!sListOpen && sIgnoreHeld) {
        /* The list is closed (insert / Empty / B): release the menu once the tap or A that closed it is up.
           listInput() only runs while the list is open, so without this the menu stayed blocked for good. */
        hidScanInput();
        if (!(hidKeysHeld() & (KEY_TOUCH | KEY_A | KEY_B))) sIgnoreHeld = 0;
    }
    if (sListOpen) {
        listInput();
        if (sListOpen) {
            drawList(bot);
            drawTop(top);
        } else {
            drawBtn(bot);
            drawTop(top);
        }
        return;
    }
    if (sInContinue) {
        drawBtn(bot);
        drawTop(top);
    }
}

void P3DS_Slot2PrepareCart(void)
{
    char last[64];
    int i;
    if (sPrepared) return;
    PSPNativeMemLog("[SLOT2] PrepareCart cfg=%d present=%d scanned=%d inserted=%d",
                    P3DS_Slot2Cfg(), P3DS_CtrdgPresent(), sScanned, sInserted);
    if (!P3DS_Slot2Cfg()) {
        P3DS_CtrdgEject();
        sPrepared = 0;
        return;
    }
    if (P3DS_CtrdgPresent()) { sPrepared = 1; return; }
    if (!sScanned) slot2Scan();
    if (sInserted >= 0 && sInserted < sN) {
        if (!attachIndex(sInserted)) sInserted = -1;
        sPrepared = 1;
        return;
    }
    if (!readLast(last, sizeof last) || !last[0] || strcmp(last, "EMPTY") == 0) {
        sPrepared = 1;
        return;
    }
    for (i = 0; i < sN; i++) {
        if (strcmp(sCarts[i].file, last) == 0) {
            if (attachIndex(i)) {
                sInserted = i;
                PSPNativeMemLog("[SLOT2] last-pick %s reinserted", last);
            }
            break;
        }
    }
    sPrepared = 1;
}

int P3DS_Slot2VirtualCartPresent(void)
{
    return P3DS_Slot2Cfg() && P3DS_CtrdgPresent();
}

int P3DS_Slot2ConsumeRedraw(void)
{
    int n = sNeedRedraw;
    sNeedRedraw = 0;
    return n;
}

void P3DS_Slot2UnlinkAgbSpec(void)
{
    extern void *ssdata_overlay_74_thumb_sPmAgbCartridgeSpec;
    ssdata_overlay_74_thumb_sPmAgbCartridgeSpec = 0;
}

/* SS3DS 0296: DetectInsertedGBACart's result, so the top card can say why the
   official MIGRATE FROM row is missing (it is only built when all three pass). */
void P3DS_Slot2Detected(int offsets, int natdex, int langOk)
{
    if (!P3DS_Slot2Cfg() || !P3DS_CtrdgPresent()) { sStatus[0] = 0; return; }
    if (offsets == 4) snprintf(sStatus, sizeof sStatus, "BAD DUMP");
    else if (offsets == 2) snprintf(sStatus, sizeof sStatus, "NO MIGRATE");
    else if (offsets == 3) snprintf(sStatus, sizeof sStatus, "NO FLASH SAVE");
    else if (offsets != 0) snprintf(sStatus, sizeof sStatus, "NOT DETECTED");
    else if (!natdex) snprintf(sStatus, sizeof sStatus, "NEED NATIONAL DEX");
    else if (!langOk) snprintf(sStatus, sizeof sStatus, "OTHER LANGUAGE");
    else snprintf(sStatus, sizeof sStatus, "MIGRATE READY");
    PSPNativeMemLog("[SLOT2] detect offsets=%d natdex=%d lang_ok=%d -> %s", offsets, natdex, langOk, sStatus);
}
