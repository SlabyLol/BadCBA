#ifndef BADCBA_PSP_ERROR_H
#define BADCBA_PSP_ERROR_H

typedef enum {
    ERR_OK = 0,
    ERR_MS_NOT_FOUND = 726,
    ERR_MS_READ = 727,
    ERR_MS_WRITE = 728,
    ERR_NO_MEDIA = 801,
    ERR_MEDIA_OPEN = 802,
    ERR_MEDIA_INVALID = 803,
    ERR_CONVERT = 810,
    ERR_FLASH_BACKUP = 901,
    ERR_FLASH_INSTALL = 902,
    ERR_FLASH_RESTORE = 903,
    ERR_FLASH_MISSING = 905,
    ERR_SETTINGS = 990,
    ERR_UNKNOWN = 999
} BadCbaError;

void error_init(void);
void error_shutdown(void);
void error_raise(BadCbaError code);
void error_clear(void);          /* hide overlay only */
int  error_is_active(void);
int  error_sound_playing(void);
const char *error_get_message(void);
void error_update(void);
void error_draw(void);           /* debug-screen overlay */

#endif
