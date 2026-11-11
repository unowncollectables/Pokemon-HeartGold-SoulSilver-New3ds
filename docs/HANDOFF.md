# Handoff

Live run notes and patch status live in the repo-root [`HANDOFF.md`](../HANDOFF.md).

This file used to hold a long Oct-2026 snapshot. It was replaced in run 116 so agents and humans have a single place to update. Do not resurrect a second handoff here.

## Union Room battle (patch 0264, 2026-11-10, untested, no build yet)
Union Room Battle needs overlay 5 (link battle party select), overlay 86 and the comm club helpers behind the `unk_02091564` table, all stubbed as `[SS-MISSING]` before. 0264 seeds `overlay_05.s` / `overlay_86.s` plus 16 club helpers into the maploader translator and drops their traps; 38 closure functions already compiled from decomp C are marked native-provided. If the build stops with "multiple definition", add that name to `native-provided.json` (new patch); if "undefined", seed it. Test: two consoles, Union Room, Battle. The wireless error screen itself (`ShowCommunicationError`) is decomp C and already works; `ov00_021E6A70` is only a weak 0-returning DWC stub.

### Experimental GTS server picker

Enable **GTS server picker** in L+R+Y > Setup, then reopen the title menu. **Online Settings** offers Poké Classic (the existing DNS path, including a manual `wifi_dns` override), StarMe, and Wi-Fi setup instructions. The picker is off by default. Turning it off makes the port use its existing DNS path even if StarMe was selected. Network logs identify the resolver and `[GTS]` records a manual choice. The title menu hook rebuilds both the overlay archive and the directly linked `main_menu_native.o`. Verified in Azahar 2126.2 with a copied save: Poké Classic selected on first use, StarMe persisted across restart, Wi-Fi Setup opened, and disabling the toggle in Setup restored the original title menu after reopening it. Wireless was off during the UI test. Live GTS and hardware testing remain pending.

The picker can display the last GTS deposit seen for the loaded trainer on Poké Classic or StarMe. It uses the ROM's party icon and the level and requested species from the normal Gen 4 GTS response. A trade ready for pickup has its own status. The preview cache is separate from the cartridge save, and the observer issues no extra requests. Offline checks covered deposit metadata, pickup, clearing, malformed replies, and trainer/server separation. An Azahar fixture confirmed the preview stays with the original server when the active server changes. Live responses and pickup are unverified.
