/*
 * BadCBA – BadCustomBootAnimation for PSP
 * Homebrew GUI: pick media from ms0:, prepare custom boot files
 */

#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <psppower.h>
#include <stdio.h>
#include <string.h>

#include "gui.h"
#include "filebrowser.h"

PSP_MODULE_INFO("BadCBA", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(2048);

static int running = 1;

int exit_callback(int arg1, int arg2, void *common)
{
    (void)arg1; (void)arg2; (void)common;
    running = 0;
    return 0;
}

int callback_thread(SceSize args, void *argp)
{
    (void)args; (void)argp;
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

int setup_callbacks(void)
{
    int thid = sceKernelCreateThread("update_thread", callback_thread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0)
        sceKernelStartThread(thid, 0, 0);
    return thid;
}

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    setup_callbacks();
    scePowerSetClockFrequency(333, 333, 166);

    gui_init();
    filebrowser_init("ms0:/");

    SceCtrlData pad, old;
    memset(&old, 0, sizeof(old));
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    while (running) {
        sceCtrlReadBufferPositive(&pad, 1);

        unsigned int pressed = pad.Buttons & ~old.Buttons;
        gui_update(pressed);
        old = pad;

        gui_render();
        sceDisplayWaitVblankStart();
    }

    gui_shutdown();
    sceKernelExitGame();
    return 0;
}
