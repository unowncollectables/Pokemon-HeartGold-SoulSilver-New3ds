/* Virtual GBA Slot-2 bus. Official migrate (overlay 74) already talks CTRDG;
   these wraps satisfy that API from a work-copy .sav + the user's .gba.
   menu_native.c currently abort()s if a cart is present; we --wrap those
   stubs and forward GetOffsets / signature lookup to the real native_* path. */
#include "ss3ds_ctrdg.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

extern void PSPNativeMemLog(const char *, ...);

#define GBA_ROM_BASE  0x08000000u
#define GBA_ROM_END   0x0A000000u
#define GBA_FLASH_BASE 0x0A000000u
#define FLASH_SIZE    0x20000u
#define FLASH_SECTOR  0x1000u
#define FLASH_SHIFT   12
#define FLASH_COUNT   32

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int      BOOL;

typedef struct {
    u32 task, callback;
    u32 result;
    u8 *data;
    u8 *adr;
    u32 offset;
    u32 size;
    u8 *dst;
    u16 sec_num;
    u8 busy;
    u8 param[1];
} CTRDGTaskInfo;
typedef u32 (*CTRDG_TASK_FUNC)(CTRDGTaskInfo *);

typedef struct {
    u32 size;
    u16 shift;
    u16 count;
    u16 top;
    u8 reserved[2];
} CTRDGiFlashSector;

typedef struct {
    u32 romSize;
    CTRDGiFlashSector sector;
    u16 agbWait[2];
    u16 makerID;
    u16 deviceID;
} CTRDGFlashType;

/* Official SDK symbol; Identify must set this so sector size is known. */
extern const CTRDGFlashType *AgbFlash;

extern u32 native_ov74_02235230(u32, u32, u32, u32);
extern u32 OS_DisableIrqMask(u32);

static const CTRDGFlashType sFlash1M = {
    FLASH_SIZE,
    { FLASH_SECTOR, FLASH_SHIFT, FLASH_COUNT, 0, {0, 0} },
    { 0, 0 },
    0x00C2,
    0x0009
};

static int sPresent;
static int sEnabled = 1;
static char sRomPath[256];
static char sWorkPath[256];
static u8 sFlash[FLASH_SIZE];
static int sFlashDirty;
static FILE *sRom;
static long sRomSize;
static u32 sGameCode;
static u16 sMakerCode;
static u8 sHeader[0xC0];
static unsigned sWrites; /* flash sectors written since attach (log only) */

/* Write only the changed bytes of the work copy (a migrate writes ~28 sectors;
   rewriting all 128KB each time was slow and widened the window for a torn file). */
static void persistRange(u32 off, u32 size)
{
    FILE *f;
    if (!sPresent || !sWorkPath[0] || !size || off + size > FLASH_SIZE) return;
    f = fopen(sWorkPath, "r+b");
    if (!f) { sFlashDirty = 1; return; }
    if (fseek(f, (long)off, SEEK_SET) != 0 || fwrite(sFlash + off, 1, size, f) != size) {
        PSPNativeMemLog("[SLOT2] work copy write FAILED off=%05x size=%u", (unsigned)off, (unsigned)size);
        sFlashDirty = 1;
    }
    fflush(f);
    fclose(f);
}

static void persistWork(void)
{
    FILE *f;
    if (!sPresent || !sFlashDirty || !sWorkPath[0]) return;
    f = fopen(sWorkPath, "wb");
    if (!f) {
        PSPNativeMemLog("[SLOT2] persist FAILED %s", sWorkPath);
        return;
    }
    if (fwrite(sFlash, 1, FLASH_SIZE, f) != FLASH_SIZE)
        PSPNativeMemLog("[SLOT2] persist short write %s", sWorkPath);
    else {
        sFlashDirty = 0;
        PSPNativeMemLog("[SLOT2] persisted work copy %s", sWorkPath);
    }
    fclose(f);
}

static int loadWork(const char *path)
{
    FILE *f = fopen(path, "rb");
    long n;
    if (!f) return 0;
    memset(sFlash, 0xFF, FLASH_SIZE);
    n = (long)fread(sFlash, 1, FLASH_SIZE, f);
    fclose(f);
    if (n < 0x8000) {
        PSPNativeMemLog("[SLOT2] work copy too small (%ld) %s", n, path);
        return 0;
    }
    /* 32KB/64KB dumps are one slot; the second 64KB stays 0xFF (empty slot). */
    if (n < (long)FLASH_SIZE)
        PSPNativeMemLog("[SLOT2] padded work copy %ld -> %u (64KB-safe) %s", n, (unsigned)FLASH_SIZE, path);
    sFlashDirty = 0;
    return 1;
}

