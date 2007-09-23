// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "../common/platform.h"

#include <jsapi.h>

#include "../common/jsNames.h"
#define USE_UNSAFE_MODE
#include "../common/jsHelper.h"
#include "../common/jsClass.h"
#include "../common/jsConfiguration.h"
#include "../common/stack.h"

#include <errno.h>
#include <fcntl.h>
#ifdef XP_WIN
	#include <io.h>
#endif
#ifdef XP_UNIX
	#include <unistd.h>
#endif
#include <string.h>
#include <sys/stat.h>
#include <limits.h>


