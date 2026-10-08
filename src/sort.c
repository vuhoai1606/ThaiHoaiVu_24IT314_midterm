#include "sort.h"
#include <stdlib.h>
#include <string.h>

static const Options *current_opts = NULL;

static time_t get_entry_time(const FileEntry *entry, const Options *opts) {
    if (!entry->has_stat) return 0;
    switch (opts->time_type) {
        case TIME_CTIME:
            return entry->st.st_ctime;
        case TIME_ATIME:
            return entry->st.st_atime;
        case TIME_MTIME:
        default:
            return entry->st.st_mtime;
    }
}

static int compare_entries(const void *a, const void *b) {
    const FileEntry *ea = *(const FileEntry **)a;
    const FileEntry *eb = *(const FileEntry **)b;

    int res = 0;

    if (current_opts->sort_type == SORT_SIZE) {
        /* Sort by size: largest file first */
        off_t size_a = ea->has_stat ? ea->st.st_size : 0;
        off_t size_b = eb->has_stat ? eb->st.st_size : 0;
        if (size_a > size_b) {
            res = -1;
        } else if (size_a < size_b) {
            res = 1;
        } else {
            res = strcmp(ea->name, eb->name);
        }
    } else if (current_opts->sort_type == SORT_TIME) {
        /* Sort by time: most recent first */
        time_t t_a = get_entry_time(ea, current_opts);
        time_t t_b = get_entry_time(eb, current_opts);
        if (t_a > t_b) {
            res = -1;
        } else if (t_a < t_b) {
            res = 1;
        } else {
            res = strcmp(ea->name, eb->name);
        }
    } else {
        /* Default: lexicographical by name */
        res = strcmp(ea->name, eb->name);
    }

    if (current_opts->opt_r) {
        res = -res;
    }

    return res;
}

void sort_file_entries(FileEntry **entries, size_t count, const Options *opts) {
    if (entries == NULL || count <= 1 || opts == NULL) {
        return;
    }

    /* If -f is specified, output is not sorted */
    if (opts->sort_type == SORT_NONE || opts->opt_f) {
        return;
    }

    current_opts = opts;
    qsort(entries, count, sizeof(FileEntry *), compare_entries);
    current_opts = NULL;
}