static int openRom(const char *path)
{
    if (sRom) { fclose(sRom); sRom = 0; }
    sRom = fopen(path, "rb");
    if (!sRom) return 0;
    if (fread(sHeader, 1, 0xC0, sRom) != 0xC0) { fclose(sRom); sRom = 0; return 0; }
    fseek(sRom, 0, SEEK_END);
    sRomSize = ftell(sRom);
    memcpy(&sGameCode, sHeader + 0xAC, 4);
    memcpy(&sMakerCode, sHeader + 0xB0, 2);
    PSPNativeMemLog("[SLOT2] ROM %s size=%ld code=%.4s ver=%u", path, sRomSize,
                    (char *)(sHeader + 0xAC), (unsigned)sHeader[0xBC]);
    return 1;
}

int P3DS_CtrdgAttach(const char *rom_path, const char *work_sav_path)
{
    P3DS_CtrdgEject();
    if (!rom_path || !work_sav_path) return 0;
    if (!openRom(rom_path)) {
        PSPNativeMemLog("[SLOT2] cannot read ROM %s", rom_path);
        return 0;
    }
    if (!loadWork(work_sav_path)) {
        PSPNativeMemLog("[SLOT2] cannot load work sav %s", work_sav_path);
        fclose(sRom); sRom = 0;
        return 0;
    }
    snprintf(sRomPath, sizeof sRomPath, "%s", rom_path);
    snprintf(sWorkPath, sizeof sWorkPath, "%s", work_sav_path);
    sPresent = 1;
    sEnabled = 1;
    sWrites = 0;
    PSPNativeMemLog("[SLOT2] virtual CTRDG attached (work copy only), header %s",
                    P3DS_CtrdgCheckHeader(sHeader) == 0 ? "ok" : "BAD (migrate row will not show)");
    return 1;
}

void P3DS_CtrdgEject(void)
{
    if (sPresent && sWrites)
        PSPNativeMemLog("[SLOT2] eject: %u flash sector(s) were written to the work copy", sWrites);
    persistWork();
    if (sRom) { fclose(sRom); sRom = 0; }
    sPresent = 0;
    sRomPath[0] = 0;
    sWorkPath[0] = 0;
    sRomSize = 0;
    sGameCode = 0;
    sMakerCode = 0;
    memset(sHeader, 0, sizeof sHeader);
}

int P3DS_CtrdgPresent(void) { return sPresent; }

static int readRom(u32 off, void *dst, u32 size)
{
    if (!sRom || !dst || !size) return 0;
    if ((long)off >= sRomSize) {
        memset(dst, 0xFF, size);
        return 1;
    }
    if (fseek(sRom, (long)off, SEEK_SET) != 0) return 0;
    {
        size_t n = fread(dst, 1, size, sRom);
        if (n < size) memset((u8 *)dst + n, 0xFF, size - n);
    }
    return 1;
}

static int copyCart(u32 addr, void *dst, u32 size, int writing)
{
    if (!sPresent || !dst || !size) return 0;
    if (addr >= GBA_ROM_BASE && addr < GBA_ROM_END) {
        if (writing) return 0; /* ROM is read-only */
        return readRom(addr - GBA_ROM_BASE, dst, size);
    }
    if (addr >= GBA_FLASH_BASE && addr < GBA_FLASH_BASE + FLASH_SIZE) {
        u32 off = addr - GBA_FLASH_BASE;
        if (off + size > FLASH_SIZE) return 0;
        if (writing) {
            memcpy(sFlash + off, dst, size);
            persistRange(off, size);
        } else {
            memcpy(dst, sFlash + off, size);
        }
        return 1;
    }
    return 0;
}

/* ---- CTRDG wraps ---- */

/* Do not replace nitromain-perf/ctrdg_absent.c's __wrap_CTRDG_Init.
   That wrap publishes moduleID=0xffff / HW_IS_CTRDG_EXIST=0 so official
   unwrapped CTRDG paths never start an ARM7/PXI wait. A log-only Init
   here used to win via --allow-multiple-definition and skipped that
   publish. The Azahar continue-menu crash itself was slot2Dirs()
   recursing on first PrepareCart; keep the absent Init anyway. */

