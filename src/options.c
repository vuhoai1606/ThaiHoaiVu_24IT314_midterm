#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void init_options(Options *opts) {
    if (opts == NULL) return;

    memset(opts, 0, sizeof(Options));

    opts->blocksize = 512;
    char *env_bs = getenv("BLOCKSIZE");
    if (env_bs != NULL && *env_bs != '\0') {
        long bs = strtol(env_bs, NULL, 10);
        if (bs > 0) {
            opts->blocksize = bs;
        }
    }

    opts->is_stdout_tty = (isatty(STDOUT_FILENO) != 0);
    if (opts->is_stdout_tty) {
        opts->opt_q = true;
        opts->opt_w = false;
    } else {
        opts->opt_w = true;
        opts->opt_q = false;
    }

#ifndef _WIN32
    /* Always set for the super-user according to NetBSD manual */
    if (geteuid() == 0) {
        opts->opt_A = true;
    }
#endif

    opts->time_type = TIME_MTIME;
    opts->sort_type = SORT_NAME;
    opts->size_unit = SIZE_DEFAULT;
}

int parse_options(int argc, char *argv[], Options *opts) {
    int ch;
    
    /* Reset getopt state */
    optind = 1;

    while ((ch = getopt(argc, argv, "AacdFfhiklnqRrSstuw")) != -1) {
        switch (ch) {
            case 'A':
                opts->opt_A = true;
                opts->opt_a = false;
                break;
            case 'a':
                opts->opt_a = true;
                opts->opt_A = false;
                break;
            case 'c':
                opts->opt_c = true;
                opts->opt_u = false;
                opts->time_type = TIME_CTIME;
                break;
            case 'd':
                opts->opt_d = true;
                opts->opt_R = false;
                break;
            case 'F':
                opts->opt_F = true;
                break;
            case 'f':
                opts->opt_f = true;
                opts->opt_a = true;
                opts->opt_A = false;
                opts->sort_type = SORT_NONE;
                break;
            case 'h':
                opts->opt_h = true;
                opts->opt_k = false;
                opts->size_unit = SIZE_HUMAN;
                break;
            case 'i':
                opts->opt_i = true;
                break;
            case 'k':
                opts->opt_k = true;
                opts->opt_h = false;
                opts->size_unit = SIZE_KILO;
                opts->blocksize = 1024;
                break;
            case 'l':
                opts->opt_l = true;
                opts->opt_n = false;
                break;
            case 'n':
                opts->opt_n = true;
                opts->opt_l = false;
                break;
            case 'q':
                opts->opt_q = true;
                opts->opt_w = false;
                break;
            case 'R':
                opts->opt_R = true;
                opts->opt_d = false;
                break;
            case 'r':
                opts->opt_r = true;
                break;
            case 'S':
                opts->opt_S = true;
                opts->opt_t = false;
                if (!opts->opt_f) {
                    opts->sort_type = SORT_SIZE;
                }
                break;
            case 's':
                opts->opt_s = true;
                break;
            case 't':
                opts->opt_t = true;
                opts->opt_S = false;
                if (!opts->opt_f) {
                    opts->sort_type = SORT_TIME;
                }
                break;
            case 'u':
                opts->opt_u = true;
                opts->opt_c = false;
                opts->time_type = TIME_ATIME;
                break;
            case 'w':
                opts->opt_w = true;
                opts->opt_q = false;
                break;
            default:
                fprintf(stderr, "usage: ls [-AacdFfhiklnqRrSstuw] [file ...]\n");
                return -1;
        }
    }

    /* Final adjustments based on overrides */
    if (opts->opt_f) {
        opts->sort_type = SORT_NONE;
    }

    return optind;
}
