#ifndef BADCBA_ERROR_H
#define BADCBA_ERROR_H

#include "rsxutil.h"

/*
 * BadCBA complete error catalog
 * Ranges:
 *   700-749  USB
 *   750-799  File system / paths
 *   800-849  Media / MP4
 *   850-899  Conversion / encode
 *   900-949  Flash / coldboot install
 *   950-979  HDD / storage
 *   980-989  Memory / system
 *   990-998  Settings / PKG / GUI
 *   999      Unknown
 */
typedef enum {
    ERR_OK = 0,

    /* USB 700-749 */
    ERR_USB_NOT_FOUND       = 726,
    ERR_USB_NOT_MOUNTED     = 700,
    ERR_USB_READ            = 727,
    ERR_USB_WRITE           = 728,
    ERR_USB_REMOVED         = 701,
    ERR_USB_FULL            = 702,
    ERR_USB_PROTECTED       = 703,
    ERR_USB_IO              = 704,
    ERR_USB_TIMEOUT         = 705,
    ERR_USB_BUSY            = 706,
    ERR_USB_PATH_INVALID    = 707,
    ERR_USB_NO_MEDIA_FILES  = 708,
    ERR_USB_ENUM            = 709,

    /* File system 750-799 */
    ERR_FS_OPEN             = 750,
    ERR_FS_CLOSE            = 751,
    ERR_FS_READ             = 752,
    ERR_FS_WRITE            = 753,
    ERR_FS_SEEK             = 754,
    ERR_FS_STAT             = 755,
    ERR_FS_MKDIR            = 756,
    ERR_FS_RMDIR            = 757,
    ERR_FS_UNLINK           = 758,
    ERR_FS_RENAME           = 759,
    ERR_FS_PERMISSION       = 760,
    ERR_FS_NOT_FOUND        = 761,
    ERR_FS_EXISTS           = 762,
    ERR_FS_NOT_DIR          = 763,
    ERR_FS_IS_DIR           = 764,
    ERR_FS_PATH_TOO_LONG    = 765,
    ERR_FS_NAME_INVALID     = 766,

    /* Media / MP4 800-849 */
    ERR_NO_MP4_SELECTED     = 801,
    ERR_MP4_OPEN            = 802,
    ERR_MP4_INVALID         = 803,
    ERR_MP4_TOO_LONG        = 804,
    ERR_MP4_TOO_SHORT       = 805,
    ERR_MP4_NO_AUDIO        = 806,
    ERR_MP4_NO_VIDEO        = 807,
    ERR_MP4_CODEC           = 808,
    ERR_MP4_CORRUPT         = 809,
    ERR_MP4_UNSUPPORTED     = 812,
    ERR_MP4_RESOLUTION      = 813,
    ERR_MP4_FRAMERATE       = 814,
    ERR_MP4_BITRATE         = 815,
    ERR_MP4_STREAM_END      = 816,
    ERR_MP3_INVALID         = 820,
    ERR_MEDIA_UNKNOWN       = 829,

    /* Conversion 850-899 */
    ERR_CONVERT_FAIL        = 810,
    ERR_CONVERT_AUDIO       = 811,
    ERR_CONVERT_VIDEO       = 850,
    ERR_CONVERT_TIMEOUT     = 851,
    ERR_CONVERT_CANCELLED   = 852,
    ERR_CONVERT_PARTIAL     = 853,
    ERR_AC3_ENCODE          = 820,
    ERR_AC3_STEREO          = 860,
    ERR_AC3_MULTI           = 861,
    ERR_AC3_SAMPLE_RATE     = 862,
    ERR_AC3_CHANNELS        = 863,
    ERR_RAF_ENCODE          = 821,
    ERR_RAF_FRAMES          = 870,
    ERR_RAF_SIZE            = 871,
    ERR_RAF_HEADER          = 872,
    ERR_FADE_INVALID        = 880,
    ERR_VOLUME_INVALID      = 881,
    ERR_DURATION_INVALID    = 882,

    /* Flash / coldboot 900-949 */
    ERR_FLASH_NO_BLIND      = 900,
    ERR_FLASH_BACKUP        = 901,
    ERR_FLASH_INSTALL       = 902,
    ERR_FLASH_RESTORE       = 903,
    ERR_FLASH_PERMISSION    = 904,
    ERR_FLASH_MISSING_SRC   = 905,
    ERR_FLASH_MOUNT         = 906,
    ERR_FLASH_UMOUNT        = 907,
    ERR_FLASH_VERIFY        = 908,
    ERR_FLASH_READONLY      = 909,
    ERR_FLASH_PATH          = 910,
    ERR_COLDBOOT_MISSING    = 920,
    ERR_COLDBOOT_CORRUPT    = 921,
    ERR_COLDBOOT_VERSION    = 922,
    ERR_BACKUP_MISSING      = 930,
    ERR_BACKUP_CORRUPT      = 931,
    ERR_BACKUP_WRITE        = 932,

    /* HDD 950-979 */
    ERR_HDD_FULL            = 950,
    ERR_HDD_WRITE           = 951,
    ERR_HDD_READ            = 952,
    ERR_HDD_MOUNT           = 953,
    ERR_HDD_PATH            = 954,
    ERR_TMP_CREATE          = 960,
    ERR_TMP_CLEAN           = 961,

    /* Memory / system 980-989 */
    ERR_MEM_ALLOC           = 980,
    ERR_MEM_ALIGN           = 981,
    ERR_RSX_INIT            = 982,
    ERR_RSX_FLIP            = 983,
    ERR_PAD_INIT            = 984,
    ERR_SYSMODULE           = 985,
    ERR_THREAD              = 986,

    /* Settings / PKG / GUI 990-998 */
    ERR_SETTINGS_LOAD       = 990,
    ERR_SETTINGS_SAVE       = 991,
    ERR_SETTINGS_RANGE      = 992,
    ERR_PKG_CREATE          = 993,
    ERR_PKG_SFO             = 994,
    ERR_SELF_MISSING        = 995,
    ERR_ICON_MISSING        = 996,
    ERR_GUI_STATE           = 997,
    ERR_WAV_MISSING         = 998,

    ERR_UNKNOWN             = 999
} BadCbaError;

void error_init(void);
void error_shutdown(void);
void error_raise(BadCbaError code);
void error_clear(void);
int          error_is_active(void);
BadCbaError  error_get_code(void);
const char  *error_get_message(void);
void error_update(void);
void error_render(rsxBuffer *buf, int screen_w, int screen_h);

#endif
