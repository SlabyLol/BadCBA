/*
 * BadCBA - Audio conversion
 *
 * Note: Full MP3 → AC3 conversion on PS3 requires a port of libavcodec / ffmpeg
 * or a pre-converted intermediate format. This module provides the structure
 * and safe file handling. Real encoding can be added later or done offline.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "audio.h"
#include "settings.h"

static char source_path[512] = "";
static char output_dir[256]  = "/dev_hdd0/tmp/badcba";

void audio_set_source(const char *path)
{
    strncpy(source_path, path, sizeof(source_path) - 1);
    source_path[sizeof(source_path) - 1] = '\0';
    printf("Selected source: %s\n", source_path);
}

const char *audio_get_source(void)
{
    return source_path[0] ? source_path : "(none)";
}

int audio_convert(void)
{
    if (!source_path[0]) {
        printf("No MP3 selected.\n");
        return -1;
    }

    printf("Converting: %s\n", source_path);
    printf("  Duration : %d s\n", settings_get_duration());
    printf("  Volume   : %d %%\n", settings_get_volume());
    printf("  Fade     : %d ms\n", settings_get_fade());

    /* Create output directory */
    mkdir("/dev_hdd0/tmp", 0777);
    mkdir(output_dir, 0777);

    /*
     * Real conversion would happen here:
     * 1. Decode MP3 (needs libmpg123 or ffmpeg port)
     * 2. Resample / trim / apply volume & fade
     * 3. Encode to AC3 (coldboot_stereo.ac3 + coldboot_multi.ac3)
     *
     * For now we copy the source as a placeholder and write metadata.
     */

    char dest[512];
    snprintf(dest, sizeof(dest), "%s/source.mp3", output_dir);

    FILE *in = fopen(source_path, "rb");
    if (!in) {
        printf("Failed to open source file.\n");
        return -1;
    }

    FILE *out = fopen(dest, "wb");
    if (!out) {
        fclose(in);
        printf("Failed to create output file.\n");
        return -1;
    }

    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        fwrite(buf, 1, n, out);
    }

    fclose(in);
    fclose(out);

    /* Write a simple info file */
    char info[512];
    snprintf(info, sizeof(info), "%s/convert_info.txt", output_dir);
    FILE *f = fopen(info, "w");
    if (f) {
        fprintf(f, "BadCBA Conversion Info\n");
        fprintf(f, "Source  : %s\n", source_path);
        fprintf(f, "Duration: %d\n", settings_get_duration());
        fprintf(f, "Volume  : %d\n", settings_get_volume());
        fprintf(f, "Fade    : %d\n", settings_get_fade());
        fprintf(f, "Note    : Full AC3 encoding requires additional libraries.\n");
        fclose(f);
    }

    printf("Conversion step finished (placeholder).\n");
    printf("Output directory: %s\n", output_dir);
    return 0;
}

int audio_export_to_usb(void)
{
    printf("Exporting files to USB...\n");

    /* Preferred USB path */
    const char *usb = "/dev_usb000/BadCBA_Coldboot";
    mkdir("/dev_usb000", 0777);
    mkdir(usb, 0777);

    /* Copy whatever we have produced */
    char cmd_info[512];
    snprintf(cmd_info, sizeof(cmd_info),
             "Export target: %s\nPlace coldboot_stereo.ac3 and coldboot_multi.ac3 here.\n",
             usb);

    char readme[512];
    snprintf(readme, sizeof(readme), "%s/README.txt", usb);
    FILE *f = fopen(readme, "w");
    if (f) {
        fprintf(f, "BadCBA - Custom Coldboot Export\n\n");
        fprintf(f, "1. Convert your MP3 with BadCBA on the PS3.\n");
        fprintf(f, "2. The generated AC3 files will appear in this folder.\n");
        fprintf(f, "3. Use multiMAN / IrisMAN to copy them to:\n");
        fprintf(f, "   /dev_blind/vsh/resource/\n\n");
        fprintf(f, "Always keep a backup of the original files!\n");
        fclose(f);
    }

    printf("%s\n", cmd_info);
    return 0;
}
