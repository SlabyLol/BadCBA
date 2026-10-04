/*
 * BadCBA PSP GUI
 */

#include <pspkernel.h>
#include <pspgu.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <pspdebug.h>
#include <string.h>
#include <stdio.h>

#include "gui.h"
#include "filebrowser.h"

#define BUF_WIDTH  512
#define SCR_WIDTH  480
#define SCR_HEIGHT 272

static unsigned int __attribute__((aligned(16))) list[262144];

enum {
    MENU_MAIN = 0,
    MENU_BROWSER,
    MENU_SETTINGS,
    MENU_ABOUT,
    MENU_NO_MS
};

static int menu = MENU_MAIN;
static int selected = 0;
static int duration_s = 5;
static int volume = 80;
static int debug_inited = 0;

static const char *main_items[] = {
    "Select media (ms0:)",
    "Settings",
    "Convert / prepare boot",
    "About",
    "Exit"
};
static const int main_count = 5;

void gui_init(void)
{
    pspDebugScreenInit();
    debug_inited = 1;

    sceGuInit();
    sceGuStart(GU_DIRECT, list);
    sceGuDrawBuffer(GU_PSM_8888, (void *)0, BUF_WIDTH);
    sceGuDispBuffer(SCR_WIDTH, SCR_HEIGHT, (void *)0x88000, BUF_WIDTH);
    sceGuDepthBuffer((void *)0x110000, BUF_WIDTH);
    sceGuOffset(2048 - (SCR_WIDTH / 2), 2048 - (SCR_HEIGHT / 2));
    sceGuViewport(2048, 2048, SCR_WIDTH, SCR_HEIGHT);
    sceGuDepthRange(65535, 0);
    sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
    sceGuEnable(GU_SCISSOR_TEST);
    sceGuFinish();
    sceGuSync(0, 0);
    sceDisplayWaitVblankStart();
    sceGuDisplay(GU_TRUE);

    menu = MENU_MAIN;
    selected = 0;
}

void gui_shutdown(void)
{
    sceGuTerm();
}

void gui_update(unsigned int buttons)
{
    if (buttons & PSP_CTRL_DOWN) {
        if (menu == MENU_MAIN)
            selected = (selected + 1) % main_count;
        else if (menu == MENU_BROWSER)
            filebrowser_down();
        else if (menu == MENU_SETTINGS)
            selected = (selected + 1) % 2;
    }
    if (buttons & PSP_CTRL_UP) {
        if (menu == MENU_MAIN)
            selected = (selected - 1 + main_count) % main_count;
        else if (menu == MENU_BROWSER)
            filebrowser_up();
        else if (menu == MENU_SETTINGS)
            selected = (selected - 1 + 2) % 2;
    }
    if (buttons & PSP_CTRL_LEFT && menu == MENU_SETTINGS) {
        if (selected == 0 && duration_s > 1) duration_s--;
        if (selected == 1 && volume > 0) volume -= 5;
    }
    if (buttons & PSP_CTRL_RIGHT && menu == MENU_SETTINGS) {
        if (selected == 0 && duration_s < 15) duration_s++;
        if (selected == 1 && volume < 100) volume += 5;
    }

    if (buttons & PSP_CTRL_CIRCLE) {
        if (menu != MENU_MAIN)
            menu = MENU_MAIN;
    }

    if (buttons & PSP_CTRL_CROSS) {
        switch (menu) {
        case MENU_MAIN:
            switch (selected) {
            case 0:
                if (!filebrowser_ms_present())
                    menu = MENU_NO_MS;
                else {
                    filebrowser_open("ms0:/");
                    menu = MENU_BROWSER;
                }
                break;
            case 1: menu = MENU_SETTINGS; selected = 0; break;
            case 2:
                if (!filebrowser_ms_present())
                    menu = MENU_NO_MS;
                else
                    filebrowser_prepare_boot(duration_s, volume);
                break;
            case 3: menu = MENU_ABOUT; break;
            case 4: sceKernelExitGame(); break;
            }
            break;
        case MENU_BROWSER:
            if (filebrowser_is_dir())
                filebrowser_enter();
            else if (filebrowser_is_media())
                filebrowser_select_current();
            break;
        case MENU_NO_MS:
        case MENU_ABOUT:
            menu = MENU_MAIN;
            break;
        default:
            break;
        }
    }
}

