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

## Where the real information is

- Device `log.txt` `[SS-MISSING]` lines show which game screens still trap. `docs/archive/MISSING_AUDIT.md` is archaeology only.
- Next free patch number: check `ls pspoke-patches | tail` before numbering (0284 was the last at this writing).
- Builds are made by the owner with the patch-builder tool, which pulls the latest `main`.
