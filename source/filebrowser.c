/*
 * BadCBA - USB / filesystem browser
 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "filebrowser.h"

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

static int is_mp3_file(const char *name)
{
    size_t len = strlen(name);
    if (len < 4) return 0;
    const char *ext = name + len - 4;
    return (strcasecmp(ext, ".mp3") == 0);
}

static void scan_dir(void)
{
    entry_count = 0;
    selected = 0;

    DIR *dir = opendir(current_dir);
    if (!dir) {
        printf("Cannot open directory: %s\n", current_dir);
        return;
    }

    /* Add ".." entry */
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
        if (stat(full, &st) != 0)
            continue;

        int isdir = S_ISDIR(st.st_mode);
        if (!isdir && !is_mp3_file(de->d_name))
            continue;

        strncpy(entries[entry_count].name, de->d_name, sizeof(entries[0].name) - 1);
        entries[entry_count].is_dir = isdir;
        entry_count++;
    }

    closedir(dir);
}

void filebrowser_open(const char *path)
{
    strncpy(current_dir, path, sizeof(current_dir) - 1);
    current_dir[sizeof(current_dir) - 1] = '\0';
    scan_dir();
}

void filebrowser_up(void)
{
    if (selected > 0)
        selected--;
}

void filebrowser_down(void)
{
    if (selected < entry_count - 1)
        selected++;
}

void filebrowser_enter(void)
{
    if (!entries[selected].is_dir)
        return;

    if (strcmp(entries[selected].name, "..") == 0) {
        /* go up */
        char *slash = strrchr(current_dir, '/');
        if (slash && slash != current_dir) {
            *slash = '\0';
        } else {
            strcpy(current_dir, "/");
        }
    } else {
        char newpath[MAX_PATH];
        snprintf(newpath, sizeof(newpath), "%s/%s", current_dir, entries[selected].name);
        strncpy(current_dir, newpath, sizeof(current_dir) - 1);
    }
    scan_dir();
}

int filebrowser_is_dir(void)
{
    return entries[selected].is_dir;
}

int filebrowser_is_mp3(void)
{
    return !entries[selected].is_dir && is_mp3_file(entries[selected].name);
}

const char *filebrowser_get_path(void)
{
    snprintf(selected_path, sizeof(selected_path), "%s/%s",
             current_dir, entries[selected].name);
    return selected_path;
}

const char *filebrowser_get_current_dir(void)
{
    return current_dir;
}

void filebrowser_render(void)
{
    int start = selected - 8;
    if (start < 0) start = 0;

    for (int i = start; i < entry_count && i < start + 16; i++) {
        printf("  %s %s%s\n",
               (i == selected) ? ">" : " ",
               entries[i].name,
               entries[i].is_dir ? "/" : "");
    }
}
