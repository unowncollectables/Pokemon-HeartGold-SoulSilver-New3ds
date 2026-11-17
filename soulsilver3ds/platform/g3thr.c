/* g3_thread plumbing (g3soft.cpp includes pspoke/nitro headers that clash with <3ds.h>, so the libctru calls live here). */
#include <3ds.h>
static LightSemaphore sem;
static LightSemaphore sem2;
static LightLock vbLk;
static int vbLkInit, splitOn, splitStarted;
int G3T_Start(void (*fn)(void *)) {
    LightSemaphore_Init(&sem, 0, 32767);
    if (!vbLkInit) { LightLock_Init(&vbLk); vbLkInit = 1; }
    return threadCreate(fn, NULL, 0x8000, 0x30, 2, false) != NULL;   /* core 2 */
}
/* g3_raw_split: second raw worker on core 3 (New 3DS). Idempotent — live Settings toggle may call again. */
int G3T_Start2(void (*fn)(void *)) {
    if (splitStarted) { splitOn = 1; return 1; } /* already running: just re-enable wake/lock */
    LightSemaphore_Init(&sem2, 0, 32767);
    if (!vbLkInit) { LightLock_Init(&vbLk); vbLkInit = 1; }
    if (threadCreate(fn, NULL, 0x8000, 0x30, 3, false) == NULL) return 0;   /* core 3 */
    splitStarted = 1; splitOn = 1; return 1;
}
/* Park worker 2: stop waking it / locking vb. Thread stays asleep on Wait2 (no join — 3DS threads are sticky). */
void G3T_Stop2(void) { splitOn = 0; }
void G3T_Wait(void) { LightSemaphore_Acquire(&sem, 1); }
void G3T_Signal(void) { LightSemaphore_Release(&sem, 1); }
void G3T_Wait2(void) { LightSemaphore_Acquire(&sem2, 1); }
void G3T_Signal2(void) { LightSemaphore_Release(&sem2, 1); }
void G3T_SleepNs(long long ns) { svcSleepThread(ns); }
/* Serialise TriClip2 / batch appends when two raw workers run (g3_raw_split). No-op cost when split OFF and uncontended. */
void G3T_VbLock(void) { if (splitOn) LightLock_Lock(&vbLk); }
void G3T_VbUnlock(void) { if (splitOn) LightLock_Unlock(&vbLk); }
int G3T_SplitOn(void) { return splitOn; }
