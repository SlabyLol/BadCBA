#ifndef BADCBA_PSP_FILEBROWSER_H
#define BADCBA_PSP_FILEBROWSER_H

void filebrowser_init(const char *root);
int  filebrowser_ms_present(void);
void filebrowser_open(const char *path);
void filebrowser_up(void);
void filebrowser_down(void);
void filebrowser_enter(void);
int  filebrowser_is_dir(void);
int  filebrowser_is_media(void);
void filebrowser_select_current(void);
const char *filebrowser_cwd(void);
const char *filebrowser_selected(void);
void filebrowser_draw_list(void);

#endif
