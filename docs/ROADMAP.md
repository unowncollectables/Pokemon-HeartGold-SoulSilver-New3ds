# Project roadmap

Current direction, November 2026. Test results and open bugs live in [HANDOFF.md](../HANDOFF.md); player-facing features are in the [README](../README.md).

## Already reached

- Native SoulSilver boot, exploration, battles, menus and cartridge-compatible saves on New 3DS.
- Stereoscopic 3D, widescreen and a touch-screen Pokégear with map, party, bag and helper tools.
- The overworld performance pass: about 60 fps in mono and close to it in stereo, with the tested setup built in.
- Hardware results for local Union Room trades, the original community GTS path, WFC Mystery Gift and Pokéwalker send/return.
- A separate experimental HeartGold build from the same source.

## Current work

- Validate the newest interface changes on hardware: sharp game and panel text, battle-info overlay, finder LIST, sub-area tracking, map panning and tab swaps.
- Expand real-device coverage of visible wild behaviour, Safari Zone encounters, seasonal trees and Ambient Weather. Ambient Weather currently defaults OFF.
- Check soft-reset teardown, water-battle transitions and other recent stability repairs on real hardware.
- Exercise the seeded Battle Frontier and Safari editor screens; fill remaining missing-screen paths as they are reached.

## Next coverage priorities

- Two-console Union Room battles and remaining local activities.
- StarMe live deposit/pickup and GTS hardware re-entry, plus Pokéwalker repeatability.
- More radio tuning, roamer, breeding, Contest helper, Tap to Fly, achievement and item-shortcut coverage.
- Battle 3D depth and Poké Ball double-image fixes; remaining wide-screen layouts.
- A dedicated HeartGold emulator and hardware playtest, including its online paths.

## Longer term

Broader side-system parity, fewer load and transition hitches, simpler packaging, and release readiness backed by repeatable hardware checks. The Wi-Fi Plaza currently shows a closed message and Wi-Fi Club minigames remain unavailable; they are not completed milestones.

Keep the source and patch workflow reproducible, preserve raw 512 KiB save compatibility, and require users to supply their own game dump. Plans may change as testing exposes new blockers.
