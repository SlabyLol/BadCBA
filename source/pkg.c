/*
 * BadCBA - PKG generation helpers
 *
 * Creating a full NPDRM PKG on-console is limited.
 * This module prepares the directory structure and metadata.
 * Final packaging can be finished on PC or with additional tools.
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "pkg.h"
#include "settings.h"
#include "audio.h"

int pkg_create_coldboot(void)
{
    printf("Creating coldboot package structure...\n");

    const char *base = "/dev_hdd0/tmp/badcba_pkg";
    mkdir("/dev_hdd0/tmp", 0777);
    mkdir(base, 0777);
    mkdir("/dev_hdd0/tmp/badcba_pkg/USRDIR", 0777);

    /* Write a simple PARAM.SFO-like text for reference */
    char sfo_path[256];
    snprintf(sfo_path, sizeof(sfo_path), "%s/package_info.txt", base);

    FILE *f = fopen(sfo_path, "w");
    if (f) {
        fprintf(f, "BadCBA Generated Coldboot Package\n");
        fprintf(f, "Title      : %s\n", settings_get_pkg_name());
        fprintf(f, "Content ID : %s\n", settings_get_content_id());
        fprintf(f, "Source MP3 : %s\n", audio_get_source());
        fprintf(f, "Duration   : %d s\n", settings_get_duration());
        fprintf(f, "Volume     : %d %%\n", settings_get_volume());
        fprintf(f, "\nPlace the following files in USRDIR or use an installer PKG:\n");
        fprintf(f, "  coldboot_stereo.ac3\n");
        fprintf(f, "  coldboot_multi.ac3\n");
        fprintf(f, "  (optional) coldboot.raf\n");
        fclose(f);
    }

    printf("Package structure created at %s\n", base);
    printf("Note: Full on-console PKG signing requires additional tools.\n");
    printf("Prefer exporting the AC3 files to USB and installing manually.\n");

    return 0;
}
