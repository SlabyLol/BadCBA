#ifndef BADCBA_GUI_H
#define BADCBA_GUI_H

#include <io/pad.h>
#include <rsx/gcm_sys.h>
#include "rsxutil.h"

typedef enum {
    MENU_MAIN = 0,
    MENU_FILEBROWSER,
    MENU_SETTINGS,
    MENU_CONVERT,
    MENU_ABOUT,
    MENU_COUNT
} MenuState;

void gui_init(int width, int height);
void gui_shutdown(void);
void gui_update(padData *pad);
void gui_render(gcmContextData *context, rsxBuffer *buffer);

void gui_set_menu(MenuState menu);
MenuState gui_get_menu(void);

void gui_draw_rect(rsxBuffer *buf, int x, int y, int w, int h, u32 color);
void gui_draw_text(rsxBuffer *buf, int x, int y, const char *text, u32 color);
void gui_draw_text_scaled(rsxBuffer *buf, int x, int y, const char *text, u32 color, int scale);

#endif
