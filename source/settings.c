/*
 * BadCBA - Settings with on-screen GUI
 */

#include <stdio.h>
#include <string.h>

#include "settings.h"
#include "gui.h"

typedef struct {
    int duration_sec;
    int volume_percent;
    int fade_ms;
    int create_backup;
    int sample_rate;
    char content_id[64];
    char pkg_name[64];
} Settings;

static Settings cfg = {
    .duration_sec   = 6,
    .volume_percent = 80,
    .fade_ms        = 500,
    .create_backup  = 1,
    .sample_rate    = 48000,
    .content_id     = "UP0001-BCBA00001_00-COLDBOOT00000000",
    .pkg_name       = "BadCBA_Coldboot"
};

static int current = 0;
static const int count = 7;

void settings_load(void) {}
void settings_save(void) {}

void settings_next(void) { current = (current + 1) % count; }
void settings_prev(void) { current = (current - 1 + count) % count; }

void settings_adjust(int delta)
{
    switch (current) {
    case 0:
        cfg.duration_sec += delta;
        if (cfg.duration_sec < 1) cfg.duration_sec = 1;
        if (cfg.duration_sec > 8) cfg.duration_sec = 8;
        break;
    case 1:
        cfg.volume_percent += delta * 5;
        if (cfg.volume_percent < 0)   cfg.volume_percent = 0;
        if (cfg.volume_percent > 100) cfg.volume_percent = 100;
        break;
    case 2:
        cfg.fade_ms += delta * 100;
        if (cfg.fade_ms < 0)    cfg.fade_ms = 0;
        if (cfg.fade_ms > 2000) cfg.fade_ms = 2000;
        break;
    case 3:
        cfg.create_backup = !cfg.create_backup;
        break;
    case 4:
        cfg.sample_rate = (cfg.sample_rate == 48000) ? 44100 : 48000;
        break;
    default: break;
    }
}

void settings_render_gui(rsxBuffer *buf, int x, int y, int max_w)
{
    const char *labels[] = {
        "Max Duration (seconds)",
        "Volume (%)",
        "Fade In/Out (ms)",
        "Backup existing files",
        "Sample Rate",
        "Content ID",
        "PKG Name"
    };

    char value[80];

    for (int i = 0; i < count; i++) {
        switch (i) {
        case 0: snprintf(value, sizeof(value), "%d", cfg.duration_sec); break;
        case 1: snprintf(value, sizeof(value), "%d", cfg.volume_percent); break;
        case 2: snprintf(value, sizeof(value), "%d", cfg.fade_ms); break;
        case 3: snprintf(value, sizeof(value), "%s", cfg.create_backup ? "Yes" : "No"); break;
        case 4: snprintf(value, sizeof(value), "%d Hz", cfg.sample_rate); break;
        case 5: snprintf(value, sizeof(value), "%s", cfg.content_id); break;
        case 6: snprintf(value, sizeof(value), "%s", cfg.pkg_name); break;
        }

        if (i == current) {
            gui_draw_rect(buf, x - 10, y - 6, max_w, 32, 0xFF2D2D44);
            gui_draw_rect(buf, x - 10, y - 6, 5, 32, 0xFF6C5CE7);
            gui_draw_text(buf, x + 10, y, labels[i], 0xFFEEEEF5);
            gui_draw_text(buf, x + 380, y, value, 0xFF00CEC9);
        } else {
            gui_draw_text(buf, x + 10, y, labels[i], 0xFF8888AA);
            gui_draw_text(buf, x + 380, y, value, 0xFF666688);
        }
        y += 40;
    }
}

int settings_get_duration(void) { return cfg.duration_sec; }
int settings_get_volume(void)   { return cfg.volume_percent; }
int settings_get_fade(void)     { return cfg.fade_ms; }
int settings_get_backup(void)   { return cfg.create_backup; }
const char *settings_get_content_id(void) { return cfg.content_id; }
const char *settings_get_pkg_name(void)   { return cfg.pkg_name; }
