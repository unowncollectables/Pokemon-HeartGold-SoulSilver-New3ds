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

extern u32 native_PmAgbCartridgeGetOffsets(u32, u32, u32, u32);
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
    PSPNativeMemLog("[SLOT2] virtual CTRDG attached (work copy only)");
    return 1;
}

void P3DS_CtrdgEject(void)
{
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
            sFlashDirty = 1;
            persistWork();
        } else {
            memcpy(dst, sFlash + off, size);
        }
        return 1;
    }
    return 0;
}

/* ---- CTRDG wraps ---- */

void __wrap_CTRDG_Init(void)
{
    /* Refresh every call: a pak may have been inserted after boot. */
    PSPNativeMemLog("[SLOT2] CTRDG_Init present=%d", sPresent);
}

BOOL __wrap_CTRDG_IsExisting(void) { return sPresent; }
BOOL __wrap_CTRDG_IsAgbCartridge(void) { return sPresent; }
BOOL __wrap_CTRDG_IsPulledOut(void) { return 0; }
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

static BOOL cpuCopy(const void *src, void *dest, u32 size)
{
    u32 s = (u32)(uintptr_t)src;
    u32 d = (u32)(uintptr_t)dest;
    if ((s >= GBA_ROM_BASE && s < GBA_FLASH_BASE + FLASH_SIZE) ||
        (d >= GBA_ROM_BASE && d < GBA_FLASH_BASE + FLASH_SIZE))
        PSPNativeMemLog("[SLOT2] CpuCopy src=%08x dst=%08x size=%u present=%d", s, d, size, sPresent);
    /* Official CpuCopy8: if dest is on the cart, the transfer is a write. */
    if (d >= GBA_ROM_BASE && d < GBA_FLASH_BASE + FLASH_SIZE)
        return copyCart(d, (void *)src, size, 1);
    if (s >= GBA_ROM_BASE && s < GBA_FLASH_BASE + FLASH_SIZE)
        return copyCart(s, dest, size, 0);
    if (!src || !dest) return 0;
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
    sFlashDirty = 1;
    persistWork();
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
    sFlashDirty = 1;
    persistWork();
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

/* Forward official identification / ROM signature lookup; never abort().
   When no virtual pak is inserted, keep the port stub's no-cart results so
   overlay-74 init cannot hang in the native signature/ROM-read path. */
u32 __wrap_PmAgbCartridgeGetOffsets(u32 arg)
{
    PSPNativeMemLog("[SLOT2] GetOffsets arg=%u present=%d", arg, sPresent);
    if (!sPresent)
        return 1; /* official "no cartridge" result used by DetectInsertedGBACart */
    {
        u32 r = native_PmAgbCartridgeGetOffsets(arg, 0, 0, 0);
        PSPNativeMemLog("[SLOT2] GetOffsets native -> %u", r);
        return r;
    }
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
