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

static const char *_revision = "$Rev:$";

#include "stdafx.h"

#include "misc.h"

#include "rsa.h"
#include "prng.h"
#include "hash.h"
#include "crypt.h"

DEFINE_UNSAFE_MODE;

extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	SET_UNSAFE_MODE( GetConfigurationValue(cx, "unsafeMode" ) == JSVAL_TRUE );

	ltc_mp = ltm_desc;

	InitErrorClass( cx, obj );
	miscInitClass( cx, obj );
	rsaInitClass( cx, obj );
	prngInitClass( cx, obj );
	hashInitClass( cx, obj );
//	cipherInitClass( cx, obj );
	cryptInitClass( cx, obj );
	return JS_TRUE;
}


BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved ) {

  switch (ul_reason_for_call) {

	  case DLL_PROCESS_ATTACH:
	  case DLL_THREAD_ATTACH:
	  case DLL_THREAD_DETACH:
	  case DLL_PROCESS_DETACH:
		  break;
  }
  return TRUE;
}

