#include "traverse.h"
#include "file_entry.h"
#include "sort.h"
#include "display.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

static int list_directory(const char *dirpath, const Options *opts, bool need_header);

static bool should_include_entry(const char *name, const Options *opts) {
    if (name[0] == '.') {
        if (opts->opt_a) {
            return true;
        }
        if (opts->opt_A) {
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
                return false;
            }
            return true;
        }
        return false;
    }
    return true;
}

static int list_directory(const char *dirpath, const Options *opts, bool need_header) {
    int status = 0;

    if (need_header) {
        printf("%s:\n", dirpath);
    }

    DIR *dir = opendir(dirpath);
    if (dir == NULL) {
        fprintf(stderr, "ls: %s: %s\n", dirpath, strerror(errno));
        return 1;
    }

    size_t capacity = 32;
    size_t count = 0;
    FileEntry **entries = malloc(capacity * sizeof(FileEntry *));
    if (!entries) {
        closedir(dir);
        return 1;
    }

    struct dirent *dp;
    while ((dp = readdir(dir)) != NULL) {
        if (!should_include_entry(dp->d_name, opts)) {
            continue;
        }

        char *child_path = build_path(dirpath, dp->d_name);
        if (!child_path) continue;

        FileEntry *fe = create_file_entry(dp->d_name, child_path);
        free(child_path);

        if (!fe) continue;

        if (count >= capacity) {
            size_t new_cap = capacity * 2;
            FileEntry **new_entries = realloc(entries, new_cap * sizeof(FileEntry *));
            if (!new_entries) {
                free_file_entry(fe);
                break;
            }
            entries = new_entries;
            capacity = new_cap;
        }

        entries[count++] = fe;
    }

    closedir(dir);

    /* Sort entries */
    sort_file_entries(entries, count, opts);

    /* Display entries */
    display_entries(entries, count, opts, true);

    /* Recursive traversal if -R and not -d */
    if (opts->opt_R && !opts->opt_d) {
        for (size_t i = 0; i < count; i++) {
            FileEntry *e = entries[i];
            if (e->has_stat && S_ISDIR(e->st.st_mode)) {
                if (strcmp(e->name, ".") == 0 || strcmp(e->name, "..") == 0) {
                    continue;
                }
                putchar('\n');
                int rstatus = list_directory(e->path, opts, true);
                if (rstatus != 0) {
                    status = rstatus;
                }
            }
        }
    }

    free_file_entries(entries, count);
    return status;
}

int process_operands(char *operands[], int operand_count, const Options *opts) {
    int exit_status = 0;

    if (operand_count == 0) {
        /* Default to current directory */
        if (opts->opt_d) {
            FileEntry *entry = create_file_entry(".", ".");
            if (entry) {
                FileEntry *arr[1] = { entry };
                display_entries(arr, 1, opts, false);
                free_file_entry(entry);
            }
            return 0;
        } else {
            return list_directory(".", opts, false);
        }
    }

    /* Collect and validate operands */
    size_t file_cap = 16, file_count = 0;
    FileEntry **file_entries = malloc(file_cap * sizeof(FileEntry *));

    size_t dir_cap = 16, dir_count = 0;
    FileEntry **dir_entries = malloc(dir_cap * sizeof(FileEntry *));

    for (int i = 0; i < operand_count; i++) {
        const char *op = operands[i];
        FileEntry *fe = create_file_entry(op, op);
        if (!fe) continue;

        if (!fe->has_stat) {
            fprintf(stderr, "ls: %s: %s\n", op, strerror(fe->error_code));
            exit_status = 1;
            free_file_entry(fe);
            continue;
        }

        if (S_ISDIR(fe->st.st_mode) && !opts->opt_d) {
            if (dir_count >= dir_cap) {
                dir_cap *= 2;
                dir_entries = realloc(dir_entries, dir_cap * sizeof(FileEntry *));
            }
            dir_entries[dir_count++] = fe;
        } else {
            if (file_count >= file_cap) {
                file_cap *= 2;
                file_entries = realloc(file_entries, file_cap * sizeof(FileEntry *));
            }
            file_entries[file_count++] = fe;
        }
    }

    /* Sort both lists */
    sort_file_entries(file_entries, file_count, opts);
    sort_file_entries(dir_entries, dir_count, opts);

    bool printed_anything = false;

    /* Non-directory operands are displayed first */
    if (file_count > 0) {
        display_entries(file_entries, file_count, opts, false);
        printed_anything = true;
    }

    /* Display directories */
    bool show_headers = (operand_count > 1);

    for (size_t i = 0; i < dir_count; i++) {
        if (printed_anything) {
            putchar('\n');
        }

        int r = list_directory(dir_entries[i]->path, opts, show_headers);
        if (r != 0) {
            exit_status = r;
        }
        printed_anything = true;
    }

    free_file_entries(file_entries, file_count);
    free_file_entries(dir_entries, dir_count);

    return exit_status;
}
