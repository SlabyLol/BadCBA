/*
 * BadCBA - Media conversion (MP4 / MP3 -> coldboot AC3 + optional RAF)
 *
 * MP4 is the primary input. Audio track is extracted and prepared for
 * coldboot_stereo.ac3 / coldboot_multi.ac3.
 * Video frames can later be used for coldboot.raf generation.
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
        printf("No media file selected.\n");
        return -1;
    }

    printf("Converting: %s\n", source_path);
    printf("  Type     : %s\n", is_mp4(source_path) ? "MP4 video" : "Audio");
    printf("  Duration : %d s\n", settings_get_duration());
    printf("  Volume   : %d %%\n", settings_get_volume());
    printf("  Fade     : %d ms\n", settings_get_fade());

    mkdir("/dev_hdd0/tmp", 0777);
    mkdir(output_dir, 0777);

    /*
     * Pipeline for MP4:
     * 1. Demux audio track from MP4
     * 2. Decode / resample / trim / volume / fade
     * 3. Encode to AC3 -> coldboot_stereo.ac3 + coldboot_multi.ac3
     * 4. (Optional) Extract frames -> coldboot.raf
     *
     * Full demux/encode needs libavcodec / ffmpeg port on PS3.
     * For now we stage the source and write metadata.
     */

    char dest[512];
    snprintf(dest, sizeof(dest), "%s/source%s",
             output_dir, is_mp4(source_path) ? ".mp4" : ".mp3");

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
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
        fwrite(buf, 1, n, out);

    fclose(in);
    fclose(out);

    char info[512];
    snprintf(info, sizeof(info), "%s/convert_info.txt", output_dir);
    FILE *f = fopen(info, "w");
    if (f) {
        fprintf(f, "BadCBA Conversion Info\n");
        fprintf(f, "Source  : %s\n", source_path);
        fprintf(f, "Format  : %s\n", is_mp4(source_path) ? "MP4" : "Audio");
        fprintf(f, "Duration: %d\n", settings_get_duration());
        fprintf(f, "Volume  : %d\n", settings_get_volume());
        fprintf(f, "Fade    : %d\n", settings_get_fade());
        fprintf(f, "Targets : coldboot_stereo.ac3, coldboot_multi.ac3\n");
        if (is_mp4(source_path))
            fprintf(f, "Optional: coldboot.raf from video frames\n");
        fprintf(f, "Note    : Full AC3/RAF encoding needs extra libraries.\n");
        fclose(f);
    }

    printf("Conversion step finished.\n");
    printf("Output: %s\n", output_dir);
    return 0;
}

int audio_export_to_usb(void)
{
    printf("Exporting files to USB...\n");

    const char *usb = "/dev_usb000/BadCBA_Coldboot";
    mkdir("/dev_usb000", 0777);
    mkdir(usb, 0777);

    char readme[512];
    snprintf(readme, sizeof(readme), "%s/README.txt", usb);
    FILE *f = fopen(readme, "w");
    if (f) {
        fprintf(f, "BadCBA - Custom Coldboot Export\n\n");
        fprintf(f, "Input: MP4 (preferred) or MP3\n\n");
        fprintf(f, "1. Select your MP4 in BadCBA on the PS3.\n");
        fprintf(f, "2. Convert - generated files appear in this folder.\n");
        fprintf(f, "3. Expected output files:\n");
        fprintf(f, "   - coldboot_stereo.ac3\n");
        fprintf(f, "   - coldboot_multi.ac3\n");
        fprintf(f, "   - coldboot.raf  (if video was used)\n\n");
        fprintf(f, "4. Copy with multiMAN / IrisMAN to:\n");
        fprintf(f, "   /dev_blind/vsh/resource/\n\n");
        fprintf(f, "Always keep a backup of the original files!\n");
        fclose(f);
    }

    printf("Export target: %s\n", usb);
    return 0;
}
