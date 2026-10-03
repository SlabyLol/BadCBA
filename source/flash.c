/*
 * BadCBA - Flash install / backup
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
#include "error.h"

static char status_msg[256] = "";
static char found_usb[64] = "";

static const char *usb_candidates[] = {
    "/dev_usb000", "/dev_usb001", "/dev_usb002", "/dev_usb003",
    "/dev_usb004", "/dev_usb005", "/dev_usb006", NULL
};

static const char *flash_res = "/dev_blind/vsh/resource";
static const char *backup_dir = "/dev_hdd0/game/BCBA00001/USRDIR/backup_coldboot";
static const char *work_dir   = "/dev_hdd0/tmp/badcba";

static const char *coldboot_files[] = {
    "coldboot.raf", "coldboot_stereo.ac3", "coldboot_multi.ac3", NULL
};

static void set_status(const char *msg)
{
    strncpy(status_msg, msg, sizeof(status_msg) - 1);
    status_msg[sizeof(status_msg) - 1] = '\0';
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
    fclose(in); fclose(out);
    return 0;
}

static int ensure_dir(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) return 0;
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
        if (access(src, R_OK) != 0)
            snprintf(src, sizeof(src), "/dev_flash/vsh/resource/%s", coldboot_files[i]);
        if (copy_file(src, dst) == 0) ok++;
    }

    if (ok == 0) {
        error_raise(ERR_FLASH_BACKUP);
        set_status(error_get_message());
        return -1;
    }
    snprintf(status_msg, sizeof(status_msg), "Backup OK (%d files)", ok);
    return 0;
}

int flash_install_to_vsh(void)
{
    if (!flash_usb_present()) {
        error_raise(ERR_USB_NOT_FOUND);
        set_status(error_get_message());
        return -726;
    }

    if (settings_get_backup())
        flash_backup_originals();

    if (access("/dev_blind", F_OK) != 0) {
        error_raise(ERR_FLASH_NO_BLIND);
        set_status(error_get_message());
        return -1;
    }

    ensure_dir("/dev_blind/vsh");
    ensure_dir("/dev_blind/vsh/resource");

    int installed = 0, missing = 0;
    for (int i = 0; coldboot_files[i]; i++) {
        char src[512], dst[512];
        snprintf(src, sizeof(src), "%s/%s", work_dir, coldboot_files[i]);
        snprintf(dst, sizeof(dst), "%s/%s", flash_res, coldboot_files[i]);
        if (access(src, R_OK) != 0) { missing++; continue; }
        if (copy_file(src, dst) == 0) installed++;
        else {
            error_raise(ERR_FLASH_PERMISSION);
            set_status(error_get_message());
            return -1;
        }
    }

    if (installed == 0) {
        error_raise(missing ? ERR_FLASH_MISSING_SRC : ERR_FLASH_INSTALL);
        set_status(error_get_message());
        return -1;
    }

    snprintf(status_msg, sizeof(status_msg),
             "Installed %d file(s). Reboot to see your animation!", installed);
    return 0;
}

int flash_restore_backup(void)
{
    if (access("/dev_blind", F_OK) != 0) {
        error_raise(ERR_FLASH_NO_BLIND);
        set_status(error_get_message());
        return -1;
    }
    ensure_dir("/dev_blind/vsh/resource");

    int ok = 0;
    for (int i = 0; coldboot_files[i]; i++) {
        char src[512], dst[512];
        snprintf(src, sizeof(src), "%s/%s", backup_dir, coldboot_files[i]);
        snprintf(dst, sizeof(dst), "%s/%s", flash_res, coldboot_files[i]);
        if (copy_file(src, dst) == 0) ok++;
    }
    if (ok == 0) {
        error_raise(ERR_FLASH_RESTORE);
        set_status(error_get_message());
        return -1;
    }
    snprintf(status_msg, sizeof(status_msg), "Restored %d file(s). Reboot.", ok);
    return 0;
}
