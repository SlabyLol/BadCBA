#ifndef BADCBA_SETTINGS_H
#define BADCBA_SETTINGS_H

void settings_load(void);
void settings_save(void);
void settings_render(void);
void settings_next(void);
void settings_prev(void);
void settings_adjust(int delta);

int  settings_get_duration(void);
int  settings_get_volume(void);
int  settings_get_fade(void);
int  settings_get_backup(void);
const char *settings_get_content_id(void);
const char *settings_get_pkg_name(void);

#endif /* BADCBA_SETTINGS_H */
