/*
   This file is part of the KDE libraries
   Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>
   Copyright (C) 2006 Ralf Habacker <ralf.habacker@freenet.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef WINPOSIX_EXPORT_H
#define WINPOSIX_EXPORT_H

/* for compatibility */

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#ifndef WARNING
#ifdef _MSC_VER
/** msvc-only: WARNING preprocessor directive
 Reserved: preprocessor needs two indirections to replace __LINE__ with
 actual string.
*/
# define _MSG0(msg) #msg

/**
 msvc-only: preprocessor needs two indirections to replace __LINE__ or __FILE__
 with actual string. */
# define _MSG1(msg) _MSG0(msg)

/**
 msvc-only: creates message prolog with the name of the source file
 and the line number where a preprocessor message has been inserted.

 Example:
    \code
     #pragma WARNING(Your message)
    \endcode
 Output:
    mycode.cpp(111) : Your message
*/
# define _MSGLINENO __FILE__ "(" _MSG1(__LINE__) ") : warning: "
# define WARNING(msg) message(_MSGLINENO #msg)
#endif /*_MSC_VER*/
#endif /*WARNING*/

#endif	// WINPOSIX_EXPORT_H