BOOL __wrap_CTRDG_IsExisting(void) { return sPresent; }
BOOL __wrap_CTRDG_IsAgbCartridge(void) { return sPresent; }
/* With the pak ejected (EMPTY picked while the migrate row is still listed) the official "cartridge removed" error shows. */
BOOL __wrap_CTRDG_IsPulledOut(void) { return !sPresent; }
BOOL __wrap_CTRDG_IsAgbCartridgePulledOut(void) { return 0; }
u32  __wrap_CTRDG_GetAgbGameCode(void) { return sPresent ? sGameCode : 0; }
u16  __wrap_CTRDG_GetAgbMakerCode(void) { return sPresent ? sMakerCode : 0; }

void __wrap_CTRDG_Enable(BOOL enable) { sEnabled = enable != 0; }
BOOL __wrap_CTRDG_IsEnabled(void) { return sEnabled; }
void __wrap_CTRDG_CheckEnabled(void) {}
void __wrap_CTRDG_CheckPulledOut(void) {}
void __wrap_CTRDG_TerminateForPulledOut(void)
{
    PSPNativeMemLog("[SLOT2] TerminateForPulledOut ignored (virtual cart)");
}

BOOL __wrap_CTRDG_Read32(const u32 *address, u32 *rdata)
{
    u32 tmp;
    if (!rdata) return 0;
    if (!copyCart((u32)(uintptr_t)address, &tmp, 4, 0)) return 0;
    *rdata = tmp;
    return 1;
}

/* CTRDG_CpuCopy*: the game only ever READS the cart through these (ROM header, signature, save layout table);
   flash writes go through the dedicated flash wraps below. The cart's address range overlaps the 3DS app heap,
   so a destination in that range is just RAM and must never be treated as a cart write (that broke the header
   copies into RAM: blank trainer name and "no six Pokemon" on the migrate screen). */
static BOOL cpuCopy(const void *src, void *dest, u32 size)
{
    u32 s = (u32)(uintptr_t)src;
    if (!src || !dest) return 0;
    if (sPresent && s >= GBA_ROM_BASE && s < GBA_FLASH_BASE + FLASH_SIZE)
        return copyCart(s, dest, size, 0);
    memcpy(dest, src, size);
    return 1;
}

BOOL __wrap_CTRDG_CpuCopy8(const void *src, void *dest, u32 size) { return cpuCopy(src, dest, size); }
BOOL __wrap_CTRDG_CpuCopy16(const void *src, void *dest, u32 size) { return cpuCopy(src, dest, size); }
BOOL __wrap_CTRDG_CpuCopy32(const void *src, void *dest, u32 size) { return cpuCopy(src, dest, size); }

u16 __wrap_CTRDG_IdentifyAgbBackup(int type)
{
    (void)type;
    if (!sPresent) return 1;
    AgbFlash = &sFlash1M;
    PSPNativeMemLog("[SLOT2] IdentifyAgbBackup -> Flash-1M (128KB, 32x4KB)");
    return 0;
}

void __wrap_CTRDG_ReadAgbFlash(u16 sec, u32 offset, u8 *dst, u32 size)
{
    u32 off;
    if (!sPresent || !dst) return;
    if (sec >= FLASH_COUNT) return;
    off = ((u32)sec << FLASH_SHIFT) + offset;
    if (off + size > FLASH_SIZE) return;
    memcpy(dst, sFlash + off, size);
}

void __wrap_CTRDG_ReadAgbFlashAsync(u16 sec, u32 offset, u8 *dst, u32 size, CTRDG_TASK_FUNC cb)
{
    static CTRDGTaskInfo info;
    __wrap_CTRDG_ReadAgbFlash(sec, offset, dst, size);
    if (cb) {
        memset(&info, 0, sizeof info);
        info.result = 0;
        info.dst = dst;
        info.offset = offset;
        info.size = size;
        info.sec_num = sec;
        cb(&info);
    }
}

