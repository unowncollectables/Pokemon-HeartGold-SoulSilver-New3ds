# Missing screens round 3 (patches 0271 to 0274, 2026-11-12, not compiled here)

- 0271 seeds overlay 108: Safari Zone area customizer and decoration (6 stubs gone, 154 functions).
- 0272 seeds overlays 75 (Wii message settings on the Continue screen), 72 (Nintendo WFC app), 73, 37 and `unk_020850F4.s` (68 stubs gone, 580 functions).
- 0273 seeds the Battle Frontier: the 28 overlay 80 files, overlays 81, 82, 83 and 85, `frontier_map.s`, `frontier_system.s`, `unk_02096910.s` (245 stubs gone, 1645 functions). `port.py` gains the double compare `_dneq` (`ssd_ne`).
- `missing.c` goes from 375 traps to 56. Left: the Wi-Fi Plaza (overlay 39 with 39 stubs, 45, and its minigames 91 to 93; `overlay_93_arm.s` is ARM mode and does not translate, 45 has two untranslatable functions and unresolved calls), `ov13_0222B430` (SOC/WCM), `ov01_021EACBC`, `MovementScriptMachineSub_Done`, two `PmAgbCartridge_*`.
- 0274 (hook `0274_wifi_closed`, optional): `ScrCmd_662`, the Wi-Fi Plaza entrance (common script `scr_seq_0003_046`, host app on overlay 39), no longer aborts. It returns result 0, shows the toast `WI-FI PLAZA IS CLOSED` and logs `[WIFI] ScrCmd_662 ... refused`. The Wi-Fi Club minigames (91 to 93, picked inside the Club) still trap.
- Static check only (closure model of `port.py`, `translate2.py` on every added function, argument counts of every new external from the headers). No ROM here, so data-table extraction, compile and link are unproven. Expect the first build to need a follow-up patch (0274) for compile or link errors; paste the builder log.
- Device check: Safari Zone area customizer and object decoration screen; walk into the Battle Frontier, enter the Battle Tower and one other facility, play one round, check BP; talk to the Wi-Fi Plaza attendant in a Pokemon Center basement and see the toast instead of a crash.

# Audit fixes (2026-11-12, platform only, not compiled here)

- GTS preview cache and StarMe profile files: remove the old file before the rename (the SD card refuses a rename onto an existing file, so the preview never updated after its first write). A damaged StarMe profile is now set aside as `.bad` and a new one is made, instead of blocking StarMe for good.
- Save import (`import.sav`) is removed. Copy a save over `soulsilver.sav` by hand. Backups and Export save are unchanged.
- Debug options removed (now fixed at their old default and listed as DEAD in the log header): `g3_shot`, `tex_trace`, `dump_eyes`, `vram_trace`, `heap_watch`, `lid_test`, `celeb_test`, `fps60_probe`, `g3_reuse_verify`.
- Online logging trimmed: success-path `[GHTTP]`, `[SOC]`, `[CPS]` lines and the request / reply dumps are gone. Every failure line stays.
- One defaults table: `kCfgDefaults` (psp_ctru.c). logring.c reads it through `P3DS_CfgDefault`.
- Still `[SS-MISSING]`: the whole Battle Frontier (overlays 80 to 83, 85, 91 to 93, all `FrtCmd_*`), Safari Zone customizer and decoration (108), overlays 37, 39, 45, 72, 73, 75. Not touched here.
- Device check: deposit on the GTS twice with the picker on and see the preview change; `[SAVE] backup (launch)` still logs at boot.

# Union Room battle (patches 0264 and 0265)

Status 2026-11-12: 0263 to 0265 are in the builds since run 775, which linked and ran on hardware. The battle itself (two consoles) and the 0263 screens have not been opened on a device yet.

Union Room Battle needs overlay 5 (link battle party select), overlay 86 and the comm club helpers behind the `unk_02091564` table, all stubbed as `[SS-MISSING]` before. 0264 seeds `overlay_05.s` / `overlay_86.s` plus 16 club helpers into the maploader translator and drops their traps; 38 closure functions already compiled from decomp C are marked native-provided. If the build stops with "multiple definition", add that name to `native-provided.json` (new patch); if "undefined", seed it. Test: two consoles, Union Room, Battle. The wireless error screen itself (`ShowCommunicationError`) is decomp C and already works; `ov00_021E6A70` is only a weak 0-returning DWC stub.

# Experimental GTS server picker

Enable **GTS server picker** in L+R+Y > Setup, then reopen the title menu. **Online Settings** offers Poké Classic (the existing DNS path, including a manual `wifi_dns` override), StarMe, and Wi-Fi setup instructions. The picker is off by default. Turning it off makes the port use its existing DNS path even if StarMe was selected. Network logs identify the resolver and `[GTS]` records a manual choice. The title menu hook rebuilds both the overlay archive and the directly linked `main_menu_native.o`. Verified in Azahar 2126.2 with a copied save: Poké Classic selected on first use, StarMe persisted across restart, Wi-Fi Setup opened, and disabling the toggle in Setup restored the original title menu after reopening it. Wireless was off during the UI test. Live GTS and hardware testing remain pending.

The picker can display the last GTS deposit seen for the loaded trainer on Poké Classic or StarMe. It uses the ROM's party icon and the level and requested species from the normal Gen 4 GTS response. A trade ready for pickup has its own status. The preview cache is separate from the cartridge save, and the observer issues no extra requests. Offline checks covered deposit metadata, pickup, clearing, malformed replies, and trainer/server separation. An Azahar fixture confirmed the preview stays with the original server when the active server changes. Live responses and pickup are unverified.

StarMe profile test branch: hardware run 775 confirmed DNS switching and hit WFC error 60000 on the existing profile. StarMe now uses its own per-trainer WFC identity files in the port's SD folder. Login and the GTS profile ID use that identity while StarMe is selected. The original save-backed path is used for Poké Classic. The isolated build linked and the local SDK-stub check covered failure, retry, persistence, trainer isolation and the return to the save-backed path. Azahar verified live StarMe account creation, Mystery Gift download, and reuse of the same profile after restarting and entering the GTS. The GTS requests used that profile ID. The Mystery Gift check left the copied save byte-for-byte identical to its launch backup. The Poké Classic regression check used the original save profile, and both the baseline (04558e9) and the separate-profile build got error 60000 with that test identity. Poké Classic Mystery Gift also downloaded and displayed the Celebi Wonder Card on the separate-profile build, with the copied save unchanged. Hardware and live GTS deposit/pickup remain unverified.

# Built-in settings defaults (owner config, missing keys only)

A `config.txt` line still wins, including OFF. When the key is absent, the owner's values are now the built-in default for SoulSilver and for HeartGold (same platform tables; HeartGold has no separate defaults table). Changed when missing: `wild_vis_alive=2` (ON + TOWNS), `event_any_mon=1`, `contest_helper=1`, `hunt_stats=1`, `g3_sharp=3` (FAST AUTO), `dmg_check=1`, `pg_find=1`, `breed_tab=1`, `reusable_tms=1`, `pg_flytap=1`, `roamer_sprites=1`, `ambient_weather=1`, `pokewalker=1`. Wide view stays on. `fps_counter`, `wireless`, `log_level`, and `seasonal_trees` are unchanged. No birthday data was added. The save file is untouched. Not built in this session.

# Seasonal tree colors (0268)

Settings > Display > Seasonal trees is a separate experiment and defaults to OFF. It changes the canopy on supported outdoor tree palettes when the area loads. Spring is lighter green, summer uses the original colors, autumn is copper, and winter looks frost-tinted. Models, tree trunks, and shadows use the original data. Toggle or season changes take effect on the next area reload. Device checks are pending for visual coverage in both games.

# Local seasonal scenery (0267)

Clear spring days have more petals near Goldenrod's Flower Shop and along wooded routes. Clear autumn days have more leaves around Ecruteak and the same wooded routes, including Kanto Route 2. Most areas use fewer petals or leaves. A brief periodic gust moves the particles together. The six-particle cap and the Ambient Weather setting apply to all of these effects.

# Seasonal particle spacing (0266)

Device feedback found the original particle count ran well but the effect stayed too close to the player. Seasonal particles now spawn at staggered positions across the field, sample terrain height when they spawn, and grow and shrink at the ends of their drift. Their base dimensions are 25% larger and the cap is still six. Visibility and frame time with the new spacing are pending on New 3DS.

# Shadow Ball battle stall (0262)

