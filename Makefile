#==============================================================================
# BadCBA - PS3 Custom Boot Audio Maker
# Real PSL1GHT Makefile
#==============================================================================

APP_TITLE       := BadCBA
APP_TITLEID     := BCBA00001
APP_VERSION     := 01.00
CONTENTID       := UP0001-BCBA00001_00-0000000000000000

TARGET          := $(APP_TITLE)
BUILD           := build
SOURCES         := source
INCLUDES        := include
DATA            := data

CFILES          := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES        := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))

export INCLUDE  := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                   -I$(PSL1GHT)/ppu/include \
                   -I$(PORTLIBS)/include

export LIBPATHS := -L$(PSL1GHT)/ppu/lib -L$(PORTLIBS)/lib

export LIBS     := -lrsx -lgcm_sys -lio -lsysutil -lrt -llv2 -lm -lsysmodule -lnet -lsysfs

export VPATH    := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

CFLAGS          := -O2 -Wall -mcpu=cell $(INCLUDE)
CXXFLAGS        := $(CFLAGS)
LDFLAGS         := $(LIBPATHS) $(LIBS)

OFILES          := $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)

.PHONY: all clean pkg run

all: $(TARGET).elf

$(BUILD):
	@[ -d $@ ] || mkdir -p $@

%.o: %.c
	@echo "[CC]  $<"
	@$(CC) $(CFLAGS) -c $< -o $(BUILD)/$@

%.o: %.cpp
	@echo "[CXX] $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $(BUILD)/$@

$(TARGET).elf: $(BUILD) $(OFILES)
	@echo "[LD]  $@"
	@$(CC) $(foreach f,$(OFILES),$(BUILD)/$(f)) $(LDFLAGS) -o $@
	@$(PSL1GHT)/host/bin/sprxlinker $@ 2>/dev/null || true
	@echo "Built: $@"

# Create SELF
$(TARGET).self: $(TARGET).elf
	@echo "[SELF] $@"
	@$(PSL1GHT)/host/bin/make_self_npdrm $< $@ $(CONTENTID) 2>/dev/null || \
	 $(PSL1GHT)/host/bin/make_fself $< $@ 2>/dev/null || \
	 cp $< $@

# Create full PKG
pkg: $(TARGET).self
	@echo "=== Creating PKG ==="
	@rm -rf pkg
	@mkdir -p pkg/USRDIR
	@cp $(TARGET).self pkg/USRDIR/EBOOT.BIN
	@cp $(DATA)/ICON0.PNG pkg/ 2>/dev/null || echo "Warning: ICON0.PNG missing"
	@cp $(DATA)/PIC1.PNG  pkg/ 2>/dev/null || true
	@echo "Generating PARAM.SFO..."
	@$(PSL1GHT)/host/bin/sfo.py --title "$(APP_TITLE)" \
		--appid "$(APP_TITLEID)" --appver "$(APP_VERSION)" \
		--category "HG" -f sfo.xml pkg/PARAM.SFO 2>/dev/null || \
	 python3 $(PSL1GHT)/tools/ps3py/sfo.py -f sfo.xml pkg/PARAM.SFO
	@echo "Building $(TARGET).pkg ..."
	@$(PSL1GHT)/host/bin/pkg.py --contentid $(CONTENTID) pkg/ $(TARGET).pkg 2>/dev/null || \
	 python3 $(PSL1GHT)/tools/ps3py/pkg.py --contentid $(CONTENTID) pkg/ $(TARGET).pkg
	@echo ""
	@echo "========================================"
	@echo "  PKG ready: $(TARGET).pkg"
	@echo "========================================"

clean:
	@rm -rf $(BUILD) $(TARGET).elf $(TARGET).self $(TARGET).pkg pkg
	@echo "Clean done."

run: $(TARGET).self
	ps3load $(TARGET).self
