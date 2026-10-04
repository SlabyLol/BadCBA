#include <pspiofilemgr.h>
#include <pspdebug.h>
#include <stdio.h>
#include <string.h>
#include "settings.h"

static int duration = 5;
static int volume = 80;
static int fade_ms = 200;
static int backup = 1;
static int sel = 0;
static const int count = 4;

#define CFG "ms0:/PSP/GAME/BadCBA/settings.cfg"

void settings_load(void)
{
    SceUID fd = sceIoOpen(CFG, PSP_O_RDONLY, 0);
    if (fd < 0) return;
    char buf[128];
    int n = sceIoRead(fd, buf, sizeof(buf) - 1);
    sceIoClose(fd);
    if (n > 0) {
        buf[n] = 0;
        sscanf(buf, "%d %d %d %d", &duration, &volume, &fade_ms, &backup);
    }
}

void settings_save(void)
{
    sceIoMkdir("ms0:/PSP/GAME", 0777);
    sceIoMkdir("ms0:/PSP/GAME/BadCBA", 0777);
    SceUID fd = sceIoOpen(CFG, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd < 0) return;
    char buf[64];
    int n = snprintf(buf, sizeof(buf), "%d %d %d %d\n", duration, volume, fade_ms, backup);
    sceIoWrite(fd, buf, n);
    sceIoClose(fd);
}

void settings_next(void) { sel = (sel + 1) % count; }
void settings_prev(void) { sel = (sel - 1 + count) % count; }

void settings_adjust(int d)
{
    if (sel == 0) { duration += d; if (duration < 1) duration = 1; if (duration > 15) duration = 15; }
    if (sel == 1) { volume += d * 5; if (volume < 0) volume = 0; if (volume > 100) volume = 100; }
    if (sel == 2) { fade_ms += d * 50; if (fade_ms < 0) fade_ms = 0; if (fade_ms > 2000) fade_ms = 2000; }
    if (sel == 3) backup = backup ? 0 : 1;
}

void settings_draw(void)
{
    pspDebugScreenSetTextColor(0x00CEC9);
    pspDebugScreenPrintf(" SETTINGS\n\n");
    const char *labels[] = { "Duration (s)", "Volume (%)", "Fade (ms)", "Backup before install" };
    int vals[] = { duration, volume, fade_ms, backup };
    for (int i = 0; i < count; i++) {
        pspDebugScreenSetTextColor(i == sel ? 0x00CEC9 : 0xAAAAAA);
        if (i == 3)
            pspDebugScreenPrintf(" %c %s : %s\n", i == sel ? '>' : ' ', labels[i], vals[i] ? "ON" : "OFF");
        else
            pspDebugScreenPrintf(" %c %s : %d\n", i == sel ? '>' : ' ', labels[i], vals[i]);
    }
    pspDebugScreenSetTextColor(0x888888);
    pspDebugScreenPrintf("\n [Left/Right] Change  [O] Back\n");
}

int settings_get_duration(void) { return duration; }
int settings_get_volume(void) { return volume; }
int settings_get_fade(void) { return fade_ms; }
int settings_get_backup(void) { return backup; }