Overlay 7 passes the particle system and emitter to the delete routine. The native translator used a one-argument header and dropped the emitter at all three call sites. Patch 0262 tells the translator to forward both arguments. The normal build resyncs the argument map and regenerates the overlay. Replaying Morty on a New 3DS is pending.

# Seasonal scenery and WX season label (0261)

Enable Ambient Weather and tap WX on the Johto map to see the current season alongside the forecast legend. On eligible clear outdoor maps, spring has drifting petals, summer has a few pale seeds, autumn has copper leaves, and winter has sparse flurries in the highlands. Native weather and active ambient rain, snow or mist take priority. The native field geometry draws at most six untextured particles and uses the same scene for mono and stereo. The effects have no save or encounter state. Device checks are pending for visibility, map transitions and frame time.

# Johto panel fit (0260)

Embedded Phone and Radio fill the space above their native dialogue strip. The original app switcher is cropped and only the Johto tabs switch panels. B closes each native app from its main screen, and Phone context-menu tooltips still show. The two registered item shortcuts stack vertically with larger icons to the left of Run. On-device checks are pending.

# Johto weather radar (0259)

With Ambient Weather enabled, tap WX beside the Johto map trophy to show the seasonal forecast. Blue marks rain, icy white marks snow, and lavender marks mist. The overlay uses the same three-hour weather calculation as the field and respects the hidden Kanto area. WX mode tints the map and labels clear viewports separately from a forecast that has not arrived. Its legend only lists conditions in view. The forecast covers ambient weather. Story and other native weather still take precedence in the field. Device checks are pending.

# Johto registered item shortcuts (0258)

The bottom-right row now shows both registered Bag items before Run. The game still checks item eligibility through the native field input path. Empty slots show their slot numbers. On-device touch and item use checks are pending.

# Pokéwalker infrared bring-up

Run 740 photos show stripes around bottom-screen tiles on the Pokéwalker menu and stroll results. The renderer now uses the DS scanline compositor while the Pokéwalker application template is active, bypassing the GPU tile compositor for those frames. Logs identify when this path starts and ends. Visual verification on the New 3DS is pending.

Run 740 on the owner’s New 3DS completed send and return transfers in one game session and saved after each exchange. Successful per-packet traces now default off. Set `pokewalker_trace=1` in `sdmc:/3ds/soulsilver3ds/config.txt` before launching the game to enable them. Connection and error diagnostics stay enabled.

Run 739 reached IR ready on every retry. Seven attempts ended on checksum-invalid opcode 0e replies of 108–120 bytes after valid EEPROM read replies. The follow-up schedules the IR worker ahead of normal rendering and checks the FIFO before declaring an inter-byte timeout. Invalid packets still fail checksum validation and now log UART line status, including the overrun flag. Hardware verification is pending.

Run 738 paused extra-input polling successfully and logged valid acknowledgments through the 32-packet trace limit. Two later connection requests never reached IR ready, and no polling-resume result appeared. Patch 0255 keeps polling paused between attempts and requests restoration asynchronously only when leaving overlay 112. The trace now covers up to 160 packets per direction. The restore stall remains a hypothesis until a new device log confirms it.

Run 737 still disconnected at varying points despite faster RX turnaround. The next build pauses the New 3DS extra-input IRRST service during a Pokewalker session because it can poll another device on the same I2C bus. It leaves the shared input memory mapped and restores polling at close. Logs report both service results. Bus contention is a hypothesis and hardware verification is pending.

Run 736 acknowledged eight writes on one attempt and nine on another. The latter stopped after a 43-byte write with no ACK logged. The follow-up clears both FIFOs before transmission and switches directly to RX on TX completion, avoiding a post-send FIFO reset that could miss a fast ACK. Hardware transfer remains unverified.

Run 735 received four valid opcode 04 acknowledgments to 136-byte writes, then stopped after another write. Later attempts showed an EFCR=00 readback and streams of checksum-invalid packets. The follow-up resamples EFCR without retransmitting and drops corrupt packets before passing data to the original overlay. Repeated full transfers remain unverified.

Run 734 completed the handshake, got the valid general-data reply (22) and the reply to 52 (54), then reported a locally completed 136-byte opcode 02 send without a valid response in the log. The next build refills the TX FIFO in bursts of at least 16 bytes (or all remaining bytes) and logs any empty-FIFO fallback. Whether the walker receives the full write remains unverified.

Run 733 showed valid F8 replies and recovered from several invalid RX levels, with no logged transmit timeout. It did not record successful data packets, so the result of the later transfer exchange is unknown. The follow-up logs the first 32 TX completions and RX packet results per session to identify the last completed command.

Run 732 still timed out with TXLVL=00 and LSR=00, including after fully queuing 30- and 64-byte packets. This is broader than refilling long packets. The follow-up wakes the UART before each send, checks EFCR after enabling TX, and records mode and sleep registers on timeout. The hardware cause remains unconfirmed.

Run 731 loaded the first 64 bytes but timed out refilling three packets (136, 77 and 112 bytes). The follow-up checks the UART transmitter-empty flag when TXLVL reads zero and logs both registers on failure. Hardware refill and repeated transfers remain unverified.

Run 730 received valid handshakes after recovering from impossible RX FIFO counts of 128. Three later TX attempts timed out, with 0/59, 64/94 and 0/63 bytes queued. The follow-up preloads the reset TX FIFO for all packet sizes and uses the working probe’s bus timing during the IR session, restoring the previous settings on idle close. Repeated transfers still need device testing.

The owner confirmed a complete Pokewalker send, return and gift on run 727. Subsequent attempts disconnect, including after reboot. RX now distinguishes status read failure, invalid FIFO count and payload read failure. The worker discards incomplete packets and re-arms RX after isolated failures, closing after three consecutive failures or a failed re-arm. The cause of repeated-session failures and the separate power-off symptom remain unverified.

After one reported transfer, the next attempts still got valid F8 replies but failed to transmit 112-byte opcode 52 and 93-byte opcode 00 packets. Commit 93999e1 fills the 64-byte TX FIFO before enabling the transmitter for long packets and logs the failing stage and queued byte count. ARM syntax check passed. Repeated send and return on hardware remain unverified.

Run 726 verified several checksum-valid 8-byte F8 handshakes and the owner confirmed the accessory reset. A transfer attempt subsequently logged an IR transmit failure followed by a receive failure. The follow-up retries only I2C address selection and records the failed transmit opcode and length. Send and return remain unverified.

Run 725 reached the Pokewalker connection prompt, received IR bytes on several attempts, reported a pairing rejection and exited cleanly. Successful pairing and transfer are still unverified. The follow-up delivers the one-byte discovery signal separately, rejects impossible FIFO counts and logs the first checksum-valid packet. The original reset chord is Down + X + L on the Pokewalker menu. Reset erases data on the accessory.

Run 724 reproduced the connection crash. The ELF resolved the null dereference to `OS_LockMutex`: the native backing pointer is at offset 24, where the next DS overlay object begins. Patch 0254 routes the worker mutex, queue and thread through separate native SDK storage. The system `i2c::IR` port was full on hardware, so the dedicated transport worker returns to the probe’s register backend with controller IRQs disabled and system bus timing preserved. The restored Homebrew CIA startup packaging is unchanged. The overlay translated and the new platform sources passed ARM syntax checks. Hardware retry and transfer remain unverified.

Pokewalker IR IPC now runs on one dedicated worker. The original game only exchanges packets with bounded queues, and closing the screen does not join the IPC worker. This isolates a blocked system request from gameplay. Build and hardware transfer tests are pending.

Run 722 froze at the connection prompt before logging IR ready. The watchdog fired after 10 seconds. The follow-up uses a nonblocking I2C service lookup and records the last initialization step or register in the watchdog log. This covers service acquisition waits. Device validation is pending.

Run 721 passed the picker and reached the connection prompt, then stopped after `[PWALKER] IR ready`. Luma recorded a GPIO system error. The next build uses `i2c::IR` for register and FIFO access instead of driving the shared I2C controller directly. The ARM syntax check passed. Hardware timing, the full connection and transfer remain untested with this backend.

