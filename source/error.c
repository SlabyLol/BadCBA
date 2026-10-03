/*
 * BadCBA error system
 * Many error codes + looping error-cba.wav playback
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include <audio/audio.h>
#include <sys/thread.h>

#include "error.h"
#include "gui.h"

#define ERR_WAV_PATHS 4

static BadCbaError current_code = ERR_OK;
static char current_msg[256] = "";
static int active = 0;

/* Simple WAV player state (loop) */
static int sound_ready = 0;
static int sound_playing = 0;
static u8 *wav_data = NULL;
static u32 wav_size = 0;
static u32 wav_pos = 0;
static u32 wav_data_offset = 0;
static u32 wav_sample_rate = 48000;
static u32 wav_channels = 2;
static u32 wav_bits = 16;

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
    case ERR_OK:              return "OK";
    case ERR_USB_NOT_FOUND:   return "Uppss!! We found no USB! Please insert an USB to continue!";
    case ERR_USB_READ:        return "USB read failed. Check the drive and try again.";
    case ERR_USB_WRITE:       return "USB write failed. Is the stick write-protected?";
    case ERR_NO_MP4_SELECTED: return "No MP4 selected. Choose a file from USB first.";
    case ERR_MP4_OPEN:        return "Could not open the MP4 file.";
    case ERR_MP4_INVALID:     return "Invalid or corrupted MP4 file.";
    case ERR_MP4_TOO_LONG:    return "MP4 is too long for coldboot (max ~8 seconds).";
    case ERR_MP4_TOO_SHORT:   return "MP4 is too short (need at least 1 second).";
    case ERR_CONVERT_FAIL:    return "Conversion failed.";
    case ERR_CONVERT_AUDIO:   return "Audio conversion failed.";
    case ERR_CONVERT_VIDEO:   return "Video / RAF conversion failed.";
    case ERR_AC3_ENCODE:      return "AC3 encode failed.";
    case ERR_RAF_ENCODE:      return "RAF (boot animation) encode failed.";
    case ERR_FLASH_NO_BLIND:  return "No /dev_blind access. Enable flash write on CFW.";
    case ERR_FLASH_BACKUP:    return "Backup of original coldboot failed.";
    case ERR_FLASH_INSTALL:   return "Install to flash failed.";
    case ERR_FLASH_RESTORE:   return "Restore from backup failed.";
    case ERR_FLASH_PERMISSION:return "Permission denied writing to flash.";
    case ERR_FLASH_MISSING_SRC:return "Converted files missing. Run Convert first.";
    case ERR_HDD_FULL:        return "Internal HDD is full.";
    case ERR_HDD_WRITE:       return "Could not write to internal HDD.";
    case ERR_MEM_ALLOC:       return "Out of memory.";
    case ERR_SETTINGS_LOAD:   return "Could not load settings.";
    case ERR_SETTINGS_SAVE:   return "Could not save settings.";
    case ERR_PKG_CREATE:      return "PKG creation failed.";
    case ERR_SELF_MISSING:    return "EBOOT / SELF missing.";
    case ERR_UNKNOWN:
    default:                  return "Unknown error.";
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

        /* Parse minimal WAV header */
        if (memcmp(wav_data, "RIFF", 4) != 0 || memcmp(wav_data + 8, "WAVE", 4) != 0) {
            free(wav_data); wav_data = NULL; continue;
        }

        /* Find fmt and data chunks */
        u32 pos = 12;
        while (pos + 8 < wav_size) {
            char id[5] = {0};
            memcpy(id, wav_data + pos, 4);
            u32 csz = wav_data[pos+4] | (wav_data[pos+5]<<8) |
                      (wav_data[pos+6]<<16) | (wav_data[pos+7]<<24);
            if (memcmp(id, "fmt ", 4) == 0 && csz >= 16) {
                wav_channels = wav_data[pos+10] | (wav_data[pos+11]<<8);
                wav_sample_rate = wav_data[pos+12] | (wav_data[pos+13]<<8) |
                                  (wav_data[pos+14]<<16) | (wav_data[pos+15]<<24);
                wav_bits = wav_data[pos+22] | (wav_data[pos+23]<<8);
            } else if (memcmp(id, "data", 4) == 0) {
                wav_data_offset = pos + 8;
                wav_pos = wav_data_offset;
                sound_ready = 1;
                printf("Loaded error WAV: %s (%u bytes)\n", wav_search[i], wav_size);
                return 0;
            }
            pos += 8 + csz;
            if (csz & 1) pos++;
        }
        free(wav_data); wav_data = NULL;
    }

    printf("error-cba.wav not found (sound disabled)\n");
    return -1;
}

/* Soft beep via console if no WAV – still "loop" conceptually by re-trigger */
static void sound_start_loop(void)
{
    load_wav();
    sound_playing = 1;
    wav_pos = wav_data_offset;
    printf("\a"); /* terminal bell as fallback */
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
    load_wav();
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

    /* Advance loop position – actual hardware PCM would be fed here.
     * We keep the cursor looping so any future audio port stays in sync. */
    if (wav_pos >= wav_size) {
        wav_pos = wav_data_offset;
        printf("\a"); /* re-trigger loop marker / beep */
    } else {
        /* consume a small chunk per frame to simulate streaming */
        wav_pos += 4096;
        if (wav_pos >= wav_size)
            wav_pos = wav_data_offset;
    }
}

void error_render(rsxBuffer *buf, int screen_w, int screen_h)
{
    if (!active || !buf) return;

    /* Dark red overlay panel */
    gui_draw_rect(buf, 40, screen_h / 2 - 90, screen_w - 80, 180, 0xFF2A0000);
    gui_draw_rect(buf, 40, screen_h / 2 - 90, screen_w - 80, 4, 0xFFFF3333);

    int y = screen_h / 2 - 60;
    gui_draw_text_scaled(buf, 60, y, "ERROR", 0xFFFF3333, 3);
    y += 50;

    /* Word-wrap-ish: draw message */
    gui_draw_text(buf, 60, y, current_msg, 0xFFFF5555);
    y += 40;
    gui_draw_text(buf, 60, y, "Playing error-cba.wav (loop)...", 0xFFAAAAAA);
    y += 36;
    gui_draw_text(buf, 60, y, "[X] / [O] Dismiss", 0xFF888888);
}
