// SPDX-License-Identifier: GPL-3.0-only
// IR register sequence and handshake based on francesco265/pwalkerHax (GPL-3.0).
// Register I/O uses the 3DS i2c::IR service to share the bus with the system.
#include "ss3ds_pwalker_ir.h"
#include <3ds.h>
#include <string.h>

#define IR_DEVICE_ID 13

enum {
    FIFO = 0x00, IER = 0x08, FCR = 0x10, LCR = 0x18, LSR = 0x28,
    TXLVL = 0x40, RXLVL = 0x48, IOSTATE = 0x58, EFCR = 0x78
};

static Handle irBus;
static bool configuring;
extern void P3DS_Crumb(const char *tag, void *caller);
extern void PSPNativeMemLog(const char *fmt, ...);

static Result sendCommand(u32 *command)
{
    const u32 request = command[0];
    const u32 reg = command[2];
    if (configuring)
        P3DS_Crumb("pw-i2c-init", (void *)(uintptr_t)reg);
    Result rc = svcSendSyncRequest(irBus);
    if (R_SUCCEEDED(rc))
        rc = (Result)command[1];
    if (configuring && R_FAILED(rc))
        PSPNativeMemLog("[PWALKER] i2c init command %04lx register %02lx: %08lx",
                        (unsigned long)(request >> 16), (unsigned long)reg,
                        (unsigned long)(u32)rc);
    return rc;
}

static bool readReg(u8 reg, u8 *value)
{
    u32 *command = getThreadCommandBuffer();
    command[0] = IPC_MakeHeader(9, 2, 0);
    command[1] = IR_DEVICE_ID;
    command[2] = reg;
    if (R_FAILED(sendCommand(command)))
        return false;
    *value = (u8)command[2];
    return true;
}

static bool writeReg(u8 reg, u8 value)
{
    u32 *command = getThreadCommandBuffer();
    command[0] = IPC_MakeHeader(5, 3, 0);
    command[1] = IR_DEVICE_ID;
    command[2] = reg;
    command[3] = value;
    return R_SUCCEEDED(sendCommand(command));
}

static bool writeFifo(const u8 *data, size_t count)
{
    // The UART consumes every byte after the FIFO register in one transfer.
    // The sequential-register service command has different semantics.
    u8 packet[65] = {FIFO};
    memcpy(packet + 1, data, count);
    u32 *command = getThreadCommandBuffer();
    command[0] = IPC_MakeHeader(0x14, 2, 2);
    command[1] = IR_DEVICE_ID;
    command[2] = count + 1;
    command[3] = IPC_Desc_StaticBuffer(count + 1, 1);
    command[4] = (u32)(uintptr_t)packet;
    return R_SUCCEEDED(sendCommand(command));
}

static bool configureIr(void)
{
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

static Result openIrService(Handle *handle)
{
    const char *name = "i2c::IR";
    bool registered = false;
    P3DS_Crumb("pw-i2c-check", NULL);
    Result rc = srvIsServiceRegistered(&registered, name);
    if (R_FAILED(rc))
        return rc;
    if (!registered)
        return MAKERESULT(RL_PERMANENT, RS_NOTFOUND, RM_SRV, RD_NOT_FOUND);

    // SRV can otherwise wait indefinitely when all I2C sessions are in use.
    P3DS_Crumb("pw-i2c-open", NULL);
    srvSetBlockingPolicy(true);
    rc = srvGetServiceHandleDirect(handle, name);
    srvSetBlockingPolicy(false);
    return rc;
}

bool SS3DS_PwalkerIrAvailable(void)
{
    Handle probe;
    if (R_FAILED(openIrService(&probe)))
        return false;
    svcCloseHandle(probe);
    return true;
}

bool SS3DS_PwalkerIrOpen(void)
{
    SS3DS_PwalkerIrClose();
    Result rc = openIrService(&irBus);
    if (R_FAILED(rc)) {
        irBus = 0;
        PSPNativeMemLog("[PWALKER] i2c::IR unavailable: %08lx", (unsigned long)(u32)rc);
        return false;
    }
    PSPNativeMemLog("[PWALKER] i2c::IR opened");
    configuring = true;
    bool ready = configureIr();
    configuring = false;
    if (!ready)
        SS3DS_PwalkerIrClose();
    return ready;
}

void SS3DS_PwalkerIrClose(void)
{
    if (!irBus)
        return;
    writeReg(EFCR, 6);
    writeReg(FCR, 0);
    writeReg(IER, BIT(4));
    writeReg(IOSTATE, BIT(0));
    svcCloseHandle(irBus);
    irBus = 0;
}

bool SS3DS_PwalkerIrListen(void)
{
    return writeReg(FCR, 3) && writeReg(EFCR, 4);
}

int SS3DS_PwalkerIrRead(uint8_t *data, size_t capacity)
{
    u8 available;
    if (!readReg(RXLVL, &available))
        return -1;
    size_t count = available < capacity ? available : capacity;
    for (size_t i = 0; i < count; i++) {
        if (!readReg(FIFO, &data[i]))
            return -1;
    }
    return (int)count;
}

bool SS3DS_PwalkerIrSend(const uint8_t *data, size_t length)
{
    if (!writeReg(FCR, 5) || !writeReg(EFCR, 2))
        return false;

    const u64 start = svcGetSystemTick();
    while (svcGetSystemTick() - start < SYSCLOCK_ARM11 / 20) {
        u8 status;
        if (!readReg(length ? TXLVL : LSR, &status))
            break;
        if (length) {
            size_t count = status < length ? status : length;
            if (!count)
                continue;
            if (count > 64)
                count = 64;
            if (!writeFifo(data, count))
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
