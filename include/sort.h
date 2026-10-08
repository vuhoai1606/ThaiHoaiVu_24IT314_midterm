#ifndef SORT_H
#define SORT_H

#include "file_entry.h"
#include "options.h"

/*
 * Sort an array of FileEntry pointers according to active options.
 */
void sort_file_entries(FileEntry **entries, size_t count, const Options *opts);

#endif /* SORT_H */
