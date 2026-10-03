/*
 * BadCBA - PS3 Custom Boot Audio Maker
 * Full application with real RSX on-screen GUI
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#include <ppu-types.h>
#include <sys/process.h>
#include <sysutil/sysutil.h>
#include <sysutil/video.h>
#include <io/pad.h>
#include <sysmodule/sysmodule.h>
#include <rsx/rsx.h>
#include <rsx/gcm_sys.h>

#include "gui.h"
#include "filebrowser.h"
#include "settings.h"
#include "audio.h"
#include "pkg.h"

#define HOST_SIZE   (32*1024*1024)
#define CB_SIZE     (1*1024*1024)

static gcmContextData *context = NULL;
static void *host_addr = NULL;
static rsxBuffer buffers[2];
static int currentBuffer = 0;
static int running = 1;

static void sysutil_callback(u64 status, u64 param, void *userdata)
{
    (void)param; (void)userdata;
    switch (status) {
    case SYSUTIL_EXIT_GAME:
        running = 0;
        break;
    default:
        break;
    }
}

static void set_render_target(int idx)
{
    gcmSurface sf;
    sf.colorFormat    = GCM_TF_COLOR_X8R8G8B8;
    sf.colorTarget    = GCM_TF_TARGET_0;
    sf.colorLocation  = GCM_LOCATION_RSX;
    sf.colorOffset[0] = buffers[idx].offset;
    sf.colorPitch[0]  = buffers[idx].width * 4;
    sf.depthFormat    = GCM_TF_ZETA_Z16;
    sf.depthLocation  = GCM_LOCATION_RSX;
    sf.depthOffset    = 0;
    sf.depthPitch     = buffers[idx].width * 2;
    sf.type           = GCM_TF_TYPE_LINEAR;
    sf.antiAlias      = GCM_TF_CENTER_1;
    sf.width          = buffers[idx].width;
    sf.height         = buffers[idx].height;
    sf.x              = 0;
    sf.y              = 0;
    rsxSetSurface(context, &sf);
}

static void wait_flip(void)
{
    while (gcmGetFlipStatus() != 0)
        usleep(200);
    gcmResetFlipStatus();
}

static void flip(void)
{
    gcmSetWaitFlip(context);
    gcmSetFlip(context, currentBuffer);
    rsxFlushBuffer(context);
    gcmSetWaitFlip(context);
    currentBuffer ^= 1;
    set_render_target(currentBuffer);
}

static int init_video(void)
{
    videoState state;
    videoConfiguration vconfig;
    videoResolution res;

    videoGetState(0, 0, &state);
    videoGetResolution(state.displayMode.resolution, &res);

    memset(&vconfig, 0, sizeof(vconfig));
    vconfig.resolution = state.displayMode.resolution;
    vconfig.format     = VIDEO_BUFFER_FORMAT_XRGB;
    vconfig.pitch      = res.width * 4;
    vconfig.aspect     = state.displayMode.aspect;

    wait_flip();
    videoConfigure(0, &vconfig, NULL, 0);
    videoGetState(0, 0, &state);

    host_addr = memalign(1024*1024, HOST_SIZE);
    context = rsxInit(CB_SIZE, HOST_SIZE, host_addr);
    if (!context) return -1;

    gcmSetFlipMode(GCM_FLIP_VSYNC);

    for (int i = 0; i < 2; i++) {
        buffers[i].width  = res.width;
        buffers[i].height = res.height;
        rsxBufferAllocate(&buffers[i]);
        gcmSetDisplayBuffer(i, buffers[i].offset, buffers[i].width*4,
                            buffers[i].width, buffers[i].height);
    }

    gcmResetFlipStatus();
    flip();
    return 0;
}

int main(int argc, const char* argv[])
{
    (void)argc; (void)argv;

    sysModuleLoad(SYSMODULE_FS);
    sysModuleLoad(SYSMODULE_IO);
    sysModuleLoad(SYSMODULE_SYSUTIL);

    ioPadInit(7);
    sysUtilRegisterCallback(0, sysutil_callback, NULL);

    if (init_video() != 0) {
        printf("Video init failed\n");
        return -1;
    }

    settings_load();
    gui_init(buffers[0].width, buffers[0].height);

    while (running) {
        padData pad;
        ioPadGetData(0, &pad);

        gui_update(&pad);

        /* Clear screen */
        rsxSetClearColor(context, 0xFF0A0A12);
        rsxSetClearDepth(context, 0xFFFF);
        rsxClear(context, GCM_CLEAR_R | GCM_CLEAR_G | GCM_CLEAR_B | GCM_CLEAR_A | GCM_CLEAR_Z);

        gui_render(context, &buffers[currentBuffer]);

        flip();
        sysUtilCheckCallback();
    }

    gui_shutdown();
    settings_save();
    ioPadEnd();

    return 0;
}
