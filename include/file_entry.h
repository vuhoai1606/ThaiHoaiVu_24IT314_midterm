#ifndef FILE_ENTRY_H
#define FILE_ENTRY_H

#include "compat.h"
#include <stdbool.h>

typedef struct {
    char *name;         /* Base name for display */
    char *path;         /* Full or relative path to file */
    struct stat st;     /* File status from lstat */
    bool has_stat;      /* True if lstat succeeded */
    int error_code;     /* errno if stat failed */
    char *link_target;  /* Target path if this is a symlink */
} FileEntry;

/*
 * Create a new FileEntry for the given name and full path.
 * Runs lstat and reads symlink target if applicable.
 */
FileEntry *create_file_entry(const char *name, const char *path);

/*
 * Free a FileEntry and its allocated strings.
 */
void free_file_entry(FileEntry *entry);

/*
 * Free an array of FileEntry pointers of given count.
 */
void free_file_entries(FileEntry **entries, size_t count);

/*
 * Helper to build a path by joining dir and name: "dir/name".
 * Returns dynamically allocated string. Caller must free.
 */
char *build_path(const char *dir, const char *name);

#endif /* FILE_ENTRY_H */
