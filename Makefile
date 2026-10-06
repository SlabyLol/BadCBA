# BadCBA – standard PSL1GHT-style build
# Use ppu_rules when available for correct SELF/PKG

ifneq ($(strip $(PS3DEV)),)
  export PATH := $(PS3DEV)/bin:$(PS3DEV)/ppu/bin:$(PS3DEV)/spu/bin:$(PATH)
endif
ifneq ($(strip $(PSL1GHT)),)
  export PATH := $(PSL1GHT)/host/bin:$(PATH)
endif

# Prefer official rules if PSL1GHT is installed
PSL1GHT_RULES := $(wildcard $(PSL1GHT)/ppu_rules)

TARGET		:= BadCBA
TITLE		:= BadCBA
APPID		:= BCBA00001
CONTENTID	:= UP0001-$(APPID)_00-0000000000000000

SOURCES		:= source
INCLUDES	:= include
DATA		:= data

BUILDDIR	:= build

CFILES		:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
OFILES		:= $(CFILES:.c=.o)

PPU_CC		:= $(shell which ppu-gcc 2>/dev/null)
SPRX		:= $(shell which sprxlinker 2>/dev/null)
FSELF		:= $(shell which fself 2>/dev/null)
FSELF_NPDRM	:= $(FSELF) -n
MAKE_SELF_NPDRM := $(shell which make_self_npdrm 2>/dev/null)
SFO		:= $(shell which sfo.py 2>/dev/null || which sfo 2>/dev/null)
PKG		:= $(shell which pkg.py 2>/dev/null || which pkg 2>/dev/null)

INCLUDE		:= -I$(CURDIR)/$(INCLUDES)
ifneq ($(strip $(PS3DEV)),)
  INCLUDE	+= -I$(PS3DEV)/ppu/include
endif
ifneq ($(strip $(PSL1GHT)),)
  INCLUDE	+= -I$(PSL1GHT)/ppu/include
endif

LIBPATHS	:=
ifneq ($(strip $(PS3DEV)),)
  LIBPATHS	+= -L$(PS3DEV)/ppu/lib
endif
ifneq ($(strip $(PSL1GHT)),)
  LIBPATHS	+= -L$(PSL1GHT)/ppu/lib
endif

LIBS		:= -lrsx -lgcm_sys -lio -lsysutil -lrt -llv2 -lm -lsysmodule -lnet -lsysfs
CFLAGS		:= -O2 -Wall -mcpu=cell $(INCLUDE)
LDFLAGS		:= $(LIBPATHS) $(LIBS)

VPATH		:= $(SOURCES)

.PHONY: all clean self folder-install pkg icons version

all: version $(TARGET).elf self

version:
	@echo "01.00" > version.dat
	@mkdir -p $(DATA) && cp version.dat $(DATA)/version.dat

$(BUILDDIR):
	@mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	@echo "[CC] $<"
	@$(PPU_CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(addprefix $(BUILDDIR)/,$(OFILES))
	@echo "[LD] $@"
	@$(PPU_CC) $^ $(LDFLAGS) -o $@
ifneq ($(SPRX),)
	@$(SPRX) $@
endif

self: $(TARGET).elf
	@echo "[SELF] $(TARGET).self"
ifneq ($(FSELF),)
	@$(FSELF) -n $(TARGET).elf $(TARGET).self
else ifneq ($(MAKE_SELF_NPDRM),)
	@$(MAKE_SELF_NPDRM) $(TARGET).elf $(TARGET).self $(CONTENTID)
else
	@echo "FATAL: no fself / make_self_npdrm – cannot build EBOOT"
	@false
endif
	@python3 -c "d=open('$(TARGET).self','rb').read(4); assert d!=b'\\x7fELF'; print('SELF OK', d[:4])"

folder-install: self icons version
	@rm -rf $(APPID)
	@mkdir -p $(APPID)/USRDIR
	@cp $(TARGET).self $(APPID)/USRDIR/EBOOT.BIN
	@cp version.dat $(APPID)/USRDIR/
	@test -f $(DATA)/ICON0.PNG && cp $(DATA)/ICON0.PNG $(APPID)/ || true
	@test -f $(DATA)/PIC1.PNG && cp $(DATA)/PIC1.PNG $(APPID)/ || true
	@test -f error-cba.wav && cp error-cba.wav $(APPID)/USRDIR/ || true
ifneq ($(SFO),)
	@$(SFO) --title "$(TITLE)" --appid "$(APPID)" -f sfo.xml $(APPID)/PARAM.SFO 2>/dev/null || \
	 $(SFO) -f sfo.xml $(APPID)/PARAM.SFO 2>/dev/null || true
else
	@python3 tools/sfo.py -f sfo.xml $(APPID)/PARAM.SFO 2>/dev/null || true
endif
	@echo "OK: $(APPID)/ → copy to /dev_hdd0/game/$(APPID)/"
	@ls -la $(APPID)/USRDIR/EBOOT.BIN

pkg: folder-install
ifneq ($(PKG),)
	@rm -rf pkg_build && mkdir pkg_build && cp -a $(APPID)/* pkg_build/
	@$(PKG) --contentid $(CONTENTID) pkg_build/ $(TARGET).pkg
else
	@echo "No pkg.py – use folder install only"
	@false
endif

icons:
	@mkdir -p $(DATA)/icons
	@python3 tools/gen_icons.py

clean:
	@rm -rf $(BUILDDIR) $(TARGET).elf $(TARGET).self $(TARGET).pkg $(APPID) pkg_build
