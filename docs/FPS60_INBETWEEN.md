# 60 fps field (in-between pictures)

Setting: Settings > Display > 60 fps field (0 OFF default, 1 ON, 2 A/B TEST). Log tag `[FPS60]`.

Same idea as the Platinum port: game logic stays at 30 ticks per second, and each tick shows two pictures. The first is an in-between picture of the 3D world, the second is the real one.

How it works:
- g3soft.cpp pairs each 3D draw call with the same call from the previous tick (replay array pointer, or vertex count plus first/last vertex hash, nearest to the predicted position). The in-between picture draws the same model vertices with the average of the two clip matrices. Unpaired draws shift by half the camera movement. Cull planes are widened 6% while recording so edges are not missing.
- g3gpu.c draws the in-between vertices first into midTex[] (plus strips for wide), then the real frame.
- gpucomp.c redraws the already composed 2D batches over midTex into rtM[e].
- gpuscale.c presents the in-between picture top-only, then the normal present; the tear-fix wait puts the two on consecutive V-blanks.
- Field, direct 1x frames only. Sharp 3D rests in the field while this is on.
- Auto rest: if more than 24 of 60 ticks exceed 36 ms, it rests for 300 ticks (doubling, up to 3600).

Status: compiles with devkitARM in the cloud. Not run on hardware or in the emulator yet. The emulator gives no real timings.
Test: set A/B TEST, walk the field for about two minutes, send the one log. Compare the OFF and ON halves.
Remove the setting once tested (toggle rule).
