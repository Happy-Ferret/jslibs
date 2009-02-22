// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include "../common/platform.h"

#include <jsapi.h>
#include "../common/jsNames.h"

#include "../common/jsHelper.h"
#include "../common/jsClass.h"
#include "../common/jsConfiguration.h"

namespace sdl {

	#include <SDL.h>
}

using namespace sdl;

//#include <SDL_error.h>
