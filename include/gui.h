#ifndef BADCBA_GUI_H
#define BADCBA_GUI_H

#include <io/pad.h>

typedef enum {
    MENU_MAIN = 0,
    MENU_FILEBROWSER,
    MENU_SETTINGS,
    MENU_CONVERT,
    MENU_ABOUT,
    MENU_COUNT
} MenuState;

void gui_init(void);
void gui_shutdown(void);
void gui_update(padData *pad);
void gui_render(void);

void gui_set_menu(MenuState menu);
MenuState gui_get_menu(void);

#endif /* BADCBA_GUI_H */
