/*
 * BadCBA - GUI implementation (simple text-based for maximum compatibility)
 * Can later be extended with Tiny3D / RSX rendering.
 */

#include <stdio.h>
#include <string.h>
#include <io/pad.h>

#include "gui.h"
#include "filebrowser.h"
#include "settings.h"
#include "audio.h"
#include "pkg.h"

static MenuState current_menu = MENU_MAIN;
static int selected_item = 0;

static const char *main_items[] = {
    "Select MP3 from USB",
    "Settings",
    "Convert & Create PKG",
    "Export files to USB",
    "About",
    "Exit"
};
static const int main_item_count = 6;

void gui_init(void)
{
    current_menu = MENU_MAIN;
    selected_item = 0;
    printf("BadCBA GUI initialized\n");
}

void gui_shutdown(void)
{
    printf("BadCBA GUI shutdown\n");
}

void gui_set_menu(MenuState menu)
{
    current_menu = menu;
    selected_item = 0;
}

MenuState gui_get_menu(void)
{
    return current_menu;
}

static void handle_main_menu(padData *pad)
{
    if (pad->BTN_DOWN) {
        selected_item = (selected_item + 1) % main_item_count;
    }
    if (pad->BTN_UP) {
        selected_item = (selected_item - 1 + main_item_count) % main_item_count;
    }

    if (pad->BTN_CROSS) {
        switch (selected_item) {
        case 0: /* Select MP3 */
            gui_set_menu(MENU_FILEBROWSER);
            filebrowser_open("/dev_usb000");
            break;
        case 1: /* Settings */
            gui_set_menu(MENU_SETTINGS);
            break;
        case 2: /* Convert */
            gui_set_menu(MENU_CONVERT);
            break;
        case 3: /* Export */
            audio_export_to_usb();
            break;
        case 4: /* About */
            gui_set_menu(MENU_ABOUT);
            break;
        case 5: /* Exit */
            /* Will be handled by sysutil */
            break;
        }
    }
}

static void handle_settings_menu(padData *pad)
{
    if (pad->BTN_CIRCLE) {
        gui_set_menu(MENU_MAIN);
        return;
    }

    /* Simple settings navigation - expand later */
    if (pad->BTN_LEFT) {
        settings_adjust(-1);
    }
    if (pad->BTN_RIGHT) {
        settings_adjust(+1);
    }
    if (pad->BTN_UP) {
        settings_prev();
    }
    if (pad->BTN_DOWN) {
        settings_next();
    }
}

static void handle_filebrowser_menu(padData *pad)
{
    if (pad->BTN_CIRCLE) {
        gui_set_menu(MENU_MAIN);
        return;
    }

    if (pad->BTN_UP) {
        filebrowser_up();
    }
    if (pad->BTN_DOWN) {
        filebrowser_down();
    }
    if (pad->BTN_CROSS) {
        if (filebrowser_is_dir()) {
            filebrowser_enter();
        } else if (filebrowser_is_mp3()) {
            audio_set_source(filebrowser_get_path());
            gui_set_menu(MENU_MAIN);
        }
    }
}

void gui_update(padData *pad)
{
    switch (current_menu) {
    case MENU_MAIN:
        handle_main_menu(pad);
        break;
    case MENU_FILEBROWSER:
        handle_filebrowser_menu(pad);
        break;
    case MENU_SETTINGS:
        handle_settings_menu(pad);
        break;
    case MENU_CONVERT:
        if (pad->BTN_CROSS) {
            audio_convert();
            pkg_create_coldboot();
            gui_set_menu(MENU_MAIN);
        }
        if (pad->BTN_CIRCLE) {
            gui_set_menu(MENU_MAIN);
        }
        break;
    case MENU_ABOUT:
        if (pad->BTN_CIRCLE) {
            gui_set_menu(MENU_MAIN);
        }
        break;
    default:
        break;
    }
}

void gui_render(void)
{
    /* Currently text output via TTY / network debug.
     * Replace with RSX / Tiny3D rendering for a real on-screen GUI. */

    printf("\033[2J\033[H"); /* clear */
    printf("========================================\n");
    printf("          BadCBA v1.0\n");
    printf("   PS3 Custom Boot Audio Maker\n");
    printf("========================================\n\n");

    switch (current_menu) {
    case MENU_MAIN:
        printf("Main Menu:\n\n");
        for (int i = 0; i < main_item_count; i++) {
            printf("  %s %s\n", (i == selected_item) ? ">" : " ", main_items[i]);
        }
        break;

    case MENU_FILEBROWSER:
        printf("USB File Browser\n");
        printf("Path: %s\n\n", filebrowser_get_current_dir());
        filebrowser_render();
        printf("\n[X] Select / Enter   [O] Back\n");
        break;

    case MENU_SETTINGS:
        printf("Settings\n\n");
        settings_render();
        printf("\n[Left/Right] Change   [Up/Down] Select   [O] Back\n");
        break;

    case MENU_CONVERT:
        printf("Convert & Create PKG\n\n");
        printf("Source : %s\n", audio_get_source());
        printf("Duration: %d s\n", settings_get_duration());
        printf("Volume  : %d %%\n", settings_get_volume());
        printf("Fade    : %d ms\n", settings_get_fade());
        printf("\n[X] Start conversion   [O] Cancel\n");
        break;

    case MENU_ABOUT:
        printf("About BadCBA\n\n");
        printf("Bad Custom Boot Audio\n");
        printf("Create custom PS3 coldboot sounds\n");
        printf("directly on your console.\n\n");
        printf("Title ID : BCBA00001\n");
        printf("Version  : 1.0\n");
        printf("License  : MIT\n\n");
        printf("[O] Back\n");
        break;

    default:
        break;
    }

    fflush(stdout);
}
