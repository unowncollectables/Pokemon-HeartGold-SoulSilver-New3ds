# Release notes

## Black top with 3D front overlap (run 887)
`g3_front_ov=1` on main after #498 blanked the top screen from boot. The always-on stereo-GPU vertex/shader change is reverted; front overlap stays race-free with a session auto-OFF if a frame draws no tris after geometry was submitted. Platform rebuild; device retest pending.

## Stereo GPU path flip + raw_split live toggle
On main e8900f9, `g3_stereo_gpu=1` still flipped gpuRight↔cpuExpand (~half the frames) so the right eye jumped; `g3_raw_split` Settings ON logged the cfg but never started worker 2. Hard-clip/translucent now write left+dsh; End prefers gpuRight (skips mid); 60f path log includes fallback reasons. Live raw_split drains then Start2/park and logs per-worker busy us. Platform rebuild; device A/B pending.

## Stereo GPU flash + next wall cuts
`g3_stereo_gpu=1` could skip CPU expand while End did not take the shader right-eye path, leaving an empty right eye (flash / stereo died). End now inits g3s lazily and always expands as fallback; log shows gpuRight vs cpuExpand frame counts. New Debug toggles (default OFF): `g3_raw_split`, `g3_xf_map`, `g3_front_bg`. Platform rebuild; device A/B pending.

## Front overlap check regression (run 889)
After #500, `front_ov=0` thr_check showed rare texCrc (0 on c2087a3). Cause: GetTexture could still redecode under a fence pin; freeze was front_ov-only and the check formula changed. Fixed by freezing on any fence pin, restoring the live-gen CRC, and ignoring legit boot 0-tris in the front_ov safety net. `stereo_gpu` texCrc was the same race. Platform rebuild; device retest pending.

## Front overlap texCrc + stereo GPU (run 888)
`g3_front_ov=1` still had rare texCrc mismatches; #500 froze pinned textures under front_ov and used submit-time gen (superseded by run 889). `g3_stereo_gpu` is an isolated shader path (default layout untouched). Platform rebuild; hardware A/B pending.

## Built-in settings

A fresh or incomplete `config.txt` now starts from the owner's tuned values. A line already in the file is kept, including OFF. Alive wild Pokémon starts at ON + TOWNS, Sharp 3D at FAST AUTO, and Pokéwalker, Ambient weather, Roamer sprites, Tap to Fly, the Pokédex finder, the Breeding tab, Hunt stats, Damage check, Contest helper, Reusable TMs, and Event Pokémon without the event flag start on. Wide view stays on. HeartGold uses the same defaults. Not built in this session.

## Battle animation cleanup

The native battle code now passes the particle emitter to Shadow Ball cleanup. Requires a game rebuild. A Morty rematch on New 3DS is pending.

## Running shoes toggle

RUN ON means you run by default. RUN OFF means you walk by default. Holding B temporarily reverses that choice. Auto run in Settings sets the startup mode.

## Overworld Pokémon behavior
Set Alive wild Pokémon to ON + TOWNS in Settings > Battle to meet town residents. Try pressing A next to one, then walk and sprint toward a shy Pokémon outside town to see the difference. Fliers can bank along clear low lanes or pass overhead. Nearby Pokémon sometimes play together, and rain can send land Pokémon into tall grass. Watch for leaves moving in the grass and ripples after a water Pokémon lands. In town, residents may linger beside water or greenery. Outside town, a startled Pokémon may duck into tall grass and peek out if you wait quietly. With Visible wild cries enabled, listen for rustling, calls between Pokémon, and approaching fliers. In town, a berry can roll past two residents and start a chase. Some solo fliers land briefly along clear routes. After rain, look for fading tracks and Pokémon shaking off water. Check the Goldenrod Flower Shop in the morning, daytime and evening for its Bellossom. Look for Mareep around Violet’s Pokémon School, Aipom near Kurt’s house, and Vulpix near the Dance Theater. Catch the time change to see a resident head toward the doorway. Towns now vary in where Pokémon linger, how often they play, and when they nap or get showers. In Kanto, check Vermilion’s Fan Club, Lavender’s Radio Station, and Pewter’s Museum. In Goldenrod, Bellossom can follow the Flower Shop clerk when its daytime routine starts. Additional residents visit Cherrygrove’s Guide Gent, Olivine Café, Cianwood Pharmacy, and Celadon Department Store. Pending a game rebuild and New 3DS testing.

## Johto map head, zoom, and roamer icons
The player head on the bottom map stays on the town it is marking while the map recentres. Map zoom in Settings > Input defaults to 1X; 2X (or the - and + on the map) is a closer view of the same map. Roamer sprites default on, so the ROM menu icon is used unless a saved line turns it off. Needs a platform rebuild. Not checked on a 3DS yet.

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
