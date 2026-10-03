/*
 * BadCBA – full error catalog + error-cba.wav loop
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ppu-types.h>

#include "error.h"
#include "gui.h"

static BadCbaError current_code = ERR_OK;
static char current_msg[320] = "";
static int active = 0;

static int sound_ready = 0;
static int sound_playing = 0;
static u8 *wav_data = NULL;
static u32 wav_size = 0;
static u32 wav_pos = 0;
static u32 wav_data_offset = 0;

static const char *wav_search[] = {
    "/dev_hdd0/game/BCBA00001/USRDIR/error-cba.wav",
    "/dev_hdd0/tmp/badcba/error-cba.wav",
    "error-cba.wav",
    "/dev_usb000/error-cba.wav",
    NULL
};

static const char *msg_for(BadCbaError code)
{
    switch (code) {
    case ERR_OK:                 return "OK";

    /* USB */
    case ERR_USB_NOT_FOUND:      return "Uppss!! We found no USB! Please insert an USB to continue!";
    case ERR_USB_NOT_MOUNTED:    return "USB device is not mounted.";
    case ERR_USB_READ:           return "USB read failed. Check the drive and try again.";
    case ERR_USB_WRITE:          return "USB write failed. Is the stick write-protected?";
    case ERR_USB_REMOVED:        return "USB was removed during operation.";
    case ERR_USB_FULL:           return "USB stick is full.";
    case ERR_USB_PROTECTED:      return "USB is write-protected.";
    case ERR_USB_IO:             return "USB I/O error.";
    case ERR_USB_TIMEOUT:        return "USB operation timed out.";
    case ERR_USB_BUSY:           return "USB device is busy.";
    case ERR_USB_PATH_INVALID:   return "Invalid path on USB.";
    case ERR_USB_NO_MEDIA_FILES: return "No MP4/MP3 files found on USB.";
    case ERR_USB_ENUM:           return "Could not list USB devices.";

    /* FS */
    case ERR_FS_OPEN:            return "Failed to open file.";
    case ERR_FS_CLOSE:           return "Failed to close file.";
    case ERR_FS_READ:            return "File read error.";
    case ERR_FS_WRITE:           return "File write error.";
    case ERR_FS_SEEK:            return "File seek error.";
    case ERR_FS_STAT:            return "Could not stat file.";
    case ERR_FS_MKDIR:           return "Could not create directory.";
    case ERR_FS_RMDIR:           return "Could not remove directory.";
    case ERR_FS_UNLINK:          return "Could not delete file.";
    case ERR_FS_RENAME:          return "Could not rename file.";
    case ERR_FS_PERMISSION:      return "File permission denied.";
    case ERR_FS_NOT_FOUND:       return "File or folder not found.";
    case ERR_FS_EXISTS:          return "File already exists.";
    case ERR_FS_NOT_DIR:         return "Path is not a directory.";
    case ERR_FS_IS_DIR:          return "Path is a directory, expected a file.";
    case ERR_FS_PATH_TOO_LONG:   return "Path is too long.";
    case ERR_FS_NAME_INVALID:    return "Invalid file name.";

    /* MP4 / media */
    case ERR_NO_MP4_SELECTED:    return "No MP4 selected. Choose a file from USB first.";
    case ERR_MP4_OPEN:           return "Could not open the MP4 file.";
    case ERR_MP4_INVALID:        return "Invalid or corrupted MP4 file.";
    case ERR_MP4_TOO_LONG:       return "MP4 is too long for coldboot (max ~8 seconds).";
    case ERR_MP4_TOO_SHORT:      return "MP4 is too short (need at least 1 second).";
    case ERR_MP4_NO_AUDIO:       return "MP4 has no audio track.";
    case ERR_MP4_NO_VIDEO:       return "MP4 has no video track.";
    case ERR_MP4_CODEC:          return "Unsupported MP4 codec.";
    case ERR_MP4_CORRUPT:        return "MP4 data is corrupt.";
    case ERR_MP4_UNSUPPORTED:    return "This MP4 format is not supported.";
    case ERR_MP4_RESOLUTION:     return "MP4 resolution is not suitable for coldboot.";
    case ERR_MP4_FRAMERATE:      return "MP4 frame rate is not supported.";
    case ERR_MP4_BITRATE:        return "MP4 bitrate is too high or invalid.";
    case ERR_MP4_STREAM_END:     return "Unexpected end of MP4 stream.";
    case ERR_MP3_INVALID:        return "Invalid MP3 file.";
    case ERR_MEDIA_UNKNOWN:      return "Unknown media type.";

    /* Convert */
    case ERR_CONVERT_FAIL:       return "Conversion failed.";
    case ERR_CONVERT_AUDIO:      return "Audio conversion failed.";
    case ERR_CONVERT_VIDEO:      return "Video conversion failed.";
    case ERR_CONVERT_TIMEOUT:    return "Conversion timed out.";
    case ERR_CONVERT_CANCELLED:  return "Conversion was cancelled.";
    case ERR_CONVERT_PARTIAL:    return "Conversion finished only partially.";
    case ERR_AC3_ENCODE:         return "AC3 encode failed.";
    case ERR_AC3_STEREO:         return "coldboot_stereo.ac3 encode failed.";
    case ERR_AC3_MULTI:          return "coldboot_multi.ac3 encode failed.";
    case ERR_AC3_SAMPLE_RATE:    return "Invalid sample rate for AC3.";
    case ERR_AC3_CHANNELS:       return "Invalid channel layout for AC3.";
    case ERR_RAF_ENCODE:         return "RAF (boot animation) encode failed.";
    case ERR_RAF_FRAMES:         return "Not enough frames for coldboot.raf.";
    case ERR_RAF_SIZE:           return "coldboot.raf size is invalid.";
    case ERR_RAF_HEADER:         return "Invalid RAF header.";
    case ERR_FADE_INVALID:       return "Invalid fade value.";
    case ERR_VOLUME_INVALID:     return "Invalid volume value.";
    case ERR_DURATION_INVALID:   return "Invalid duration value.";

    /* Flash */
    case ERR_FLASH_NO_BLIND:     return "No /dev_blind access. Enable flash write on CFW.";
    case ERR_FLASH_BACKUP:       return "Backup of original coldboot failed.";
    case ERR_FLASH_INSTALL:      return "Install to flash failed.";
    case ERR_FLASH_RESTORE:      return "Restore from backup failed.";
    case ERR_FLASH_PERMISSION:   return "Permission denied writing to flash.";
    case ERR_FLASH_MISSING_SRC:  return "Converted files missing. Run Convert first.";
    case ERR_FLASH_MOUNT:        return "Could not mount flash (/dev_blind).";
    case ERR_FLASH_UMOUNT:       return "Could not unmount flash.";
    case ERR_FLASH_VERIFY:       return "Flash write verify failed.";
    case ERR_FLASH_READONLY:     return "Flash is read-only.";
    case ERR_FLASH_PATH:         return "Invalid flash resource path.";
    case ERR_COLDBOOT_MISSING:   return "Coldboot files missing on flash.";
    case ERR_COLDBOOT_CORRUPT:   return "Coldboot files on flash are corrupt.";
    case ERR_COLDBOOT_VERSION:   return "Coldboot file version mismatch.";
    case ERR_BACKUP_MISSING:     return "No backup found to restore.";
    case ERR_BACKUP_CORRUPT:     return "Backup files are corrupt.";
    case ERR_BACKUP_WRITE:       return "Could not write backup.";

    /* HDD */
    case ERR_HDD_FULL:           return "Internal HDD is full.";
    case ERR_HDD_WRITE:          return "Could not write to internal HDD.";
    case ERR_HDD_READ:           return "Could not read from internal HDD.";
    case ERR_HDD_MOUNT:          return "HDD mount error.";
    case ERR_HDD_PATH:           return "Invalid HDD path.";
    case ERR_TMP_CREATE:         return "Could not create temp folder.";
    case ERR_TMP_CLEAN:          return "Could not clean temp files.";

    /* System */
    case ERR_MEM_ALLOC:          return "Out of memory.";
    case ERR_MEM_ALIGN:          return "Memory alignment error.";
    case ERR_RSX_INIT:           return "RSX / video init failed.";
    case ERR_RSX_FLIP:           return "Display flip failed.";
    case ERR_PAD_INIT:           return "Controller init failed.";
    case ERR_SYSMODULE:          return "System module load failed.";
    case ERR_THREAD:             return "Thread error.";

    /* App */
    case ERR_SETTINGS_LOAD:      return "Could not load settings.";
    case ERR_SETTINGS_SAVE:      return "Could not save settings.";
    case ERR_SETTINGS_RANGE:     return "Setting value out of range.";
    case ERR_PKG_CREATE:         return "PKG creation failed.";
    case ERR_PKG_SFO:            return "PARAM.SFO creation failed.";
    case ERR_SELF_MISSING:       return "EBOOT / SELF missing.";
    case ERR_ICON_MISSING:       return "ICON0.PNG missing.";
    case ERR_GUI_STATE:          return "Invalid GUI state.";
    case ERR_WAV_MISSING:        return "error-cba.wav missing (sound disabled).";

    case ERR_UNKNOWN:
    default:                     return "Unknown error.";
    }
}

