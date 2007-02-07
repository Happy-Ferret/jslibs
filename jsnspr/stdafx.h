// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

// #pragma once

#ifndef stdafx_h___
#define stdafx_h___

#ifdef _MSC_VER
	#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
	#include <windows.h>
	#define XP_WIN
#else
	#define XP_UNIX
#endif

#include <jsapi.h>

#define USE_UNSAFE_MODE

#include "../common/jshelper.h"
#include "../common/jsclass.h"

#include <nspr.h>

#endif // stdafx_h___