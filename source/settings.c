/*
 * BadCBA - Settings management
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "settings.h"

typedef struct {
    int duration_sec;      /* 1-8 */
    int volume_percent;    /* 0-100 */
    int fade_ms;           /* 0-2000 */
    int create_backup;     /* 0/1 */
    int sample_rate;       /* 44100 / 48000 */
    char content_id[64];
    char pkg_name[64];
    char title[64];
} Settings;

static Settings cfg = {
    .duration_sec   = 6,
    .volume_percent = 80,
    .fade_ms        = 500,
    .create_backup  = 1,
    .sample_rate    = 48000,
    .content_id     = "UP0001-BCBA00001_00-COLDBOOT00000000",
    .pkg_name       = "BadCBA_Coldboot",
    .title          = "Custom Coldboot"
};

static int current_setting = 0;
static const int setting_count = 7;

void settings_load(void)
{
    /* TODO: load from /dev_hdd0/game/BCBA00001/USRDIR/settings.cfg */
    printf("Settings loaded (defaults)\n");
}

void settings_save(void)
{
    /* TODO: save to USRDIR */
    printf("Settings saved\n");
}

void settings_next(void)
{
    current_setting = (current_setting + 1) % setting_count;
}

void settings_prev(void)
{
    current_setting = (current_setting - 1 + setting_count) % setting_count;
}

void settings_adjust(int delta)
{
    switch (current_setting) {
    case 0: /* duration */
        cfg.duration_sec += delta;
        if (cfg.duration_sec < 1) cfg.duration_sec = 1;
        if (cfg.duration_sec > 8) cfg.duration_sec = 8;
        break;
    case 1: /* volume */
        cfg.volume_percent += delta * 5;
        if (cfg.volume_percent < 0) cfg.volume_percent = 0;
        if (cfg.volume_percent > 100) cfg.volume_percent = 100;
        break;
    case 2: /* fade */
        cfg.fade_ms += delta * 100;
        if (cfg.fade_ms < 0) cfg.fade_ms = 0;
        if (cfg.fade_ms > 2000) cfg.fade_ms = 2000;
        break;
    case 3: /* backup */
        cfg.create_backup = !cfg.create_backup;
        break;
    case 4: /* sample rate */
        cfg.sample_rate = (cfg.sample_rate == 48000) ? 44100 : 48000;
        break;
    default:
        break;
    }
}

void settings_render(void)
{
    const char *labels[] = {
        "Max Duration (s)",
        "Volume (%)",
        "Fade In/Out (ms)",
        "Backup existing",
        "Sample Rate",
        "Content ID",
        "PKG Name"
    };

    for (int i = 0; i < setting_count; i++) {
        printf("  %s %s : ", (i == current_setting) ? ">" : " ", labels[i]);
        switch (i) {
        case 0: printf("%d\n", cfg.duration_sec); break;
        case 1: printf("%d\n", cfg.volume_percent); break;
        case 2: printf("%d\n", cfg.fade_ms); break;
        case 3: printf("%s\n", cfg.create_backup ? "Yes" : "No"); break;
        case 4: printf("%d Hz\n", cfg.sample_rate); break;
        case 5: printf("%s\n", cfg.content_id); break;
        case 6: printf("%s\n", cfg.pkg_name); break;
        }
    }
}

int settings_get_duration(void) { return cfg.duration_sec; }
int settings_get_volume(void)   { return cfg.volume_percent; }
int settings_get_fade(void)     { return cfg.fade_ms; }
int settings_get_backup(void)   { return cfg.create_backup; }
const char *settings_get_content_id(void) { return cfg.content_id; }
const char *settings_get_pkg_name(void)   { return cfg.pkg_name; }