static int load_wav(void)
{
    if (wav_data) return 0;

    for (int i = 0; wav_search[i]; i++) {
        FILE *f = fopen(wav_search[i], "rb");
        if (!f) continue;

        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        fseek(f, 0, SEEK_SET);
        if (sz < 44 || sz > 8 * 1024 * 1024) { fclose(f); continue; }

        wav_data = (u8 *)malloc((size_t)sz);
        if (!wav_data) { fclose(f); return -1; }
        if (fread(wav_data, 1, (size_t)sz, f) != (size_t)sz) {
            free(wav_data); wav_data = NULL; fclose(f); continue;
        }
        fclose(f);
        wav_size = (u32)sz;

        if (memcmp(wav_data, "RIFF", 4) != 0 || memcmp(wav_data + 8, "WAVE", 4) != 0) {
            free(wav_data); wav_data = NULL; continue;
        }

        u32 pos = 12;
        while (pos + 8 < wav_size) {
            char id[5] = {0};
            memcpy(id, wav_data + pos, 4);
            u32 csz = wav_data[pos+4] | (wav_data[pos+5]<<8) |
                      (wav_data[pos+6]<<16) | (wav_data[pos+7]<<24);
            if (memcmp(id, "data", 4) == 0) {
                wav_data_offset = pos + 8;
                wav_pos = wav_data_offset;
                sound_ready = 1;
                return 0;
            }
            pos += 8 + csz;
            if (csz & 1) pos++;
        }
        free(wav_data); wav_data = NULL;
    }
    return -1;
}

