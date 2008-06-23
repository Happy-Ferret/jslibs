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

#include "error.h"
#include "database.h"
#include "result.h"

static bool _defaultUnsafeMode = false;
extern bool *_pUnsafeMode = &_defaultUnsafeMode;

/** t:header
#summary jssqlite module
#labels doc
- [http://jslibs.googlecode.com/svn/trunk/jssqlite/ source] - [JSLibs main] -
= jssqlite module =
**/

/** t:footer
----
- [http://jslibs.googlecode.com/svn/trunk/jssqlite/ source] - [#jssqlite_module top] - [JSLibs main] -
**/

extern "C" DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	jsval unsafeModePtrVal;
	J_CHK( GetConfigurationValue(cx, NAME_CONFIGURATION_UNSAFE_MODE_PTR, &unsafeModePtrVal) );
	if ( unsafeModePtrVal != JSVAL_VOID )
		_pUnsafeMode = (bool*)JSVAL_TO_PRIVATE(unsafeModePtrVal);

	
	if ( sqlite3_enable_shared_cache(true) != SQLITE_OK ) {
		
		// manage error
	}


	INIT_CLASS( SqliteError )
	INIT_CLASS( Result )
	INIT_CLASS( Database )

	return JS_TRUE;
}


extern "C" DLLEXPORT void ModuleRelease (JSContext *cx) {

	REMOVE_CLASS( SqliteError );
	REMOVE_CLASS( Result );
	REMOVE_CLASS( Database );
}


#ifdef XP_WIN
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

	if ( fdwReason == DLL_PROCESS_ATTACH )
		DisableThreadLibraryCalls(hinstDLL);
	return TRUE;
}
#endif // XP_WIN
