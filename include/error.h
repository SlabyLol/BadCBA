#ifndef BADCBA_ERROR_H
#define BADCBA_ERROR_H

#include "rsxutil.h"

/* BadCBA error codes */
typedef enum {
    ERR_OK                = 0,
    ERR_USB_NOT_FOUND     = 726,
    ERR_USB_READ          = 727,
    ERR_USB_WRITE         = 728,
    ERR_NO_MP4_SELECTED   = 801,
    ERR_MP4_OPEN          = 802,
    ERR_MP4_INVALID       = 803,
    ERR_MP4_TOO_LONG      = 804,
    ERR_MP4_TOO_SHORT     = 805,
    ERR_CONVERT_FAIL      = 810,
    ERR_CONVERT_AUDIO     = 811,
    ERR_CONVERT_VIDEO     = 812,
    ERR_AC3_ENCODE        = 820,
    ERR_RAF_ENCODE        = 821,
    ERR_FLASH_NO_BLIND    = 900,
    ERR_FLASH_BACKUP      = 901,
    ERR_FLASH_INSTALL     = 902,
    ERR_FLASH_RESTORE     = 903,
    ERR_FLASH_PERMISSION  = 904,
    ERR_FLASH_MISSING_SRC = 905,
    ERR_HDD_FULL          = 910,
    ERR_HDD_WRITE         = 911,
    ERR_MEM_ALLOC         = 920,
    ERR_SETTINGS_LOAD     = 930,
    ERR_SETTINGS_SAVE     = 931,
    ERR_PKG_CREATE        = 940,
    ERR_SELF_MISSING      = 950,
    ERR_UNKNOWN           = 999
} BadCbaError;

void error_init(void);
void error_shutdown(void);

/* Raise error: shows message + starts looping error-cba.wav */
void error_raise(BadCbaError code);

/* Clear error + stop sound */
void error_clear(void);

int          error_is_active(void);
BadCbaError  error_get_code(void);
const char  *error_get_message(void);

/* Call each frame – keeps WAV looping while error active */
void error_update(void);

/* Draw full-screen error overlay (red) */
void error_render(rsxBuffer *buf, int screen_w, int screen_h);

#endif
