// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// TODO: reference additional headers your program requires here
#include <windows.h>

#define XP_WIN
#include <jsapi.h>

#define JSHELPER_UNSAFE_DEFINED
#include "../common/jshelper.h"

#include <tomcrypt.h>
#include "cryptError.h"
