/* Visual-only Slot 2 (GBA SLOT) prototype on the continue menu. Default OFF.
   No CTRDG access, no migrate, no save writes. */
#ifndef SS3DS_SLOT2_H
#define SS3DS_SLOT2_H

#ifdef __cplusplus
extern "C" {
#endif

int P3DS_Slot2Cfg(void);
void P3DS_Slot2SetCfg(int on);
void P3DS_Slot2ContinueTick(void);
void P3DS_Slot2ContinueEnd(void);
int P3DS_Slot2ListOpen(void);
int P3DS_Slot2BlockMenuInput(void);
int P3DS_Slot2ButtonHit(int x, int y);
void P3DS_Slot2OpenList(void);
void P3DS_Slot2Present(unsigned *top, unsigned *bot);

#ifdef __cplusplus
}
#endif
#endif
