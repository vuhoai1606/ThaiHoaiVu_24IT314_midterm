#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    TIME_MTIME = 0,
    TIME_CTIME,
    TIME_ATIME
} TimeType;

typedef enum {
    SORT_NAME = 0,
    SORT_TIME,
    SORT_SIZE,
    SORT_NONE
} SortType;

typedef enum {
    SIZE_DEFAULT = 0,
    SIZE_HUMAN,
    SIZE_KILO
} SizeUnit;

typedef struct {
    /* Flags */
    bool opt_A;     /* List all except . and .. */
    bool opt_a;     /* Include . and .. and hidden files */
    bool opt_c;     /* Use ctime (status change) */
    bool opt_d;     /* Directories as plain files */
    bool opt_F;     /* Type indicator suffix (/ * @ % = |) */
    bool opt_f;     /* Output not sorted */
    bool opt_h;     /* Human readable sizes */
    bool opt_i;     /* Print inode number */
    bool opt_k;     /* Sizes in kilobytes */
    bool opt_l;     /* Long listing */
    bool opt_n;     /* Numeric UID/GID long listing */
    bool opt_q;     /* Non-printable characters as '?' */
    bool opt_R;     /* Recursive listing */
    bool opt_r;     /* Reverse sort order */
    bool opt_S;     /* Sort by size */
    bool opt_s;     /* Display block count */
    bool opt_t;     /* Sort by time */
    bool opt_u;     /* Use atime (access time) */
    bool opt_w;     /* Raw printing of non-printable chars */

    /* Derived configuration */
    TimeType time_type;
    SortType sort_type;
    SizeUnit size_unit;
    long blocksize;     /* Block size in bytes (from BLOCKSIZE env or 512) */
    bool is_stdout_tty; /* true if stdout is a terminal */
} Options;

/*
 * Initialize options with defaults (checking isatty and BLOCKSIZE env).
 */
void init_options(Options *opts);

/*
 * Parse command line arguments and apply NetBSD override rules.
 * Returns the index of the first operand in argv, or -1 on error.
 */
int parse_options(int argc, char *argv[], Options *opts);

#endif /* OPTIONS_H */
