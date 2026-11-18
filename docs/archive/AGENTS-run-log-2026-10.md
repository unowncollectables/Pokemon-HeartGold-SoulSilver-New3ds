# Old AGENTS.md snapshot and run log (2026-10-25 to 2026-10-29)

Moved out of AGENTS.md on 2026-11-18. History only; trust the code and HANDOFF.md.

### Snapshot of 2026-10-26 (main 1795c7f)
Delivered as one `F:\cias\soulsilver3ds.cia` (3D HOME banner is the default and works on a real HOME Menu; no TEST or FLAT CIAs). Latest build on the card is main 436bde7 (wild-battle crash fix, patches 0054/0090/0091); main has since gained PRs #67-#69 (bottom overlay repaint only on change, GPU-compose both eyes outside the overworld, reuse of the converted 3D texture per eye), built into no CIA yet.

Working and tested on device: boot, title, overworld, battles up to the crash fix, touch, 3D depth, wide view, GPU 3D, GPU 2D compositor, GPU blend (default on, toggle removed), zone prefetch, rolling save backups, `.sav` round trip with a retail save (the owner, 2026-10-26), SELECT reset, 3D HOME banner, fast text, battle speed/catch info/hunt counter, ZL run, IV/EV, reusable TMs, field moves w/o HM, Repel prompt, no wild battles.

Merged but **untested on device**: Speed tweaks (`fps_tweaks`, Debug tab, default OFF) and `[PERF4]` stage timing (2026-10-27), wild battle entry after patches 0090/0091 (the crash was a raw 0x04001000 read), JOHTO bottom map (default layout; Bottom menu is the real on/off, first row of Controls) with the wild list (0084) and Kanto gate, audio cleanup (stall fade, DSP watchdog, Volume row), PRs #67-#69 fps work. `cheat_*` keys exist only in config.txt and may never be tested. Lugia title fixes are emulator-tested only and parked as last priority.

Known issues: Poke Ball opening shows a double image with the 3D slider up; Wide FX costs ~29 ms/frame, keep it OFF; menu frame rate < 60 fps in places (3D-bound); first visit to a map can hitch (SD read). Next: 60 fps overworld, see `docs/archive/SPEEDUPS.md` and `docs/archive/FPS60.md`.

## 8a. Run log
### Run 1 (2026-10-25, box build, version string "run 1")
- Version is AUTOMATIC: the Makefile writes `build/ss3ds_buildid.h` from git (run = commit count of HEAD, plus short hash, `+` if edited), shown in the boot line, `[CFG]`, settings app and CIA/3dsx metadata (CIA = run/64 major, run%64 minor). Never edit `ss3ds_version.h` or bump anything by hand; if a log shows "run 0 (no git)" the build tree has no full git history.
- Crash fix (pc=002a0b34 lr=001b8574, first entry to map 67 with a fade): `ov01_021F0CDC` (window-off task run by SysTaskQueue) builds 0x04000000 as `mov r2,#1; lsl r2,#26` and read/wrote REG_DISPCNT raw; the translator only routed functions whose literal pool holds a 0x04xxxxxx address, so the read hit unmapped memory on the 3DS. pspoke patch 0053: translate2.py also routes functions that build 1<<26, and prepare.py rewrites the decomp's raw `*(vu32 *)0x4000000` (ov02_02249968 / ov02_0224A700, same window fade) to `reg_GX_DISPCNT`. A disassembly scan of the ELF finds no other raw 0x04000000 access.
- Build fix: patch 0052 seeds `sub_0208B278` (patch 0051 retargeted its only caller, so a from-scratch build dropped it and the link failed).
- Title Lugia: the title projection has z == w, so g3soft gives raw tris a depth linear in w; the PICA interpolated it as z/w (affine in screen space), wrong across big triangles: spikes floated in front of the back, holes in the body. `g3_wdepth` (Settings > 3D > W-buffer depth, default ON) uses the PICA W-buffer (`C3D_DepthMap(false)`, vertex z = -d) so it is interpolated perspective-correctly. Emulator matches the software renderer now; overworld unchanged. Still open: thin black wavy lines on the title water (also there before the fix, GPU path only).
- CIA icon: like the Platinum port, the HOME Menu icon comes from your ROM's own DS icon at build time (`tools/home_menu_art.py`, GabeSud0, GPL-3.0; never committed); no ROM -> the original icon in `cia/art/`. Banner: see run 3 note below.
- Box: ROM in `roms/` (gitignored), makerom/bannertool from the Platinum box setup in /opt/devkitpro/tools/bin, `tools/emu_smoke.sh` = Azahar headless smoke test (3dsx or CIA: the CIA is installed into the emulated SD first). Tests always use a copy of the save.

### Run 2 (2026-10-25): Lugia W-buffer on hardware
- Hardware log (run 1 CIA): g3_wdepth=1 PICA W mode active at title, no crash, reached map 33; Lugia unchanged on hardware.
- PICA W-buffer (C3D_DepthMap / z/=w) behaves differently on real PICA vs Azahar, so emulator can't validate GPU depth fixes. Same-frame g3_shot dumps (GPU vs CPU) match in Azahar.
- Fix: g3_wdepth=1 now routes W-buffer frames to the CPU rasterizer (gpuOn=2 = GPU ready, CPU this frame). 2 = run-1 PICA W, 3 = hyperbolic GPU z (looked worse), 0 = off.
- Platinum port Giratina fixes (TEXGEN 2/3, shadow skip, G3X_GetVectorMtx, linear w/g3_wmax) were checked: all already ported (patch 0016, platinum3ds tags). Credit to the Platinum 3DS port.
- TODO: speed up CPU W frames (core-1 split) if title fps matters.

### Run 3 (2026-10-25)
- Hardware run 2 (build 16:00, from main by Claude): `[SS-MISSING] sub_02038D90` -> abort during DoSoftReset. Fixed by hand ports in platform/ss3ds_glue.c + 0061 (traps removed from missing.c, OS_ResetSystem -> P3DS_SoftReset). If generate_traps.py ever regenerates missing.c, drop those two names again (duplicate definition at link).
- Run 2 hardware: title frames really went through CPU 3D (log: "W-buffer frame -> CPU 3D renderer") and Lugia looked the same, so the hardware Lugia bug is NOT depth. Next suspects: data cache flush before GPU/texture reads, VFP flush-to-zero, uninitialised data. Compare `[WSUM]` lines and `g3_shot` dumps between hardware and Azahar for the same frame.
- Settings keys: g3_wmode (0 off, 1 PICA W default, 2 GPU Z; config.txt only since the 2026-10-26 cleanup). g3_wcpu, g3_wpersp, GPU SPLIT and the [WSUM]/[LUGIA] diagnostics were removed after Grok's patch 0095 fixed Lugia; boot.txt now only gets crash-handler output.

