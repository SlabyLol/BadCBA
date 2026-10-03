/*
 * BadCBA - RSX helpers matching current PSL1GHT API
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#include <sysutil/video.h>
#include <rsx/gcm_sys.h>
#include <rsx/rsx.h>

#include "rsxutil.h"

#define HOST_SIZE   (32 * 1024 * 1024)
#define CB_SIZE     (1 * 1024 * 1024)

static gcmContextData *context = NULL;
static void *host_addr = NULL;
static rsxBuffer buffers[2];
static int currentBuffer = 0;
static u32 display_width = 0;
static u32 display_height = 0;

static void wait_flip(void)
{
    while (gcmGetFlipStatus() != 0)
        usleep(200);
    gcmResetFlipStatus();
}

static int allocate_buffer(rsxBuffer *buf, u32 width, u32 height)
{
    buf->width  = width;
    buf->height = height;
    buf->pitch  = width * 4;

    /* Allocate in RSX memory */
    buf->ptr = (u32 *)rsxMemalign(64, buf->pitch * height);
    if (!buf->ptr)
        return -1;

    if (rsxAddressToOffset(buf->ptr, &buf->offset) != 0)
        return -1;

    return 0;
}

static void set_render_target(int idx)
{
    gcmSurface sf;
    memset(&sf, 0, sizeof(sf));

    sf.colorFormat    = GCM_SURFACE_X8R8G8B8;
    sf.colorTarget    = GCM_SURFACE_TARGET_0;
    sf.colorLocation[0] = GCM_LOCATION_RSX;
    sf.colorOffset[0] = buffers[idx].offset;
    sf.colorPitch[0]  = buffers[idx].pitch;

    sf.depthFormat    = GCM_SURFACE_ZETA_Z16;
    sf.depthLocation  = GCM_LOCATION_RSX;
    sf.depthOffset    = 0;
    sf.depthPitch     = buffers[idx].pitch / 2;

    sf.type           = GCM_SURFACE_TYPE_LINEAR;
    sf.antiAlias      = GCM_SURFACE_CENTER_1;

    sf.width          = buffers[idx].width;
    sf.height         = buffers[idx].height;
    sf.x              = 0;
    sf.y              = 0;

    rsxSetSurface(context, &sf);
}

int rsxutil_init(u32 *out_width, u32 *out_height)
{
    videoState state;
    videoConfiguration vconfig;
    videoResolution res;

    if (videoGetState(0, 0, &state) != 0)
        return -1;

    videoGetResolution(state.displayMode.resolution, &res);
    display_width  = res.width;
    display_height = res.height;

    memset(&vconfig, 0, sizeof(vconfig));
    vconfig.resolution = state.displayMode.resolution;
    vconfig.format     = VIDEO_BUFFER_FORMAT_XRGB;
    vconfig.pitch      = res.width * 4;
    vconfig.aspect     = state.displayMode.aspect;

    wait_flip();
    if (videoConfigure(0, &vconfig, NULL, 0) != 0)
        return -1;

    videoGetState(0, 0, &state);

    host_addr = memalign(1024 * 1024, HOST_SIZE);
    if (!host_addr)
        return -1;

    /* PSL1GHT signature: rsxInit(gcmContextData **ctx, cmdSize, ioSize, ioAddress) */
    if (rsxInit(&context, CB_SIZE, HOST_SIZE, host_addr) != 0)
        return -1;

    gcmSetFlipMode(GCM_FLIP_VSYNC);

    for (int i = 0; i < 2; i++) {
        if (allocate_buffer(&buffers[i], display_width, display_height) != 0)
            return -1;
        gcmSetDisplayBuffer(i, buffers[i].offset, buffers[i].pitch,
                            buffers[i].width, buffers[i].height);
    }

    gcmResetFlipStatus();
    currentBuffer = 0;
    set_render_target(currentBuffer);

    if (out_width)  *out_width  = display_width;
    if (out_height) *out_height = display_height;

    return 0;
}

void rsxutil_flip(void)
{
    rsxFlushBuffer(context);
    gcmSetFlip(context, currentBuffer);
    rsxSetWaitFlip(context);
    wait_flip();

    currentBuffer ^= 1;
    set_render_target(currentBuffer);
}

rsxBuffer *rsxutil_get_current(void)
{
    return &buffers[currentBuffer];
}

gcmContextData *rsxutil_get_context(void)
{
    return context;
}

void rsxutil_clear(u32 color)
{
    rsxBuffer *buf = rsxutil_get_current();
    if (!buf || !buf->ptr) return;

    u32 count = buf->width * buf->height;
    for (u32 i = 0; i < count; i++)
        buf->ptr[i] = color;
}

void rsxutil_finish(void)
{
    /* Buffers stay until process exit */
}