static u32 writeSector(u16 sec, u8 *src, u32 verifysize)
{
    u32 off;
    if (!sPresent || !src) return 0x8000;
    if (sec >= FLASH_COUNT) return 0x8000;
    off = (u32)sec << FLASH_SHIFT;
    memcpy(sFlash + off, src, FLASH_SECTOR);
    if (verifysize && verifysize <= FLASH_SECTOR && memcmp(sFlash + off, src, verifysize) != 0)
        return 0x8000;
    persistRange(off, FLASH_SECTOR);
    sWrites++;
    return 0;
}

u32 __wrap_CTRDG_WriteAndVerifyAgbFlash(u16 sec, u8 *src, u32 verifysize)
{
    return writeSector(sec, src, verifysize);
}

void __wrap_CTRDG_WriteAndVerifyAgbFlashAsync(u16 sec, u8 *src, u32 verifysize, CTRDG_TASK_FUNC cb)
{
    static CTRDGTaskInfo info;
    u32 result = writeSector(sec, src, verifysize);
    if (cb) {
        memset(&info, 0, sizeof info);
        info.result = result;
        info.data = src;
        info.size = verifysize;
        info.sec_num = sec;
        cb(&info);
    }
}

u16 __wrap_CTRDG_WriteAgbFlashSector(u16 sec, u8 *src)
{
    return (u16)writeSector(sec, src, FLASH_SECTOR);
}

void __wrap_CTRDG_WriteAgbFlashSectorAsync(u16 sec, u8 *src, CTRDG_TASK_FUNC cb)
{
    __wrap_CTRDG_WriteAndVerifyAgbFlashAsync(sec, src, FLASH_SECTOR, cb);
}

u16 __wrap_CTRDG_EraseAgbFlashSector(u16 sec)
{
    u32 off;
    if (!sPresent || sec >= FLASH_COUNT) return 0x8000;
    off = (u32)sec << FLASH_SHIFT;
    memset(sFlash + off, 0xFF, FLASH_SECTOR);
    persistRange(off, FLASH_SECTOR);
    return 0;
}

void __wrap_CTRDG_EraseAgbFlashSectorAsync(u16 sec, CTRDG_TASK_FUNC cb)
{
    static CTRDGTaskInfo info;
    u16 result = __wrap_CTRDG_EraseAgbFlashSector(sec);
    if (cb) {
        memset(&info, 0, sizeof info);
        info.result = result;
        info.sec_num = sec;
        cb(&info);
    }
}

/* ---- Official cartridge identification, ported from overlay 74 ----
   PmAgbCartridgeGetOffsets / IdentifyPmAgbCartridge (asm/overlay_74_thumb.s,
   0x02233938 / 0x022339D4) in C, so the result does not depend on the
   translated copy's stack buffer and every failure says why in the log.
   Same tables and globals as the game: sPmAgbRomCodeMappings (30 x 8 bytes:
   game code, kind, version, language), sAgbCartNintendoLogo (0x9C bytes),
   sPmAgbCartridgeSpec, sPmAgbRomHeader, ov74_0223D33C. */
extern u8 ssdata_overlay_74_thumb_sPmAgbRomCodeMappings[];
extern u8 ssdata_overlay_74_thumb_sAgbCartNintendoLogo[];
extern u8 ssdata_overlay_74_thumb_sPmAgbRomHeader[];
extern u8 ssdata_overlay_74_thumb_ov74_0223D33C[];
extern void *ssdata_overlay_74_thumb_sPmAgbCartridgeSpec;

#define AGB_MAPPINGS 30

static void put32(u8 *p, u32 v) { memcpy(p, &v, 4); }

/* 0 ok, 4 logo or header checksum wrong (official codes). */
int P3DS_CtrdgCheckHeader(const unsigned char *hdr)
{
    unsigned i;
    u8 sum = 0;
    if (memcmp(hdr + 4, ssdata_overlay_74_thumb_sAgbCartNintendoLogo, 0x9C) != 0) return 4;
    for (i = 0xA0; i <= 0xBC; i++) sum = (u8)(sum + hdr[i]);
    if (hdr[0xBD] != (u8)-(sum + 0x19)) return 4;
    return 0;
}

