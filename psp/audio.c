#include <pspiofilemgr.h>
#include <stdio.h>
#include <string.h>
#include "audio.h"
#include "settings.h"
#include "error.h"
#include "filebrowser.h"

static char source[512] = "";

void audio_set_source(const char *path)
{
    strncpy(source, path, sizeof(source) - 1);
    source[sizeof(source) - 1] = '\0';
}

const char *audio_get_source(void)
{
    return source[0] ? source : "(none)";
}

int audio_convert(void)
{
    if (!filebrowser_ms_present()) {
        error_raise(ERR_MS_NOT_FOUND);
        return -726;
    }
    if (!source[0]) {
        error_raise(ERR_NO_MEDIA);
        return -1;
    }

    sceIoMkdir("ms0:/BadCBA_Boot", 0777);

    SceUID in = sceIoOpen(source, PSP_O_RDONLY, 0);
    if (in < 0) {
        error_raise(ERR_MEDIA_OPEN);
        return -1;
    }

    SceUID out = sceIoOpen("ms0:/BadCBA_Boot/source_media.bin",
                           PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (out < 0) {
        sceIoClose(in);
        error_raise(ERR_MS_WRITE);
        return -1;
    }

    char buf[4096];
    int r, total = 0;
    while ((r = sceIoRead(in, buf, sizeof(buf))) > 0) {
        sceIoWrite(out, buf, r);
        total += r;
    }
    sceIoClose(in);
    sceIoClose(out);

    if (total < 64) {
        error_raise(ERR_MEDIA_INVALID);
        return -1;
    }

    SceUID info = sceIoOpen("ms0:/BadCBA_Boot/boot_info.txt",
                            PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (info >= 0) {
        char line[256];
        int n = snprintf(line, sizeof(line),
            "BadCBA PSP conversion\nSource: %s\nDuration: %ds\nVolume: %d%%\nFade: %dms\nBytes: %d\n",
            source, settings_get_duration(), settings_get_volume(),
            settings_get_fade(), total);
        sceIoWrite(info, line, n);
        sceIoClose(info);
    }

    SceUID rd = sceIoOpen("ms0:/BadCBA_Boot/README.txt",
                          PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (rd >= 0) {
        const char *t =
            "BadCBA – BadCustomBootAnimation (PSP)\n"
            "1. Convert finished – files in ms0:/BadCBA_Boot/\n"
            "2. Use Install to copy to installed/\n"
            "3. CFW: place boot media per your CFW docs\n";
        sceIoWrite(rd, t, strlen(t));
        sceIoClose(rd);
    }

    return 0;
}

int audio_export(void)
{
    if (!filebrowser_ms_present()) {
        error_raise(ERR_MS_NOT_FOUND);
        return -726;
    }
    sceIoMkdir("ms0:/BadCBA_Export", 0777);
    SceUID fd = sceIoOpen("ms0:/BadCBA_Export/README.txt",
                          PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd < 0) {
        error_raise(ERR_MS_WRITE);
        return -1;
    }
    char buf[200];
    int n = snprintf(buf, sizeof(buf),
        "BadCBA export\nSource: %s\nCopy files from ms0:/BadCBA_Boot/ here as needed.\n",
        audio_get_source());
    sceIoWrite(fd, buf, n);
    sceIoClose(fd);
    return 0;
}
