#==============================================================================
# BadCBA - PS3 Custom Boot Maker
# PSL1GHT / ps3dev cross-compile Makefile
#==============================================================================

ifneq ($(strip $(PS3DEV)),)
  export PATH := $(PS3DEV)/bin:$(PS3DEV)/ppu/bin:$(PS3DEV)/spu/bin:$(PATH)
endif

ifneq ($(strip $(PSL1GHT)),)
  export PATH := $(PSL1GHT)/host/bin:$(PATH)
endif

PPU_CC      := $(shell which ppu-gcc 2>/dev/null || echo "")
SPRXLINKER  := $(shell which sprxlinker 2>/dev/null || echo "")
MAKE_SELF   := $(shell which make_self_npdrm 2>/dev/null || which make_fself 2>/dev/null || echo "")
SFO_PY      := $(shell which sfo.py 2>/dev/null || echo "")
PKG_PY      := $(shell which pkg.py 2>/dev/null || echo "")

ifeq ($(PPU_CC),)
  $(error No PS3 cross-compiler found. Install ps3toolchain + PSL1GHT and set PS3DEV / PSL1GHT)
endif

APP_TITLE   := BadCBA
APP_TITLEID := BCBA00001
APP_VERSION := 01.00
CONTENTID   := UP0001-BCBA00001_00-0000000000000000

TARGET      := $(APP_TITLE)
BUILD       := build
SOURCES     := source
INCLUDES    := include
DATA        := data

CFILES      := $(notdir $(wildcard $(SOURCES)/*.c))
OFILES      := $(CFILES:.c=.o)

INCLUDE     := -I$(CURDIR)/$(INCLUDES)
ifneq ($(strip $(PSL1GHT)),)
  INCLUDE   += -I$(PSL1GHT)/ppu/include -I$(PSL1GHT)/include
endif
ifneq ($(strip $(PS3DEV)),)
  INCLUDE   += -I$(PS3DEV)/ppu/include
endif

LIBPATHS    :=
ifneq ($(strip $(PSL1GHT)),)
  LIBPATHS  += -L$(PSL1GHT)/ppu/lib
endif
ifneq ($(strip $(PS3DEV)),)
  LIBPATHS  += -L$(PS3DEV)/ppu/lib
endif

LIBS        := -lrsx -lgcm_sys -lio -lsysutil -lrt -llv2 -lm -lsysmodule -lnet -lsysfs

CFLAGS      := -O2 -Wall $(INCLUDE)
LDFLAGS     := $(LIBPATHS) $(LIBS)

VPATH       := $(SOURCES)

.PHONY: all clean pkg check-toolchain icons version

all: check-toolchain version $(TARGET).elf

check-toolchain:
	@echo "Using compiler: $(PPU_CC)"
	@$(PPU_CC) --version | head -1

version:
	@echo "$(APP_VERSION)" > version.dat
	@mkdir -p $(DATA)
	@cp version.dat $(DATA)/version.dat
	@echo "version.dat = $(APP_VERSION)"

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
	@echo "Built $@"

$(TARGET).self: $(TARGET).elf
	@echo "[SELF] $@"
ifneq ($(MAKE_SELF),)
	@$(MAKE_SELF) $< $@ $(CONTENTID) 2>/dev/null || $(MAKE_SELF) $< $@ 2>/dev/null || cp $< $@
else
	@cp $< $@
endif

pkg: $(TARGET).self icons version
	@echo "=== Creating PKG ==="
	@rm -rf pkg
	@mkdir -p pkg/USRDIR
	@cp $(TARGET).self pkg/USRDIR/EBOOT.BIN
	@cp version.dat pkg/USRDIR/version.dat
	@cp $(DATA)/ICON0.PNG pkg/ 2>/dev/null || true
	@cp $(DATA)/PIC1.PNG  pkg/ 2>/dev/null || true
ifneq ($(SFO_PY),)
	@$(SFO_PY) --title "$(APP_TITLE)" --appid "$(APP_TITLEID)" \
		--appver "$(APP_VERSION)" --category "HG" -f sfo.xml pkg/PARAM.SFO 2>/dev/null || true
endif
ifneq ($(PKG_PY),)
	@$(PKG_PY) --contentid $(CONTENTID) pkg/ $(TARGET).pkg 2>/dev/null || true
endif
	@ls -la $(TARGET).pkg 2>/dev/null || echo "PKG tools missing – ELF/SELF + version.dat ready"
	@echo "Done. version=$(APP_VERSION)"

icons:
	@mkdir -p $(DATA)
	@python3 tools/gen_icons.py

clean:
	@rm -rf $(BUILD) $(TARGET).elf $(TARGET).self $(TARGET).pkg pkg
	@echo "Clean done."
