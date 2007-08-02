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
#include "../common/jshelper.h"
#include "../common/jsclass.h"
#include "../common/jsConversionHelper.h"
#include "../common/jsConfiguration.h"

#include <malloc.h>

namespace ode {
	#include <ode/ode.h>
}

typedef ode::dReal real;

#include "support.h"