Probe v2 in `tools/pokewalker_probe` received a valid session reply from a physical Pokéwalker on the owner’s New 3DS via the Homebrew Launcher. The probe stops before registration and never opens a save. Patch 0251 translates the original overlay 112 and replaces only the cartridge SPI transport and native worker lifecycle. Setup > Pokewalker defaults ON when `pokewalker` is absent from config.txt (a saved 0 still keeps it off). The app only launches when enabled and the IR register page is mapped. The CIA requests that page. The overlay translation and ARM C syntax checks passed. The game has not been linked or tested on hardware. First device test: use a copy of the save and verify the original menu opens and exits, then send and return a spare Pokemon. Log tag `[PWALKER]`.

# Johto Phone, Radio and Eggs

Settings > Input > Pokégear tabs defaults to Original. Phone and Radio open the native Pokégear in the Party/Bag panel compositor. Hook 0256 selects the app without dialing or tuning. The touch image maps to the native controls and the dialogue strip below it shows the game's live call or program text. Previews retains the former contact and station lists. The Eggs tab has a Day-Care summary and up to six hatch cards. Hook 0257 samples the ring manager every field frame and the banner hold advances once per render frame. Visual and touch checks on New 3DS are pending.

# Town Pokémon greeting cries (0250)

Pressing A next to a town resident now plays its species cry with Visible Wild Cries enabled. The existing audio cooldown and cry-busy check keep repeat presses from cutting off a cry. The greeting still turns the resident toward the player without starting a battle. The generated sources passed devkitARM syntax and hook idempotence checks. A host check covered the A press, mute setting, cooldown and busy audio. Device listening is pending.

# Running shoes toggle (0249)

The JOHTO RUN tile, ZL, Auto run settings and original running-shoes control share the runtime toggle. The field no longer injects B from the saved DS lock. The port starts from `auto_run` and keeps runtime changes in memory. Rebuilds `field__field_control.o` and `player_avatar.o`. Pending New 3DS testing.

# CIA startup splash

Restored the Homebrew startup logo and ExeFS logo flag. On the owner’s New 3DS, run 719 and a repack without the IR mapping both produced an SD-card ErrDisp before game logging. The same run 719 executable repackaged with the pre-run-714 settings booted. Run 720 booted with both restored logo settings and the Pokewalker IR mapping. Selecting a Pokemon then stopped on `[SS-MISSING] sub_020957B0`. Patch 0253 translates the original picker cursor table and forwards its fifth argument. Translation and ARM syntax checks passed. The picker fix still needs a game build and a hardware test on a copied save.

# Seasonal ambient weather (0248)

Settings > Display > Ambient weather defaults to ON when the key is absent from config.txt. A saved 0 still keeps it off. Seasons rotate every seven calendar days using the console clock, for a 28-day cycle. ON replaces the short town showers with regional conditions shared across eligible clear towns and routes for each three-hour clock window. Spring is wetter, summer is mostly clear, autumn brings some rain and morning or evening mist, and winter brings snow most often around Mahogany and Blackthorn. The same winter front can bring rain to the lowlands and snow to the highlands. Native weather and field tasks take precedence. Connected outdoor maps pass the destination forecast to the existing renderer transition, so a shared shower does not first request clear weather. The native weather manager draws the ambient effects, and Alive wild Pokémon uses its existing rain shelter and weather-dependent flyover behavior once the weather manager reports the effect. The forecast stays in RAM and does not update the native weather field used by saves, encounters, or battles.

The first active season and season changes log as `[WEATHER] season ...`. Weather changes log as `[WEATHER] map ... ambient ...`. Generated field and encounter C and the platform changes passed devkitARM syntax checks. Host checks covered three-hour consistency, native weather priority, task gating, and toggling back to clear. The new conditions still need visual and frame-time checks in mono and wide stereo on New 3DS.

# Flower Shop clerk and landmark expansion (0247)

When the morning Bellossom is present and Goldenrod's Flower Shop daytime routine starts in dry weather, a temporary outdoor clerk can walk to the real door using the game's native NPC movement. Bellossom follows the moving clerk and finishes its doorway animation. The clerk is removed at the door or if the scene times out. The original shopkeeper stays inside and retains her dialogue and scripts. The outdoor clerk is excluded from map-object saves and field interaction by its map, sprite and reserved object ID. Its last step onto the warp tile requires the matching doorway and an unoccupied tile.

Named schedules also cover Sentret at the Cherrygrove Guide Gent's house, Marill at Olivine Café, Machop at the Cianwood Pharmacy, and Eevee at the Celadon Department Store. Azalea's Pokémon Center has a measured flat roof perch from model 40 in land block 33.

Generated encounter, field, map-object and input sources passed devkitARM syntax and staged-hook idempotence checks. The native movement and creation symbols exist in the current game ELF. A host harness covered the temporary object, blocked approach, following, door entry and cleanup. Device checks are still needed for sprite loading, native held movement, actual door timing and landmark visibility. There are eleven named resident locations, three measured rooftops and two Violet overhangs. Other towns use their local profiles and shared behaviors.

# Johto and Kanto town identities (0246)

Alive wild Pokémon ON + TOWNS now includes city maps that have encounter tables. When a spawn candidate is on an encounter tile it uses the wild table and keeps its normal battle behavior. Open town tiles can support local residents on the same map. City showers use the transient renderer effect introduced for Violet, with different intervals and durations and no save writes. Native nonclear weather takes precedence.

Town profiles set scenery preferences, play frequency, and nap tendencies. Ports lean toward water, wooded towns toward greenery, and rocky towns toward cliffs and walls. Goldenrod and Cianwood are more playful, while Lavender and Ecruteak residents are less inclined to nap at night. Kanto gets named schedules for Persian at Vermilion's Fan Club, Shuppet at Lavender's Radio Station, and Geodude at Pewter's Museum. Three additional areas have species mixes, and unlisted towns use a shared fallback profile.

Generated C passed devkitARM syntax and staged-hook idempotence checks. Host tests covered encounter-bearing town classification, town profile differences, and the Kanto landmark schedules. Device checks still need to cover how often the behaviors occur, encounter/resident coexistence in towns with surf tables, and transient rain on city maps other than Violet. Authored roof perches and overhangs still apply only to the landmarks with measured geometry.

# Violet overhangs and passing showers (0245)

Violet has covered outdoor tiles under the Pokémon School and Gym models. The covered tiles were sampled against the US map prop meshes and terrain flags. Nearby residents can look for an open spot under the cover, route toward it, and stay dry there. The existing live checks keep doorways and occupied tiles clear.

When Alive wild Pokémon is set to ON + TOWNS and Violet's native weather is clear, short local showers use the game's weather renderer. They pause during field tasks. The timer survives a visit to the Pokémon School, so the rain can still be going when the player comes back out. No native weather or save values are written, and existing nonclear weather takes precedence. Patch 0246 extends these showers to other city and town maps.

Generated C passed devkitARM syntax and staged-hook idempotence checks. Host tests covered overhang tiles, shower timing and enable/disable behavior. The native weather helpers are present in the current linked ELF. Rain appearance and outdoor routing still need a device playtest. Logs include [WILD] Violet shower starts / clears and seeks an overhang.

# Goldenrod and Violet roof perches (0244)

A solo flier can descend onto a flat patch of the Goldenrod Flower Shop roof or Violet Pokémon Center roof. Both points were measured against the US SoulSilver prop mesh and placement. The flower shop is model 77 in land block 39, and the center is model 62 in block 24. The flier pauses directly over the roof while descending and climbing. The landing and shadow use the measured roof height. The cruise height exceeds the sampled model bounds along the approach lanes.

The generated C passed devkitARM syntax and staged-hook idempotence checks. A host harness covered roof lookup, descent, rest, ascent and ordinary terrain-landing timing. The actual sprite contact and visibility still need a device playtest. Roof landings log [WILD] ... (roof landing pass).

# Terrain ledges for fliers (0243)

A solo flier looking for a landing now checks the map height at the landing point and around its feet. Flat terrain above a nearby drop can serve as a ledge, including during rain. A steep or missing surface is skipped. The flier can consider another clear lane if the first offers nowhere to land, and nearby players can hear wingbeats again on takeoff. The height check samples field terrain. Building model roofs still need their own landing data.

The generated C passed devkitARM syntax and staged-hook idempotence checks. Host tests covered flat ground, a ledge, a steep slope, and missing terrain. The next device check should verify actual landing frequency and visible ground contact. Ledge candidates that become flights log [WILD] ... (ledge landing pass).

# Resident detours and landmark rain shelter (0242)

Named residents use a bounded search for door and home routes. They can go around an obstruction, wait if no path is available, and retry when the way clears. A resident scheduled outside can switch to the linked building during rain. The matching indoor resident is available if the player enters that building, and the outdoor schedule resumes when that map is dry. The shelter association is transient and follows the current map visit.

