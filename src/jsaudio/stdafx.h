#pragma once

#include "../common/platform.h"

#include "stdlib.h"

#include <jsapi.h>

#include "../common/jsHelper.h"
#include "../common/jsClass.h"
#include "../common/jsConversionHelper.h"
#include "../common/jsConfiguration.h"


// OpenAL from src
//#include <AL/al.h>
//#include <AL/alc.h>


// OpenAL from sdk
#include <al.h>
#include <alc.h>
#include <efx.h>

// once ?
#pragma comment(lib, "../../libs/openal/sdk/libs/Win32/OpenAL32.lib")
#pragma comment(lib, "../../libs/openal/sdk/libs/Win32/EFX-Util.lib")


#define LOAD_OPENAL_EXTENSION( name, proto ) \
	static proto name = (proto) alGetProcAddress( #name );
