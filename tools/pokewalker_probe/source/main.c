// SPDX-License-Identifier: GPL-3.0-only
// IR register sequence and handshake based on francesco265/pwalkerHax (GPL-3.0).
// The I2C transaction sequence follows pwalkerHax and profi200/libn3ds.
#include <3ds.h>
#include <stdbool.h>
#include <stdio.h>

#define I2C_BASE (OS_MMIO_VADDR + 0x48000)
#define IR_ADDRESS 0x9a
#define IR_XOR 0xaa

enum {
    I2C_STOP = BIT(0), I2C_START = BIT(1), I2C_ERROR = BIT(2),
    I2C_ACK = BIT(4), I2C_READ = BIT(5), I2C_IRQ = BIT(6), I2C_BUSY = BIT(7)
};

enum {
    FIFO = 0x00, IER = 0x08, FCR = 0x10, LCR = 0x18, LSR = 0x28,
    RXLVL = 0x48, IOSTATE = 0x58, EFCR = 0x78
};

static vu8 *const bus = (vu8 *)I2C_BASE;

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
    bus[1] = I2C_BUSY | I2C_IRQ | flags;
    if (!waitIdle())
        return false;
    if (bus[1] & I2C_ACK)
        return true;
    bus[1] = I2C_BUSY | I2C_IRQ | I2C_ERROR | I2C_STOP;
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
            bus[1] = I2C_BUSY | I2C_IRQ | I2C_READ | (last ? I2C_STOP : I2C_ACK);
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

static bool configureIr(void)
{
    if (!waitIdle())
        return false;
    *(vu16 *)(bus + 2) = BIT(1); // Clock stretching.
    *(vu16 *)(bus + 4) = 5u << 8;
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

static bool waitRegister(u8 reg, u8 mask, u32 timeoutMs)
{
    const u64 start = svcGetSystemTick();
    const u64 timeout = (u64)SYSCLOCK_ARM11 * timeoutMs / 1000;
    do {
        u8 value;
        if (!transfer(reg, &value, 1, true))
            return false;
        if (value & mask)
            return true;
    } while (svcGetSystemTick() - start < timeout);
    return false;
}

static size_t receive(u8 *data, size_t length, u32 timeoutMs)
{
    if (!writeReg(FCR, 3) || !writeReg(EFCR, 4))
        return 0;
    for (size_t i = 0; i < length; i++) {
        if (!waitRegister(RXLVL, 0xff, i == 0 ? timeoutMs : 20) ||
            !transfer(FIFO, &data[i], 1, true))
            return i;
    }
    return length;
}

static u16 checksum(const u8 *packet)
{
    // Eight-byte handshake header with the checksum field cleared.
    u32 sum = 2;
    for (size_t i = 0; i < 8; i += 2) {
        sum += (u32)packet[i] << 8 | packet[i + 1];
        sum = (sum & 0xffff) + (sum >> 16);
    }
    return (u16)sum;
}

static const char *handshake(void)
{
    u8 advertisement;
    if (!configureIr())
        return "Could not initialize the IR chip.";
    if (!receive(&advertisement, 1, 3500) || advertisement != 0x56)
        return "No Pokewalker advertisement received.";

    u8 syn[8] = {0xfa, 1}; // Start a session with ID zero.
    const u16 sum = checksum(syn);
    // The checksum travels low byte first.
    syn[2] = sum;
    syn[3] = sum >> 8;
    for (size_t i = 0; i < sizeof syn; i++)
        syn[i] ^= IR_XOR;

    if (!writeReg(FCR, 5) || !writeReg(EFCR, 2) ||
        !transfer(FIFO, syn, sizeof syn, false) ||
        !waitRegister(LSR, BIT(6), 50) || !writeReg(EFCR, 6) ||
        !writeReg(FCR, 0))
        return "Saw the Pokewalker, but sending failed.";

    u8 reply[8];
    const size_t received = receive(reply, sizeof reply, 500);
    if (received != sizeof reply) {
        printf("Reply: %u of %u bytes.\n", (unsigned)received, (unsigned)sizeof reply);
        return "Saw the Pokewalker, but no complete reply arrived.";
    }
    for (size_t i = 0; i < sizeof reply; i++)
        reply[i] ^= IR_XOR;
    const u16 expected = reply[2] | (u16)reply[3] << 8;
    reply[2] = reply[3] = 0;
    if (reply[0] != 0xf8 || checksum(reply) != expected)
        return "Received IR data, but the reply was invalid.";
    return "Success: Pokewalker returned a valid handshake.";
}

int main(void)
{
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    MemInfo memory;
    PageInfo page;
    const Result result = svcQueryMemory(&memory, &page, I2C_BASE);
    const bool available = R_SUCCEEDED(result) && memory.state == MEMSTATE_IO &&
                           (memory.perm & MEMPERM_READWRITE) == MEMPERM_READWRITE;
    puts("Pokewalker IR probe v2");
    puts("Point the two IR windows at each other.");
    puts("A: Listen, then select Connect on the walker.");
    puts("START: Exit.");
    if (!available)
        puts("IR registers are unavailable in this launch environment.");

    while (aptMainLoop()) {
        hidScanInput();
        const u32 pressed = hidKeysDown();
        if (pressed & KEY_START)
            break;
        if (available && (pressed & KEY_A)) {
            puts(handshake());
            writeReg(EFCR, 6);
            writeReg(FCR, 0);
            writeReg(IER, BIT(4));
            writeReg(IOSTATE, BIT(0));
        }
        gspWaitForVBlank();
    }
    gfxExit();
    return 0;
}
