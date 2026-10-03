#==============================================================================
# BadCBA - PS3 Custom Boot Audio Maker
# Proper PSL1GHT / ps3dev cross-compile Makefile
#==============================================================================

# Prefer environment from ps3dev
ifneq ($(strip $(PS3DEV)),)
  export PATH := $(PS3DEV)/bin:$(PS3DEV)/ppu/bin:$(PS3DEV)/spu/bin:$(PATH)
endif

ifneq ($(strip $(PSL1GHT)),)
  export PATH := $(PSL1GHT)/host/bin:$(PATH)
endif

# Detect the real PowerPC cross compiler
PPU_CC      := $(shell which ppu-gcc 2>/dev/null || which powerpc64-ps3-elf-gcc 2>/dev/null || echo "")
PPU_CXX     := $(shell which ppu-g++ 2>/dev/null || which powerpc64-ps3-elf-g++ 2>/dev/null || echo "")
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

CFLAGS      := -O2 -Wall -m64 -mabi=elfv1 -mcpu=cell -mtune=cell $(INCLUDE)
LDFLAGS     := $(LIBPATHS) $(LIBS)

VPATH       := $(SOURCES)

.PHONY: all clean pkg check-toolchain icons

all: check-toolchain $(TARGET).elf

check-toolchain:
	@echo "Using compiler: $(PPU_CC)"
	@$(PPU_CC) --version | head -1

$(BUILD):
	@mkdir -p $(BUILD)

$(BUILD)/%.o: $(SOURCES)/%.c | $(BUILD)
	@echo "[PPU-CC] $<"
	@$(PPU_CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(addprefix $(BUILD)/,$(OFILES))
	@echo "[PPU-LD] $@"
	@$(PPU_CC) $^ $(LDFLAGS) -o $@
ifneq ($(SPRXLINKER),)
	@$(SPRXLINKER) $@
endif
	@echo "Built $@"

$(TARGET).self: $(TARGET).elf
	@echo "[SELF] $@"
ifneq ($(MAKE_SELF),)
	@$(MAKE_SELF) $< $@ $(CONTENTID) 2>/dev/null || $(MAKE_SELF) $< $@
else
	@cp $< $@
	@echo "Warning: make_self not found, copied ELF as SELF"
endif

pkg: $(TARGET).self icons
	@echo "=== Creating PKG ==="
	@rm -rf pkg
	@mkdir -p pkg/USRDIR
	@cp $(TARGET).self pkg/USRDIR/EBOOT.BIN
	@cp $(DATA)/ICON0.PNG pkg/
	@cp $(DATA)/PIC1.PNG  pkg/ 2>/dev/null || true
	@echo "Generating PARAM.SFO..."
ifneq ($(SFO_PY),)
	@$(SFO_PY) --title "$(APP_TITLE)" --appid "$(APP_TITLEID)" \
		--appver "$(APP_VERSION)" --category "HG" -f sfo.xml pkg/PARAM.SFO
else
	@python3 tools/sfo.py -f sfo.xml pkg/PARAM.SFO 2>/dev/null || \
	 python3 -c "print('sfo.py missing - copy a prebuilt PARAM.SFO')"
endif
	@echo "Building $(TARGET).pkg ..."
ifneq ($(PKG_PY),)
	@$(PKG_PY) --contentid $(CONTENTID) pkg/ $(TARGET).pkg
else
	@python3 tools/pkg.py --contentid $(CONTENTID) pkg/ $(TARGET).pkg 2>/dev/null || \
	 echo "pkg.py missing - install PSL1GHT tools"
endif
	@echo ""
	@echo "========================================"
	@echo "  PKG ready: $(TARGET).pkg"
	@echo "========================================"

icons:
	@mkdir -p $(DATA)
	@python3 tools/gen_icons.py

clean:
	@rm -rf $(BUILD) $(TARGET).elf $(TARGET).self $(TARGET).pkg pkg
	@echo "Clean done."
