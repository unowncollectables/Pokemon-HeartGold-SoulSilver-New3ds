# HeartGold 3DS link (EXPERIMENTAL): the SoulSilver port built as a separate game.
#
# Nothing here is a second copy of the port to maintain. Every `make` first writes filtered copies of the SoulSilver
# sources (scripts/hg_rename.py: SD folder sdmc:/3ds/heartgold3ds/, heartgold.nds, heartgold.sav, heartgold3ds.3dsx/.cia,
# HOME Menu title 000400000F7D4100) and then runs the copied link Makefile on HeartGold's own game objects (pspoke-hg):
#   ../platform/        <- ../../soulsilver3ds/platform/        (generated, not committed)
#   gen.mk              <- ../../soulsilver3ds/full/Makefile    (generated, not committed)
#   mkld_ss.py, README_settings.txt, cia/make_art.py, cia/heartgold3ds.rsf   (generated, not committed)
#   cia/art/            HeartGold's own icon and flat banner (committed, original art)
# So a change to the SoulSilver port reaches HeartGold at its next build, and nothing built here can change SoulSilver.
#
#   bash scripts/hg_build.sh /path/to/HeartGold.nds      compile the game (long the first time)
#   make -C heartgold3ds/full                            heartgold3ds.3dsx
#   make -C heartgold3ds/full cia                        heartgold3ds.cia (HOME Menu)
ROOT := $(abspath ../..)
SS   := $(ROOT)/soulsilver3ds
REN  := python3 $(ROOT)/scripts/hg_rename.py
# The HeartGold ROM (icon and the Ho-Oh banner): HGROM=..., P3DS_HEARTGOLD, the path hg_build.sh was given, then the
# usual places. Never SoulSilver's ROM: tools/find_rom.sh is not used here.
HGROM ?= $(shell bash $(ROOT)/tools/find_rom_hg.sh)
# HOME Menu banner, as SoulSilver's: BANNER=3d (default) is the 3D banner with the title screen's Ho-Oh read from the
# HeartGold ROM at build time (never committed); no ROM or any failed safety check -> BANNER=flat (cia/art/banner.png).
BANNER ?= 3d
GEN  := $(MAKE) --no-print-directory -f gen.mk SSROM="$(HGROM)" BANNER=$(BANNER)

all: heartgold3ds.3dsx
sources:
	@$(REN) copy $(SS)/platform ../platform
	@$(REN) copy $(SS)/full/Makefile gen.mk
	@$(REN) copy $(SS)/full/mkld_ss.py mkld_ss.py
	@$(REN) copy $(SS)/full/README_settings.txt README_settings.txt
	@$(REN) copy $(SS)/full/cia/make_art.py cia/make_art.py
	@$(REN) copy $(SS)/full/cia/soulsilver3ds.rsf cia/heartgold3ds.rsf
check: sources
	@$(GEN) check
heartgold3ds.elf: sources
	@$(GEN) heartgold3ds.elf
	@$(REN) check heartgold3ds.elf
heartgold3ds.3dsx: heartgold3ds.elf
	@$(GEN) heartgold3ds.3dsx
	@echo "[HG] heartgold3ds.3dsx ready (experimental). SD folder: sdmc:/3ds/heartgold3ds/ with heartgold.nds"
cia: heartgold3ds.elf
	@$(GEN) heartgold3ds.cia
	@$(REN) check heartgold3ds.cia
	@echo "[HG] heartgold3ds.cia ready (experimental). Its own HOME Menu title; SoulSilver's stays installed beside it."
clean:
	rm -rf build ../platform gen.mk mkld_ss.py README_settings.txt cia/make_art.py cia/heartgold3ds.rsf heartgold3ds.elf heartgold3ds.3dsx heartgold3ds.smdh heartgold3ds.cia
.PHONY: all sources check heartgold3ds.elf heartgold3ds.3dsx cia clean