Generated sources passed devkitARM syntax and idempotence checks. Host checks covered routes around walls, occupied paths, blocked goals, search limits, and the rain shelter transfer and reset. Actual routing and timing still need a device playtest.

# Scheduled resident doorway walks (0241)

A named resident already on screen can now walk toward its landmark doorway when the schedule changes to a matching location on the other side. It follows legal ground tiles and fades toward the entrance before the warp tile. Indoors, a resident can appear near the entrance and walk to its usual spot when there is room. The map transition itself is inferred from the schedule. NPC scripts and player warps are untouched.

Doorway direction and schedule checks passed in a host harness, including a blocked approach, a legal step and the fade at the staging tile. The generated C passed devkitARM syntax checks and the hook was idempotent on staged copies. The device build still needs a look at actual doorway routing and fade visibility.

# Named resident schedules in Johto (0240)

The schedule table adds Mareep near Violet's Pokémon School, Aipom near Kurt's house in Azalea, and Vulpix near Ecruteak's Dance Theater. Mareep and Aipom have daytime indoor spots and morning/evening outdoor spots. Vulpix spends the morning and day outdoors and visits the theater in the evening. Bellossom keeps its Goldenrod routine. When an outdoor routine changes to a different spot on the same map, an undisturbed resident can walk toward the new spot using the existing tile and crowd checks.

The door and NPC anchors were checked against the map event data. The generated C passed devkitARM syntax checks, and host checks covered all four resident schedules and the indoor/outdoor phase rules. The standing tiles and frequency still need a device playtest. Transitions between maps are still clock-based.

# Goldenrod Flower Shop resident (0239)

The first authored schedule uses the Goldenrod Flower Shop map events. A Bellossom can spawn near the entrance in the morning, near the shopkeeper during the day, and a few tiles to the east in the evening. The spawn still requires a legal open tile and respects the town count. The indoor map only participates in this named schedule. The shopkeeper keeps their existing scripts, and the resident's appearance is derived from the clock and current map.

The RTC/map combinations passed host checks, and the generated sources passed devkitARM syntax checks. The exact in-game standing tiles and frequency still need a playtest. A continuous shopkeeper escort, further authored locations and roof ledges remain open.

# Weather, tracks, berry play and landings (0238)

Exposed land Pokémon can shake off water when rain ends. Mud and damp grass can retain short-lived prints at the previous tile. Weather-appropriate species now take precedence over the local species list for regular flyovers in rain or snow. Some low solo fliers can land and rest along a clear route beside scenery, or near a terrain edge in rain. A close player, NPC or nearby resident can shorten the stop. Rain edges are inferred from terrain collision and are not authored roof ledges.

A rare town play event rolls a small decorative berry across an open tile. Residents can follow it and pause to sniff. The prop uses field coordinates and is cleared with the play state, so it does not create a bag item or save entry.

Generated C passed devkitARM syntax and staged-hook idempotence checks. Host checks covered weather recovery, ripple separation, berry route and cleanup, and the glide/rest/climb timing. Build both the game hook and platform together. The transient buffer grew to 576 bytes and uses P3DS_WildMemV2 so mismatched builds fail at link time. Needs New 3DS verification for effect visibility, ground contact, route frequency and audio.

Named shopkeeper and terrain routines can build on the Goldenrod Flower Shop example above.

# Overworld sound cues (0237)

With Alive wild Pokémon and Visible wild cries enabled, nearby fliers can play a wing cue as they approach and a species cry as they pass. Nearby Pokémon sometimes call back during their play animation. A startled wild Pokémon uses the game's grass rustle sound when it tucks into nearby cover. New cues share a short cooldown, and ambient calls skip a cry channel that is already in use. Legendary flyovers retain their midpoint call.

Generated C passed devkitARM syntax checks, and the existing game ELF exports the cry-status helper. A host harness verified the flyover sound windows in both directions, proximity and mute rules, cue deduplication, and shared cooldown. The designated build and a listening test on New 3DS are pending.

# Wild Pokémon peek out of grass (0236)

With Alive wild Pokémon on, ordinary startled wild Pokémon can stop once they reach tall grass. They tuck down with a short rustle, track the player, and gradually peek back up after a quiet interval. Close footsteps can send them fleeing again. The cover state changes the sprite and movement decisions while the existing collision and encounter hooks stay in effect.

The staged hook was idempotent and the generated C passed devkitARM syntax checks. A host harness verified the response to nearby noise, a quiet approach, getting too close and leaving the area. Peek visibility and how the timing feels need a New 3DS check. The transition into cover logs [WILD] ... ducks into grass.

# Town scenery routines (0235)

With Alive wild Pokémon set to ON + TOWNS, some land residents look for safe spots by water or vegetation when spawning. When undisturbed near home, a resident can face the scenery and linger. In rain, residents can pause beside a wall or in grass and favor grass when choosing a nearby tile. Wall-side spots are inferred from terrain collision, which also includes fences. There is no roof, awning or shop-opening metadata in this behavior.

The devkitARM syntax checks and staged-hook idempotence check passed. A host harness covered pond detection, open ground, rain-side terrain and the existing doorway and movement safeguards. The designated build and a New 3DS playtest are pending. Rain transitions log [WILD] ... seeks a rain spot.

# Overworld grass and water cues (0234)

With Alive wild Pokémon enabled, moving into tall grass or being startled while in it briefly draws bending leaves at the Pokémon's feet. The leaves are a small sprite effect. A water Pokémon's jump now ends in an expanding ripple at the surface, and the Pokémon stays on its landing tile until the ripple fades. The effects use the existing field camera, ground height and cave visibility checks.

The staged hook was idempotent and the generated sources passed the devkitARM syntax checks. An isolated timing test covered the splash-to-ripple transition, ripple expiry, and the rustle lingering after a step. The designated build and a device look at the effect size, water-surface alignment and stereo are pending.

# Overworld routines, play and rain (0233)

The Alive wild Pokémon setting also enables species-weighted temperaments and timed naps. Each spawn keeps its own roll. Some land Pokémon return toward their spawn to nap, and nocturnal species are more likely to nap by day. Nearby Pokémon sometimes stop to face each other and take turns hopping or splashing. Pairs end the exchange when approached or when either participant leaves. Wild strangers can still squabble. During rain, ordinary land Pokémon can seek adjacent tall grass and stay there. Those without nearby cover can leave when the player is far away. The behavior uses existing spawn tiles and terrain, with no authored shop or awning anchors.

The generated C passed devkitARM syntax checks and the hook was idempotent on staged copies. A host harness checked species biases, alternating hops and splashes, interruption by the player, and natural play expiry. Needs the designated build and a New 3DS walk through a town, grass route and rainy route. Look for [WILD] slots ... play and [WILD] ... steps into cover.

# Overworld Pokémon behavior (0232)

Under Settings > Battle > Alive wild Pokémon, ON enables the wild and flight changes and ON + TOWNS also enables town residents. Land residents use the existing tile collision hook on roomy tiles, leave warp approaches clear, and turn toward the player on A. Their spawn tile acts as a home for idle wandering and naps. Shy wild Pokémon watch a quiet approach, while a sprint or bike can trigger retreat into available tall grass. Solo fliers sample three possible low lanes against terrain collision. A flier without a clear lane, a flock, or a legendary uses a higher pass.

The hook was applied twice to copies of staged sources to verify idempotence. The changed C files passed the devkitARM syntax check. A host harness checked narrow corridors, warp approaches, solid town interaction, the wild battle path, and blocked flight lanes. Needs the designated build and a device walk through Goldenrod and Route 34 to check where residents spawn, roof clearance and high-pass visibility. Flight clearance uses terrain tiles and a fixed higher altitude, so roof clearance still needs a visual check. No save data is written.

# Mystery Gift menu

Mystery Gift (0231): draw the selection arrow inside the original overlay 74 list windows. With Grey unsupported menu on, the receive list disables local methods unless Wireless is LOCAL and Nintendo WFC unless Wireless is ONLINE. The port logs blocked selections as [SS-MG]. Needs a game rebuild and device checks on the receive list and Wonder Card menus. Local gift delivery still needs a compatible sender and hardware verification.

# Bag pocket palette and JOHTO panel

