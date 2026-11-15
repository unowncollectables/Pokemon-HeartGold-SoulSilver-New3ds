/* g3_thread plumbing (g3soft.cpp includes pspoke/nitro headers that clash with <3ds.h>, so the libctru calls live here). */
#include <3ds.h>
static LightSemaphore sem;
int G3T_Start(void (*fn)(void *)) {
    LightSemaphore_Init(&sem, 0, 32767);
    return threadCreate(fn, NULL, 0x8000, 0x30, 2, false) != NULL;   /* core 2: only the prefetch and log threads (lowest priority) live there */
}
void G3T_Wait(void) { LightSemaphore_Acquire(&sem, 1); }
void G3T_Signal(void) { LightSemaphore_Release(&sem, 1); }
void G3T_SleepNs(long long ns) { svcSleepThread(ns); }
