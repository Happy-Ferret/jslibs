// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "../common/platform.h"

#ifdef XP_WIN
	#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
	#include <windows.h>
#endif


#ifdef XP_UNIX
	#include <dlfcn.h>
#endif

#include <jsapi.h>
#include "../common/jsHelper.h"
#include "../common/jsNames.h"
#include "../common/queue.h"
