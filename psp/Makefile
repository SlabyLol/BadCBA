# BadCBA PSP – BadCustomBootAnimation
TARGET = BadCBA
OBJS = main.o gui.o filebrowser.o

PSP_EBOOT_TITLE = BadCBA
PSP_FW_VERSION = 371
BUILD_PRX = 1

INCDIR =
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LIBS = -lpspgu -lpspgum -lpspge -lpspaudio -lpspaudiolib -lpspctrl -lpsppower -lpsprtc -lpspvfpu -lm
LDFLAGS =

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_ICON = ICON0.PNG

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
