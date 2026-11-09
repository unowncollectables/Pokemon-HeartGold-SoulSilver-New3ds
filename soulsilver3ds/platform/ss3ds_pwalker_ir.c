// SPDX-License-Identifier: GPL-3.0-only
// IR register sequence and handshake based on francesco265/pwalkerHax (GPL-3.0).
// The I2C transaction sequence follows pwalkerHax and profi200/libn3ds.
#include "ss3ds_pwalker_ir.h"
#include <3ds.h>
#include <string.h>

#define I2C_BASE (OS_MMIO_VADDR + 0x48000)
#define IR_ADDRESS 0x9a

enum {
    I2C_STOP = BIT(0), I2C_START = BIT(1), I2C_ERROR = BIT(2),
    I2C_ACK = BIT(4), I2C_READ = BIT(5), I2C_BUSY = BIT(7)
};

enum {
    FIFO = 0x00, IER = 0x08, FCR = 0x10, LCR = 0x18, LSR = 0x28,
    TXLVL = 0x40, RXLVL = 0x48, IOSTATE = 0x58, EFCR = 0x78
};

// Poll for completion without raising the system I2C interrupt.
static vu8 *const bus = (vu8 *)I2C_BASE;
static bool opened;
static u16 savedControl;
static u16 savedClock;
extern void PSPNativeMemLog(const char *fmt, ...);

static bool waitIdle(void)
{
    const u64 start = svcGetSystemTick();
    while (bus[1] & I2C_BUSY) {
        if (svcGetSystemTick() - start >= SYSCLOCK_ARM11 / 100)
            return false;
    }
    return true;
}

static bool sendByte(u8 byte, u8 flags)
{
    bus[0] = byte;
    bus[1] = I2C_BUSY | flags;
    if (!waitIdle())
        return false;
    if (bus[1] & I2C_ACK)
        return true;
    bus[1] = I2C_BUSY | I2C_ERROR | I2C_STOP;
    return false;
}

static bool beginTransfer(u8 reg, bool read)
{
    // Retry only address selection. Once FIFO bytes have moved, retrying the
    // whole transaction could duplicate or drop part of an IR packet.
    for (unsigned attempt = 0; attempt < 3; attempt++) {
        if (!waitIdle())
            return false;
        if (sendByte(IR_ADDRESS, I2C_START) && sendByte(reg, 0) &&
            (!read || sendByte(IR_ADDRESS | 1, I2C_START)))
            return true;
    }
    return false;
}

static bool transfer(u8 reg, u8 *data, size_t length, bool read)
{
    if (!beginTransfer(reg, read))
        return false;

    for (size_t i = 0; i < length; i++) {
        const bool last = i + 1 == length;
        if (read) {
            bus[1] = I2C_BUSY | I2C_READ | (last ? I2C_STOP : I2C_ACK);
            if (!waitIdle())
                return false;
            data[i] = bus[0];
        } else if (!sendByte(data[i], last ? I2C_STOP : 0)) {
            return false;
        }
    }
    return true;
}

static bool writeReg(u8 reg, u8 value)
{
    return transfer(reg, &value, 1, false);
}

static bool irAvailable(void);

bool SS3DS_PwalkerIrOpen(void)
{
    if (!irAvailable() || !waitIdle())
        return false;
    // Match the standalone probe's bus timing for the IR session.
    savedControl = *(vu16 *)(bus + 2);
    savedClock = *(vu16 *)(bus + 4);
    *(vu16 *)(bus + 2) = BIT(1);
    *(vu16 *)(bus + 4) = 5u << 8;
    opened = true;
    if (!writeReg(EFCR, 6) || !writeReg(FCR, 6))
        return false;
    svcSleepThread(2000000);
    if (!writeReg(IER, 0) || !writeReg(IOSTATE, 0))
        return false;
    // With DLAB set, FIFO and IER address the baud divisor.
    for (int pass = 0; pass < 2; pass++) {
        if (!writeReg(LCR, 0x83) || !writeReg(FIFO, 10) ||
            !writeReg(IER, 0) || !writeReg(LCR, 3))
            return false;
        svcSleepThread(2000000);
    }
    return true;
}

static bool irAvailable(void)
{
    MemInfo memory;
    PageInfo page;
    return R_SUCCEEDED(svcQueryMemory(&memory, &page, I2C_BASE)) &&
           memory.state == MEMSTATE_IO &&
           (memory.perm & MEMPERM_READWRITE) == MEMPERM_READWRITE;
}

