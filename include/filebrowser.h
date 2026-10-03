#ifndef BADCBA_FILEBROWSER_H
#define BADCBA_FILEBROWSER_H

#include <rsx/rsx.h>

void filebrowser_open(const char *path);
void filebrowser_up(void);
void filebrowser_down(void);
void filebrowser_enter(void);
int  filebrowser_is_dir(void);
int  filebrowser_is_mp3(void);
const char *filebrowser_get_path(void);
const char *filebrowser_get_current_dir(void);
void filebrowser_render_gui(rsxBuffer *buf, int x, int y, int max_w);

#endif
