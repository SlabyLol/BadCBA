/*
 * Error catalog + error-cba.wav loops until app exit
 */
#include <pspkernel.h>
#include <pspaudio.h>
#include <pspaudiolib.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"

static BadCbaError code = ERR_OK;
static char msg[256] = "";
static int overlay = 0;
static int sound_on = 0;

/* minimal WAV state */
static char *wav = NULL;
static int wav_size = 0;
static int wav_pos = 0;
static int wav_data = 0;
static int audio_ch = -1;

static const char *wav_paths[] = {
    "ms0:/PSP/GAME/BadCBA/error-cba.wav",
    "ms0:/error-cba.wav",
    "error-cba.wav",
    NULL
};

static const char *msg_for(BadCbaError c)
{
    switch (c) {
    case ERR_MS_NOT_FOUND: return "Uppss!! We found no Memory Stick! Please insert an MS to continue!";
    case ERR_MS_READ:      return "Memory Stick read failed.";
    case ERR_MS_WRITE:     return "Memory Stick write failed.";
    case ERR_NO_MEDIA:     return "No media selected.";
    case ERR_MEDIA_OPEN:   return "Could not open media file.";
    case ERR_MEDIA_INVALID:return "Invalid media file.";
    case ERR_CONVERT:      return "Conversion / prepare failed.";
    case ERR_FLASH_BACKUP: return "Backup failed.";
    case ERR_FLASH_INSTALL:return "Install failed.";
    case ERR_FLASH_RESTORE:return "Restore failed.";
    case ERR_FLASH_MISSING:return "Prepared files missing. Convert first.";
    case ERR_SETTINGS:     return "Settings error.";
    default:               return "Unknown error.";
    }
}

static int load_wav(void)
{
    if (wav) return 0;
    for (int i = 0; wav_paths[i]; i++) {
        SceUID fd = sceIoOpen(wav_paths[i], PSP_O_RDONLY, 0);
        if (fd < 0) continue;
        wav_size = sceIoLseek(fd, 0, PSP_SEEK_END);
        sceIoLseek(fd, 0, PSP_SEEK_SET);
        if (wav_size < 44 || wav_size > 4 * 1024 * 1024) { sceIoClose(fd); continue; }
        wav = (char *)malloc(wav_size);
        if (!wav) { sceIoClose(fd); return -1; }
        sceIoRead(fd, wav, wav_size);
        sceIoClose(fd);
        if (memcmp(wav, "RIFF", 4) != 0) { free(wav); wav = NULL; continue; }
        /* find data chunk */
        int pos = 12;
        while (pos + 8 < wav_size) {
            int csz = (unsigned char)wav[pos+4] | ((unsigned char)wav[pos+5]<<8) |
                      ((unsigned char)wav[pos+6]<<16) | ((unsigned char)wav[pos+7]<<24);
            if (memcmp(wav + pos, "data", 4) == 0) {
                wav_data = pos + 8;
                wav_pos = wav_data;
                return 0;
            }
            pos += 8 + csz;
            if (csz & 1) pos++;
        }
        free(wav); wav = NULL;
    }
    return -1;
}

void error_init(void)
{
    code = ERR_OK;
    overlay = 0;
    sound_on = 0;
    load_wav();
}

void error_shutdown(void)
{
    sound_on = 0;
    overlay = 0;
    if (audio_ch >= 0) {
        sceAudioChRelease(audio_ch);
        audio_ch = -1;
    }
    if (wav) { free(wav); wav = NULL; }
}

void error_raise(BadCbaError c)
{
    if (c == ERR_OK) return;
    code = c;
    snprintf(msg, sizeof(msg), "%s (err:%d)", msg_for(c), (int)c);
    overlay = 1;
    if (!sound_on) {
        load_wav();
        sound_on = 1;
        wav_pos = wav_data;
    }
}

void error_clear(void)
{
    overlay = 0; /* sound keeps going until exit */
}

int error_is_active(void) { return overlay; }
int error_sound_playing(void) { return sound_on; }
const char *error_get_message(void) { return msg; }

void error_update(void)
{
    if (!sound_on || !wav) return;
    wav_pos += 4096;
    if (wav_pos >= wav_size)
        wav_pos = wav_data;
}

void error_draw(void)
{
    if (!overlay) return;
    pspDebugScreenSetXY(0, 8);
    pspDebugScreenSetTextColor(0x0000FF);
    pspDebugScreenPrintf("\n  ERROR\n\n");
    pspDebugScreenPrintf("  %s\n\n", msg);
    pspDebugScreenSetTextColor(0xAAAAAA);
    pspDebugScreenPrintf("  error-cba.wav looping until exit...\n");
    pspDebugScreenPrintf("  [X]/[O] Hide message (sound continues)\n");
}