void SS3DS_PwalkerIrClose(void)
{
    if (!opened)
        return;
    opened = false;
    writeReg(EFCR, 6);
    writeReg(FCR, 0);
    writeReg(IER, BIT(4));
    writeReg(IOSTATE, BIT(0));
    if (waitIdle()) {
        *(vu16 *)(bus + 4) = savedClock;
        *(vu16 *)(bus + 2) = savedControl;
    } else {
        PSPNativeMemLog("[PWALKER] bus still busy at close; timing restore skipped");
    }
}

bool SS3DS_PwalkerIrListen(void)
{
    return writeReg(FCR, 3) && writeReg(EFCR, 4);
}

int SS3DS_PwalkerIrRead(uint8_t *data, size_t capacity)
{
    u8 available;
    if (!transfer(RXLVL, &available, 1, true)) {
        PSPNativeMemLog("[PWALKER] RX FIFO level read failed");
        return -1;
    }
    if (available > 64) {
        PSPNativeMemLog("[PWALKER] RX invalid FIFO level: %u", (unsigned)available);
        return -1;
    }
    size_t count = available < capacity ? available : capacity;
    if (count && !transfer(FIFO, data, count, true)) {
        PSPNativeMemLog("[PWALKER] RX FIFO read failed: requested %u, level %u",
                        (unsigned)count, (unsigned)available);
        return -1;
    }
    return (int)count;
}

static bool writeFifo(const u8 *data, size_t count)
{
    u8 chunk[64];
    memcpy(chunk, data, count);
    return transfer(FIFO, chunk, count, false);
}

bool SS3DS_PwalkerIrSend(const uint8_t *data, size_t length)
{
    const char *stage = "setup";
    const size_t total = length;
    bool complete = false;
    u8 fifoLevel = 0;
    u8 lineStatus = 0;
    // The system IR module can touch the same UART. Re-select the normal
    // register bank and wake the transceiver before arming each send.
    if (!writeReg(LCR, 3) || !writeReg(IER, 0) || !writeReg(IOSTATE, 0) ||
        !writeReg(EFCR, 6) || !writeReg(FCR, 5))
        goto finish;

    // An empty FIFO was just reset. Load it before enabling transmission
    // instead of waiting for the first TXLVL read to advertise space.
    stage = "preload";
    const size_t initial = length < 64 ? length : 64;
    if (!writeFifo(data, initial))
        goto finish;
    data += initial;
    length -= initial;

    stage = "enable";
    if (!writeReg(EFCR, 2))
        goto finish;
    u8 mode;
    stage = "enable readback";
    if (!transfer(EFCR, &mode, 1, true))
        goto finish;
    if ((mode & 6) != 2) {
        PSPNativeMemLog("[PWALKER] TX mode changed after enable: EFCR=%02x",
                        (unsigned)mode);
        goto finish;
    }

    const u64 start = svcGetSystemTick();
    stage = "timeout";
    while (svcGetSystemTick() - start < SYSCLOCK_ARM11 / 20) {
        u8 status;
        if (!transfer(length ? TXLVL : LSR, &status, 1, true)) {
            stage = length ? "FIFO status" : "line status";
            break;
        }
        if (length) {
            fifoLevel = status;
            if (!status) {
                // THR empty independently confirms the FIFO can be refilled.
                if (!transfer(LSR, &lineStatus, 1, true)) {
                    stage = "refill line status";
                    break;
                }
                if (lineStatus & BIT(5))
                    status = 64;
            }
            if (status > 64) {
                stage = "invalid FIFO level";
                break;
            }
            size_t count = status < length ? status : length;
            if (!count)
                continue;
            if (!writeFifo(data, count)) {
                stage = "FIFO write";
                break;
            }
            data += count;
            length -= count;
        } else {
            lineStatus = status;
            if (status & BIT(6)) {
                complete = true;
                break;
            }
        }
    }

finish:
    if (!complete && !strcmp(stage, "timeout")) {
        u8 lcr = 0xff, efcr = 0xff, ier = 0xff, io = 0xff;
        transfer(LCR, &lcr, 1, true);
        transfer(EFCR, &efcr, 1, true);
        transfer(IER, &ier, 1, true);
        transfer(IOSTATE, &io, 1, true);
        PSPNativeMemLog("[PWALKER] TX timeout state: LCR=%02x EFCR=%02x IER=%02x IO=%02x",
                        (unsigned)lcr, (unsigned)efcr, (unsigned)ier, (unsigned)io);
    }
    bool disabled = writeReg(EFCR, 6);
    bool cleared = writeReg(FCR, 0);
    if (!complete || !disabled || !cleared) {
        PSPNativeMemLog("[PWALKER] TX %s: queued %u/%u bytes, TXLVL=%02x LSR=%02x",
                        complete ? "cleanup" : stage,
                        (unsigned)(total - length), (unsigned)total,
                        (unsigned)fifoLevel, (unsigned)lineStatus);
    }
    return complete && disabled && cleared;
}
