/*
 * BadCBA – minimal safe entry then full app
 * Avoids early crash (80010009) from failed video/sys init
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
    (void)param;
    (void)userdata;
    if (status == SYSUTIL_EXIT_GAME)
        running = 0;
}

int main(int argc, const char *argv[])
{
    (void)argc;
    (void)argv;

    /* Load modules – ignore failures so HEN can still boot */
    sysModuleLoad(SYSMODULE_FS);
    sysModuleLoad(SYSMODULE_IO);

    sysUtilRegisterCallback(0, sysutil_callback, NULL);

    if (ioPadInit(7) != 0) {
        /* still try to continue */
    }

    u32 width = 640, height = 480;
    int video_ok = (rsxutil_init(&width, &height) == 0);

    settings_load();
    error_init();

    if (video_ok)
        gui_init((int)width, (int)height);

    while (running) {
        padData pad;
        memset(&pad, 0, sizeof(pad));
        ioPadGetData(0, &pad);

        /* Hold START + SELECT to exit if GUI broken */
        if (pad.BTN_START && pad.BTN_SELECT)
            running = 0;

        if (video_ok) {
            if (error_is_active()) {
                if (pad.BTN_CROSS || pad.BTN_CIRCLE)
                    error_clear();
            } else {
                gui_update(&pad);
            }
            error_update();
            rsxutil_clear(0xFF0A0A12);
            gui_render(rsxutil_get_context(), rsxutil_get_current());
            error_render(rsxutil_get_current(), (int)width, (int)height);
            rsxutil_flip();
        } else {
            /* No video – just wait for exit */
            usleep(50000);
        }

        sysUtilCheckCallback();
    }

    error_shutdown();
    if (video_ok)
        gui_shutdown();
    settings_save();
    if (video_ok)
        rsxutil_finish();
    ioPadEnd();

    return 0;
}
