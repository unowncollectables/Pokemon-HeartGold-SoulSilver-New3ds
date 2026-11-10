# Handoff

Live run notes and patch status live in the repo-root [`HANDOFF.md`](../HANDOFF.md).

This file used to hold a long Oct-2026 snapshot. It was replaced in run 116 so agents and humans have a single place to update. Do not resurrect a second handoff here.

## Union Room battle (patch 0263, 2026-11-10, untested, no build yet)
Union Room Battle needs overlay 5 (link battle party select), overlay 86 and the comm club helpers behind the `unk_02091564` table, all stubbed as `[SS-MISSING]` before. 0263 seeds `overlay_05.s` / `overlay_86.s` plus 16 club helpers into the maploader translator and drops their traps; 38 closure functions already compiled from decomp C are marked native-provided. If the build stops with "multiple definition", add that name to `native-provided.json` (new patch); if "undefined", seed it. Test: two consoles, Union Room, Battle. The wireless error screen itself (`ShowCommunicationError`) is decomp C and already works; `ov00_021E6A70` is only a weak 0-returning DWC stub.
