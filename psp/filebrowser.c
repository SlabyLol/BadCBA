/*
 * BadCBA PSP – Memory Stick browser + boot prepare
 */

#include <pspkernel.h>
#include <pspiofilemgr.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "filebrowser.h"

#define MAX_ENTRIES 256
#define MAX_NAME    256

typedef struct {
    char name[MAX_NAME];
    int is_dir;
} Entry;

static Entry entries[MAX_ENTRIES];
static int entry_count = 0;
static int cursor = 0;
static char cwd[512] = "ms0:/";
static char selected_path[512] = "";

static int str_ends_with_ci(const char *s, const char *suf)
{
    size_t ls = strlen(s), lf = strlen(suf);
    if (ls < lf) return 0;
    for (size_t i = 0; i < lf; i++) {
        if (tolower((unsigned char)s[ls - lf + i]) != tolower((unsigned char)suf[i]))
            return 0;
    }
    return 1;
}

int filebrowser_ms_present(void)
{
    SceUID d = sceIoDopen("ms0:/");
    if (d >= 0) {
        sceIoDclose(d);
        return 1;
    }
    return 0;
}

void filebrowser_init(const char *root)
{
    strncpy(cwd, root, sizeof(cwd) - 1);
    entry_count = 0;
    cursor = 0;
}

static void scan(void)
{
    entry_count = 0;
    cursor = 0;
    SceUID d = sceIoDopen(cwd);
    if (d < 0) return;

    SceIoDirent de;
    memset(&de, 0, sizeof(de));
    while (sceIoDread(d, &de) > 0 && entry_count < MAX_ENTRIES) {
        if (strcmp(de.d_name, ".") == 0) continue;
        Entry *e = &entries[entry_count];
        strncpy(e->name, de.d_name, MAX_NAME - 1);
        e->name[MAX_NAME - 1] = '\0';
        e->is_dir = FIO_S_ISDIR(de.d_stat.st_mode);
        entry_count++;
        memset(&de, 0, sizeof(de));
    }
    sceIoDclose(d);
}

void filebrowser_open(const char *path)
{
    strncpy(cwd, path, sizeof(cwd) - 1);
    cwd[sizeof(cwd) - 1] = '\0';
    scan();
}

void filebrowser_up(void)
{
    if (entry_count == 0) return;
    cursor = (cursor - 1 + entry_count) % entry_count;
}

void filebrowser_down(void)
{
    if (entry_count == 0) return;
    cursor = (cursor + 1) % entry_count;
}

void filebrowser_enter(void)
{
    if (entry_count == 0) return;
    Entry *e = &entries[cursor];
    if (!e->is_dir) return;

    if (strcmp(e->name, "..") == 0) {
        /* go up one level */
        char *p = strrchr(cwd, '/');
        if (p && p != cwd + strlen(cwd) - 1) {
            *p = '\0';
            p = strrchr(cwd, '/');
            if (p) *(p + 1) = '\0';
            else strcpy(cwd, "ms0:/");
        }
    } else {
        size_t n = strlen(cwd);
        if (n > 0 && cwd[n - 1] != '/')
            strncat(cwd, "/", sizeof(cwd) - n - 1);
        strncat(cwd, e->name, sizeof(cwd) - strlen(cwd) - 1);
        strncat(cwd, "/", sizeof(cwd) - strlen(cwd) - 1);
    }
    scan();
}

int filebrowser_is_dir(void)
{
    if (entry_count == 0) return 0;
    return entries[cursor].is_dir;
}

int filebrowser_is_media(void)
{
    if (entry_count == 0 || entries[cursor].is_dir) return 0;
    const char *n = entries[cursor].name;
    return str_ends_with_ci(n, ".mp4") || str_ends_with_ci(n, ".mp3") ||
           str_ends_with_ci(n, ".pmf") || str_ends_with_ci(n, ".avi") ||
           str_ends_with_ci(n, ".aa3") || str_ends_with_ci(n, ".oma");
}

void filebrowser_select_current(void)
{
    if (!filebrowser_is_media()) return;
    snprintf(selected_path, sizeof(selected_path), "%s%s",
             cwd, entries[cursor].name);
}

const char *filebrowser_cwd(void)
{
    return cwd;
}

void filebrowser_draw_list(void)
{
    int start = cursor > 8 ? cursor - 8 : 0;
    for (int i = start; i < entry_count && i < start + 12; i++) {
        if (i == cursor)
            pspDebugScreenSetTextColor(0x00CEC9);
        else
            pspDebugScreenSetTextColor(0xCCCCCC);
        pspDebugScreenPrintf(" %c %s%s\n",
                             i == cursor ? '>' : ' ',
                             entries[i].name,
                             entries[i].is_dir ? "/" : "");
    }
    if (selected_path[0]) {
        pspDebugScreenSetTextColor(0x00B894);
        pspDebugScreenPrintf("\n Selected: %s\n", selected_path);
    }
}

void filebrowser_prepare_boot(int duration_s, int volume)
{
    sceIoMkdir("ms0:/BadCBA_Boot", 0777);
    SceUID fd = sceIoOpen("ms0:/BadCBA_Boot/README.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd < 0) return;

    char buf[512];
    int n = snprintf(buf, sizeof(buf),
        "BadCBA PSP – BadCustomBootAnimation\n"
        "Source : %s\n"
        "Duration: %d s\n"
        "Volume  : %d %%\n\n"
        "Place converted boot files here / follow CFW docs\n"
        "to replace PSP boot animation / sound.\n",
        selected_path[0] ? selected_path : "(none)",
        duration_s, volume);
    sceIoWrite(fd, buf, n);
    sceIoClose(fd);

    if (selected_path[0]) {
        char dest[560];
        snprintf(dest, sizeof(dest), "ms0:/BadCBA_Boot/source_media.bin");
        SceUID in = sceIoOpen(selected_path, PSP_O_RDONLY, 0);
        SceUID out = sceIoOpen(dest, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
        if (in >= 0 && out >= 0) {
            char chunk[4096];
            int r;
            while ((r = sceIoRead(in, chunk, sizeof(chunk))) > 0)
                sceIoWrite(out, chunk, r);
        }
        if (in >= 0) sceIoClose(in);
        if (out >= 0) sceIoClose(out);
    }
}
