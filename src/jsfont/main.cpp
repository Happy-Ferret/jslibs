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

#include "font.h"

#include <ft2build.h>
#include FT_FREETYPE_H

FT_Library _freetype;

static bool _defaultUnsafeMode = false;
extern bool *_pUnsafeMode = &_defaultUnsafeMode;

/**doc t:header
$MODULE_HEADER
 Support text rendering (text to image) from the following font format:
 TrueType, Type 1, CID-keyed Type 1, CFF, OpenType TrueType, OpenType CFF,
 SFNT-based bitmap, X11 PCF, Windows FNT, BDF, PFR, Type 42
**/

/**doc t:footer
$MODULE_FOOTER
**/

EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	jsval unsafeModePtrVal;
	J_CHK( GetConfigurationValue(cx, NAME_CONFIGURATION_UNSAFE_MODE_PTR, &unsafeModePtrVal) );
	if ( !JSVAL_IS_VOID( unsafeModePtrVal ) )
		_pUnsafeMode = (bool*)JSVAL_TO_PRIVATE(unsafeModePtrVal);

	FT_Error status;
	status = FT_Init_FreeType(&_freetype);
	J_S_ASSERT( status == 0, "Unable to initialize FreeType2 library." );

	INIT_CLASS(Font);

	return JS_TRUE;
}


EXTERN_C DLLEXPORT JSBool ModuleRelease(JSContext *cx) {

	return JS_FALSE;
}


EXTERN_C DLLEXPORT void ModuleFree() {

	FT_Error status;
	status = FT_Done_FreeType(_freetype);
//	J_S_ASSERT( status == 0, "Unable to destroy FreeType2 library." );
}

#ifdef XP_WIN
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

	if ( fdwReason == DLL_PROCESS_ATTACH )
		DisableThreadLibraryCalls(hinstDLL);
	return TRUE;
}
#endif // XP_WIN
