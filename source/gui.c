/*
 * BadCBA - Full on-screen GUI with RSX framebuffer drawing
 * Clean dark theme, controller navigation, real menus
 * Primary media: MP4
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rsx/rsx.h>
#include <io/pad.h>

#include "gui.h"
#include "filebrowser.h"
#include "settings.h"
#include "audio.h"
#include "pkg.h"

#define COL_BG          0xFF0A0A12
#define COL_PANEL       0xFF14141F
#define COL_ACCENT      0xFF6C5CE7
#define COL_ACCENT2     0xFF00CEC9
#define COL_TEXT        0xFFEEEEF5
#define COL_TEXT_DIM    0xFF8888AA
#define COL_SELECT      0xFF2D2D44
#define COL_GREEN       0xFF00B894
#define COL_RED         0xFFE17055
#define COL_YELLOW      0xFFFDCB6E

static MenuState current_menu = MENU_MAIN;
static int selected = 0;
static int screen_w = 1280;
static int screen_h = 720;
static int debounce = 0;

static const unsigned char font8x8_basic[96][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00},
    {0x36,0x36,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x36,0x36,0x7F,0x36,0x7F,0x36,0x36,0x00},
    {0x0C,0x3E,0x03,0x1E,0x30,0x1F,0x0C,0x00},
    {0x00,0x63,0x33,0x18,0x0C,0x66,0x63,0x00},
    {0x1C,0x36,0x1C,0x6E,0x3B,0x33,0x6E,0x00},
    {0x06,0x06,0x03,0x00,0x00,0x00,0x00,0x00},
    {0x18,0x0C,0x06,0x06,0x06,0x0C,0x18,0x00},
    {0x06,0x0C,0x18,0x18,0x18,0x0C,0x06,0x00},
    {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00},
    {0x00,0x0C,0x0C,0x3F,0x0C,0x0C,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x06},
    {0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x00},
    {0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0x00},
    {0x3E,0x63,0x73,0x7B,0x6F,0x67,0x3E,0x00},
    {0x0C,0x0E,0x0C,0x0C,0x0C,0x0C,0x3F,0x00},
    {0x1E,0x33,0x30,0x1C,0x06,0x33,0x3F,0x00},
    {0x1E,0x33,0x30,0x1C,0x30,0x33,0x1E,0x00},
    {0x38,0x3C,0x36,0x33,0x7F,0x30,0x78,0x00},
    {0x3F,0x03,0x1F,0x30,0x30,0x33,0x1E,0x00},
    {0x1C,0x06,0x03,0x1F,0x33,0x33,0x1E,0x00},
    {0x3F,0x33,0x30,0x18,0x0C,0x0C,0x0C,0x00},
    {0x1E,0x33,0x33,0x1E,0x33,0x33,0x1E,0x00},
    {0x1E,0x33,0x33,0x3E,0x30,0x18,0x0E,0x00},
    {0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x00},
    {0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x06},
    {0x18,0x0C,0x06,0x03,0x06,0x0C,0x18,0x00},
    {0x00,0x00,0x3F,0x00,0x00,0x3F,0x00,0x00},
    {0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0x00},
    {0x1E,0x33,0x30,0x18,0x0C,0x00,0x0C,0x00},
    {0x3E,0x63,0x7B,0x7B,0x7B,0x03,0x1E,0x00},
    {0x0C,0x1E,0x33,0x33,0x3F,0x33,0x33,0x00},
    {0x3F,0x66,0x66,0x3E,0x66,0x66,0x3F,0x00},
    {0x3C,0x66,0x03,0x03,0x03,0x66,0x3C,0x00},
    {0x1F,0x36,0x66,0x66,0x66,0x36,0x1F,0x00},
    {0x7F,0x46,0x16,0x1E,0x16,0x46,0x7F,0x00},
    {0x7F,0x46,0x16,0x1E,0x16,0x06,0x0F,0x00},
    {0x3C,0x66,0x03,0x03,0x73,0x66,0x7C,0x00},
    {0x33,0x33,0x33,0x3F,0x33,0x33,0x33,0x00},
    {0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00},
    {0x78,0x30,0x30,0x30,0x33,0x33,0x1E,0x00},
    {0x67,0x66,0x36,0x1E,0x36,0x66,0x67,0x00},
    {0x0F,0x06,0x06,0x06,0x46,0x66,0x7F,0x00},
    {0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00},
    {0x63,0x67,0x6F,0x7B,0x73,0x63,0x63,0x00},
    {0x1C,0x36,0x63,0x63,0x63,0x36,0x1C,0x00},
    {0x3F,0x66,0x66,0x3E,0x06,0x06,0x0F,0x00},
    {0x1E,0x33,0x33,0x33,0x3B,0x1E,0x38,0x00},
    {0x3F,0x66,0x66,0x3E,0x36,0x66,0x67,0x00},
    {0x1E,0x33,0x07,0x0E,0x38,0x33,0x1E,0x00},
    {0x3F,0x2D,0x0C,0x0C,0x0C,0x0C,0x1E,0x00},
    {0x33,0x33,0x33,0x33,0x33,0x33,0x3F,0x00},
    {0x33,0x33,0x33,0x33,0x33,0x1E,0x0C,0x00},
    {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00},
    {0x63,0x63,0x36,0x1C,0x1C,0x36,0x63,0x00},
    {0x33,0x33,0x33,0x1E,0x0C,0x0C,0x1E,0x00},
    {0x7F,0x63,0x31,0x18,0x4C,0x66,0x7F,0x00},
};

void gui_draw_rect(rsxBuffer *buf, int x, int y, int w, int h, u32 color)
{
    if (!buf || !buf->ptr) return;
    u32 *ptr = (u32*)buf->ptr;
    int bw = buf->width;
    int bh = buf->height;

    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > bw) w = bw - x;
    if (y + h > bh) h = bh - y;
    if (w <= 0 || h <= 0) return;

    for (int j = 0; j < h; j++) {
        u32 *row = ptr + (y + j) * bw + x;
        for (int i = 0; i < w; i++)
            row[i] = color;
    }
}

void gui_draw_text(rsxBuffer *buf, int x, int y, const char *text, u32 color)
{
    gui_draw_text_scaled(buf, x, y, text, color, 2);
}

void gui_draw_text_scaled(rsxBuffer *buf, int x, int y, const char *text, u32 color, int scale)
{
    if (!buf || !buf->ptr || !text) return;
    u32 *ptr = (u32*)buf->ptr;
    int bw = buf->width;
    int bh = buf->height;

    int cx = x;
    while (*text) {
        unsigned char c = (unsigned char)*text;
        if (c < 32 || c > 127) { text++; continue; }
        int idx = c - 32;
        if (idx >= 96) idx = 0;

        const unsigned char *glyph = font8x8_basic[idx];

        for (int gy = 0; gy < 8; gy++) {
            unsigned char row = glyph[gy];
            for (int gx = 0; gx < 8; gx++) {
                if (row & (1 << gx)) {
                    for (int sy = 0; sy < scale; sy++) {
                        for (int sx = 0; sx < scale; sx++) {
                            int px = cx + gx * scale + sx;
                            int py = y + gy * scale + sy;
                            if (px >= 0 && px < bw && py >= 0 && py < bh)
                                ptr[py * bw + px] = color;
                        }
                    }
                }
            }
        }
        cx += 8 * scale + 2;
        text++;
    }
}

static const char *main_items[] = {
    "Select MP4 from USB",
    "Settings",
    "Convert & Create PKG",
    "Export to USB",
    "About",
    "Exit"
};
static const int main_count = 6;

void gui_init(int width, int height)
{
    screen_w = width;
    screen_h = height;
    current_menu = MENU_MAIN;
    selected = 0;
    debounce = 0;
}

void gui_shutdown(void) {}

void gui_set_menu(MenuState menu)
{
    current_menu = menu;
    selected = 0;
    debounce = 10;
}

MenuState gui_get_menu(void)
{
    return current_menu;
}

void gui_update(padData *pad)
{
    if (debounce > 0) { debounce--; return; }

    int up    = pad->BTN_UP;
    int down  = pad->BTN_DOWN;
    int left  = pad->BTN_LEFT;
    int right = pad->BTN_RIGHT;
    int cross = pad->BTN_CROSS;
    int circle= pad->BTN_CIRCLE;

    switch (current_menu) {
    case MENU_MAIN:
        if (down) { selected = (selected + 1) % main_count; debounce = 8; }
        if (up)   { selected = (selected - 1 + main_count) % main_count; debounce = 8; }
        if (cross) {
            debounce = 12;
            switch (selected) {
            case 0:
                filebrowser_open("/dev_usb000");
                gui_set_menu(MENU_FILEBROWSER);
                break;
            case 1: gui_set_menu(MENU_SETTINGS); break;
            case 2: gui_set_menu(MENU_CONVERT); break;
            case 3: audio_export_to_usb(); break;
            case 4: gui_set_menu(MENU_ABOUT); break;
            case 5: break;
            }
        }
        break;

    case MENU_FILEBROWSER:
        if (circle) { gui_set_menu(MENU_MAIN); break; }
        if (up)   { filebrowser_up();   debounce = 6; }
        if (down) { filebrowser_down(); debounce = 6; }
        if (cross) {
            debounce = 10;
            if (filebrowser_is_dir())
                filebrowser_enter();
            else if (filebrowser_is_media()) {
                audio_set_source(filebrowser_get_path());
                gui_set_menu(MENU_MAIN);
            }
        }
        break;

    case MENU_SETTINGS:
        if (circle) { gui_set_menu(MENU_MAIN); break; }
        if (up)    { settings_prev(); debounce = 6; }
        if (down)  { settings_next(); debounce = 6; }
        if (left)  { settings_adjust(-1); debounce = 6; }
        if (right) { settings_adjust(+1); debounce = 6; }
        break;

    case MENU_CONVERT:
        if (circle) { gui_set_menu(MENU_MAIN); break; }
        if (cross) {
            debounce = 20;
            audio_convert();
            pkg_create_coldboot();
            gui_set_menu(MENU_MAIN);
        }
        break;

    case MENU_ABOUT:
        if (circle) gui_set_menu(MENU_MAIN);
        break;

    default: break;
    }
}

void gui_render(gcmContextData *context, rsxBuffer *buffer)
{
    (void)context;

    gui_draw_rect(buffer, 0, 0, screen_w, 70, COL_PANEL);
    gui_draw_rect(buffer, 0, 70, screen_w, 3, COL_ACCENT);

    gui_draw_text_scaled(buffer, 40, 22, "BadCBA", COL_ACCENT2, 3);
    gui_draw_text(buffer, 200, 30, "Custom Boot Maker (MP4)", COL_TEXT_DIM);

    int panel_x = 80;
    int panel_y = 110;
    int panel_w = screen_w - 160;
    int panel_h = screen_h - 180;

    gui_draw_rect(buffer, panel_x, panel_y, panel_w, panel_h, COL_PANEL);

    int y = panel_y + 30;

    switch (current_menu) {
    case MENU_MAIN:
        gui_draw_text(buffer, panel_x + 30, y, "MAIN MENU", COL_ACCENT);
        y += 50;
        for (int i = 0; i < main_count; i++) {
            if (i == selected) {
                gui_draw_rect(buffer, panel_x + 20, y - 8, panel_w - 40, 36, COL_SELECT);
                gui_draw_rect(buffer, panel_x + 20, y - 8, 6, 36, COL_ACCENT);
                gui_draw_text(buffer, panel_x + 50, y, main_items[i], COL_TEXT);
            } else {
                gui_draw_text(buffer, panel_x + 50, y, main_items[i], COL_TEXT_DIM);
            }
            y += 48;
        }
        break;

    case MENU_FILEBROWSER:
        gui_draw_text(buffer, panel_x + 30, y, "USB FILE BROWSER (MP4)", COL_ACCENT);
        y += 40;
        gui_draw_text(buffer, panel_x + 30, y, filebrowser_get_current_dir(), COL_TEXT_DIM);
        y += 40;
        filebrowser_render_gui(buffer, panel_x + 30, y, panel_w - 60);
        gui_draw_text(buffer, panel_x + 30, screen_h - 90, "[X] Select   [O] Back", COL_TEXT_DIM);
        break;

    case MENU_SETTINGS:
        gui_draw_text(buffer, panel_x + 30, y, "SETTINGS", COL_ACCENT);
        y += 50;
        settings_render_gui(buffer, panel_x + 30, y, panel_w - 60);
        gui_draw_text(buffer, panel_x + 30, screen_h - 90, "[Left/Right] Change   [Up/Down] Navigate   [O] Back", COL_TEXT_DIM);
        break;

    case MENU_CONVERT:
        gui_draw_text(buffer, panel_x + 30, y, "CONVERT & CREATE PKG", COL_ACCENT);
        y += 50;
        char line[128];
        snprintf(line, sizeof(line), "Source  : %s", audio_get_source());
        gui_draw_text(buffer, panel_x + 40, y, line, COL_TEXT); y += 40;
        snprintf(line, sizeof(line), "Duration: %d seconds", settings_get_duration());
        gui_draw_text(buffer, panel_x + 40, y, line, COL_TEXT); y += 40;
        snprintf(line, sizeof(line), "Volume  : %d %%", settings_get_volume());
        gui_draw_text(buffer, panel_x + 40, y, line, COL_TEXT); y += 40;
        snprintf(line, sizeof(line), "Fade    : %d ms", settings_get_fade());
        gui_draw_text(buffer, panel_x + 40, y, line, COL_TEXT); y += 60;
        gui_draw_text(buffer, panel_x + 40, y, "Press [X] to start conversion", COL_GREEN);
        gui_draw_text(buffer, panel_x + 30, screen_h - 90, "[X] Start   [O] Cancel", COL_TEXT_DIM);
        break;

    case MENU_ABOUT:
        gui_draw_text(buffer, panel_x + 30, y, "ABOUT BadCBA", COL_ACCENT);
        y += 50;
        gui_draw_text(buffer, panel_x + 40, y, "Bad Custom Boot Audio", COL_TEXT); y += 36;
        gui_draw_text(buffer, panel_x + 40, y, "Create custom PS3 coldboot from MP4", COL_TEXT_DIM); y += 36;
        gui_draw_text(buffer, panel_x + 40, y, "directly on your console.", COL_TEXT_DIM); y += 50;
        gui_draw_text(buffer, panel_x + 40, y, "Title ID : BCBA00001", COL_TEXT); y += 36;
        gui_draw_text(buffer, panel_x + 40, y, "Version  : 1.0", COL_TEXT); y += 36;
        gui_draw_text(buffer, panel_x + 40, y, "License  : MIT", COL_TEXT); y += 50;
        gui_draw_text(buffer, panel_x + 40, y, "Use at your own risk.", COL_YELLOW);
        gui_draw_text(buffer, panel_x + 30, screen_h - 90, "[O] Back", COL_TEXT_DIM);
        break;

    default: break;
    }

    gui_draw_rect(buffer, 0, screen_h - 40, screen_w, 40, COL_PANEL);
    gui_draw_text(buffer, 40, screen_h - 28, "BadCBA v1.0  |  MP4 Custom Boot  |  CFW / HEN", COL_TEXT_DIM);
}
