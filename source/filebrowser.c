/*
 * BadCBA - USB / filesystem browser (MP4 + MP3)
 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "filebrowser.h"
#include "gui.h"

#define MAX_ENTRIES 256
#define MAX_PATH    512

typedef struct {
    char name[256];
    int  is_dir;
} DirEntry;

static char current_dir[MAX_PATH] = "/dev_usb000";
static DirEntry entries[MAX_ENTRIES];
static int entry_count = 0;
static int selected = 0;
static char selected_path[MAX_PATH];

static int is_media_file(const char *name)
{
    size_t len = strlen(name);
    if (len < 4) return 0;
    const char *ext = name + len - 4;
    if (strcasecmp(ext, ".mp4") == 0) return 1;
    if (strcasecmp(ext, ".mp3") == 0) return 1;
    if (len >= 5) {
        const char *ext5 = name + len - 5;
        if (strcasecmp(ext5, ".m4v") == 0) return 1;
    }
    return 0;
}

static void scan_dir(void)
{
    entry_count = 0;
    selected = 0;

    DIR *dir = opendir(current_dir);
    if (!dir) return;

    strcpy(entries[entry_count].name, "..");
    entries[entry_count].is_dir = 1;
    entry_count++;

    struct dirent *de;
    while ((de = readdir(dir)) != NULL && entry_count < MAX_ENTRIES) {
        if (de->d_name[0] == '.' && strcmp(de->d_name, "..") != 0)
            continue;

        char full[MAX_PATH];
        snprintf(full, sizeof(full), "%s/%s", current_dir, de->d_name);

        struct stat st;
        if (stat(full, &st) != 0) continue;

        int isdir = S_ISDIR(st.st_mode);
        if (!isdir && !is_media_file(de->d_name)) continue;

        strncpy(entries[entry_count].name, de->d_name, 255);
        entries[entry_count].is_dir = isdir;
        entry_count++;
    }
    closedir(dir);
}

void filebrowser_open(const char *path)
{
    strncpy(current_dir, path, MAX_PATH - 1);
    scan_dir();
}

void filebrowser_up(void)
{
    if (selected > 0) selected--;
}

void filebrowser_down(void)
{
    if (selected < entry_count - 1) selected++;
}

void filebrowser_enter(void)
{
    if (!entries[selected].is_dir) return;

    if (strcmp(entries[selected].name, "..") == 0) {
        char *slash = strrchr(current_dir, '/');
        if (slash && slash != current_dir) *slash = '\0';
        else strcpy(current_dir, "/");
    } else {
        char newpath[MAX_PATH];
        snprintf(newpath, sizeof(newpath), "%s/%s", current_dir, entries[selected].name);
        strncpy(current_dir, newpath, MAX_PATH - 1);
    }
    scan_dir();
}

int filebrowser_is_dir(void) { return entries[selected].is_dir; }
int filebrowser_is_media(void) { return !entries[selected].is_dir && is_media_file(entries[selected].name); }

const char *filebrowser_get_path(void)
{
    snprintf(selected_path, sizeof(selected_path), "%s/%s", current_dir, entries[selected].name);
    return selected_path;
}

const char *filebrowser_get_current_dir(void) { return current_dir; }

void filebrowser_render_gui(rsxBuffer *buf, int x, int y, int max_w)
{
    int start = selected - 6;
    if (start < 0) start = 0;

    for (int i = start; i < entry_count && i < start + 12; i++) {
        char line[300];
        snprintf(line, sizeof(line), "%s%s", entries[i].name, entries[i].is_dir ? "/" : "");

        if (i == selected) {
            gui_draw_rect(buf, x - 10, y - 6, max_w, 32, 0xFF2D2D44);
            gui_draw_rect(buf, x - 10, y - 6, 5, 32, 0xFF6C5CE7);
            gui_draw_text(buf, x + 10, y, line, 0xFFEEEEF5);
        } else {
            gui_draw_text(buf, x + 10, y, line, 0xFF8888AA);
        }
        y += 36;
    }
}
