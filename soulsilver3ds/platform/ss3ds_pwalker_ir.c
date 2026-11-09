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

static bool transfer(u8 reg, u8 *data, size_t length, bool read)
{
    if (!waitIdle() || !sendByte(IR_ADDRESS, I2C_START) || !sendByte(reg, 0))
        return false;
    if (read && !sendByte(IR_ADDRESS | 1, I2C_START))
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
}

bool SS3DS_PwalkerIrListen(void)
{
    return writeReg(FCR, 3) && writeReg(EFCR, 4);
}

int SS3DS_PwalkerIrRead(uint8_t *data, size_t capacity)
{
    u8 available;
    if (!transfer(RXLVL, &available, 1, true))
        return -1;
    size_t count = available < capacity ? available : capacity;
    if (count && !transfer(FIFO, data, count, true))
        return -1;
    return (int)count;
}

bool SS3DS_PwalkerIrSend(const uint8_t *data, size_t length)
{
    if (!writeReg(FCR, 5) || !writeReg(EFCR, 2))
        return false;

    const u64 start = svcGetSystemTick();
    while (svcGetSystemTick() - start < SYSCLOCK_ARM11 / 20) {
        u8 status;
        if (!transfer(length ? TXLVL : LSR, &status, 1, true))
            break;
        if (length) {
            size_t count = status < length ? status : length;
            if (!count)
                continue;
            uint8_t chunk[64];
            if (count > sizeof chunk)
                count = sizeof chunk;
            memcpy(chunk, data, count);
            if (!transfer(FIFO, chunk, count, false))
                break;
            data += count;
            length -= count;
        } else if (status & BIT(6)) {
            return writeReg(EFCR, 6) && writeReg(FCR, 0);
        }
    }
    writeReg(EFCR, 6);
    writeReg(FCR, 0);
    return false;
}
