// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include "../common/platform.h"

#include <jsapi.h>

#define USE_UNSAFE_MODE
#include "../common/jsHelper.h"
#include "../common/jsClass.h"
#include "../common/jsConfiguration.h"
#include "../common/stack.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>

