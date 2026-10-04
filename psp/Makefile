# BadCBA PSP – full feature build
TARGET = BadCBA
OBJS = main.o gui.o filebrowser.o settings.o flash.o error.o audio.o

PSP_EBOOT_TITLE = BadCBA
PSP_FW_VERSION = 371
BUILD_PRX = 1

CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS = -lpspgu -lpspgum -lpspge -lpspaudio -lpspaudiolib -lpspctrl -lpsppower -lpsprtc -lpspvfpu -lm

EXTRA_TARGETS = EBOOT.PBP

ifeq ($(wildcard ICON0.PNG),)
  PSP_EBOOT_ICON = NULL
else
  PSP_EBOOT_ICON = ICON0.PNG
endif

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
