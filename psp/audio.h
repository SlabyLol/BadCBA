#ifndef BADCBA_PSP_AUDIO_H
#define BADCBA_PSP_AUDIO_H

void audio_set_source(const char *path);
const char *audio_get_source(void);
int  audio_convert(void);
int  audio_export(void);

#endif
