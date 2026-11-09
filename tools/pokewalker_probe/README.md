# Pokéwalker infrared probe

This standalone New Nintendo 3DS diagnostic waits for a physical Pokéwalker,
sends the session handshake and checks the reply. It stops before registration
and never opens a game save.

Build with devkitPro `3ds-dev` using `make` in this directory. Open
`pokewalker_probe.3dsx` in the Homebrew Launcher. Point the Pokéwalker at the
3DS infrared window, press A on the 3DS and select Connect on the Pokéwalker.
The result distinguishes missing hardware access, a missing advertisement,
send failure and a missing or invalid reply.

The hardware sequence is based on
[pwalkerHax](https://github.com/francesco265/pwalkerHax) and
[libn3ds](https://github.com/profi200/libn3ds). The probe checks the I/O mapping
before touching the transceiver. The native game's CIA currently has no
explicit mapping for that I/O page, so a successful Homebrew Launcher test
still leaves CIA transport integration to verify.
