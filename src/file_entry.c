#include "file_entry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

char *build_path(const char *dir, const char *name) {
    if (dir == NULL || *dir == '\0') {
        char *res = malloc(strlen(name) + 1);
        if (res) strcpy(res, name);
        return res;
    }

    size_t dlen = strlen(dir);
    size_t nlen = strlen(name);
    bool has_slash = (dir[dlen - 1] == '/');

    size_t total_len = dlen + (has_slash ? 0 : 1) + nlen + 1;
    char *res = malloc(total_len);
    if (!res) return NULL;

    if (has_slash) {
        snprintf(res, total_len, "%s%s", dir, name);
    } else {
        snprintf(res, total_len, "%s/%s", dir, name);
    }

    return res;
}

FileEntry *create_file_entry(const char *name, const char *path) {
    FileEntry *entry = malloc(sizeof(FileEntry));
    if (!entry) return NULL;

    memset(entry, 0, sizeof(FileEntry));

    entry->name = malloc(strlen(name) + 1);
    if (entry->name) {
        strcpy(entry->name, name);
    }

    const char *target_path = path ? path : name;
    entry->path = malloc(strlen(target_path) + 1);
    if (entry->path) {
        strcpy(entry->path, target_path);
    }

    if (lstat(entry->path, &entry->st) == -1) {
        entry->has_stat = false;
        entry->error_code = errno;
    } else {
        entry->has_stat = true;
        entry->error_code = 0;

        if (S_ISLNK(entry->st.st_mode)) {
            size_t bufsize = entry->st.st_size > 0 ? (size_t)entry->st.st_size + 1 : 1024;
            char *buf = malloc(bufsize);
            if (buf) {
                ssize_t n = readlink(entry->path, buf, bufsize - 1);
                if (n >= 0) {
                    buf[n] = '\0';
                    entry->link_target = buf;
                } else {
                    free(buf);
                    entry->link_target = NULL;
                }
            }
        }
    }

    return entry;
}

void free_file_entry(FileEntry *entry) {
    if (!entry) return;
    if (entry->name) free(entry->name);
    if (entry->path) free(entry->path);
    if (entry->link_target) free(entry->link_target);
    free(entry);
}

void free_file_entries(FileEntry **entries, size_t count) {
    if (!entries) return;
    for (size_t i = 0; i < count; i++) {
        free_file_entry(entries[i]);
    }
    free(entries);
}
