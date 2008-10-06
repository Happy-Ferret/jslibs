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

#ifndef KDEWIN_SYS_TYPES_H
#define KDEWIN_SYS_TYPES_H

#include <winposix_export.h>

/* regular header from msvc includes */
#include <../include/sys/types.h>

#include <BaseTsd.h>

typedef unsigned int useconds_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef int caddr_t;

typedef unsigned int nlink_t;

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned int size_t;
#endif

#ifndef _SSIZE_T_DEFINED
#define _SSIZE_T_DEFINED
typedef SSIZE_T ssize_t;
#endif

#ifndef _SOCKLEN_T_DEFINED
#define _SOCKLEN_T_DEFINED
typedef int socklen_t;
#endif 

#ifndef _PID_T_
#define	_PID_T_
typedef int	_pid_t;

#ifndef	_NO_OLDNAMES
typedef int	pid_t;
#endif
#endif	/* Not _PID_T_ */

#ifndef _MODE_T_
#define	_MODE_T_
typedef int _mode_t;

#ifndef	_NO_OLDNAMES
typedef int mode_t;
#endif
#endif	/* Not _MODE_T_ */

//additional:
#ifndef _ASM_TYPES_H
#define _ASM_TYPES_H

//typedef __signed__ char __s8;
typedef unsigned char __u8;

//typedef __signed__ short __s16;
typedef unsigned short __u16;

//typedef __signed__ int __s32;
typedef unsigned int __u32;

#endif /* _ASM_TYPES_H */


#endif  // KDEWIN_SYS_TYPES_H
