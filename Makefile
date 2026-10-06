# BadCBA – PS3 Makefile (prefers real ps3dev pkg tools)
ifneq ($(strip $(PS3DEV)),)
  export PATH := $(PS3DEV)/bin:$(PS3DEV)/ppu/bin:$(PS3DEV)/spu/bin:$(PATH)
endif
ifneq ($(strip $(PSL1GHT)),)
  export PATH := $(PSL1GHT)/host/bin:$(PATH)
endif

PPU_CC     := $(shell which ppu-gcc 2>/dev/null)
SPRXLINKER := $(shell which sprxlinker 2>/dev/null)
# Prefer NPDRM self tools from toolchain
MAKE_SELF  := $(shell which make_self_npdrm 2>/dev/null || which fself 2>/dev/null || which make_fself 2>/dev/null)
SFO_BIN    := $(shell which sfo.py 2>/dev/null || which sfo 2>/dev/null)
PKG_BIN    := $(shell which pkg.py 2>/dev/null || which pkg 2>/dev/null)
# Fallbacks in repo (SFO only – PKG fallback is incomplete)
SFO_FALLBACK := $(CURDIR)/tools/sfo.py

ifeq ($(PPU_CC),)
  $(error ppu-gcc not found)
endif

APP_TITLE   := BadCBA
APP_TITLEID := BCBA00001
APP_VERSION := 01.00
CONTENTID   := UP0001-BCBA00001_00-0000000000000000

TARGET := $(APP_TITLE)
BUILD  := build
SOURCES := source
INCLUDES := include
DATA := data

CFILES := $(notdir $(wildcard $(SOURCES)/*.c))
OFILES := $(CFILES:.c=.o)

INCLUDE := -I$(CURDIR)/$(INCLUDES)
ifneq ($(strip $(PS3DEV)),)
  INCLUDE += -I$(PS3DEV)/ppu/include
endif
LIBPATHS :=
ifneq ($(strip $(PS3DEV)),)
  LIBPATHS += -L$(PS3DEV)/ppu/lib
endif
LIBS := -lrsx -lgcm_sys -lio -lsysutil -lrt -llv2 -lm -lsysmodule -lnet -lsysfs
CFLAGS := -O2 -Wall $(INCLUDE)
LDFLAGS := $(LIBPATHS) $(LIBS)
VPATH := $(SOURCES)

.PHONY: all clean pkg icons version folder-install

all: version $(TARGET).elf

version:
	@echo "$(APP_VERSION)" > version.dat
	@mkdir -p $(DATA) && cp version.dat $(DATA)/version.dat

$(BUILD):
	@mkdir -p $(BUILD)

$(BUILD)/%.o: $(SOURCES)/%.c | $(BUILD)
	@echo "[PPU-CC] $<"
	@$(PPU_CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(addprefix $(BUILD)/,$(OFILES))
	@echo "[PPU-LD] $@"
	@$(PPU_CC) $^ $(LDFLAGS) -o $@
ifneq ($(SPRXLINKER),)
	@$(SPRXLINKER) $@ 2>/dev/null || true
endif

$(TARGET).self: $(TARGET).elf
	@echo "[SELF] $@"
ifneq ($(MAKE_SELF),)
	@# fself -n for NPDRM homebrew EBOOT
	@$(MAKE_SELF) -n $< $@ 2>/dev/null || \
	 $(MAKE_SELF) $< $@ $(CONTENTID) 2>/dev/null || \
	 $(MAKE_SELF) $< $@ 2>/dev/null || cp $< $@
else
	@cp $< $@
	@echo "WARNING: no fself/make_self – EBOOT may not run on console"
endif

# Folder layout for multiMAN copy-install (no PKG needed)
folder-install: $(TARGET).self icons version
	@rm -rf $(APP_TITLEID)
	@mkdir -p $(APP_TITLEID)/USRDIR
	@cp $(TARGET).self $(APP_TITLEID)/USRDIR/EBOOT.BIN
	@cp version.dat $(APP_TITLEID)/USRDIR/version.dat
	@test -f $(DATA)/ICON0.PNG && cp $(DATA)/ICON0.PNG $(APP_TITLEID)/ || true
	@test -f $(DATA)/PIC1.PNG && cp $(DATA)/PIC1.PNG $(APP_TITLEID)/ || true
	@test -f error-cba.wav && cp error-cba.wav $(APP_TITLEID)/USRDIR/ || true
ifneq ($(SFO_BIN),)
	@$(SFO_BIN) --title "$(APP_TITLE)" --appid "$(APP_TITLEID)" -f sfo.xml $(APP_TITLEID)/PARAM.SFO 2>/dev/null || \
	 $(SFO_BIN) -f sfo.xml $(APP_TITLEID)/PARAM.SFO 2>/dev/null || true
else
	@python3 $(SFO_FALLBACK) -f sfo.xml $(APP_TITLEID)/PARAM.SFO 2>/dev/null || true
endif
	@echo "Folder install ready: $(APP_TITLEID)/  → copy to /dev_hdd0/game/"
	@ls -la $(APP_TITLEID)/ $(APP_TITLEID)/USRDIR/

pkg: folder-install
	@echo "=== Creating PKG ==="
	@rm -rf pkg
	@cp -a $(APP_TITLEID) pkg
	@# Rename to expected pkg layout (PARAM at root of pkg dir)
	@rm -rf pkg_build && mkdir -p pkg_build
	@cp -a $(APP_TITLEID)/* pkg_build/
ifneq ($(PKG_BIN),)
	@$(PKG_BIN) --contentid $(CONTENTID) pkg_build/ $(TARGET).pkg
	@ls -la $(TARGET).pkg
	@echo "PKG created with toolchain $(PKG_BIN)"
else
	@echo "ERROR: No pkg.py from ps3dev in PATH."
	@echo "XMB Package Manager will fail with 80029564 on fake PKGs."
	@echo "Use: make folder-install  then copy BCBA00001 to /dev_hdd0/game/"
	@echo "Or install ps3dev pkg.py and re-run make pkg."
	@# Do NOT write a fake PKG that triggers 80029564
	@false
endif

icons:
	@mkdir -p $(DATA)/icons
	@python3 tools/gen_icons.py

clean:
	@rm -rf $(BUILD) $(TARGET).elf $(TARGET).self $(TARGET).pkg pkg pkg_build $(APP_TITLEID)
