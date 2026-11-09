#include "ss3ds_pwalker_ir.h"
#include <3ds.h>
#include <stdatomic.h>

extern void PSPNativeMemLog(const char *fmt, ...);

// The original overlay holds its IR mutex around reads, writes and teardown.
static bool listening;
static bool reportedFailure;

static void reportFailure(const char *operation)
{
    if (!reportedFailure) {
        PSPNativeMemLog("[PWALKER] IR %s failed", operation);
        reportedFailure = true;
    }
}

uint32_t SS3DS_Pwalker_SpiWaitInit(void)
{
    reportedFailure = false;
    listening = SS3DS_PwalkerIrOpen() && SS3DS_PwalkerIrListen();
    if (listening)
        PSPNativeMemLog("[PWALKER] IR ready");
    else {
        SS3DS_PwalkerIrClose();
        reportFailure("initialization");
    }
    return 0;
}

uint32_t SS3DS_Pwalker_SpiWaitGetStatus(void)
{
    return listening ? 0xaa : 0;
}

// These delays apply to the cartridge SPI bus, which the 3DS does not use.
uint32_t SS3DS_Pwalker_SetSpiWriteWaitCycles(uint32_t cycles)
{
    (void)cycles;
    return 0;
}

uint32_t SS3DS_Pwalker_SetSpiReadWaitCycles(uint32_t cycles)
{
    (void)cycles;
    return 0;
}

uint32_t SS3DS_Pwalker_SpiWaitReadRange(uint8_t *buffer)
{
    // The next overlay global starts 140 bytes after this buffer.
    const size_t capacity = 136;
    if (!listening)
        return 0;
    int first = SS3DS_PwalkerIrRead(buffer, capacity);
    if (first <= 0) {
        if (first < 0)
            reportFailure("receive");
        return 0;
    }

    // Once a packet starts, drain it until the UART goes quiet. With no
    // incoming data the call above returns immediately to the game worker.
    size_t length = (size_t)first;
    u64 lastByte = svcGetSystemTick();
    while (length < capacity &&
           svcGetSystemTick() - lastByte < SYSCLOCK_ARM11 / 500) {
        int count = SS3DS_PwalkerIrRead(buffer + length, capacity - length);
        if (count < 0) {
            reportFailure("receive");
            return 0;
        }
        if (count > 0) {
            length += (size_t)count;
            lastByte = svcGetSystemTick();
        }
    }
    for (size_t i = 0; i < length; i++)
        buffer[i] ^= 0xaa; // The original cartridge receive routine does this.
    return (uint32_t)length;
}

uint32_t SS3DS_Pwalker_SpiWaitWriteRange(const uint8_t *buffer, uint32_t length)
{
    if (!listening || !length || length > 136)
        return 0;
    // The game has already encoded the outbound bytes.
    bool sent = SS3DS_PwalkerIrSend(buffer, length);
    listening = SS3DS_PwalkerIrListen();
    if (!sent || !listening)
        reportFailure("transmit");
    return 0;
}

// Stop the worker cooperatively. The overlay requests shutdown with its
// mutex held, then releases that mutex before joining the worker.

extern void OS_ExitThread(void);
extern void OS_JoinThread(void *thread);
extern void OS_UnlockMutex(void *mutex);
extern void OS_Sleep(uint32_t milliseconds);

static atomic_bool stopping;
static void *stoppingThread;

uint32_t SS3DS_Pwalker_RequestStop(void *thread)
{
    SS3DS_PwalkerIrClose();
    listening = false;
    stoppingThread = thread;
    atomic_store(&stopping, true);
    return 0;
}

uint32_t SS3DS_Pwalker_UnlockAndJoin(void *mutex)
{
    OS_UnlockMutex(mutex);
    OS_JoinThread(stoppingThread);
    stoppingThread = NULL;
    atomic_store(&stopping, false);
    return 0;
}

uint32_t SS3DS_Pwalker_WorkerSleep(uint32_t milliseconds)
{
    if (atomic_load(&stopping))
        OS_ExitThread();
    OS_Sleep(milliseconds);
    return 0;
}

extern int P3DS_CfgInt(const char *key, int fallback);

int SS3DS_PwalkerCanLaunch(void)
{
    return P3DS_CfgInt("pokewalker", 0) != 0 && SS3DS_PwalkerIrAvailable();
}

uint32_t SS3DS_Pwalker_Idle(void)
{
    OS_Sleep(1);
    return 0;
}