static u32 identifyCart(void)
{
    u8 *map = ssdata_overlay_74_thumb_sPmAgbRomCodeMappings;
    int i;
    ssdata_overlay_74_thumb_sPmAgbCartridgeSpec = NULL;
    if (!sPresent) return 1;
    for (i = 0; i < AGB_MAPPINGS; i++) {
        u32 code;
        memcpy(&code, map + i * 8, 4);
        if (code == sGameCode) {
            ssdata_overlay_74_thumb_sPmAgbCartridgeSpec = map + i * 8;
            break;
        }
    }
    if (!ssdata_overlay_74_thumb_sPmAgbCartridgeSpec) {
        PSPNativeMemLog("[SLOT2] identify: game code %.4s is not a migrate game -> 2", (char *)(sHeader + 0xAC));
        return 2;
    }
    __wrap_CTRDG_Enable(1);
    if (P3DS_CtrdgCheckHeader(sHeader) != 0) {
        PSPNativeMemLog("[SLOT2] identify: Nintendo logo / header checksum wrong -> 4 (is %s a full ROM dump?)", sRomPath);
        return 4;
    }
    return 0;
}

u32 __wrap_PmAgbCartridgeGetOffsets(u32 arg)
{
    u8 *spec;
    u32 r;
    put32(ssdata_overlay_74_thumb_ov74_0223D33C + 8, arg);
    if (ssdata_overlay_74_thumb_sPmAgbCartridgeSpec) return 12; /* already linked */
    if (!sPresent) {
        ssdata_overlay_74_thumb_sPmAgbCartridgeSpec = NULL;
        return 1; /* official "no cartridge" */
    }
    r = identifyCart();
    if (r) return r;
    spec = (u8 *)ssdata_overlay_74_thumb_sPmAgbCartridgeSpec;
    if (spec[4] == 0) {
        /* Ruby / Sapphire: fixed save offsets, as the original writes them. */
        u8 *h = ssdata_overlay_74_thumb_sPmAgbRomHeader + 0x68;
        put32(h + 0x20, 0x890);
        put32(h + 0x24, 0x3AC0);
        put32(ssdata_overlay_74_thumb_ov74_0223D33C + 0x68, 0x1220);
        put32(h + 0x30, 9);
    } else {
        /* FRLG / Emerald: the save layout table at ROM 0x100. */
        readRom(0x100, ssdata_overlay_74_thumb_sPmAgbRomHeader, 0xFC);
    }
    if (__wrap_CTRDG_IdentifyAgbBackup(1) != 0) {
        PSPNativeMemLog("[SLOT2] identify: no flash -> 3");
        return 3;
    }
    {
        u16 lang;
        memcpy(&lang, spec + 6, 2);
        PSPNativeMemLog("[SLOT2] identify %.4s ok -> 0 (kind %u version %u language %u)",
                        (char *)(sHeader + 0xAC), spec[4], spec[5], lang);
    }
    return 0;
}

/* Overlay 74 PmAgbCartridge_GetVersionInternal / PmAgbCartridge_GetLanguage (asm 0x022339BC / 0x022339C8):
   read the linked spec (8 bytes: game code, kind, version, u16 language). The port had Missing() traps for both,
   which aborted DetectInsertedGBACart the moment a pak was identified. */
u32 __wrap_PmAgbCartridge_GetVersionInternal(void)
{
    const u8 *spec = (const u8 *)ssdata_overlay_74_thumb_sPmAgbCartridgeSpec;
    return spec ? spec[5] : 0;
}

u32 __wrap_PmAgbCartridge_GetLanguage(void)
{
    const u8 *spec = (const u8 *)ssdata_overlay_74_thumb_sPmAgbCartridgeSpec;
    u16 lang = 0;
    if (spec) memcpy(&lang, spec + 6, 2);
    return lang;
}

u32 __wrap_ov74_02235230(void)
{
    PSPNativeMemLog("[SLOT2] ov74_02235230 present=%d", sPresent);
    if (!sPresent)
        return 0;
    {
        u32 r = native_ov74_02235230(0, 0, 0, 0);
        PSPNativeMemLog("[SLOT2] ov74_02235230 native -> %u", r);
        return r;
    }
}

void __wrap_ov74_02236034(int enable)
{
    PSPNativeMemLog("[SLOT2] ov74_02236034 enable=%d ignored (virtual CTRDG)", enable);
}

/* DetectInsertedGBACart arms the cart-removed IRQ (overlay 74 0x02236074). There is no cart IRQ here; pull-out is
   always "not pulled out", so this is a no-op like ov74_02236034. */
void __wrap_ov74_02236074(void)
{
    PSPNativeMemLog("[SLOT2] ov74_02236074 (arm cart IRQ) ignored (virtual CTRDG)");
}
