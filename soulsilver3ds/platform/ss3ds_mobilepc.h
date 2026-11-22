/* ss3ds mobile PC (Settings > SYSTEM, default OFF): a 7th JOHTO tab opens the game's own
   box app (PCBox_LaunchApp mode 2, Move Pokémon) from an idle overworld. Hook 0293 does the
   launch and the first-time unlock message. Never writes the .sav; unlock "already shown"
   is sdmc:/3ds/soulsilver3ds/mobile_pc.txt keyed by trainer ID. Field message + box launch
   follow pret/pokeheartgold (GPL-3.0) Task_PrintRegisteredKeyItemUseMessage and ScrCmd_158. */
#ifndef SS3DS_MOBILEPC_H
#define SS3DS_MOBILEPC_H

#ifdef __cplusplus
extern "C" {
#endif

int P3DS_MobilePcCfg(void);
int P3DS_MobilePcDevCfg(void);
int P3DS_MobilePcTabOn(void); /* cfg on, and (game clear published or dev bypass) */
void P3DS_MobilePcSetCfg(int on);
void P3DS_MobilePcSetDev(int on);
int P3DS_MobilePcUnlockShown(unsigned tid);
void P3DS_MobilePcUnlockMarkShown(unsigned tid);
void P3DS_MobilePcRequestOpen(void);
void P3DS_MobilePcRequestPreview(void);
int P3DS_MobilePcAnimFrame(void); /* -1 = no animation; 0..kMobilePcAnimLast while playing */
void P3DS_MobilePcAnimTick(void);

extern volatile int P3DS_MobilePcReq;         /* 1 = open the box app (game thread) */
extern volatile int P3DS_MobilePcPreviewReq;  /* 1 = play unlock message+anim, do not write sidecar */
extern volatile int P3DS_MobilePcUnlockPlay;  /* 1 while the game-thread unlock task is running */
extern volatile int P3DS_GameClear;           /* hook 0293: CheckGameClearFlag, read-only */

enum { kMobilePcAnimLast = 40 };

#ifdef __cplusplus
}
#endif

#endif
