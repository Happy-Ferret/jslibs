/* This file is part of the KDE project
   Copyright (C) 2003-2007 Jarosław Staniek <staniek@kde.org>

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

#ifndef KDEWIN_STDINT_H
#define KDEWIN_STDINT_H

// include everywhere
#include <sys/types.h>

/* Exact-width integer types */
#ifndef __int8_t_defined
# define __int8_t_defined
  typedef signed char int8_t;
#endif
#ifndef __int16_t_defined
# define __int16_t_defined
  typedef signed short int16_t;
#endif
#ifndef __int32_t_defined
# define __int32_t_defined
  typedef signed long int32_t;
#endif
#ifndef __int64_t_defined
# define __int64_t_defined
  typedef signed long long int64_t;
#endif

#ifndef __uint8_t_defined
# define __uint8_t_defined
  typedef unsigned char uint8_t;
#endif
#ifndef __uint16_t_defined
# define __uint16_t_defined
  typedef unsigned short uint16_t;
#endif
#ifndef __uint32_t_defined
# define __uint32_t_defined
  typedef unsigned long uint32_t;
#endif
#ifndef __uint64_t_defined
# define __uint64_t_defined
  typedef unsigned long long uint64_t;
#endif

  /* Minimum-width integer types */

typedef signed char int_least8_t;
typedef short int_least16_t;
typedef long int_least32_t;
typedef long long int_least64_t;

typedef unsigned char uint_least8_t;
typedef unsigned short uint_least16_t;
typedef unsigned long uint_least32_t;
typedef unsigned long long uint_least64_t;

/* Fastest minimum-width integer types */

typedef signed char int_fast8_t;
typedef long int_fast16_t;
typedef long int_fast32_t;
typedef long long int_fast64_t;

typedef unsigned char uint_fast8_t;
typedef unsigned long uint_fast16_t;
typedef unsigned long uint_fast32_t;
typedef unsigned long long uint_fast64_t;

/* Integer types capable of holding object pointers */

//#ifndef __intptr_t_defined
//#define __intptr_t_defined
//typedef long intptr_t;
//#endif

/* Unix has uintptr_t in stdint.h, and it's safe not to include 
   the whole win32's stddef.h here */
#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif
#endif

#ifndef _UINTPTR_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64    uintptr_t;
#else
typedef _W64 unsigned int   uintptr_t;
#endif
#define _UINTPTR_T_DEFINED
#endif

/* Greatest-width integer types */

typedef long long intmax_t;
typedef unsigned long long uintmax_t;

/* Limits of exact-width integer types */

#define INT8_MIN (-128)
#define INT16_MIN (-32768)
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN (-9223372036854775807LL - 1LL)

#define INT8_MAX (127)
#define INT16_MAX (32767)
#define INT32_MAX (2147483647)
#define INT64_MAX (9223372036854775807LL)

#define UINT8_MAX (255)
#define UINT16_MAX (65535)
#define UINT32_MAX (4294967295UL)
#define UINT64_MAX (18446744073709551615ULL)

/* Limits of minimum-width integer types */

#define INT_LEAST8_MIN (-128)
#define INT_LEAST16_MIN (-32768)
#define INT_LEAST32_MIN (-2147483647 - 1)
#define INT_LEAST64_MIN (-9223372036854775807LL - 1LL)

#define INT_LEAST8_MAX (127)
#define INT_LEAST16_MAX (32767)
#define INT_LEAST32_MAX (2147483647)
#define INT_LEAST64_MAX (9223372036854775807LL)

#define UINT_LEAST8_MAX (255)
#define UINT_LEAST16_MAX (65535)
#define UINT_LEAST32_MAX (4294967295UL)
#define UINT_LEAST64_MAX (18446744073709551615ULL)

/* Limits of fastest minimum-width integer types */

#define INT_FAST8_MIN (-128)
#define INT_FAST16_MIN (-2147483647 - 1)
#define INT_FAST32_MIN (-2147483647 - 1)
#define INT_FAST64_MIN (-9223372036854775807LL - 1LL)

#define INT_FAST8_MAX (127)
#define INT_FAST16_MAX (2147483647)
#define INT_FAST32_MAX (2147483647)
#define INT_FAST64_MAX (9223372036854775807LL)

#define UINT_FAST8_MAX (255)
#define UINT_FAST16_MAX (4294967295UL)
#define UINT_FAST32_MAX (4294967295UL)
#define UINT_FAST64_MAX (18446744073709551615ULL)

/* Limits of integer types capable of holding object pointers */

#define INTPTR_MIN (-2147483647 - 1)
#define INTPTR_MAX (2147483647)
#define UINTPTR_MAX (4294967295UL)

/* Limits of greatest-width integer types */

#define INTMAX_MIN (-9223372036854775807LL - 1LL)
#define INTMAX_MAX (9223372036854775807LL)
#define UINTMAX_MAX (18446744073709551615ULL)

/* Limits of other integer types */

#ifndef PTRDIFF_MIN
#define PTRDIFF_MIN (-2147483647 - 1)
#define PTRDIFF_MAX (2147483647)
#endif

#ifndef SIG_ATOMIC_MIN
#define SIG_ATOMIC_MIN (-2147483647 - 1)
#endif
#ifndef SIG_ATOMIC_MAX
#define SIG_ATOMIC_MAX (2147483647)
#endif

#ifndef SIZE_MAX
#define SIZE_MAX (4294967295UL)
#endif
/*
#ifndef WCHAR_MIN
#ifdef __WCHAR_MIN__
#define WCHAR_MIN __WCHAR_MIN__
#define WCHAR_MAX __WCHAR_MAX__
#else
#define WCHAR_MIN (0)
#define WCHAR_MAX (65535)
#endif
#endif
*/
#ifndef WINT_MIN
#define WINT_MIN (-2147483647 - 1)
#define WINT_MAX (2147483647)
#endif

/* Macros for minimum-width integer constant expressions */

#define INT8_C(x) x
#define INT16_C(x) x
#define INT32_C(x) x ## L
#define INT64_C(x) x ## LL

#define UINT8_C(x) x ## U
#define UINT16_C(x) x ## U
#define UINT32_C(x) x ## UL
#define UINT64_C(x) x ## ULL

/* Macros for greatest-width integer constant expressions */

#define INTMAX_C(x) x ## L
#define UINTMAX_C(x) x ## UL

#endif /* _STDINT_H */
