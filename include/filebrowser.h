#ifndef BADCBA_FILEBROWSER_H
#define BADCBA_FILEBROWSER_H

void filebrowser_open(const char *path);
void filebrowser_up(void);
void filebrowser_down(void);
void filebrowser_enter(void);
int  filebrowser_is_dir(void);
int  filebrowser_is_mp3(void);
const char *filebrowser_get_path(void);
const char *filebrowser_get_current_dir(void);
void filebrowser_render(void);

#endif /* BADCBA_FILEBROWSER_H */
