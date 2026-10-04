#include <pspkernel.h>
#include <pspiofilemgr.h>
#include <pspdebug.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "filebrowser.h"
#include "audio.h"

#define MAX_ENTRIES 256
#define MAX_NAME 256

typedef struct { char name[MAX_NAME]; int is_dir; } Entry;

static Entry entries[MAX_ENTRIES];
static int entry_count, cursor;
static char cwd[512] = "ms0:/";
static char selected_path[512] = "";

static int ends_ci(const char *s, const char *suf)
{
    size_t ls = strlen(s), lf = strlen(suf);
    if (ls < lf) return 0;
    for (size_t i = 0; i < lf; i++)
        if (tolower((unsigned char)s[ls-lf+i]) != tolower((unsigned char)suf[i]))
            return 0;
    return 1;
}

static int is_dir_mode(unsigned mode)
{
#ifdef FIO_S_ISDIR
    return FIO_S_ISDIR(mode);
#else
    return (mode & 0x1000) || (mode & 0x10);
#endif
}

int filebrowser_ms_present(void)
{
    SceUID d = sceIoDopen("ms0:/");
    if (d >= 0) { sceIoDclose(d); return 1; }
    return 0;
}

void filebrowser_init(const char *root)
{
    strncpy(cwd, root, sizeof(cwd)-1);
    entry_count = cursor = 0;
}

static void scan(void)
{
    entry_count = cursor = 0;
    SceUID d = sceIoDopen(cwd);
    if (d < 0) return;
    SceIoDirent de;
    memset(&de, 0, sizeof(de));
    while (sceIoDread(d, &de) > 0 && entry_count < MAX_ENTRIES) {
        if (!strcmp(de.d_name, ".")) continue;
        Entry *e = &entries[entry_count++];
        strncpy(e->name, de.d_name, MAX_NAME-1);
        e->name[MAX_NAME-1] = 0;
        e->is_dir = is_dir_mode(de.d_stat.st_mode);
        memset(&de, 0, sizeof(de));
    }
    sceIoDclose(d);
}

void filebrowser_open(const char *path)
{
    strncpy(cwd, path, sizeof(cwd)-1);
    cwd[sizeof(cwd)-1] = 0;
    scan();
}

void filebrowser_up(void)
{
    if (entry_count) cursor = (cursor - 1 + entry_count) % entry_count;
}
void filebrowser_down(void)
{
    if (entry_count) cursor = (cursor + 1) % entry_count;
}

void filebrowser_enter(void)
{
    if (!entry_count || !entries[cursor].is_dir) return;
    Entry *e = &entries[cursor];
    if (!strcmp(e->name, "..")) {
        char *p = strrchr(cwd, '/');
        if (p && p != cwd + strlen(cwd) - 1) {
            *p = 0;
            p = strrchr(cwd, '/');
            if (p) *(p+1) = 0; else strcpy(cwd, "ms0:/");
        }
    } else {
        size_t n = strlen(cwd);
        if (n && cwd[n-1] != '/') strncat(cwd, "/", sizeof(cwd)-n-1);
        strncat(cwd, e->name, sizeof(cwd)-strlen(cwd)-1);
        strncat(cwd, "/", sizeof(cwd)-strlen(cwd)-1);
    }
    scan();
}

int filebrowser_is_dir(void)
{
    return entry_count && entries[cursor].is_dir;
}

int filebrowser_is_media(void)
{
    if (!entry_count || entries[cursor].is_dir) return 0;
    const char *n = entries[cursor].name;
    return ends_ci(n,".mp4")||ends_ci(n,".mp3")||ends_ci(n,".pmf")||
           ends_ci(n,".avi")||ends_ci(n,".aa3")||ends_ci(n,".oma")||
           ends_ci(n,".at3")||ends_ci(n,".wav");
}

void filebrowser_select_current(void)
{
    if (!filebrowser_is_media()) return;
    snprintf(selected_path, sizeof(selected_path), "%s%s", cwd, entries[cursor].name);
    audio_set_source(selected_path);
}

const char *filebrowser_cwd(void) { return cwd; }
const char *filebrowser_selected(void) { return selected_path; }

void filebrowser_draw_list(void)
{
    int start = cursor > 8 ? cursor - 8 : 0;
    for (int i = start; i < entry_count && i < start + 12; i++) {
        pspDebugScreenSetTextColor(i == cursor ? 0x00CEC9 : 0xCCCCCC);
        pspDebugScreenPrintf(" %c %s%s\n", i == cursor ? '>' : ' ',
                             entries[i].name, entries[i].is_dir ? "/" : "");
    }
    if (selected_path[0]) {
        pspDebugScreenSetTextColor(0x00B894);
        pspDebugScreenPrintf("\n Selected: %s\n", selected_path);
    }
}
