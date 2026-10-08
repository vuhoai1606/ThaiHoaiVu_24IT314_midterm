#ifndef COMPAT_H
#define COMPAT_H

#ifdef _WIN32
#ifndef __USE_MINGW_ANSI_STDIO
#define __USE_MINGW_ANSI_STDIO 1
#endif
#endif

#define LLD_FMT "%lld"
#define LLU_FMT "%llu"

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

#ifndef S_ISUID
#define S_ISUID 0004000
#endif
#ifndef S_ISGID
#define S_ISGID 0002000
#endif
#ifndef S_ISVTX
#define S_ISVTX 0001000
#endif
#ifndef S_IRUSR
#define S_IRUSR 0000400
#endif
#ifndef S_IWUSR
#define S_IWUSR 0000200
#endif
#ifndef S_IXUSR
#define S_IXUSR 0000100
#endif
#ifndef S_IRGRP
#define S_IRGRP 0000040
#endif
#ifndef S_IWGRP
#define S_IWGRP 0000020
#endif
#ifndef S_IXGRP
#define S_IXGRP 0000010
#endif
#ifndef S_IROTH
#define S_IROTH 0000004
#endif
#ifndef S_IWOTH
#define S_IWOTH 0000002
#endif
#ifndef S_IXOTH
#define S_IXOTH 0000001
#endif

#ifndef _WIN32
#include <pwd.h>
#include <grp.h>
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__APPLE__)
#include <sys/types.h>
#else
#include <sys/sysmacros.h>
#endif
#else
/* Fallback compatibility wrappers for MinGW / Windows environments */
typedef unsigned int uid_t;
typedef unsigned int gid_t;

struct passwd {
    char *pw_name;
    uid_t pw_uid;
};

struct group {
    char *gr_name;
    gid_t gr_gid;
};

static inline struct passwd *getpwuid(uid_t uid) {
    static struct passwd pw;
    static char uname[32];
    snprintf(uname, sizeof(uname), "user%u", (unsigned int)uid);
    pw.pw_name = uname;
    pw.pw_uid = uid;
    return &pw;
}

static inline struct group *getgrgid(gid_t gid) {
    static struct group gr;
    static char gname[32];
    snprintf(gname, sizeof(gname), "group%u", (unsigned int)gid);
    gr.gr_name = gname;
    gr.gr_gid = gid;
    return &gr;
}

#ifndef lstat
#define lstat(path, sb) stat(path, sb)
#endif

#ifndef S_ISLNK
#define S_ISLNK(m) 0
#endif

#ifndef S_ISSOCK
#define S_ISSOCK(m) 0
#endif

#ifndef S_ISFIFO
#define S_ISFIFO(m) 0
#endif

#ifndef major
#define major(dev) 0
#endif

#ifndef minor
#define minor(dev) 0
#endif

static inline ssize_t readlink(const char *path, char *buf, size_t bufsiz) {
    (void)path;
    (void)buf;
    (void)bufsiz;
    return -1;
}
#endif

#ifndef S_ISWHT
#define S_ISWHT(m) 0
#endif

#endif /* COMPAT_H */
