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

extern bool _unsafeMode = false;

/**doc t:header
$MODULE_HEADER
 This module manage jpeg and png image decomppression and png image compression.
 Supported output format:
  1:Gray 2:Gray,Alpha 3:Red,Green,Blue 4:Red,Green,Blue,Alpha
$FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/

EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	_unsafeMode = GetHostPrivate(cx)->unsafeMode;

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
	JL_BAD;
}

EXTERN_C DLLEXPORT JSBool ModuleRelease(JSContext *cx, JSObject *obj) {

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
