/* ss3ds wide_layouts: the built-in wide layout of each 2D screen (see ss3ds_widescreens.c for the row kinds).
   Columns and lines are the DS's (0..255, 0..191); the wide screen shows DS columns -32..287.
   WL_MOVE(bgs, top, bottom, x0, x1, dx): those BG columns drawn dx px to the right (negative: left).
   WL_FILL(bgs, top, bottom, x0, x1, col): wide columns x0..x1-1 show the BGs' column col.
   WL_OBJS(top, bottom, x0, x1, dx): sprites whose middle is in that box move dx px.
   bgs: bit n = BG n; add WL_SCREEN when x0..x1 are screen columns of a scrolling BG. */
static const short kWlNone[1][8] = { { 0 } };
/* Pokemon summary (engine A on top; BG0 3D Pokemon, BG1 text, BG2 page extras, BG3 the panel art). The left page panel gets
   64 px wider (its lined interior, column 135, continued), its text stays at the panel's left; the name / portrait / item
   block moves to the right edge with the Pokemon (3D) and its sprites. Same for all three pages. */
static const short kWlSummaryA[][8] = {
    WL_MOVE(0x8, 0, 192, 0, 136, -32), WL_FILL(0x8, 0, 192, 104, 168, 135), WL_MOVE(0x8, 0, 192, 136, 256, 32),
    WL_MOVE(0x6, 0, 192, 0, 141, -32), WL_MOVE(0x6, 0, 192, 141, 256, 32),
    WL_OBJS(0, 192, 0, 141, -32), WL_OBJS(0, 192, 141, 256, 32), WL_SHIFT3D(32),
};
/* Party menu (engine B on top: BG1 the framed panel with the wave and Poke Ball, BG0 the bar along the bottom edge). The frame's
   left edge goes to the screen edge, everything else 32 px right; the 64 px between continue the panel (column 21). */
static const short kWlPartyB[][8] = {
    WL_MOVE(0x2, 0, 192, 0, 20, -32), WL_FILL(0x2, 0, 192, -12, 52, 21), WL_MOVE(0x2, 0, 192, 20, 256, 32),
    WL_MOVE(0x1, 0, 192, 0, 16, -32), WL_FILL(0x1, 0, 192, -16, 48, 16), WL_MOVE(0x1, 0, 192, 16, 256, 32),
};
static const WlScreen kWlScreens[] = {
    { "summary", kWlSummaryA, WL_N(kWlSummaryA), kWlNone, 0 },
    { "party", kWlNone, 0, kWlPartyB, WL_N(kWlPartyB) },
};