void gui_render(void)
{
    if (!debug_inited)
        pspDebugScreenInit();

    pspDebugScreenSetXY(0, 0);
    pspDebugScreenSetTextColor(0x00CEC9);
    pspDebugScreenPrintf(" BadCBA  |  BadCustomBootAnimation (PSP)\n");
    pspDebugScreenSetTextColor(0x888888);
    pspDebugScreenPrintf(" ----------------------------------------\n");

    switch (menu) {
    case MENU_MAIN:
        pspDebugScreenSetTextColor(0xFFFFFF);
        pspDebugScreenPrintf(" MAIN MENU\n\n");
        for (int i = 0; i < main_count; i++) {
            if (i == selected) {
                pspDebugScreenSetTextColor(0x00CEC9);
                pspDebugScreenPrintf(" > %s\n", main_items[i]);
            } else {
                pspDebugScreenSetTextColor(0xAAAAAA);
                pspDebugScreenPrintf("   %s\n", main_items[i]);
            }
        }
        break;
    case MENU_BROWSER:
        pspDebugScreenSetTextColor(0x00CEC9);
        pspDebugScreenPrintf(" MEMORY STICK BROWSER\n");
        pspDebugScreenSetTextColor(0x888888);
        pspDebugScreenPrintf(" %s\n\n", filebrowser_cwd());
        filebrowser_draw_list();
        pspDebugScreenSetTextColor(0x888888);
        pspDebugScreenPrintf("\n [X] Select  [O] Back\n");
        break;
    case MENU_SETTINGS:
        pspDebugScreenSetTextColor(0x00CEC9);
        pspDebugScreenPrintf(" SETTINGS\n\n");
        pspDebugScreenSetTextColor(selected == 0 ? 0x00CEC9 : 0xAAAAAA);
        pspDebugScreenPrintf(" %c Duration : %d s\n", selected == 0 ? '>' : ' ', duration_s);
        pspDebugScreenSetTextColor(selected == 1 ? 0x00CEC9 : 0xAAAAAA);
        pspDebugScreenPrintf(" %c Volume   : %d %%\n", selected == 1 ? '>' : ' ', volume);
        pspDebugScreenSetTextColor(0x888888);
        pspDebugScreenPrintf("\n [Left/Right] Change  [O] Back\n");
        break;
    case MENU_ABOUT:
        pspDebugScreenSetTextColor(0x00CEC9);
        pspDebugScreenPrintf(" ABOUT\n\n");
        pspDebugScreenSetTextColor(0xFFFFFF);
        pspDebugScreenPrintf(" BadCustomBootAnimation for PSP\n");
        pspDebugScreenPrintf(" Pick media from ms0:, prepare boot files.\n");
        pspDebugScreenPrintf(" CFW required for real boot replace.\n\n");
        pspDebugScreenSetTextColor(0x888888);
        pspDebugScreenPrintf(" [X]/[O] Back\n");
        break;
    case MENU_NO_MS:
        pspDebugScreenSetTextColor(0x0000FF);
        pspDebugScreenPrintf("\n\n  Uppss!! We found no Memory Stick!\n");
        pspDebugScreenPrintf("  Please insert an MS to continue!\n");
        pspDebugScreenPrintf("  (err:726)\n\n");
        pspDebugScreenSetTextColor(0x888888);
        pspDebugScreenPrintf("  [X]/[O] Back\n");
        break;
    }

    sceDisplayWaitVblankStart();
}
