# BadCBA Makefile
ifneq ($(strip $(PS3DEV)),)
  export PATH := $(PS3DEV)/bin:$(PS3DEV)/ppu/bin:$(PS3DEV)/spu/bin:$(PATH)
endif
ifneq ($(strip $(PSL1GHT)),)
  export PATH := $(PSL1GHT)/host/bin:$(PATH)
endif

PPU_CC     := $(shell which ppu-gcc 2>/dev/null)
SPRXLINKER := $(shell which sprxlinker 2>/dev/null)
MAKE_SELF  := $(shell which make_self_npdrm 2>/dev/null || which make_fself 2>/dev/null)
SFO_PY     := $(shell which sfo.py 2>/dev/null || echo "$(CURDIR)/tools/sfo.py")
PKG_PY     := $(shell which pkg.py 2>/dev/null || echo "$(CURDIR)/tools/pkg.py")

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

.PHONY: all clean pkg icons version

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
ifneq ($(MAKE_SELF),)
	@$(MAKE_SELF) $< $@ $(CONTENTID) 2>/dev/null || $(MAKE_SELF) $< $@ 2>/dev/null || cp $< $@
else
	@cp $< $@
endif

pkg: $(TARGET).self icons version
	@rm -rf pkg && mkdir -p pkg/USRDIR
	@cp $(TARGET).self pkg/USRDIR/EBOOT.BIN
	@cp version.dat pkg/USRDIR/version.dat
	@test -f $(DATA)/ICON0.PNG && cp $(DATA)/ICON0.PNG pkg/ || true
	@test -f $(DATA)/PIC1.PNG && cp $(DATA)/PIC1.PNG pkg/ || true
	@# Error sound lives in repo root
	@if [ -f error-cba.wav ]; then cp error-cba.wav pkg/USRDIR/error-cba.wav; \
	 elif [ -f $(DATA)/error-cba.wav ]; then cp $(DATA)/error-cba.wav pkg/USRDIR/; \
	 else echo "Warning: error-cba.wav not found"; fi
	@python3 $(SFO_PY) -f sfo.xml pkg/PARAM.SFO 2>/dev/null || true
	@python3 $(PKG_PY) --contentid $(CONTENTID) pkg/ $(TARGET).pkg 2>/dev/null || true
	@ls -la $(TARGET).pkg pkg/USRDIR/ 2>/dev/null || echo "ELF/SELF ready"

icons:
	@mkdir -p $(DATA) && python3 tools/gen_icons.py

clean:
	@rm -rf $(BUILD) $(TARGET).elf $(TARGET).self $(TARGET).pkg pkg
