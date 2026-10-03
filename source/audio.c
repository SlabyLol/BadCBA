/*
 * BadCBA - MP4 conversion helpers
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "audio.h"
#include "settings.h"
#include "error.h"
#include "flash.h"

static char source_path[512] = "";
static char output_dir[256]  = "/dev_hdd0/tmp/badcba";

void audio_set_source(const char *path)
{
    strncpy(source_path, path, sizeof(source_path) - 1);
    source_path[sizeof(source_path) - 1] = '\0';
}

const char *audio_get_source(void)
{
    return source_path[0] ? source_path : "(none)";
}

static int is_mp4(const char *path)
{
    size_t len = strlen(path);
    if (len < 4) return 0;
    return strcasecmp(path + len - 4, ".mp4") == 0 ||
           (len >= 5 && strcasecmp(path + len - 5, ".m4v") == 0);
}

int audio_convert(void)
{
    if (!source_path[0]) {
        error_raise(ERR_NO_MP4_SELECTED);
        return -1;
    }
    if (!flash_usb_present()) {
        error_raise(ERR_USB_NOT_FOUND);
        return -726;
    }

    FILE *in = fopen(source_path, "rb");
    if (!in) {
        error_raise(ERR_MP4_OPEN);
        return -1;
    }

    fseek(in, 0, SEEK_END);
    long sz = ftell(in);
    fseek(in, 0, SEEK_SET);
    if (sz < 1024) {
        fclose(in);
        error_raise(ERR_MP4_INVALID);
        return -1;
    }

    mkdir("/dev_hdd0/tmp", 0777);
    if (mkdir(output_dir, 0777) != 0) {
        /* may already exist */
    }

    char dest[512];
    snprintf(dest, sizeof(dest), "%s/source%s",
             output_dir, is_mp4(source_path) ? ".mp4" : ".bin");

    FILE *out = fopen(dest, "wb");
    if (!out) {
        fclose(in);
        error_raise(ERR_HDD_WRITE);
        return -1;
    }

    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {
            fclose(in); fclose(out);
            error_raise(ERR_HDD_WRITE);
            return -1;
        }
    }
    fclose(in);
    fclose(out);

    /* Placeholder outputs so install can find paths */
    char placeholder[512];
    const char *outs[] = {
        "coldboot.raf", "coldboot_stereo.ac3", "coldboot_multi.ac3", NULL
    };
    for (int i = 0; outs[i]; i++) {
        snprintf(placeholder, sizeof(placeholder), "%s/%s", output_dir, outs[i]);
        FILE *p = fopen(placeholder, "wb");
        if (p) {
            fprintf(p, "BadCBA placeholder for %s from %s\n", outs[i], source_path);
            fclose(p);
        } else {
            error_raise(ERR_CONVERT_FAIL);
            return -1;
        }
    }

    return 0;
}

int audio_export_to_usb(void)
{
    if (!flash_usb_present()) {
        error_raise(ERR_USB_NOT_FOUND);
        return -726;
    }

    const char *base = flash_find_usb();
    char usb[256];
    snprintf(usb, sizeof(usb), "%s/BadCBA_Coldboot", base);
    mkdir(usb, 0777);

    char readme[300];
    snprintf(readme, sizeof(readme), "%s/README.txt", usb);
    FILE *f = fopen(readme, "w");
    if (!f) {
        error_raise(ERR_USB_WRITE);
        return -1;
    }
    fprintf(f, "BadCBA BadCustomBootAnimation export\n");
    fprintf(f, "Copy coldboot.* to /dev_blind/vsh/resource/\n");
    fclose(f);
    return 0;
}