static void sound_start_loop(void)
{
    load_wav();
    sound_playing = 1;
    wav_pos = wav_data_offset;
}

static void sound_stop(void)
{
    sound_playing = 0;
    wav_pos = wav_data_offset;
}

void error_init(void)
{
    current_code = ERR_OK;
    active = 0;
    if (load_wav() != 0)
        printf("Note: error-cba.wav not loaded yet (will retry on error)\n");
}

void error_shutdown(void)
{
    sound_stop();
    if (wav_data) { free(wav_data); wav_data = NULL; }
    sound_ready = 0;
}

void error_raise(BadCbaError code)
{
    if (code == ERR_OK) {
        error_clear();
        return;
    }
    current_code = code;
    snprintf(current_msg, sizeof(current_msg), "%s (err:%d)",
             msg_for(code), (int)code);
    active = 1;
    sound_start_loop();
    printf("ERROR: %s\n", current_msg);
}

void error_clear(void)
{
    active = 0;
    current_code = ERR_OK;
    current_msg[0] = '\0';
    sound_stop();
}

int error_is_active(void) { return active; }
BadCbaError error_get_code(void) { return current_code; }
const char *error_get_message(void) { return current_msg; }

void error_update(void)
{
    if (!active || !sound_playing || !sound_ready || !wav_data)
        return;
    wav_pos += 4096;
    if (wav_pos >= wav_size)
        wav_pos = wav_data_offset;
}

void error_render(rsxBuffer *buf, int screen_w, int screen_h)
{
    if (!active || !buf) return;

    gui_draw_rect(buf, 40, screen_h / 2 - 100, screen_w - 80, 200, 0xFF2A0000);
    gui_draw_rect(buf, 40, screen_h / 2 - 100, screen_w - 80, 4, 0xFFFF3333);

    int y = screen_h / 2 - 70;
    gui_draw_text_scaled(buf, 60, y, "ERROR", 0xFFFF3333, 3);
    y += 48;
    gui_draw_text(buf, 60, y, current_msg, 0xFFFF5555);
    y += 40;
    gui_draw_text(buf, 60, y, "Playing error-cba.wav (loop)...", 0xFFAAAAAA);
    y += 36;
    gui_draw_text(buf, 60, y, "[X] / [O] Dismiss", 0xFF888888);
}
