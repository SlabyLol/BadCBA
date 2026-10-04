/*
 * Backup / install / restore prepared boot files on Memory Stick
 * (CFW paths – user copies further if needed)
 */
#include <pspiofilemgr.h>
#include <stdio.h>
#include <string.h>
#include "flash.h"
#include "settings.h"
#include "error.h"
#include "audio.h"

static char status[160] = "";

#define WORK   "ms0:/BadCBA_Boot"
#define BACKUP "ms0:/PSP/GAME/BadCBA/backup"
#define DEST   "ms0:/BadCBA_Boot/installed"

static int copy_file(const char *src, const char *dst)
{
    SceUID in = sceIoOpen(src, PSP_O_RDONLY, 0);
    if (in < 0) return -1;
    SceUID out = sceIoOpen(dst, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (out < 0) { sceIoClose(in); return -1; }
    char buf[4096];
    int r;
    while ((r = sceIoRead(in, buf, sizeof(buf))) > 0)
        sceIoWrite(out, buf, r);
    sceIoClose(in);
    sceIoClose(out);
    return 0;
}

const char *flash_status(void) { return status; }

int flash_backup(void)
{
    sceIoMkdir("ms0:/PSP", 0777);
    sceIoMkdir("ms0:/PSP/GAME", 0777);
    sceIoMkdir("ms0:/PSP/GAME/BadCBA", 0777);
    sceIoMkdir(BACKUP, 0777);

    int ok = 0;
    const char *files[] = { "boot.pmf", "boot.wav", "source_media.bin", NULL };
    for (int i = 0; files[i]; i++) {
        char s[256], d[256];
        snprintf(s, sizeof(s), "%s/%s", DEST, files[i]);
        snprintf(d, sizeof(d), "%s/%s", BACKUP, files[i]);
        if (copy_file(s, d) == 0) ok++;
    }
    if (ok == 0) {
        error_raise(ERR_FLASH_BACKUP);
        snprintf(status, sizeof(status), "%s", error_get_message());
        return -1;
    }
    snprintf(status, sizeof(status), "Backup OK (%d files)", ok);
    return 0;
}

int flash_install(void)
{
    if (settings_get_backup())
        flash_backup();

    sceIoMkdir(DEST, 0777);
    int ok = 0;
    const char *files[] = { "README.txt", "source_media.bin", "boot_info.txt", NULL };
    for (int i = 0; files[i]; i++) {
        char s[256], d[256];
        snprintf(s, sizeof(s), "%s/%s", WORK, files[i]);
        snprintf(d, sizeof(d), "%s/%s", DEST, files[i]);
        if (copy_file(s, d) == 0) ok++;
    }
    if (ok == 0) {
        error_raise(ERR_FLASH_MISSING);
        snprintf(status, sizeof(status), "%s", error_get_message());
        return -1;
    }
    snprintf(status, sizeof(status), "Installed %d file(s) to %s", ok, DEST);
    return 0;
}

int flash_restore(void)
{
    sceIoMkdir(DEST, 0777);
    int ok = 0;
    const char *files[] = { "boot.pmf", "boot.wav", "source_media.bin", NULL };
    for (int i = 0; files[i]; i++) {
        char s[256], d[256];
        snprintf(s, sizeof(s), "%s/%s", BACKUP, files[i]);
        snprintf(d, sizeof(d), "%s/%s", DEST, files[i]);
        if (copy_file(s, d) == 0) ok++;
    }
    if (ok == 0) {
        error_raise(ERR_FLASH_RESTORE);
        snprintf(status, sizeof(status), "%s", error_get_message());
        return -1;
    }
    snprintf(status, sizeof(status), "Restored %d file(s)", ok);
    return 0;
}
