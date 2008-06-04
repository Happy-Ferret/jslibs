// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// TODO: reference additional headers your program requires here

//#include <windows.h>

#include "../common/platform.h"

#include <jsapi.h>
#include "../common/jsNames.h"

#define USE_UNSAFE_MODE
#include "../common/jsHelper.h"
#include "../common/jsClass.h"
#include "../common/jsConfiguration.h"

#include <nspr.h>

#include "error.h"