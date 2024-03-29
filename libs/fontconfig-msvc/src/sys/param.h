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

#ifndef KDEWIN_SYS_PARAM_H
#define KDEWIN_SYS_PARAM_H

// include everywhere
#include <sys/types.h>

/* This is defined to be the same as MAX_PATH which is used internally.
   The Posix version is PATH_MAX.  */
#define MAXPATHLEN      (260 - 1 /*NUL*/)

/* some programs use this: */
#ifndef PATH_MAX
# define PATH_MAX MAXPATHLEN
#endif

#define BYTE_ORDER LITTLE_ENDIAN

#ifndef NULL
#define NULL 0L
#endif

#endif  // KDEWIN_SYS_PARAM_H
