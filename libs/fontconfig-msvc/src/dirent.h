/* This file is part of the KDE project
   Copyright (C) 2000 Werner Almesberger

   libc/sys/linux/sys/dirent.h - Directory entry as returned by readdir

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEWIN_DIRENT_H
#define KDEWIN_DIRENT_H

// include everywhere
#include <sys/types.h>

#include <io.h>
#include <stdio.h>
#include <stdlib.h>

//#include <sys/lock.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAVE_NO_D_NAMLEN	/* no struct dirent->d_namlen */
#define HAVE_DD_LOCK  		/* have locking mechanism */

#define MAXNAMLEN 255		/* sizeof(struct dirent.d_name)-1 */

#define __dirfd(dir) (dir)->dd_fd

#define KDEWIN32_HAVE_DIRENT_D_TYPE
enum
  {
    DT_UNKNOWN = 0,
    DT_FIFO = 1,
    DT_CHR = 2,
    DT_DIR = 4,
    DT_BLK = 6,
    DT_REG = 8,
    DT_LNK = 10,
    DT_SOCK = 12,
    DT_WHT = 14
  };

/* struct dirent - same as Unix */
struct dirent {
    long d_ino;                    /* inode (always 1 in WIN32) */
    off_t d_off;                /* offset to this dirent */
    unsigned short d_reclen;    /* length of d_name */
    unsigned char d_type;
    char d_name[_MAX_FNAME+1];    /* filename (null terminated) */
};

/* typedef DIR - not the same as Unix */
typedef struct {
    long handle;                /* _findfirst/_findnext handle */
    short offset;                /* offset into directory */
    short finished;             /* 1 if there are not more files */
    struct _finddata_t fileinfo;  /* from _findfirst/_findnext */
    char *dir;                  /* the dir we are reading */
    struct dirent dent;         /* the dirent to return */
} DIR;

/* --- redundant --- */

//DIR *opendir(const char *);
//struct dirent *readdir(DIR *);
//void rewinddir(DIR *);
//int closedir(DIR *);

/* internal prototype */
void _seekdir(DIR *dir,off_t offset);

//#ifndef _POSIX_SOURCE
//long telldir (DIR *);
//void seekdir (DIR *, off_t loc);

 int scandir (const char *__dir,
             struct dirent ***__namelist,
             int (*select) (const struct dirent *),
             int (*compar) (const struct dirent **, const struct dirent **));

 DIR * opendir(const char *);
 int closedir(DIR *);
 struct dirent* readdir(DIR *);

// Implement readdir_r(). For now do not provide dummy function to avoid setting HAVE_READDIR_R.
// Code like DirectoryListThread::run() in kio/kio/kurlcompletion.cpp uses readdir() when !HAVE_READDIR_R.

//  struct dirent* readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);

int alphasort (const struct dirent **__a, const struct dirent **__b);

#ifdef __cplusplus
}
#endif

#endif  // KDEWIN_DIRENT_H
