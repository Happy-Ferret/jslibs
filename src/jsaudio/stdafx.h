/* ***** BEGIN LICENSE BLOCK *****
 * Version: GNU GPL 2.0
 *
 * The contents of this file are subject to the
 * GNU General Public License Version 2.0; you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * http://www.gnu.org/licenses/gpl.html
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * ***** END LICENSE BLOCK ***** */


#pragma once

#define USE_JSHANDLES

#include <jlhelper.h>
#include <jlclass.h>

#include "stdlib.h"


//#include <jsvalserializer.h>

#include <queue.h>


// OpenAL from src
//#include <AL/al.h>
//#include <AL/alc.h>

// OpenAL from sdk
#include <al.h>
#include <alc.h>
#include <efx.h>
//#include <EFX-Util.h>

// once ?
#pragma comment(lib, "../../libs/openal/sdk/libs/Win32/OpenAL32.lib")
#pragma comment(lib, "../../libs/openal/sdk/libs/Win32/EFX-Util.lib")

#include "oalefxapi.h"

#include "error.h"


//#define LOAD_OPENAL_EXTENSION( name, proto ) \
//	static proto name = (proto) alGetProcAddress( #name );
