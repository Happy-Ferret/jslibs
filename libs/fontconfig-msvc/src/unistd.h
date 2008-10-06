/* This file is part of the KDE project
   Copyright (C) 2003-2004 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDEWIN_UNISTD_H
#define KDEWIN_UNISTD_H

// include everywhere
#include <sys/types.h>

#include <direct.h>
#include <io.h> /* access(), etc.*/
#include <process.h> /* getpid(), etc.*/

/* include most headers here to avoid redefining gethostname() */
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include <winsock2.h>
#include "fixwinh.h"

#include <sys/stat.h>

#define environ _environ

#ifdef __cplusplus
extern "C" {
#endif

#define	F_OK	0
#define	R_OK	4
#define	W_OK	2
#define	X_OK	1 

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

 int chown(const char *__path, uid_t __owner, gid_t __group);

 int fchown(int __fd, uid_t __owner, gid_t __group );

/* Get the real user ID of the calling process.  */
 uid_t getuid (void);

/* Get the effective user ID of the calling process.  */
 uid_t geteuid (void);

/* Get the real group ID of the calling process.  */
 gid_t getgid (void);

/* Get the effective group ID of the calling process.  */
 gid_t getegid (void);

 int getgroups(int size, gid_t list[]);

/* On win32 we do not have fs-links, so simply 0 (success) is returned
   when __path is accessible. It is then just copied to __buf.
*/
 int readlink(const char *__path, char *__buf, int __buflen);

/* just copies __name1 to __name2 */
 int symlink(const char *__name1, const char *__name2);

/* just copies __name1 to __name2 */
 int link(const char *__name1, const char *__name2);

 int pipe(int *fd);

 pid_t fork(void);

 pid_t setsid(void);

#undef gethostname
#define gethostname kde_gethostname

 int kde_gethostname(char *__name, size_t __len);

 unsigned alarm(unsigned __secs ); 

 char* getlogin();

 int fsync (int fd);

 void usleep(useconds_t useconds);

 void sleep(unsigned int sec);

 int setreuid(uid_t ruid, uid_t euid);

 int mkstemps(char* _template, int suffix_len);

 int initgroups(const char *name, int basegid);

// from kdecore/fakes.c

 int seteuid(uid_t euid);

 int mkstemp (char* _template);

 char* mkdtemp (char* _template);

 int revoke(const char *tty);

 long getpagesize(void);

 int getopt(int argc, char **argv, const char *optstring);

extern  char *optarg;

extern  int optind, opterr;/*, optopt */

 int truncate(const char *path, off_t length);

 int ftruncate(int fd, off_t length);

#ifdef __cplusplus
}
#endif

#endif // KDEWIN_UNISTD_H
