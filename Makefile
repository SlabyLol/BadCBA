# BadCBA - PS3 Custom Boot Audio Maker
# PSL1GHT Makefile

APP_TITLE       := BadCBA
APP_TITLE_ID    := BCBA00001
APP_VERSION     := 01.00
CONTENT_ID      := UP0001-BCBA00001_00-0000000000000000

TARGET          := BadCBA
BUILD_DIR       := build
SOURCE_DIR      := source
INCLUDE_DIR     := include
DATA_DIR        := data

SOURCES         := $(wildcard $(SOURCE_DIR)/*.c)
OBJECTS         := $(SOURCES:$(SOURCE_DIR)/%.c=$(BUILD_DIR)/%.o)

include $(PSL1GHT)/ppu_rules

CFLAGS          += -I$(INCLUDE_DIR) -I$(PSL1GHT)/ppu/include -O2 -Wall
LDFLAGS         += -L$(PSL1GHT)/ppu/lib
LIBS            += -lrsx -lgcm_sys -lio -lsysutil -lrt -llv2 -lm -lsysmodule

# Optional: add more libraries when available
# LIBS          += -lpng -lz -ltiny3d -lfreetype

.PHONY: all clean pkg run

all: $(TARGET).elf

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $@
	$(SPRX) $@

pkg: $(TARGET).elf
	@echo "Creating package directory..."
	mkdir -p pkg/USRDIR
	cp $(TARGET).self pkg/USRDIR/EBOOT.BIN 2>/dev/null || cp $(TARGET).elf pkg/USRDIR/EBOOT.BIN
	cp $(DATA_DIR)/ICON0.PNG pkg/ 2>/dev/null || true
	cp $(DATA_DIR)/PIC1.PNG pkg/ 2>/dev/null || true
	@echo "Generating PARAM.SFO..."
	$(PSL1GHT)/host/bin/sfo.py --title "$(APP_TITLE)" --appid "$(APP_TITLE_ID)" \
		--appver "$(APP_VERSION)" --category "HG" -f sfo.xml pkg/PARAM.SFO 2>/dev/null || \
		python3 $(PSL1GHT)/tools/ps3py/sfo.py -f sfo.xml pkg/PARAM.SFO
	@echo "Building PKG..."
	$(PSL1GHT)/host/bin/pkg.py --contentid $(CONTENT_ID) pkg/ $(TARGET).pkg 2>/dev/null || \
		python3 $(PSL1GHT)/tools/ps3py/pkg.py --contentid $(CONTENT_ID) pkg/ $(TARGET).pkg
	@echo "Done: $(TARGET).pkg"

clean:
	rm -rf $(BUILD_DIR) $(TARGET).elf $(TARGET).self $(TARGET).pkg pkg

run: $(TARGET).elf
	ps3load $(TARGET).self
