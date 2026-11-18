/* Host build of p3ds_gamefont.c for tools/sharp_text_preview.cpp: stubs for the 3DS-side calls, and the font bytes are handed in directly. */
#include <stdint.h>
#include <stdarg.h>
void PSPNativeMemLog(const char *f, ...) { (void)f; }
int P3DS_CfgInt(const char *k, int d) { (void)k; return d; }
unsigned long long svcGetSystemTick(void) { return 0; }
#include "p3ds_gamefont.c"
void host_set_font(uint8_t *d, uint32_t n) { gf[1] = d; gfLen[1] = n; gf[0] = d; gfLen[0] = n; }
