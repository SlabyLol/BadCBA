/*
 * BadCBA - BadCustomBootAnimation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ppu-types.h>
#include <sys/process.h>
#include <sysutil/sysutil.h>
#include <io/pad.h>
#include <sysmodule/sysmodule.h>

#include "rsxutil.h"
#include "gui.h"
#include "filebrowser.h"
#include "settings.h"
#include "audio.h"
#include "pkg.h"
#include "error.h"

static int running = 1;

static void sysutil_callback(u64 status, u64 param, void *userdata)
{
    (void)param; (void)userdata;
    if (status == SYSUTIL_EXIT_GAME)
        running = 0;
}

int main(int argc, const char *argv[])
{
    (void)argc; (void)argv;

    sysModuleLoad(SYSMODULE_FS);
    sysModuleLoad(SYSMODULE_IO);

    ioPadInit(7);
    sysUtilRegisterCallback(0, sysutil_callback, NULL);

    u32 width = 0, height = 0;
    if (rsxutil_init(&width, &height) != 0) {
        printf("Video init failed\n");
        return -1;
    }

    settings_load();
    error_init();
    gui_init((int)width, (int)height);

    while (running) {
        padData pad;
        ioPadGetData(0, &pad);

        /* Dismiss error with X or O */
        if (error_is_active()) {
            if (pad.BTN_CROSS || pad.BTN_CIRCLE)
                error_clear();
            error_update();
        } else {
            gui_update(&pad);
        }

        rsxutil_clear(0xFF0A0A12);
        gui_render(rsxutil_get_context(), rsxutil_get_current());
        error_render(rsxutil_get_current(), (int)width, (int)height);

        rsxutil_flip();
        sysUtilCheckCallback();
    }

    error_shutdown();
    gui_shutdown();
    settings_save();
    rsxutil_finish();
    ioPadEnd();

    return 0;
}
