/*
 * BadCBA GUI – errors via error_raise() + error-cba.wav loop
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
#include "flash.h"
#include "error.h"

#define COL_PANEL    0xFF14141F
#define COL_ACCENT   0xFF6C5CE7
#define COL_ACCENT2  0xFF00CEC9
#define COL_TEXT     0xFFEEEEF5
#define COL_TEXT_DIM 0xFF8888AA
#define COL_SELECT   0xFF2D2D44
#define COL_GREEN    0xFF00B894
#define COL_RED      0xFFFF3333
#define COL_YELLOW   0xFFFDCB6E

static MenuState current_menu = MENU_MAIN;
static int selected = 0;
static int screen_w = 1280, screen_h = 720, debounce = 0;

static const unsigned char font8x8_basic[96][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},{0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00},
    {0x36,0x36,0x00,0x00,0x00,0x00,0x00,0x00},{0x36,0x36,0x7F,0x36,0x7F,0x36,0x36,0x00},
    {0x0C,0x3E,0x03,0x1E,0x30,0x1F,0x0C,0x00},{0x00,0x63,0x33,0x18,0x0C,0x66,0x63,0x00},
    {0x1C,0x36,0x1C,0x6E,0x3B,0x33,0x6E,0x00},{0x06,0x06,0x03,0x00,0x00,0x00,0x00,0x00},
    {0x18,0x0C,0x06,0x06,0x06,0x0C,0x18,0x00},{0x06,0x0C,0x18,0x18,0x18,0x0C,0x06,0x00},
    {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00},{0x00,0x0C,0x0C,0x3F,0x0C,0x0C,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x06},{0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x00},{0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0x00},
    {0x3E,0x63,0x73,0x7B,0x6F,0x67,0x3E,0x00},{0x0C,0x0E,0x0C,0x0C,0x0C,0x0C,0x3F,0x00},
    {0x1E,0x33,0x30,0x1C,0x06,0x33,0x3F,0x00},{0x1E,0x33,0x30,0x1C,0x30,0x33,0x1E,0x00},
    {0x38,0x3C,0x36,0x33,0x7F,0x30,0x78,0x00},{0x3F,0x03,0x1F,0x30,0x30,0x33,0x1E,0x00},
    {0x1C,0x06,0x03,0x1F,0x33,0x33,0x1E,0x00},{0x3F,0x33,0x30,0x18,0x0C,0x0C,0x0C,0x00},
    {0x1E,0x33,0x33,0x1E,0x33,0x33,0x1E,0x00},{0x1E,0x33,0x33,0x3E,0x30,0x18,0x0E,0x00},
    {0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x00},{0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x06},
    {0x18,0x0C,0x06,0x03,0x06,0x0C,0x18,0x00},{0x00,0x00,0x3F,0x00,0x00,0x3F,0x00,0x00},
    {0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0x00},{0x1E,0x33,0x30,0x18,0x0C,0x00,0x0C,0x00},
    {0x3E,0x63,0x7B,0x7B,0x7B,0x03,0x1E,0x00},{0x0C,0x1E,0x33,0x33,0x3F,0x33,0x33,0x00},
    {0x3F,0x66,0x66,0x3E,0x66,0x66,0x3F,0x00},{0x3C,0x66,0x03,0x03,0x03,0x66,0x3C,0x00},
    {0x1F,0x36,0x66,0x66,0x66,0x36,0x1F,0x00},{0x7F,0x46,0x16,0x1E,0x16,0x46,0x7F,0x00},
    {0x7F,0x46,0x16,0x1E,0x16,0x06,0x0F,0x00},{0x3C,0x66,0x03,0x03,0x73,0x66,0x7C,0x00},
    {0x33,0x33,0x33,0x3F,0x33,0x33,0x33,0x00},{0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00},
    {0x78,0x30,0x30,0x30,0x33,0x33,0x1E,0x00},{0x67,0x66,0x36,0x1E,0x36,0x66,0x67,0x00},
    {0x0F,0x06,0x06,0x06,0x46,0x66,0x7F,0x00},{0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00},
    {0x63,0x67,0x6F,0x7B,0x73,0x63,0x63,0x00},{0x1C,0x36,0x63,0x63,0x63,0x36,0x1C,0x00},
    {0x3F,0x66,0x66,0x3E,0x06,0x06,0x0F,0x00},{0x1E,0x33,0x33,0x33,0x3B,0x1E,0x38,0x00},
    {0x3F,0x66,0x66,0x3E,0x36,0x66,0x67,0x00},{0x1E,0x33,0x07,0x0E,0x38,0x33,0x1E,0x00},
    {0x3F,0x2D,0x0C,0x0C,0x0C,0x0C,0x1E,0x00},{0x33,0x33,0x33,0x33,0x33,0x33,0x3F,0x00},
    {0x33,0x33,0x33,0x33,0x33,0x1E,0x0C,0x00},{0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00},
    {0x63,0x63,0x36,0x1C,0x1C,0x36,0x63,0x00},{0x33,0x33,0x33,0x1E,0x0C,0x0C,0x1E,0x00},
    {0x7F,0x63,0x31,0x18,0x4C,0x66,0x7F,0x00},
};

void gui_draw_rect(rsxBuffer *buf, int x, int y, int w, int h, u32 color)
{
    if (!buf || !buf->ptr) return;
    u32 *ptr = (u32*)buf->ptr;
    int bw = buf->width, bh = buf->height;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > bw) w = bw - x;
    if (y + h > bh) h = bh - y;
    if (w <= 0 || h <= 0) return;
    for (int j = 0; j < h; j++) {
        u32 *row = ptr + (y + j) * bw + x;
        for (int i = 0; i < w; i++) row[i] = color;
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
    int bw = buf->width, bh = buf->height, cx = x;
    while (*text) {
        unsigned char c = (unsigned char)*text;
        if (c < 32 || c > 127) { text++; continue; }
        int idx = c - 32; if (idx >= 96) idx = 0;
        const unsigned char *glyph = font8x8_basic[idx];
        for (int gy = 0; gy < 8; gy++) {
            unsigned char row = glyph[gy];
            for (int gx = 0; gx < 8; gx++) {
                if (row & (1 << gx)) {
                    for (int sy = 0; sy < scale; sy++)
                        for (int sx = 0; sx < scale; sx++) {
                            int px = cx + gx * scale + sx, py = y + gy * scale + sy;
                            if (px >= 0 && px < bw && py >= 0 && py < bh)
                                ptr[py * bw + px] = color;
                        }
                }
            }
        }
        cx += 8 * scale + 2; text++;
    }
}

static const char *main_items[] = {
    "Select MP4 from USB",
    "Settings",
    "Convert MP4",
    "Install to Flash (auto)",
    "Restore original boot",
    "Export to USB",
    "About",
    "Exit"
};
static const int main_count = 8;

void gui_init(int w, int h) { screen_w = w; screen_h = h; current_menu = MENU_MAIN; selected = 0; debounce = 0; }
void gui_shutdown(void) {}
void gui_set_menu(MenuState m) { current_menu = m; selected = 0; debounce = 10; }
MenuState gui_get_menu(void) { return current_menu; }

void gui_update(padData *pad)
{
    if (debounce > 0) { debounce--; return; }
    if (error_is_active()) return;

    int up = pad->BTN_UP, down = pad->BTN_DOWN;
    int left = pad->BTN_LEFT, right = pad->BTN_RIGHT;
    int cross = pad->BTN_CROSS, circle = pad->BTN_CIRCLE;

    switch (current_menu) {
    case MENU_MAIN:
        if (down) { selected = (selected + 1) % main_count; debounce = 8; }
        if (up) { selected = (selected - 1 + main_count) % main_count; debounce = 8; }
        if (cross) {
            debounce = 12;
            switch (selected) {
            case 0:
                if (!flash_usb_present()) { error_raise(ERR_USB_NOT_FOUND); break; }
                filebrowser_open(flash_find_usb());
                gui_set_menu(MENU_FILEBROWSER);
                break;
            case 1: gui_set_menu(MENU_SETTINGS); break;
            case 2: gui_set_menu(MENU_CONVERT); break;
            case 3:
                if (!flash_usb_present()) { error_raise(ERR_USB_NOT_FOUND); break; }
                if (audio_convert() == 0)
                    flash_install_to_vsh();
                if (!error_is_active()) gui_set_menu(MENU_STATUS);
                break;
            case 4:
                flash_restore_backup();
                if (!error_is_active()) gui_set_menu(MENU_STATUS);
                break;
            case 5:
                if (audio_export_to_usb() == 0 && !error_is_active())
                    gui_set_menu(MENU_STATUS);
                break;
            case 6: gui_set_menu(MENU_ABOUT); break;
            case 7: break;
            }
        }
        break;
    case MENU_FILEBROWSER:
        if (circle) { gui_set_menu(MENU_MAIN); break; }
        if (up) { filebrowser_up(); debounce = 6; }
        if (down) { filebrowser_down(); debounce = 6; }
        if (cross) {
            debounce = 10;
            if (filebrowser_is_dir()) filebrowser_enter();
            else if (filebrowser_is_media()) {
                audio_set_source(filebrowser_get_path());
                gui_set_menu(MENU_MAIN);
            } else error_raise(ERR_MP4_INVALID);
        }
        break;
    case MENU_SETTINGS:
        if (circle) gui_set_menu(MENU_MAIN);
        if (up) settings_prev();
        if (down) settings_next();
        if (left) settings_adjust(-1);
        if (right) settings_adjust(+1);
        if (up||down||left||right) debounce = 6;
        break;
    case MENU_CONVERT:
        if (circle) { gui_set_menu(MENU_MAIN); break; }
        if (cross) {
            debounce = 20;
            if (audio_convert() == 0) {
                pkg_create_coldboot();
                if (!error_is_active()) gui_set_menu(MENU_STATUS);
            }
        }
        break;
    case MENU_STATUS:
    case MENU_ABOUT:
        if (circle || cross) { debounce = 10; gui_set_menu(MENU_MAIN); }
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
    gui_draw_text(buffer, 200, 30, "BadCustomBootAnimation", COL_TEXT_DIM);

    int px = 60, py = 100, pw = screen_w - 120, ph = screen_h - 160;
    gui_draw_rect(buffer, px, py, pw, ph, COL_PANEL);
    int y = py + 28;

    switch (current_menu) {
    case MENU_MAIN:
        gui_draw_text(buffer, px + 30, y, "MAIN MENU", COL_ACCENT); y += 44;
        for (int i = 0; i < main_count; i++) {
            if (i == selected) {
                gui_draw_rect(buffer, px + 16, y - 6, pw - 32, 34, COL_SELECT);
                gui_draw_rect(buffer, px + 16, y - 6, 5, 34, COL_ACCENT);
                gui_draw_text(buffer, px + 40, y, main_items[i], COL_TEXT);
            } else gui_draw_text(buffer, px + 40, y, main_items[i], COL_TEXT_DIM);
            y += 40;
        }
        break;
    case MENU_FILEBROWSER:
        gui_draw_text(buffer, px + 30, y, "USB BROWSER", COL_ACCENT); y += 36;
        gui_draw_text(buffer, px + 30, y, filebrowser_get_current_dir(), COL_TEXT_DIM); y += 36;
        filebrowser_render_gui(buffer, px + 30, y, pw - 60);
        break;
    case MENU_SETTINGS:
        gui_draw_text(buffer, px + 30, y, "SETTINGS", COL_ACCENT); y += 44;
        settings_render_gui(buffer, px + 30, y, pw - 60);
        break;
    case MENU_CONVERT: {
        gui_draw_text(buffer, px + 30, y, "CONVERT", COL_ACCENT); y += 44;
        char line[160];
        snprintf(line, sizeof(line), "Source: %s", audio_get_source());
        gui_draw_text(buffer, px + 40, y, line, COL_TEXT); y += 40;
        gui_draw_text(buffer, px + 40, y, "[X] Start convert", COL_GREEN);
        break;
    }
    case MENU_STATUS:
        gui_draw_text(buffer, px + 30, y, "STATUS", COL_ACCENT); y += 50;
        gui_draw_text(buffer, px + 40, y,
                      flash_get_status()[0] ? flash_get_status() : "OK",
                      COL_GREEN);
        break;
    case MENU_ABOUT:
        gui_draw_text(buffer, px + 30, y, "ABOUT", COL_ACCENT); y += 44;
        gui_draw_text(buffer, px + 40, y, "BadCustomBootAnimation", COL_TEXT); y += 32;
        gui_draw_text(buffer, px + 40, y, "Errors play error-cba.wav in a loop", COL_TEXT_DIM);
        break;
    default: break;
    }

    gui_draw_rect(buffer, 0, screen_h - 40, screen_w, 40, COL_PANEL);
    gui_draw_text(buffer, 40, screen_h - 28, "BadCBA v1.0 | err:726 = no USB", COL_TEXT_DIM);
}
