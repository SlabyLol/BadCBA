#ifndef BADCBA_RSXUTIL_H
#define BADCBA_RSXUTIL_H

#include <ppu-types.h>
#include <rsx/gcm_sys.h>
#include <rsx/rsx.h>

/* Framebuffer description compatible with this PSL1GHT build */
typedef struct {
    u32 *ptr;       /* host-mapped pointer to framebuffer */
    u32 offset;     /* RSX offset */
    u32 width;
    u32 height;
    u32 pitch;      /* bytes per row */
} rsxBuffer;

int  rsxutil_init(u32 *out_width, u32 *out_height);
void rsxutil_flip(void);
rsxBuffer *rsxutil_get_current(void);
gcmContextData *rsxutil_get_context(void);
void rsxutil_clear(u32 color);
void rsxutil_finish(void);

#endif
