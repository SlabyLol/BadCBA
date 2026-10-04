#ifndef BADCBA_PSP_SETTINGS_H
#define BADCBA_PSP_SETTINGS_H

void settings_load(void);
void settings_save(void);
void settings_next(void);
void settings_prev(void);
void settings_adjust(int delta);
void settings_draw(void);

int settings_get_duration(void);
int settings_get_volume(void);
int settings_get_fade(void);
int settings_get_backup(void);

#endif