Rapidly alternating Items and Medicine in Azahar reproduced missing Bag graphics from both entry points. Native Bag state still changed pockets, and software and GPU composition reproduced the same pixel loss. The background palette bytes retained by the native Bag changed during the repro. The platform now snapshots the Bag's background and object palette sources on entry and uses those copies for the matching pocket uploads. The underlying palette overwrite is still unexplained.

Azahar's rapid-click trace kept roughly 49,000 visible pixels after this change, compared with a drop to roughly 6,500 before it. The JOHTO panel also avoids temporary text stand-ins and skips rendering the hidden top screen where possible. Roamer icons use their species palettes. Full-size opening, remaining open latency, visual roamer verification, and hardware testing are pending.


# Visible wild ground height (2026-11-09)

Visible wild sprites were drawn at the player's height (`PlayerAvatar_GetPositionVector()->y` shared by every sprite in `SS3DS_WildDraw`). Walking up stairs lifted them; walking down pushed them into the ground. Patch 0230 samples the map mesh at each sprite's own world x/z through `sub_02061248` (the query map objects already use). Hop, the land shadow, and the flyer altitude (28 or 40 plus bob) stay added on top of that height. The player's height is only the hint when a bridge sits over ground, and the fallback when that spot has no surface (`unk60` null or the query returns false). `wild_vis` off still returns before any of this. Wide view and stereo are unchanged: the quads stay in world space.

Needs a game rebuild so hook 0110 refreshes `fieldmap_native.c` (`fieldmap_native.o`). No platform rebuild is required for this patch alone. Not compiled here. Save file is untouched. First four mismatches log `[WILD] own ground y A, player y B`. A missing surface logs `[WILD] ground height missing, keep player y N` up to four times.

# Built-in 3D defaults (2026-11-09)

Built-in defaults when the key is absent from `config.txt`: `g3_packets=1`, `g3_fast_bounds=1`, `gpu_obj_affine=1`, `gpu_b_fades=1`, `gpu_compose_b=1` (ON; CHECK stays 2), `title_clean_sides=2` (WIDE). A saved line wins, including OFF and LEGACY. The menu still writes a key only when that row changes. Wide title on the title screen is still unconfirmed on hardware.

Hardware run 702 of `1fcccb2`: those five options were active in the field and in battles, including stereo. Samples were mostly 29–30 fps, the exit was clean, and the log had no crash. Play of that build found the options working well. That pass does not cover every scene and is not a measured fps comparison. Title sides were set to WIDE later in the run and the title was not opened again, so the wide title is still unconfirmed on hardware. Occasional fades and battle speed dips may remain.

GTS support on hardware is unchanged. A revival-server outage reported on 8 Nov 2026 is server-side. Mystery Gift is a separate path and was still working.

# Johto bottom map: player head, zoom, roamer icons (2026-11-09)

Platform only. No pspoke patch, no save write, no gameplay or roamer-position change. Not compiled here and not run in Azahar or on a 3DS.

The player head is a 32x32 overlay. The map under it is doubled nearest, then sampled linearly onto the 320x240 bottom screen. `mark * 5 / 4` matches that scale when the DS pixel mod 4 is 0 or 1. When it is 2, the sample sits on the texel edge. When it is 3, it lands a full screen pixel into the previous pixel. The head then crawls against the town while the view recentres and sits off. Standing still does not move it. The head now uses the screen pixel whose sample falls on that same DS pixel (`pgDsToNative`). Real location changes still move it. The old 2px upward bias (`-18` instead of `-16`) is kept.

Settings > Input "Map zoom" (`map_zoom`, default 0 = 1X). 1 = 2X. The `-` and `+` chips at the bottom-left of the map write the same key. 1X uses the previous horizontal view. 2X repeats each map pixel as a 2x2 block, keeps the place you were centred on, and centres vertically on you while the view follows you (or on a place opened from the phone list or the progress list). Taps, the outline, finder pins, phone pins, Fly, and roamer marks go through that same map-to-screen step. Captions and the side column are not moved.

Settings > Input "Roamer sprites" (`roamer_sprites`, default 0). OFF keeps the coloured R/E/L letter. ON draws that species' menu icon from the ROM (`a/0/2/0`, the finder card's reader), 16x16, with a coloured corner. The per-species palette index is not in that file, so it is the same silver/navy tint as the finder. A missing icon keeps the letter. Who is shown, and where, is still the tracker. Kanto stays hidden until the gate says so.

Log: `[PGMAP] zoom 1x -> 2x`, `[TRACK] roamer sprite slot N species S ok|missing, letter kept`. Rebuild: `soulsilver3ds/full` `make` only. No clean game rebuild.

# Rendering performance hardware trial (2026-11-08)

Platform rebuild/link against main after PR #401. The field command and bounds shortcuts were checked against paired stereo frames. The user confirmed smoother battle send-outs and approved the GPU wide title appearance in Azahar. A steady 300-frame title sample averaged 38.79 ms. GPU title verification compared the centre and side panels against the exact CPU layout and reported zero differing pixels across 240 checks. These measurements are from the local experiment. The hardware pass above covers the five shortcuts in field and battles; wide-title hardware and full scene coverage are still open.

See Release Notes for the settings to reproduce the trial. Second screen GPU CHECK presents the CPU result and logs pixel differences. Use ON for timing. Unsupported composition falls back to the CPU. Keep the ROM, 512 KiB save, and build artifacts outside Git.

# JOHTO panel visual polish (2026-11-08)

Platform-only spacing and icon polish for the bottom-screen JOHTO panel. No new setting, no pspoke patch, no save write, no achievement unlock/date/reward change, no toast change. Wide view (`wideview`, `wide_menu_screens`, `gpu_direct3d_wide`) was not edited.

Tabs: when the sharp column is on (the normal scaled bottom screen), the MAP/PHONE/RADIO/PARTY/BAG(/EGGS) buttons are drawn again at native resolution with the same box, shade, and green pixel icons as the right-hand menu (`bnTabStrip` in `ss3ds_blnative.h`, registered on the existing bottom overlay). Hit rectangles are unchanged. On the map, the place-name line keeps the location and adds the selected tab's word in a green chip on the left. 1:1 scale still uses the DS-resolution buttons.

Achievements: the title stops before the trophy and dots buttons. The point total and progress bar sit on their own band underneath. Each row is the name and the points, then the description wrapped inside the margins, then the earned date on its own smaller line. Rows are taller, so fewer fit. Dragging still scrolls by pixels (`achScroll -=` the DS y delta, finger up reveals later rows). A line that crosses the list edge is clipped to the list instead of being skipped, so the next row cannot vanish into a blank gap.

Build: platform rebuild and link only (`soulsilver3ds/full` `make`). No clean game rebuild. No new objects outside `render3ds.cpp` and the headers it already includes.

`ea3b911` built from scratch on the owner's machine (devkitARM gcc 16.1.0, all 168 patches, platform `make` exit 0; preexisting `-fpermissive` notes at `render3ds.cpp:2039` and the `crtn` linker warning). Archive string `run 0 (no git)` is expected. Azahar 2126.2: tabs PASS, list layout PASS. Scroll FAIL. `d759a06` (clip partial lines) was rebuilt (`render3ds.o` / `tracker.o` forced) and retested in the same Azahar: still no movement. Many upward and reverse drags left Zephyr and Hive on screen. The tester could not confirm the drag was delivered as a held touch.

The list does not move unless a touch goes down on the panel (`blHit` 10 -> `bmDown` 21), stays held, and travels more than 16 DS px. Until that gate, `achScroll` is unchanged. The host check before `d759a06` wrote `achScroll` directly, so it never ran this dispatch. A 10px move is still a tap. The sample that passes 16px now applies the whole stroke (`pgAchDragDelta`). UP and DN chips on the totals band page the list with an ordinary tap, the same kind of press that already opens the list. `[ACH] DN scroll A -> B` and `[ACH] drag scroll A -> B` (first 8 drags) are the only new logs.

Host check covers that dispatch, a DN tap moving the top row, the 16 gym rows, both clamps, and the scaled / 1:1 touch mapping. Not re-run in Azahar. On the next local build, tap DN (right of the score) to bring in Plain Badge. A real held drag should also move the list and write one `[ACH] drag scroll` line.

# Audio boundary fix (2026-11-08)

Bound NNS_SndPlayerMoveVolume targets to 0..127 before libntr's decibel lookup. SoulSilver's field code requests 128; in the inspected US ROM the next signed word after SNDi_DecibelTable is 0, but the port places VolumeTable there (bytes 00 01 = +256). The resulting Goldenrod player fade was +207 instead of -49, boosting the music and triggering the loudness workarounds.

