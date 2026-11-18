# Handoff (current state)

Short on purpose. The old running notes (920 lines of per-run test results, 2026-10 to 2026-11-18) are kept in [docs/archive/HANDOFF-2026-11-18.md](docs/archive/HANDOFF-2026-11-18.md). Rules and file map: [AGENTS.md](AGENTS.md). What changed: [docs/CHANGELOG.md](docs/CHANGELOG.md).

## State (2026-11-18)

- Core game, battles, saves, 3D, Wide view, touch-screen Pokégear panel, GTS, Mystery Gift, Union Room trades and the Pokéwalker work on a New 3DS.
- 60 fps push finished (#517-#540). Mono is about 60 fps. Stereo is near the ceiling because the GPU is busy the whole frame. Always on: GPU right eye, hidden-rows skip, 3D state cache, screen hand-off. Features that tested worse were removed.
- Settings menu (L+R+Y) is trimmed for release: Debug and Render tabs are gone and the tuned 60 fps setup is the default (PR #541). Players never need `config.txt`.
- GTS network debug logging is off by default (build with `-DSS3DS_NET_LOG` to bring it back).
- HeartGold is the same port built as a second game (see AGENTS.md section 12).

## Known open items

- Poké Ball opening in battle can show a double image with the 3D slider up (issue #484 covers battle 3D depth).
- Old-fill screens that were never re-laid out for Wide view: Pokéathlon, Photo Album, Hall of Fame, evolution, Battle Frontier.
- Battle Frontier, Safari editors and Plaza screens were seeded but not opened on a console. The Wi-Fi Plaza shows a closed message. Wi-Fi Club minigames still stop.
- Union Room battles need two consoles to test.
- Normal play: leave "3D profile" and "GPU pass timing" OFF, log detail SHORT.

## Soft-reset teardown (run 934)

Select / L+R+START+SELECT soft reset in the field crashed (`pc` past `.text`, crumbs: engine-B `w-job` still running after main stopped). Root cause: platform workers (engine B / c1, g3thr, par) were never joined before heap/GPU teardown; SoftReset could also be re-entered via `sceKernelLoadExec` after `OS_ResetSystem`. Fix: SoftReset once-guard; `hardExit` joins render → g3thr → par before `ShutdownThreads` / `gfxExit`. Save sync/backup order unchanged. Device retest pending.

## If X breaks, do Y (stability notes, 2026-11-18)

Start with the device `log.txt` on the SD card (`sdmc:/3ds/soulsilver3ds/`); the owner pastes it. The run number at the top shows which build it came from. Never ask the owner to edit `config.txt`.

- **Red "not in the 3DS port yet" screen (name in brackets), or `[SS-MISSING] name` in log.txt.** A game screen that is only reached through a pointer table was never translated. Fix it like 0141/0142/0271-0273: add the overlay's `.s` files to `seed-files.json` and drop its `Missing()` stubs (recipe: `docs/archive/MISSING_AUDIT.md` and the missing-screens notes), or, if the screen is not worth porting, close its entrance with a hook like `0274_wifi_closed` (toast + `[WIFI]` log line, command returns its "not started" result). Known still-closed: Wi-Fi Plaza (0274), Wi-Fi Club minigames (overlays 90-93, only reachable from the Plaza). The save is not touched by the trap: the game only writes on its own save command.
- **Red "GAME CRASHED" screen.** Read the `[CRASH]` and `[CRUMB]` lines. `far=04xxxxxx` inside a libntr C function is a raw DS register passed as a pointer: wrap the callee (see `GXx_SetMasterBrightness_` in `ss3ds_glue.c`). `pc` far outside `.text` right after a reset or exit is a worker thread outliving teardown (run 934; `hardExit` joins render, g3thr, par first). Decode `pc` with `arm-none-eabi-addr2line -e soulsilver3ds.elf` from the same build (build number must match).
- **"game stalled: no frame for 10 s" (red box from the watchdog).** The `[WATCHDOG]` lines name the lock holder and the last crumbs. Long waits that are on purpose (settings menu, followers.bin build, GTS login) already pause it; a new long blocking job must set a `P3DS_*` busy flag like `P3DS_NetBusy`.
- **Game crashes only on a visible-wild battle or encounter.** Turn "Alive wild Pokemon" off in L+R+Y (Play tab) to confirm, then read the last `[WILD]` lines. Run 939 was a forced surf battle from the shore (fixed in 0284). The wild code lives in `scripts/ss3ds_hooks.d/0110_wildvis.py` (block `SS3DS-WILD-BEGIN`).
- **Save problems.** Look for `[SAVE] backup ... ok` after each in-game save and at exit. `FAILED` means the SD card is full, locked or flaky; since 2026-11-18 a failed backup write leaves all eight `soulsilver.sav.bakN` untouched and shows one toast "SAVE BACKUP FAILED". Restore by copying a `.bak` over `soulsilver.sav` on the card (validate with `tools/ss_savecheck.py`). `soulsilver.sav` stays the raw 512 KiB image, byte-compatible with a DS cart or R4.
- **Build fails.** The owner's patch-builder pulls latest `main`. A leftover half-applied patch is handled by `scripts/bootstrap.sh`. A hook that logs "NOT applied" / "anchor not found" stops the build on purpose (patch 0159). Fix with a NEW numbered patch, never edit an applied one; check `ls pspoke-patches | tail` right before numbering.

## Where the real information is

- Device `log.txt` `[SS-MISSING]` lines show which game screens still trap. `docs/archive/MISSING_AUDIT.md` is archaeology only.
- Next free patch number: check `ls pspoke-patches | tail` before numbering (0284 was the last at this writing).
- Builds are made by the owner with the patch-builder tool, which pulls the latest `main`.
