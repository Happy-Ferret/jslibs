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

#include "../configuration/configuration.h"


#include "static.h"
#include "buffer.h"

DEFINE_UNSAFE_MODE;

extern JSFunction *stdoutFunction = NULL;

extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

// read configuration
	jsval stdoutFunctionValue;
	JSBool jsStatus = GetConfigurationValue(cx, "stdout", &stdoutFunctionValue );
	RT_ASSERT( jsStatus != JS_FALSE, "Unable to read stdout function from configuration object." );

	stdoutFunction = JS_ValueToFunction(cx, stdoutFunctionValue); // returns NULL if the function is not defined

	jsval unsafeModeValue;
	jsStatus = GetConfigurationValue(cx, "unsafeMode", &unsafeModeValue);
	RT_ASSERT( jsStatus != JS_FALSE, "Unable to read unsafeMode state from configuration object." );

	if ( unsafeModeValue != JSVAL_VOID && JSVAL_IS_BOOLEAN(unsafeModeValue) )
		SET_UNSAFE_MODE( JSVAL_TO_BOOLEAN(unsafeModeValue) == JS_TRUE );

	INIT_STATIC();
	INIT_CLASS( Buffer );
	return JS_TRUE;
}

extern "C" __declspec(dllexport) JSBool ModuleRelease(JSContext *cx) {

	return JS_FALSE;
}


extern "C" __declspec(dllexport) void ModuleFree() {
}



BOOL APIENTRY DllMain( HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved ) {

  switch (ul_reason_for_call) {

	  case DLL_PROCESS_ATTACH:
	  case DLL_THREAD_ATTACH:
	  case DLL_THREAD_DETACH:
	  case DLL_PROCESS_DETACH:
		  break;
  }
  return TRUE;
}

