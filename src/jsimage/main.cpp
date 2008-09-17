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

DECLARE_STATIC()
//DECLARE_CLASS(Jpeg)
//DECLARE_CLASS(Png)
//DECLARE_CLASS(Image)

#include "jspng.h"
#include "jsjpeg.h"

static bool _defaultUnsafeMode = false;
extern bool *_pUnsafeMode = &_defaultUnsafeMode;

/**doc t:header
$MODULE_HEADER
 This module manage jpeg and png image decomppression and png image compression.
 Supported output format:
  1:Gray 2:Gray,Alpha 3:Red,Green,Blue 4:Red,Green,Blue,Alpha
**/

/**doc t:footer
$MODULE_FOOTER
**/

extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	jsval unsafeModePtrVal;
	J_CHK( GetConfigurationValue(cx, NAME_CONFIGURATION_UNSAFE_MODE_PTR, &unsafeModePtrVal) );
	if ( !JSVAL_IS_VOID( unsafeModePtrVal ) )
		_pUnsafeMode = (bool*)JSVAL_TO_PRIVATE(unsafeModePtrVal);

	INIT_STATIC();
//	INIT_CLASS( Image );
//	INIT_CLASS( Png );
//	INIT_CLASS( Jpeg );

/*
	need this:

		var dec = new JpegDecoder();
		var image = dec( new File('test.jpeg') );
*/

	return JS_TRUE;
}

extern "C" __declspec(dllexport) JSBool ModuleRelease(JSContext *cx, JSObject *obj) {

	return JS_TRUE;
}

/*
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

#ifdef XP_WIN
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

	if ( fdwReason == DLL_PROCESS_ATTACH )
		DisableThreadLibraryCalls(hinstDLL);
	return TRUE;
}
#endif // XP_WIN
