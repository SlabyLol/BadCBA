/*
 * BadCBA PSP – full menu GUI
 */
#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <string.h>
#include <stdio.h>

#include "gui.h"
#include "filebrowser.h"
#include "settings.h"
#include "flash.h"
#include "audio.h"
#include "error.h"

enum {
    MENU_MAIN = 0,
    MENU_BROWSER,
    MENU_SETTINGS,
    MENU_CONVERT,
    MENU_STATUS,
    MENU_ABOUT
};

static int menu = MENU_MAIN;
static int selected = 0;

static const char *main_items[] = {
    "Select media (ms0:)",
    "Settings",
    "Convert / prepare",
    "Install to Boot folder",
    "Restore backup",
    "Export to MS",
    "About",
    "Exit"
};
static const int main_count = 8;

void gui_init(void)
{
    pspDebugScreenInit();
    menu = MENU_MAIN;
    selected = 0;
}

void gui_shutdown(void) {}

void gui_update(unsigned int buttons)
{
    if (error_is_active()) {
        if (buttons & (PSP_CTRL_CROSS | PSP_CTRL_CIRCLE))
            error_clear();
        return;
    }

    if (buttons & PSP_CTRL_DOWN) {
        if (menu == MENU_MAIN) selected = (selected + 1) % main_count;
        else if (menu == MENU_BROWSER) filebrowser_down();
        else if (menu == MENU_SETTINGS) settings_next();
    }
    if (buttons & PSP_CTRL_UP) {
        if (menu == MENU_MAIN) selected = (selected - 1 + main_count) % main_count;
        else if (menu == MENU_BROWSER) filebrowser_up();
        else if (menu == MENU_SETTINGS) settings_prev();
    }
    if (buttons & PSP_CTRL_LEFT && menu == MENU_SETTINGS) settings_adjust(-1);
    if (buttons & PSP_CTRL_RIGHT && menu == MENU_SETTINGS) settings_adjust(+1);

    if (buttons & PSP_CTRL_CIRCLE) {
        if (menu == MENU_SETTINGS) settings_save();
        if (menu != MENU_MAIN) menu = MENU_MAIN;
    }

    if (buttons & PSP_CTRL_CROSS) {
        switch (menu) {
        case MENU_MAIN:
            switch (selected) {
            case 0:
                if (!filebrowser_ms_present()) { error_raise(ERR_MS_NOT_FOUND); break; }
                filebrowser_open("ms0:/");
                menu = MENU_BROWSER;
                break;
            case 1: menu = MENU_SETTINGS; break;
            case 2: menu = MENU_CONVERT; break;
            case 3:
                if (!filebrowser_ms_present()) { error_raise(ERR_MS_NOT_FOUND); break; }
                if (audio_convert() == 0)
                    flash_install();
                if (!error_is_active()) menu = MENU_STATUS;
                break;
            case 4:
                flash_restore();
                if (!error_is_active()) menu = MENU_STATUS;
                break;
            case 5:
                if (audio_export() == 0 && !error_is_active())
                    menu = MENU_STATUS;
                break;
            case 6: menu = MENU_ABOUT; break;
            case 7: sceKernelExitGame(); break;
            }
            break;
        case MENU_BROWSER:
            if (filebrowser_is_dir()) filebrowser_enter();
            else if (filebrowser_is_media()) {
                filebrowser_select_current();
                menu = MENU_MAIN;
            } else error_raise(ERR_MEDIA_INVALID);
            break;
        case MENU_CONVERT:
            if (audio_convert() == 0 && !error_is_active())
                menu = MENU_STATUS;
            break;
        case MENU_STATUS:
        case MENU_ABOUT:
            menu = MENU_MAIN;
            break;
        default: break;
        }
    }
}

void gui_render(void)
{
    pspDebugScreenSetXY(0, 0);
    pspDebugScreenSetTextColor(0x00CEC9);
    pspDebugScreenPrintf(" BadCBA | BadCustomBootAnimation (PSP) v1.0\n");
    pspDebugScreenSetTextColor(0x666666);
    pspDebugScreenPrintf(" ------------------------------------------\n");

    if (error_is_active()) {
        error_draw();
        sceDisplayWaitVblankStart();
        return;
    }

    switch (menu) {
    case MENU_MAIN:
        pspDebugScreenSetTextColor(0xFFFFFF);
        pspDebugScreenPrintf(" MAIN MENU\n\n");
        for (int i = 0; i < main_count; i++) {
            pspDebugScreenSetTextColor(i == selected ? 0x00CEC9 : 0xAAAAAA);
            pspDebugScreenPrintf(" %c %s\n", i == selected ? '>' : ' ', main_items[i]);
        }
        pspDebugScreenSetTextColor(0x666666);
        pspDebugScreenPrintf("\n Source: %s\n", audio_get_source());
        break;
    case MENU_BROWSER:
        pspDebugScreenSetTextColor(0x00CEC9);
        pspDebugScreenPrintf(" MS BROWSER\n %s\n\n", filebrowser_cwd());
        filebrowser_draw_list();
        break;
    case MENU_SETTINGS:
        settings_draw();
        break;
    case MENU_CONVERT:
        pspDebugScreenSetTextColor(0x00CEC9);
        pspDebugScreenPrintf(" CONVERT\n\n");
        pspDebugScreenSetTextColor(0xFFFFFF);
        pspDebugScreenPrintf(" Source  : %s\n", audio_get_source());
        pspDebugScreenPrintf(" Duration: %ds  Vol: %d%%  Fade: %dms\n\n",
                             settings_get_duration(), settings_get_volume(), settings_get_fade());
        pspDebugScreenSetTextColor(0x00B894);
        pspDebugScreenPrintf(" [X] Start convert   [O] Back\n");
        break;
    case MENU_STATUS:
        pspDebugScreenSetTextColor(0x00CEC9);
        pspDebugScreenPrintf(" STATUS\n\n");
        pspDebugScreenSetTextColor(0x00B894);
        pspDebugScreenPrintf(" %s\n\n",
            flash_status()[0] ? flash_status() : "OK");
        pspDebugScreenSetTextColor(0x888888);
        pspDebugScreenPrintf(" [X]/[O] Back\n");
        break;
    case MENU_ABOUT:
        pspDebugScreenSetTextColor(0x00CEC9);
        pspDebugScreenPrintf(" ABOUT\n\n");
        pspDebugScreenSetTextColor(0xFFFFFF);
        pspDebugScreenPrintf(" BadCustomBootAnimation for PSP\n");
        pspDebugScreenPrintf(" Full feature homebrew port.\n");
        pspDebugScreenPrintf(" Convert + install + backup + export.\n");
        pspDebugScreenPrintf(" Errors play error-cba.wav until exit.\n\n");
        pspDebugScreenSetTextColor(0x888888);
        pspDebugScreenPrintf(" MIT License  |  CFW recommended\n");
        break;
    }

    sceDisplayWaitVblankStart();
}