Platform-only: `psp_ctru.c` wraps `NNS_SndPlayerMoveVolume`, enabled by `full/Makefile`'s `--wrap`. No pspoke patch or clean game rebuild is required; normal platform rebuild/link. Existing audio settings remain unchanged. `[AUDIO] fader target ...` logs the first eight corrections. Local diagnostic code is excluded.

Validation: isolated Azahar 2126.2 build, owner's copied save at Goldenrod / Route 34. Pre-fix playerfade=207; US ROM table and port ELF inspected to verify the out-of-bounds word. Fixed test logged nine corrected requests (uncapped diagnostic logging), maximum input peak 31,141 across five audio summary windows, zero clipped samples, leveller/guard at 100%; owner reported improved sound. New 3DS hardware remains untested. Owner builds and merges; no SD deployment performed.


- 0147 (unbuilt, Grok): NULL guard in SysTask_TimedPaletteFade for the run 67 crash leaving Kurt's house after the Fast Ball. User builds and tests.
- 0148 (unbuilt, Grok): seed overlay_01_021F1348.s, drop the ov01_021F1A34 stub (run 67 Slowpoke Well crash, [SS-MISSING] ov01_021F1A34).
- 0149 (unbuilt, Grok): seed unk_0206979C.s, drop 3 stubs (Slowpoke Well [SS-MISSING] sub_02069998 after 0148).
- 0150 (unbuilt, Grok): native-provided sub_02020DA4 (link dupe after 0149, same as 0144).
- 0151 (unbuilt, Grok): sub_020154C4 only SPL_Deletes an emitter still in act_emtr_list (run 67 battle crash pc in SPLList_PopFront via SPL_Delete).
- 0152 (unbuilt, Grok): SPLManager_DeleteEmitter skips emitters not in activeEmitters (0151 only covered sub_020154C4; Supersonic still crashed). libss-particles.a must rebuild.
- 0153 (Grok): ss3ds_game.sh re-syncs particle glue.c and applies 0152 to a staged build.py, then rebuilds libss-particles.a (0151/0152 never reached an already-staged tree).
- 0153 is now also a pspoke patch: the builder runs pspoke/scripts/ss3ds_game.sh (live copy), not scripts/ss3ds_game.sh (reference). Put build-script fixes in a pspoke patch.
- 0154 (Grok): sub_020154B0 counts the real act_emtr_list instead of trusting node_num (Supersonic hang after 0151/0152 skipped deletes).
- 0155 (Grok): was stuck-emitter printf diagnostic for Supersonic hang; run 116 drops the printf (comment only; hang fixed by 0156).
- 0156 (Grok): run 67 log showed "skip delete ... active=1": the battle waits forever on one stale emitter. If the delete target is not active and exactly one emitter is, delete that one.
- 0157 (Grok): Kurt Apricorn Box (overlay 58) crashed at ov58_02237CCC writing *(u32*)0x04000000; hook drops the raw DISPCNT writes. Run 67 also confirmed 0156 cleared the Supersonic hang.
- 0185 (Grok): seed Mystery Gift (overlay 74 / overlay_74_thumb.s); drop 58 ov74_ Missing() stubs so Init ov74_0222AE6C / Main / Exit are translated (mirror of 0164 Global Terminal).
- 0186 (Grok): maploader translate2 accepts `$` in literal-pool symbols (nitrocrypto TEMPNAME__$p16 etc. pulled in by Mystery Gift RC4 after 0185).
- 0187: CRYPTO_RC4Encrypt done natively in missing.c (it's an ARM routine, translator raised 'fallthrough without return'); added to native-provided.json.
- 0188: CRYPTO_SetAllocator (no-op) and CRYPTO_VerifySignature (returns TRUE) done natively; ParseSignHash and the BN_* RSA code are ARM and failed with 'unsupported stack layout'.
- 0189: native SVC_GetCRC16 + CRYPTO_RC4Init in missing.c; ov00_* wifi calls from ov74_02236AE0 stubbed with Missing() (download path, needs real DWC later).

- 0196 (Grok): Nd setupHttp - restore getDSdesc/attrs/mac so GetFileListNumAsync is not folded to FALSE (run 111). CLEAN network rebuild.
- 0199 (Grok): strip [SS-MG-CURSOR] dumps from ov74_02235FF8 + ListMenuUpdateCursorObj; keep PlaySE(1500) and gSS_MG_CursorDiag.
- 0198 (Grok): drop 0155 stuck-SPL printf spam from glue.c (hang already fixed by 0156).
- 0197 (Grok): hook 0197 grey unsupported title-menu options (main_menu.c); Settings > Setup menu_grey_unsupported default ON. CLEAN game (ov74 main_menu) rebuild.
- 0195 (Grok): SVL Http_StartThread priority - compile-network.py rewrites dwc_svl.c 0x69 to 16 (CreateThreadReal aborts on >31; run 110 abort). CLEAN network rebuild.
- 0194 (Grok): MG WFC path — log DWC_Init in sub_02039FD8; platform wraps ov74_0222A560/A4CC so wireless=2 redirects GET VIA WIRELESS to WFC download; ov00_021EC294 is real DWC_SetMemFunc.
- 0193 (Grok): Mystery Gift WFC download — drop Missing() on 10 ov00 Nd/inet trampolines; real DWC_Nd* + CleanupInetAsync + NASLoginAbort in ss3ds_dwcstub.c; plain HTTP DLS URL + NdCancel fill in compile-network.py; IPGE→IPKE. Need ONLINE.
- 0192 (Grok): Mystery Gift moveCursorFunc — native ov74_02235FF8 PlaySE(1500); run 116 strips [SS-MG-CURSOR] dumps (keeps gSS_MG_CursorDiag for patch compatibility).

## Run log (Grok Bot)
- 2026-11-04 solid-30 fps thread, part 2 (Claude): `gc_direct` direct screen (gpucomp P3DS_GcDeferOut / P3DS_GcResolve*, gpuscale dirTop/dirTopR, render3ds topTouch/holdSync; field + battle, mono + both eyes; menus still read back). Any new CPU code that reads or draws on display[0]/eyeL/eyeR after the compose must call topTouch() first. Also: tab-swap hold, battle-start column flash, window/black-BG aware wide strips (winEdge, edgeBgBlack), hook 0214 type-hint off. Azahar: wild battle with slider 17 -> 30 fps, Goldenrod slider walking 29.5 avg. Hardware untested.
- 2026-11-04 solid-30 fps thread (Claude): `g3_batchcull` (3D tab "Off-screen skip", default ON, log `[G3CULL]`) skips raw 3D batches / 16-triangle runs whose bounding box is off screen before transforming them; stereo raw triangles are written for both eyes in one pass (`G3G_TriClip2`); colour divides -> table. Azahar Goldenrod: mono 26.1 -> 29.3 fps, slider up 21.0 -> 25.0 fps. Not on hardware yet. Profiling method: link `--wrap` timers around NNS_G3dDraw / PSPNativeG3CommandsBorrowed etc. in a throwaway build (Azahar svcGetSystemTick is a fair CPU proxy; GPU waits are not modelled).
- run 133: panel_party tab-tap pause: after the PARTY/BAG tap the game fades the field to black for ~9 frames before the app Init (hardware log: SS-FADE, MBRIGHT 8005->8010), so the top went black and the panel jumped. While Want=1 and On=0 the compositor now keeps showing the last held field frame on top; the PARTY pane says 'Opening party...' instead of 'Reading your save...' before the first open. Platform-only; verified in Azahar (top stays the field through the open), not yet on the 3DS.
- run 132: panel_party now also carries the Pokemon SUMMARY and the BAG. Hook 0203 (party_menu.c) stops 0201 clearing P3DS_PanelPartyOn in Init/Exit; the platform (ss3ds_glue.c __wrap_OverlayManager_New) now owns it: a bag or summary manager claims a pending Want (or keeps On when chained from the panel), and the field or a battle template clears it. Bag and summary are lifted code, so they are matched by Init (Bag_Init / PokemonSummary_Init; Bag_LaunchApp uses a stack copy of its template) with no game hook. The BAG tab chip opens the real bag the same way the PARTY chip opens the party (Want=1 + StartReq 2). The panel picture is the engine on the bottom LCD (POWCNT bit15 set => engine B, as the bag does), and master brightness of both engines is cleared while On. Top screen stays the held field (summary/bag top content is dropped). Verified in Azahar on copies: PARTY tab -> party -> A -> summary (stays in the panel, no MAP flash) -> B -> party -> B -> field; BAG tab -> bag (D-pad and stylus via PanelDest) -> Potion -> 'Use on which Pokemon?' in the panel -> B -> bag -> B -> field, panel back on MAP. Not yet device-tested. CLEAN game rebuild needed (0203 + party_menu.o).
- run 131: run 130 hardware log: panel_party WORKS (engine A snap rgbNz=48352, party shows in the panel, Exit returns to MAP). Tab tap -> party Init is ~9 frames, but the PARTY pane showed the read-only list for that moment (looked like a pause/jump). With panel_party on, the PARTY pane now draws 'Opening party...' while Want/On is set (ss3ds_pgextra.h pxParty). Platform-only.
- run 130: panel_party touch. While the party is in the JOHTO panel, a stylus tap inside the drawn party picture (P3DS_PanelDest, published by the compose block in render3ds.cpp) is mapped back to the party's 256x192 touch space in main3ds.c (taps outside are ignored for the game; the chrome keeps its own handling). The run 120 blanket stylus block is gone, and this check now runs BEFORE the P3DS_BmOn mask (tiles are shown while the party is open). Platform-only, panel_party default OFF; not compiled/tested.
- run 129: JOHTO tab chips are as wide as their own label (pad 3) instead of equal width (widest label + 10): the five equal chips ran the whole bar and covered the location name on the MAP pane. `pxChips` in ss3ds_pgextra.h. Platform-only; not compiled/tested.
- run 128: panel_party fix + sharper + tab opens it. (1) Party list was black/recursive: hook 0202 skips the party fade-in so engine A kept the field->party fade-to-black in MASTER_BRIGHT (0x6c) (run 126 log bot rgbNz=0), and the snap read engine B (a stale chrome copy). Now MASTER_BRIGHT A is cleared while On&&!InField and the snap reads display[0] (engine A). Reproduced and verified in Azahar with hooks 0201/0202 built in. (2) Panel downscale is now an area average instead of nearest-neighbour (text was garbled). (3) Tapping the PARTY tab itself opens the party in the panel (panel_party=1); leaving it returns the panel to MAP so the next tab tap re-opens. Platform-only, panel_party still default OFF. Not yet device-tested for (2) and (3). Local trees staged before hooks 0159-0202 have no panel hook: run scripts/ss3ds_build.sh once.
- run 126: panel_party showed a recursive JOHTO bottom (tabs + DEX/PKMN) inside the panel. Cause: snap fell back to `bot` when nBot>nTop; bot/display[0] still held chrome from the previous frame. Fix: snap LCD-top only (SUB_MAIN party), never bot/sPanelBot; keep last good snap if top empty. Scratch present kept. Live OFF clear kept. Default OFF. Platform-only. No CLEAN.
- run 125: panel_party still black on HW despite CPU panelSamp having pixels: composed into display[0]/bot) which engine A / GPU compose can rewrite before upload. Fix: memcpy chrome to private sPanelBot, fit party there, present that scratch (not a game FB). Keep last good snap if a frame is rgbNz=0. Live toggle: turning panel_party OFF clears Want/On immediately. Default still OFF. Platform-only. No CLEAN.
- run 124: Fix panel_party black/misplaced party. Root cause: 1:1 top-left crop of the 256x192 party into the ~184x169 JOHTO panel showed mostly backdrop (looked black; brief flash was a corner of the UI at the wrong size/place). Also the run 123 "nonblack" metric treated opaque black (A=0xFF, RGB=0) as content (49152=every pixel). Now: centred nearest-neighbour fit of the whole party UI into the panel; RGB-only stats + distinct estimate + snap/panel samples; log dest rect; force bottom re-upload while On (GpuBotSame=0). wideview / gpu_direct3d_wide unchanged. panel_party still default OFF. Platform-only. ASCII-only. Platform rebuild only.
- run 123: Fix run 121/122 panel_party black box. Root cause: party_menu Init uses GX_DISP_SELECT_SUB_MAIN so the party UI is on the TOP LCD (engine B when POWCNT bit15 clear); run 121 snapped `bot` (engine A / touch), which is black, then 1:1-cropped that into the JOHTO panel. Now prefer LCD-top, else whichever of top/bot has more non-black pixels; one-time `[PANEL] party source ... nonblack top=/bot=` log plus compose nonblack count. WideStrip clear kept (hold field). `built Oct 30` in the log is an unrebuilt game TU __DATE__; unrelated (0201 was live). panel_party still default OFF. Platform-only. No save/GTS/MG/battle changes. ASCII-only. Platform rebuild only (no new hook).
- run 122: Restyle JOHTO pg_extras top tab strip (MAP/PHONE/RADIO/PARTY/BAG) in `ss3ds_pgextra.h`. Even equal-width chips, 2px rounded corners, navy border, blue selected fill with top lip + bottom bar, centered crisp game-font-3/4 labels (no layout/behaviour change; hit rects still from pxChips). Behind existing pg_extras. Platform-only. No save/GTS/MG/battle changes. ASCII-only. Platform rebuild only.
- run 121: Fix run 120 panel_party hardware bugs. (1) Freeze field hold only from clean overworld frames (Want|On); do not capture fade-to-black. (2) Compose: 1:1 crop of party into JOHTO panel (no NN downscale); clear WideStrip/G3G_Wide so cyan bars cannot sit on a black hold. (3) Live chrome via ppChrome while On (keeps painting JOHTO + bottom_hd); keep P3DS_OvN so the native POKEDEX/POKEMON column stays - clearing OvN in run 120 dug up the legacy blDraw short DEX/PKMN/... alts. Dropped dead saveJohtoChrome (stale snapshot path). While On, feed full labels as alts so short DEX/PKMN cannot paint even if overlays are off. Ignore PARTY/chrome re-taps while On. Hook 0202: skip party BeginNormalPaletteFade when PanelPartyOn. panel_party still default OFF; OFF = run 119. No save/GTS/MG/battle changes. ASCII-only. CLEAN game rebuild required (0202 + party_menu.o). Remaining: summary-in-panel, tall party, bag-in-panel, panel touch.
- run 120: Phase 0+1 first increment — inline Party in the JOHTO left panel (user-approved). NEW setting Settings > Input "Party in panel" / `panel_party` (default OFF). When ON, a PARTY-tab row tap sets P3DS_PanelPartyWant and opens the real Pokemon menu (same StartReq/0200 slot select + StartAvail gating as run 119); hook 0201 (party_menu.c) sets P3DS_PanelPartyOn. Compositor holds the last overworld top frame and nearest-neighbor blits the party UI into the JOHTO panel; D-pad/A/B (stylus blocked while On). B closes via vanilla Exit -> walking + panel. OFF = run 119 full-screen Party. Summary/reorder use vanilla party paths (summary still leaves the panel for now). GabeSud0 design reference only (HoldUpper + real app in panel); HGSS-native code, credited in THIRD_PARTY.md. No save/GTS/MG/battle changes. ASCII-only. CLEAN game rebuild required (0201 + party_menu.o). Remaining: summary-in-panel handoff, tall party layout, bag-in-panel, panel touch mapping.
- run 119: Full experience for JOHTO PARTY/BAG tabs (user-approved). NEW patch 0200 (start_menu.c): PARTY row tap sets P3DS_VwPartySlot and opens the Pokemon menu on that slot via PartyMenu_LaunchApp_Unk1 (vanilla focus; return path unchanged — safer than auto-Summary/submenu). BAG row tap sets P3DS_VwBagPocket+P3DS_VwBagSlot and writes BagCursor pocket+position before sub_0203E3FC so the Bag opens on that pocket with the cursor on that item (use/give/toss still needs in-game A; overlay 15 not in C). Same overworld-safe StartAvail/StartReq gating as side tiles / run 118. Behind existing pg_extras (off = unchanged); no sub-toggle (BagCursor + partySlot are vanilla APIs). No save/GTS/MG changes. ASCII-only. CLEAN game rebuild required (0200 + start_menu.o).
- run 118: JOHTO panel list taps + polish (platform-only). A1: tap-vs-drag threshold 16 DS px (was 6) and list scroll only after drag so Phone/Radio/MAP-chip/Party/Bag row taps are not swallowed by stylus jitter. A2/A4: Phone call + Radio tune paths unchanged (0128/0129); now reachable. A3: PARTY/BAG row taps open the real Start-menu Pokemon (action 1) / Bag (action 2); drop VIEW ONLY label (TAP TO OPEN). B1: game-font space advances without drawing NFTR ink (underscores). B2: pxFit ellipsis. B3: clear/hide stale MAP player-head overlay on list panes. Incoming-call ANSWER banner: sticky ~20 quiet frames (IsRinging flap) + pane/ring in overlay cache key. Still behind bottom_menu / pg_extras; off = unchanged. No pspoke patches, no save/GTS/MG changes. Platform rebuild only (no CLEAN game required unless 0128/0129 missing on device).
- run 117: Fix run 116 builder break. Restored 0155/0192/0193 patch files byte-identical to 6d84c24 (never edit already-shipped patches; always append new ones). Same cleanups as NEW patches 0198 (drop 0155 stuck-SPL printf) and 0199 (strip [SS-MG-CURSOR] dumps; keep PlaySE(1500)). Docs/platform dwcstub thins from run 116 kept. User stuck mid-am: git am --abort then pull. CLEAN game rebuild for 0198/0199.
- run 116: Conservative cleanup only (no functional online changes). Docs: README + WIRELESS status say GTS and Mystery Gift WFC work on hardware; archive docs/MISSING_AUDIT.md; docs/HANDOFF.md is now a pointer to this file. Patch 0192: strip [SS-MG-CURSOR] dumps, keep minimal ov74_02235FF8 PlaySE(1500) + gSS_MG_CursorDiag. Patch 0155: drop stuck-emitter printf (comment only). Platform ss3ds_dwcstub.c: thin per-call [SS-MG] spam (drop SetAttr/POST dump/GetProgress success spam); keep NdCallback, path=, NdInit result, DLS body, null/error guards, Cleanup/Cancel/abort; trim ConnectInet __DATE__/__TIME__. Untouched: soctr sleep, SOC cleanup no-ops, EC294 stub, 0194/0195/0196, GTS/LOGIN/COMM/CPS, logring/crash. CLEAN game rebuild needed (0155/0192/0193 patch text). Platform-only dwcstub also rebuilds with a normal platform link.
- run 115: Grey out unsupported title/continue-menu options (hook 0197 + Settings > Setup "Grey unsupported menu" / menu_grey_unsupported, default ON). Greys Connect to Wii, Nintendo WFC Settings, Wii Message Settings, Connect to Ranger, Migrate from AGB when shown (muted FontPal grey; A/touch plays SEQ_SE_DP_BOX03 and stays on the menu). Continue / New Game / Mystery Gift / Pokewalker unchanged. OFF = pre-0197 behaviour. Layout identical. No MG / save-format change. CLEAN game rebuild (main_menu.o / ov74) required.
- run 114: Run 113 DLS count HTTP 200 but entrynum=0 / "No gift found". Cause: we rewrote SoulSilver ROM gamecd IPGE->IPKE; AltWFC/Wiimmfi dlc.py treats IPGE (not IPKE) as Gen4 mystery-gift and InternalLoss DLS1-Files has IPGE gifts (no IPKE dir). HGSS NdSetAttr uses empty attrs (omitted from POST). Fix: keep ROM gamecd; stop NAS cal IPKE force; log masked DLS POST fields + NHTTP body. Platform+Makefile only. GTS untouched.
- run 113: Run 112 GetFileListNumAsync->1, DLS HTTP/1.1 200 bytewise, then NdCleanupAsync->1 crashed in SOCLi_CleanupCommandPacketQueue (pc=0x63d958 lr=SOCL_Cleanup) via AC CloseSocket: SOCL BSS literals are untranslated DS addrs (0x02238428 ConfigPtr); 3DS never SOCL_Startup. Fix: strong SOC_Cleanup/SOCL_Cleanup no-op; skip soctr 150ms sleep during MG so 1-byte NHTTP headers finish; wrap Nd callback to log reason/err/entrynum (gift count). Platform only, no CLEAN network. GTS untouched.
- run 112: Run 111 SVCLOC OK (svchost=dls1.ilostmymind.xyz) and NdSetAttr OK, but NdGetFileListNumAsync returned 0 with no DLS HTTP. ELF showed DWCi_NdGetFileListNumAsync always sets error 8: setupHttp tested uninitialized iVar1 after commented getDSdesc and -O2 folded the failure path. Fix: patch 0196 rewrites setupHttp (fill rhgamecd/userid, correct attr1-3) and getMacAddress. GTS untouched. CLEAN network rebuild required.
- run 111: Run 110 aborted in OS_CreateThreadReal (0x16bf58) during NdInit/SVL: DWC_Svl_GetTokenAsync passed Nitro priority 0x68 (>31). Fix: clamp in __wrap_DWC_Http_StartThread + pspoke-patch 0195 sets SVL prio to 16; MG NdInit also SetCustomNas(http) + ignoreca and forces IPKE in cal when ROM header is IPGE. GTS StartThread uses prio 0 (unchanged).
- run 110: Run 109 crashed in DWC_Http_Create (pc=0 lr=0x61d010) during NdInitAsync/SVL: DWCauthhttpparam.alloc was NULL because NAS login is skipped. Fix: MG-only NdInit wrapper sets DWCauthhttpparam.alloc/free to DWC_Alloc/DWC_Free and re-asserts SetMemFunc; null-guards on later Nd Get* wrappers. GTS-shared EC294/ConnectInet/NAS login unchanged.
- run 109: reverted the run 108 real DWC_SetMemFunc (ov00_021EC294) to the old stub; GTS shares it and was working. Kept only the MG-only redirect + logs.
- run 108: Run 107 hung on local wireless MG search (WM stand-in), not ov74_02236AE0. Evidence: WM init + icon redraw, no ConnectInet / [SS-MG] Nd / DWC_Init logs; cursor itemsAbove=1 selects GET VIA WIRELESS (submenu order Friend, Wireless, WFC). Fix: platform --wrap A4CC redirects to WFC when wireless=2; A560 logs path=WFC-download; EC294=SetMemFunc; patch 0194 logs DWC_Init. Test: ONLINE, Receive > GET VIA NINTENDO WFC (or Wireless, now redirected) and expect [SS-MG] path=WFC-download then DWC_Init / ConnectInet / Nd*.
- run 107: patch 0193 had UTF-8 em dashes; builder flagged it "[MISSING] patch not in the build" and stopped. Made the patch pure ASCII (same code). Clean build to test MG WFC download.
- Run 106 (patch 0193): Mystery Gift Wi-Fi download path wired to libntrdwc DWC_Nd* (same ONLINE stack as GTS). ov00_021ED1F0/ED354/ED388/ED3AC/ED3F4/ED444/ED428/ED308/ECEC0/EC938 no longer Missing(); [SS-MG] logs each stage. DLS URL forced to http://; SoulSilver gamecode IPKE. Test: Settings > Play > Wireless = ONLINE, no cheats, Mystery Gift > Receive > Nintendo WFC. Local wireless MG search still hangs (documented). Leftover: confirm SVL/Nd HTTP against the revival DLS host on hardware; NASLogin vs FriendsMatch if NdInit fails after netcheck.
- Run 102 (Claude): quit save before thread teardown. Device log: Mystery Gift opened, then crashed in native_ov74_0222AB70 (DS BG palette 0x05000000 built as mov #5; lsl #24, unmapped on 3DS).
- Run 103 (patches 0187-0190): Mystery Gift builds (native RC4/CRC16/signature, ov00 wifi stubs) + maploader maps mov #5-7/lsl #24 DS palette/VRAM/OAM bases to the backing arrays.
- Run 104 (patch 0191): same fix limited to ov74_* functions so nothing outside Mystery Gift changes.
- Run 105 (patch 0192): Mystery Gift cursor still invisible after 104; no confident root cause. Diagnostic only: native ov74_02235FF8 + gated ListMenuUpdateCursorObj logs ([SS-MG-CURSOR]) dump ListMenu template/window, BG/OBJ palette, and OAM0-2 while navigating MG. Open MG, move the d-pad, capture the device log.
- 0229 (Claude, unbuilt): Bug-Catching Contest helper data, read-only (field_control.c FieldSystem_UpdateBugContest -> platform tracker.c -> card in ss3ds_pgmap.h ctCardDraw; toggle contest_helper in Settings > Input; log [CONTEST]). Untested on device. Pokeathlon not done (own overlay app, no field hook).
