#include "display.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void format_mode_string(mode_t mode, char out[11]) {
    /* Entry type */
    if (S_ISDIR(mode)) {
        out[0] = 'd';
    } else if (S_ISLNK(mode)) {
        out[0] = 'l';
    } else if (S_ISCHR(mode)) {
        out[0] = 'c';
    } else if (S_ISBLK(mode)) {
        out[0] = 'b';
    } else if (S_ISSOCK(mode)) {
        out[0] = 's';
    } else if (S_ISFIFO(mode)) {
        out[0] = 'p';
    } else if (S_ISWHT(mode)) {
        out[0] = 'w';
    } else {
        out[0] = '-';
    }

    /* Owner permissions */
    out[1] = (mode & S_IRUSR) ? 'r' : '-';
    out[2] = (mode & S_IWUSR) ? 'w' : '-';
    if (mode & S_ISUID) {
        out[3] = (mode & S_IXUSR) ? 's' : 'S';
    } else {
        out[3] = (mode & S_IXUSR) ? 'x' : '-';
    }

    /* Group permissions */
    out[4] = (mode & S_IRGRP) ? 'r' : '-';
    out[5] = (mode & S_IWGRP) ? 'w' : '-';
    if (mode & S_ISGID) {
        out[6] = (mode & S_IXGRP) ? 's' : 'S';
    } else {
        out[6] = (mode & S_IXGRP) ? 'x' : '-';
    }

    /* Other permissions */
    out[7] = (mode & S_IROTH) ? 'r' : '-';
    out[8] = (mode & S_IWOTH) ? 'w' : '-';
    if (mode & S_ISVTX) {
        out[9] = (mode & S_IXOTH) ? 't' : 'T';
    } else {
        out[9] = (mode & S_IXOTH) ? 'x' : '-';
    }

    out[10] = '\0';
}

void format_human_size(off_t size, char *buf, size_t buflen) {
    const char *units[] = {"B", "K", "M", "G", "T", "P"};
    int unit_idx = 0;
    double dsize = (double)size;

    while (dsize >= 1024.0 && unit_idx < 5) {
        dsize /= 1024.0;
        unit_idx++;
    }

    if (unit_idx == 0) {
        snprintf(buf, buflen, LLD_FMT "%s", (long long)size, units[unit_idx]);
    } else if (dsize < 10.0) {
        snprintf(buf, buflen, "%.1f%s", dsize, units[unit_idx]);
    } else {
        snprintf(buf, buflen, "%.0f%s", dsize, units[unit_idx]);
    }
}

long calculate_blocks(const struct stat *st, const Options *opts) {
#ifdef _WIN32
    long bytes = (long)st->st_size;
    long blk = (bytes + opts->blocksize - 1) / opts->blocksize;
    return blk > 0 ? blk : 0;
#else
    /* st_blocks is in 512-byte units */
    long long total_bytes = (long long)st->st_blocks * 512LL;
    long blk = (long)((total_bytes + opts->blocksize - 1) / opts->blocksize);
    return blk;
#endif
}

static void print_file_name(const char *name, const Options *opts) {
    if (!name) return;
    for (const char *p = name; *p != '\0'; p++) {
        unsigned char c = (unsigned char)*p;
        if (opts->opt_q && !isprint(c)) {
            putchar('?');
        } else {
            putchar(c);
        }
    }
}

static char get_file_type_indicator(mode_t mode) {
    if (S_ISDIR(mode)) return '/';
    if (S_ISLNK(mode)) return '@';
    if (S_ISSOCK(mode)) return '=';
    if (S_ISFIFO(mode)) return '|';
    if (S_ISWHT(mode)) return '%';
    if (mode & (S_IXUSR | S_IXGRP | S_IXOTH)) return '*';
    return '\0';
}

static time_t get_selected_time(const struct stat *st, const Options *opts) {
    switch (opts->time_type) {
        case TIME_CTIME:
            return st->st_ctime;
        case TIME_ATIME:
            return st->st_atime;
        case TIME_MTIME:
        default:
            return st->st_mtime;
    }
}

