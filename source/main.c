/*
 * BadCBA - PS3 Custom Boot Audio Maker
 * Main entry point
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/process.h>
#include <sysutil/sysutil.h>
#include <io/pad.h>
#include <sysmodule/sysmodule.h>

#include "gui.h"
#include "filebrowser.h"
#include "settings.h"
#include "audio.h"
#include "pkg.h"

static int running = 1;

static void sysutil_callback(uint64_t status, uint64_t param, void *userdata)
{
    (void)param;
    (void)userdata;

    switch (status) {
    case SYSUTIL_EXIT_GAME:
        running = 0;
        break;
    case SYSUTIL_MENU_OPEN:
    case SYSUTIL_MENU_CLOSE:
        break;
    default:
        break;
    }
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    sysModuleLoad(SYSMODULE_FS);
    sysModuleLoad(SYSMODULE_IO);

    ioPadInit(7);
    sysUtilRegisterCallback(0, sysutil_callback, NULL);

    settings_load();
    gui_init();

    while (running) {
        padData paddata;
        ioPadGetData(0, &paddata);

        gui_update(&paddata);
        gui_render();

        sysUtilCheckCallback();
        usleep(16000); /* ~60 fps */
    }

    gui_shutdown();
    settings_save();
    ioPadEnd();

    return 0;
}
