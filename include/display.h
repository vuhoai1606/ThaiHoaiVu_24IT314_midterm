#ifndef DISPLAY_H
#define DISPLAY_H

#include "file_entry.h"
#include "options.h"
#include <stdio.h>

/*
 * Convert file mode to a 10-character string (e.g. "drwxr-xr-x").
 * The buffer must be at least 11 bytes long (10 chars + null terminator).
 */
void format_mode_string(mode_t mode, char out[11]);

/*
 * Format size into human readable string (e.g. "1.2K", "34M", "512B").
 */
void format_human_size(off_t size, char *buf, size_t buflen);

/*
 * Calculate file block count according to BLOCKSIZE / -k / -h options.
 */
long calculate_blocks(const struct stat *st, const Options *opts);

/*
 * Display an array of FileEntry items according to active Options.
 * is_dir_listing is true when displaying contents of a directory.
 */
void display_entries(FileEntry **entries, size_t count, const Options *opts, bool is_dir_listing);

#endif /* DISPLAY_H */