void display_entries(FileEntry **entries, size_t count, const Options *opts, bool is_dir_listing) {
    if (entries == NULL || count == 0 || opts == NULL) {
        return;
    }

    bool is_long = (opts->opt_l || opts->opt_n);

    /* Total block count calculation for directories or -s */
    if (is_dir_listing && (is_long || opts->opt_s)) {
        long long total_blocks = 0;
        for (size_t i = 0; i < count; i++) {
            if (entries[i]->has_stat) {
                total_blocks += calculate_blocks(&entries[i]->st, opts);
            }
        }
        if (opts->size_unit == SIZE_HUMAN) {
            char hbuf[32];
            format_human_size(total_blocks * opts->blocksize, hbuf, sizeof(hbuf));
            printf("total %s\n", hbuf);
        } else {
            printf("total " LLD_FMT "\n", (long long)total_blocks);
        }
    }

    /* Compute column widths for neat alignment if long format */
    int max_ino_len = 0;
    int max_blk_len = 0;
    int max_nlink_len = 0;
    int max_user_len = 0;
    int max_group_len = 0;
    int max_size_len = 0;

    for (size_t i = 0; i < count; i++) {
        FileEntry *e = entries[i];
        if (!e->has_stat) continue;

        char buf[64];
        if (opts->opt_i) {
            int len = snprintf(buf, sizeof(buf), LLU_FMT, (unsigned long long)e->st.st_ino);
            if (len > max_ino_len) max_ino_len = len;
        }

        if (opts->opt_s) {
            long blk = calculate_blocks(&e->st, opts);
            int len;
            if (opts->size_unit == SIZE_HUMAN) {
                char hbuf[32];
                format_human_size(blk * opts->blocksize, hbuf, sizeof(hbuf));
                len = (int)strlen(hbuf);
            } else {
                len = snprintf(buf, sizeof(buf), "%ld", blk);
            }
            if (len > max_blk_len) max_blk_len = len;
        }

        if (is_long) {
            int nlen = snprintf(buf, sizeof(buf), "%u", (unsigned int)e->st.st_nlink);
            if (nlen > max_nlink_len) max_nlink_len = nlen;

            /* Owner */
            if (opts->opt_n) {
                int ulen = snprintf(buf, sizeof(buf), "%u", (unsigned int)e->st.st_uid);
                if (ulen > max_user_len) max_user_len = ulen;
            } else {
                struct passwd *pw = getpwuid(e->st.st_uid);
                int ulen = pw ? (int)strlen(pw->pw_name) : snprintf(buf, sizeof(buf), "%u", (unsigned int)e->st.st_uid);
                if (ulen > max_user_len) max_user_len = ulen;
            }

            /* Group */
            if (opts->opt_n) {
                int glen = snprintf(buf, sizeof(buf), "%u", (unsigned int)e->st.st_gid);
                if (glen > max_group_len) max_group_len = glen;
            } else {
                struct group *gr = getgrgid(e->st.st_gid);
                int glen = gr ? (int)strlen(gr->gr_name) : snprintf(buf, sizeof(buf), "%u", (unsigned int)e->st.st_gid);
                if (glen > max_group_len) max_group_len = glen;
            }

            /* Size */
            if (S_ISCHR(e->st.st_mode) || S_ISBLK(e->st.st_mode)) {
                int slen = snprintf(buf, sizeof(buf), "%u, %u", (unsigned int)major(e->st.st_rdev), (unsigned int)minor(e->st.st_rdev));
                if (slen > max_size_len) max_size_len = slen;
            } else if (opts->size_unit == SIZE_HUMAN) {
                char hbuf[32];
                format_human_size(e->st.st_size, hbuf, sizeof(hbuf));
                int slen = (int)strlen(hbuf);
                if (slen > max_size_len) max_size_len = slen;
            } else {
                int slen = snprintf(buf, sizeof(buf), LLD_FMT, (long long)e->st.st_size);
                if (slen > max_size_len) max_size_len = slen;
            }
        }
    }

    /* Print each entry */
    for (size_t i = 0; i < count; i++) {
        FileEntry *e = entries[i];

        if (opts->opt_i) {
            if (e->has_stat) {
                char inobuf[32];
                snprintf(inobuf, sizeof(inobuf), LLU_FMT, (unsigned long long)e->st.st_ino);
                printf("%*s ", max_ino_len, inobuf);
            } else {
                printf("%*s ", max_ino_len, "?");
            }
        }

        if (opts->opt_s) {
            if (e->has_stat) {
                long blk = calculate_blocks(&e->st, opts);
                if (opts->size_unit == SIZE_HUMAN) {
                    char hbuf[32];
                    format_human_size(blk * opts->blocksize, hbuf, sizeof(hbuf));
                    printf("%*s ", max_blk_len, hbuf);
                } else {
                    printf("%*ld ", max_blk_len, blk);
                }
            } else {
                printf("%*s ", max_blk_len, "?");
            }
        }

        if (is_long) {
            if (e->has_stat) {
                char mode_str[11];
                format_mode_string(e->st.st_mode, mode_str);

                /* Mode and Links */
                printf("%s %*u ", mode_str, max_nlink_len, (unsigned int)e->st.st_nlink);

                /* Owner */
                if (opts->opt_n) {
                    printf("%-*u ", max_user_len, (unsigned int)e->st.st_uid);
                } else {
                    struct passwd *pw = getpwuid(e->st.st_uid);
                    if (pw) {
                        printf("%-*s ", max_user_len, pw->pw_name);
                    } else {
                        printf("%-*u ", max_user_len, (unsigned int)e->st.st_uid);
                    }
                }

                /* Group */
                if (opts->opt_n) {
                    printf("%-*u ", max_group_len, (unsigned int)e->st.st_gid);
                } else {
                    struct group *gr = getgrgid(e->st.st_gid);
                    if (gr) {
                        printf("%-*s ", max_group_len, gr->gr_name);
                    } else {
                        printf("%-*u ", max_group_len, (unsigned int)e->st.st_gid);
                    }
                }

                /* Size */
                if (S_ISCHR(e->st.st_mode) || S_ISBLK(e->st.st_mode)) {
                    printf("%*u, %*u ", max_size_len - 3, (unsigned int)major(e->st.st_rdev), 3, (unsigned int)minor(e->st.st_rdev));
                } else if (opts->size_unit == SIZE_HUMAN) {
                    char hbuf[32];
                    format_human_size(e->st.st_size, hbuf, sizeof(hbuf));
                    printf("%*s ", max_size_len, hbuf);
                } else {
                    char szbuf[32];
                    snprintf(szbuf, sizeof(szbuf), LLD_FMT, (long long)e->st.st_size);
                    printf("%*s ", max_size_len, szbuf);
                }

                /* Date & Time */
                time_t t = get_selected_time(&e->st, opts);
                struct tm *lt = localtime(&t);
                char time_buf[32];
                if (lt) {
                    char mon_buf[16];
                    strftime(mon_buf, sizeof(mon_buf), "%b", lt);
                    snprintf(time_buf, sizeof(time_buf), "%s %2d %02d:%02d",
                             mon_buf, lt->tm_mday, lt->tm_hour, lt->tm_min);
                } else {
                    snprintf(time_buf, sizeof(time_buf), "Unknown Date");
                }
                printf("%s ", time_buf);
            }
        }

        /* Print file name */
        print_file_name(e->name, opts);

        /* Indicator if -F */
        if (opts->opt_F && e->has_stat) {
            char ind = get_file_type_indicator(e->st.st_mode);
            if (ind != '\0') {
                putchar(ind);
            }
        }

        /* Symbolic link destination if long format */
        if (is_long && e->has_stat && S_ISLNK(e->st.st_mode) && e->link_target) {
            printf(" -> ");
            print_file_name(e->link_target, opts);
        }

        putchar('\n');
    }
}
