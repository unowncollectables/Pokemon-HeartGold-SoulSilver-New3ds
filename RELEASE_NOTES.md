# Release notes

## Rendering performance hardware trial
Use L+R+Y to open settings. Under 3D, enable Command shortcut, Bounds shortcut, Second screen GPU (ON), Affine object GPU, and Second screen fades. Under Display, set Title layout to WIDE. These settings default to the existing renderer and layout. Start with the title reveal, Goldenrod, and a battle send-out, then repeat gameplay with the 3D slider down. The wide title averaged about 26 FPS in the local Azahar trial. Fades and some battle effects can still dip. New 3DS testing is pending.

## JOHTO panel polish
The bottom-screen tabs match the sharp green icons and button shading of the menu on the right. The achievement list keeps the score, names, points, descriptions, and dates apart. Azahar confirmed that layout. Dragging the list did not move it; UP and DN next to the score page it with a tap. Unlocks, the toast, and the save are unchanged. Needs another platform rebuild before that is checked.

## Audio fix awaiting device validation
Field music requests of 128 are bounded to the decibel table's valid 0..127 range, preventing an accidental +256 boost. Goldenrod / Route 34 tested in local Azahar; owner reported improved sound, and the checked test intervals had no clipping or automatic gain reduction. Existing audio settings remain unchanged. Requires a normal platform rebuild/link; real New 3DS testing is pending.

Current build (main). Not an official release; see README for install and known issues.

## Highlights (2026-10-29, run 59)
- Local wireless trading between two 3DS consoles works end to end (Union Room, animation, both Pokémon intact).
- Pokégear MAP / PHONE / RADIO tabs on the touch screen; tapping a contact places a real call.
- X opens a tile cursor on the Bottom menu; battle move badges; FAST 3D crisp mono; automatic save backups with a fixed checker.
- Untested: radio tap, roamer/breeding tracker, Union Room Battle/Records/Draw/Greet/Spin.

## What is in it
- Native New 3DS port of SoulSilver: boot, title, overworld and battles on the pspoke base, with 3D depth, wide view, GPU 3D and a GPU 2D compositor.
- JOHTO map on the touch screen with an icon column; it steps aside for the game's own YES/NO and starter Poké Ball screens.
- Shiny hunt suite, visible wild Pokémon and Speed tweaks are in the tree (opt-in, hardware testing ongoing).
- Settings app on L+R+Y with Display, 3D, Controls, Battle, System and Play tabs (hidden Debug tab on SELECT).
- GPU blend is now on by default (tested clean on device: 299-300 of 300 frames composed on the GPU, zero pixel mismatches). The test toggle is gone from the menu; `gpu_blend=0` in `config.txt` turns it off.
- Fast text (2x, 4x, instant), hold B to skip text, auto-advance text.
- Faster battles, catch info panel (Y), encounter counter.
- ZL run toggle, IV/EV on the summary screen, reusable TMs, field moves without HMs, Repel prompt, no wild battles.
- Zone prefetch and ROM cache for fewer load hitches; three rolling save backups at every launch.

## Needs testing on device
Roamer + breeding tracker, Pokégear radio tap, Sprite depth, speed tweaks and the other Union Room options are untested on hardware. Use a copy of your save.

Visible wild Pokemon on stairs (patch 0230) need a game rebuild, then a hardware look: with visible wilds and wide view on, walk up and down stairs while a wild stays on its own step. The 3D slider can stay up. `[WILD] own ground y` appears when that height differs from the player's.

## Known issues
Double image when a Poké Ball opens with the 3D slider up; Wide FX costs about 29 ms per frame (keep off); audio can clip; solid 30 fps only, 60 fps is a later goal. Back up your save before using any build.
