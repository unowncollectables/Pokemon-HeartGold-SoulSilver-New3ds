# 60 fps field (in-between pictures)

Setting: Settings > Display > 60 fps field (0 OFF default, 1 ON, 2 A/B TEST). Log tag `[FPS60]`.

Same idea as the Platinum port: game logic stays at 30 ticks per second, and each tick shows two pictures. The first is an in-between picture of the 3D world, the second is the real one.

How it works (second version, after the first hardware log):
- g3soft.cpp pairs each 3D draw call with the same call from the previous tick (replay array pointer, or vertex count plus first/last vertex hash, nearest to the predicted position) and gives g3gpu.c one matrix K per draw call: K carries this tick's clip coordinates halfway back to the previous tick's. Unpaired draws shift by half the camera movement. Cull planes are widened 6% while recording so edges are not missing.
- g3gpu.c draws the SAME vertex buffer twice. The in-between pass goes through shaders/g3k.v.pica, which multiplies the position by K (one uniform change per run of draw calls that share K; the static world shares one). Nothing is transformed twice on the CPU. The result goes to midTex[] (plus strips for wide), then the real frame is drawn.
- gpucomp.c redraws the already composed 2D batches over midTex into rtM[e].
- gpuscale.c draws both pictures in one GPU frame: the in-between into the screen targets, the real one into a second pair of targets. The real one is handed to the LCD one V-blank later by P3DS_Fps60Poll (called per 3D draw call and before the V-blank wait, with an immediate nonblocking retry after the scheduled DS VBlank wait. The retry still checks whether the LCD has consumed the in-between picture) while the next tick is already running, so the game never waits between the two pictures.
- Field, direct 1x frames only. Sharp 3D rests in the field while this is on. Needs g3_wmode 0 or 2 (depth linear in clip space).
- Step-down: only where the game itself would run slow (60-tick average over 34.5 ms, or a quarter of the ticks over 36 ms). It comes back by measurement: at 30 the spare time per tick is known, and once it is back above the learned cost of the in-between picture it is on again (checked every 30 ticks, at least 90 ticks at 30).

First hardware log (first version): picture looked right; 2.1 ms per in-between frame, but the wait for the GPU and the V-blank between the two pictures turned 30 ms ticks into 50 ms ticks in heavy areas, and it rested for 10 to 40 s at a time.

Run 835 (09dd925) on New 3DS with mono Wide produced 55–59 counted fps while interpolation was active. In one field window, 310 of 479 real pictures used the blocking fallback. The post-VBlank retry gives the handoff a chance once the LCD has accepted the in-between picture, even when there are no more 3D draws that tick. The log now reports submissions without waiting and via fallback. Those counters measure software handoff, and actual display cadence needs a console check.
Device test: leave 60 fps field ON and walk the same route for two minutes with mono Wide, then repeat with the slider up. Compare the fallback count and average tick time with run 835. Include a map transition and the Goldenrod Game Corner glass to check visible cadence and pairing.
Remove the setting once tested (toggle rule).
