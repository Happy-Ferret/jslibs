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

#include "stdafx.h"

DEFINE_UNSAFE_MODE;

#include "nsprError.h"
#include "nsprFile.h"
#include "nsprDirectory.h"
#include "nsprSocket.h"
#include "nsprEvent.h"
#include "nsprGlobal.h"

#include "../configuration/configuration.h"

extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	jsval unsafeModeValue;
	JSBool jsStatus = GetConfigurationValue(cx, "unsafeMode", &unsafeModeValue);
	RT_ASSERT( jsStatus != JS_FALSE, "Unable to read unsafeMode state from configuration object." );
	if ( JSVAL_IS_BOOLEAN(unsafeModeValue) )
		SET_UNSAFE_MODE( unsafeModeValue == JSVAL_TRUE );

	PR_STDIO_INIT();

	INIT_CLASS( NSPRError );
	INIT_CLASS( File );
	INIT_CLASS( Directory );
	INIT_CLASS( Socket );
	INIT_CLASS( Event );
	INIT_STATIC();

	// Doc: PR_Init is necessary only if a program has specific initialization-sequencing requirements.

	return JS_TRUE;
}

/*

extern "C" __declspec(dllexport) JSBool ModuleRelease(JSContext *cx, JSObject *obj) {

	return JS_TRUE;
}


extern "C" __declspec(dllexport) void ModuleFree() {
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

*/
