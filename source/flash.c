/*
 * BadCBA - Flash install / backup for custom coldboot animation
 *
 * Writes to /dev_blind/vsh/resource/ (CFW writable flash mirror).
 * Always backs up originals first when settings say so.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include "flash.h"
#include "settings.h"
#include "audio.h"

static char status_msg[256] = "";
static char found_usb[64] = "";

static const char *usb_candidates[] = {
    "/dev_usb000",
    "/dev_usb001",
    "/dev_usb002",
    "/dev_usb003",
    "/dev_usb004",
    "/dev_usb005",
    "/dev_usb006",
    NULL
};

static const char *flash_res = "/dev_blind/vsh/resource";
static const char *backup_dir = "/dev_hdd0/game/BCBA00001/USRDIR/backup_coldboot";
static const char *work_dir   = "/dev_hdd0/tmp/badcba";

static const char *coldboot_files[] = {
    "coldboot.raf",
    "coldboot_stereo.ac3",
    "coldboot_multi.ac3",
    NULL
};

static void set_status(const char *msg)
{
    strncpy(status_msg, msg, sizeof(status_msg) - 1);
    status_msg[sizeof(status_msg) - 1] = '\0';
    printf("%s\n", status_msg);
}

const char *flash_get_status(void)
{
    return status_msg[0] ? status_msg : "";
}

int flash_usb_present(void)
{
    return flash_find_usb() != NULL;
}

const char *flash_find_usb(void)
{
    found_usb[0] = '\0';
    for (int i = 0; usb_candidates[i]; i++) {
        DIR *d = opendir(usb_candidates[i]);
        if (d) {
            closedir(d);
            strncpy(found_usb, usb_candidates[i], sizeof(found_usb) - 1);
            return found_usb;
        }
    }
    return NULL;
}

static int copy_file(const char *src, const char *dst)
{
    FILE *in = fopen(src, "rb");
    if (!in) return -1;
    FILE *out = fopen(dst, "wb");
    if (!out) { fclose(in); return -1; }

    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {
            fclose(in); fclose(out);
            return -1;
        }
    }
    fclose(in);
    fclose(out);
    return 0;
}

static int ensure_dir(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
        return 0;
    return mkdir(path, 0777);
}

int flash_backup_originals(void)
{
    set_status("Backing up original coldboot files...");

    ensure_dir("/dev_hdd0/game");
    ensure_dir("/dev_hdd0/game/BCBA00001");
    ensure_dir("/dev_hdd0/game/BCBA00001/USRDIR");
    ensure_dir(backup_dir);

    int ok = 0;
    for (int i = 0; coldboot_files[i]; i++) {
        char src[512], dst[512];
        snprintf(src, sizeof(src), "%s/%s", flash_res, coldboot_files[i]);
        snprintf(dst, sizeof(dst), "%s/%s", backup_dir, coldboot_files[i]);

        if (access(src, R_OK) != 0) {
            /* also try /dev_flash if blind not mounted yet */
            snprintf(src, sizeof(src), "/dev_flash/vsh/resource/%s", coldboot_files[i]);
        }

        if (copy_file(src, dst) == 0) {
            ok++;
            printf("  backed up %s\n", coldboot_files[i]);
        }
    }

    if (ok == 0) {
        set_status("Backup failed (no flash access?). Enable /dev_blind.");
        return -1;
    }

    char msg[128];
    snprintf(msg, sizeof(msg), "Backup OK (%d files) -> USRDIR/backup_coldboot", ok);
    set_status(msg);
    return 0;
}

int flash_install_to_vsh(void)
{
    set_status("Installing custom coldboot to flash...");

    /* Ensure USB was used for source – user flow requires USB for MP4 */
    if (!flash_usb_present()) {
        set_status("Uppss!! We found no USB! Please insert an USB to continue! (err:726)");
        return -726;
    }

    if (settings_get_backup()) {
        if (flash_backup_originals() != 0) {
            /* continue only if user disabled strict backup – still try install */
            printf("Warning: backup incomplete, continuing install...\n");
        }
    }

    /* Target paths */
    ensure_dir("/dev_blind");
    ensure_dir("/dev_blind/vsh");
    ensure_dir("/dev_blind/vsh/resource");

    int installed = 0;
    for (int i = 0; coldboot_files[i]; i++) {
        char src[512], dst[512];
        snprintf(src, sizeof(src), "%s/%s", work_dir, coldboot_files[i]);
        snprintf(dst, sizeof(dst), "%s/%s", flash_res, coldboot_files[i]);

        if (access(src, R_OK) != 0) {
            printf("  skip missing %s\n", coldboot_files[i]);
            continue;
        }

        if (copy_file(src, dst) == 0) {
            installed++;
            printf("  installed %s\n", coldboot_files[i]);
        } else {
            printf("  FAILED %s (need CFW + /dev_blind)\n", coldboot_files[i]);
        }
    }

    if (installed == 0) {
        set_status("Install failed. Convert first + enable flash write (/dev_blind).");
        return -1;
    }

    char msg[160];
    snprintf(msg, sizeof(msg),
             "Installed %d file(s) to flash. Reboot to see your animation!",
             installed);
    set_status(msg);
    return 0;
}

int flash_restore_backup(void)
{
    set_status("Restoring original coldboot from backup...");

    ensure_dir("/dev_blind/vsh/resource");

    int ok = 0;
    for (int i = 0; coldboot_files[i]; i++) {
        char src[512], dst[512];
        snprintf(src, sizeof(src), "%s/%s", backup_dir, coldboot_files[i]);
        snprintf(dst, sizeof(dst), "%s/%s", flash_res, coldboot_files[i]);
        if (copy_file(src, dst) == 0)
            ok++;
    }

    if (ok == 0) {
        set_status("Restore failed – no backup found or no flash write.");
        return -1;
    }

    char msg[128];
    snprintf(msg, sizeof(msg), "Restored %d original file(s). Reboot PS3.", ok);
    set_status(msg);
    return 0;
}
