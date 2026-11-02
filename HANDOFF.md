
- 0147 (unbuilt, Grok): NULL guard in SysTask_TimedPaletteFade for the run 67 crash leaving Kurt's house after the Fast Ball. User builds and tests.
- 0148 (unbuilt, Grok): seed overlay_01_021F1348.s, drop the ov01_021F1A34 stub (run 67 Slowpoke Well crash, [SS-MISSING] ov01_021F1A34).
- 0149 (unbuilt, Grok): seed unk_0206979C.s, drop 3 stubs (Slowpoke Well [SS-MISSING] sub_02069998 after 0148).
- 0150 (unbuilt, Grok): native-provided sub_02020DA4 (link dupe after 0149, same as 0144).
- 0151 (unbuilt, Grok): sub_020154C4 only SPL_Deletes an emitter still in act_emtr_list (run 67 battle crash pc in SPLList_PopFront via SPL_Delete).
- 0152 (unbuilt, Grok): SPLManager_DeleteEmitter skips emitters not in activeEmitters (0151 only covered sub_020154C4; Supersonic still crashed). libss-particles.a must rebuild.
- 0153 (Grok): ss3ds_game.sh re-syncs particle glue.c and applies 0152 to a staged build.py, then rebuilds libss-particles.a (0151/0152 never reached an already-staged tree).
- 0153 is now also a pspoke patch: the builder runs pspoke/scripts/ss3ds_game.sh (live copy), not scripts/ss3ds_game.sh (reference). Put build-script fixes in a pspoke patch.
- 0154 (Grok): sub_020154B0 counts the real act_emtr_list instead of trusting node_num (Supersonic hang after 0151/0152 skipped deletes).
- 0155 (Grok): logs "[SS-PARTICLES] 0155 wait" every 120 polls while emitters are active, and "0155 skip delete" when 0151 skips one. Diagnostic for the Supersonic hang.
- 0156 (Grok): run 67 log showed "skip delete ... active=1": the battle waits forever on one stale emitter. If the delete target is not active and exactly one emitter is, delete that one.
- 0157 (Grok): Kurt Apricorn Box (overlay 58) crashed at ov58_02237CCC writing *(u32*)0x04000000; hook drops the raw DISPCNT writes. Run 67 also confirmed 0156 cleared the Supersonic hang.
- 0185 (Grok): seed Mystery Gift (overlay 74 / overlay_74_thumb.s); drop 58 ov74_ Missing() stubs so Init ov74_0222AE6C / Main / Exit are translated (mirror of 0164 Global Terminal).
- 0186 (Grok): maploader translate2 accepts `$` in literal-pool symbols (nitrocrypto TEMPNAME__$p16 etc. pulled in by Mystery Gift RC4 after 0185).
- 0187: CRYPTO_RC4Encrypt done natively in missing.c (it's an ARM routine, translator raised 'fallthrough without return'); added to native-provided.json.
- 0188: CRYPTO_SetAllocator (no-op) and CRYPTO_VerifySignature (returns TRUE) done natively; ParseSignHash and the BN_* RSA code are ARM and failed with 'unsupported stack layout'.
- 0189: native SVC_GetCRC16 + CRYPTO_RC4Init in missing.c; ov00_* wifi calls from ov74_02236AE0 stubbed with Missing() (download path, needs real DWC later).

- 0196 (Grok): Nd setupHttp - restore getDSdesc/attrs/mac so GetFileListNumAsync is not folded to FALSE (run 111). CLEAN network rebuild.
- 0195 (Grok): SVL Http_StartThread priority - compile-network.py rewrites dwc_svl.c 0x69 to 16 (CreateThreadReal aborts on >31; run 110 abort). CLEAN network rebuild.
- 0194 (Grok): MG WFC path — log DWC_Init in sub_02039FD8; platform wraps ov74_0222A560/A4CC so wireless=2 redirects GET VIA WIRELESS to WFC download; ov00_021EC294 is real DWC_SetMemFunc.
- 0193 (Grok): Mystery Gift WFC download — drop Missing() on 10 ov00 Nd/inet trampolines; real DWC_Nd* + CleanupInetAsync + NASLoginAbort in ss3ds_dwcstub.c; plain HTTP DLS URL + NdCancel fill in compile-network.py; IPGE→IPKE. Need ONLINE.
- 0192 (Grok): Mystery Gift cursor diagnostic — native ov74_02235FF8 + [SS-MG-CURSOR] logs in ListMenuUpdateCursorObj (MG-gated via gSS_MG_CursorDiag). No behavior change beyond PlaySE still firing on move.

## Run log (Grok Bot)
- run 112: Run 111 SVCLOC OK (svchost=dls1.ilostmymind.xyz) and NdSetAttr OK, but NdGetFileListNumAsync returned 0 with no DLS HTTP. ELF showed DWCi_NdGetFileListNumAsync always sets error 8: setupHttp tested uninitialized iVar1 after commented getDSdesc and -O2 folded the failure path. Fix: patch 0196 rewrites setupHttp (fill rhgamecd/userid, correct attr1-3) and getMacAddress. GTS untouched. CLEAN network rebuild required.
- run 111: Run 110 aborted in OS_CreateThreadReal (0x16bf58) during NdInit/SVL: DWC_Svl_GetTokenAsync passed Nitro priority 0x68 (>31). Fix: clamp in __wrap_DWC_Http_StartThread + pspoke-patch 0195 sets SVL prio to 16; MG NdInit also SetCustomNas(http) + ignoreca and forces IPKE in cal when ROM header is IPGE. GTS StartThread uses prio 0 (unchanged).
- run 110: Run 109 crashed in DWC_Http_Create (pc=0 lr=0x61d010) during NdInitAsync/SVL: DWCauthhttpparam.alloc was NULL because NAS login is skipped. Fix: MG-only NdInit wrapper sets DWCauthhttpparam.alloc/free to DWC_Alloc/DWC_Free and re-asserts SetMemFunc; null-guards on later Nd Get* wrappers. GTS-shared EC294/ConnectInet/NAS login unchanged.
- run 109: reverted the run 108 real DWC_SetMemFunc (ov00_021EC294) to the old stub; GTS shares it and was working. Kept only the MG-only redirect + logs.
- run 108: Run 107 hung on local wireless MG search (WM stand-in), not ov74_02236AE0. Evidence: WM init + icon redraw, no ConnectInet / [SS-MG] Nd / DWC_Init logs; cursor itemsAbove=1 selects GET VIA WIRELESS (submenu order Friend, Wireless, WFC). Fix: platform --wrap A4CC redirects to WFC when wireless=2; A560 logs path=WFC-download; EC294=SetMemFunc; patch 0194 logs DWC_Init. Test: ONLINE, Receive > GET VIA NINTENDO WFC (or Wireless, now redirected) and expect [SS-MG] path=WFC-download then DWC_Init / ConnectInet / Nd*.
- run 107: patch 0193 had UTF-8 em dashes; builder flagged it "[MISSING] patch not in the build" and stopped. Made the patch pure ASCII (same code). Clean build to test MG WFC download.
- Run 106 (patch 0193): Mystery Gift Wi-Fi download path wired to libntrdwc DWC_Nd* (same ONLINE stack as GTS). ov00_021ED1F0/ED354/ED388/ED3AC/ED3F4/ED444/ED428/ED308/ECEC0/EC938 no longer Missing(); [SS-MG] logs each stage. DLS URL forced to http://; SoulSilver gamecode IPKE. Test: Settings > Play > Wireless = ONLINE, no cheats, Mystery Gift > Receive > Nintendo WFC. Local wireless MG search still hangs (documented). Leftover: confirm SVL/Nd HTTP against the revival DLS host on hardware; NASLogin vs FriendsMatch if NdInit fails after netcheck.
- Run 102 (Claude): quit save before thread teardown. Device log: Mystery Gift opened, then crashed in native_ov74_0222AB70 (DS BG palette 0x05000000 built as mov #5; lsl #24, unmapped on 3DS).
- Run 103 (patches 0187-0190): Mystery Gift builds (native RC4/CRC16/signature, ov00 wifi stubs) + maploader maps mov #5-7/lsl #24 DS palette/VRAM/OAM bases to the backing arrays.
- Run 104 (patch 0191): same fix limited to ov74_* functions so nothing outside Mystery Gift changes.
- Run 105 (patch 0192): Mystery Gift cursor still invisible after 104; no confident root cause. Diagnostic only: native ov74_02235FF8 + gated ListMenuUpdateCursorObj logs ([SS-MG-CURSOR]) dump ListMenu template/window, BG/OBJ palette, and OAM0-2 while navigating MG. Open MG, move the d-pad, capture the device log.
