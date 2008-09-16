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

#include "../common/queue.h"

static bool _defaultUnsafeMode = false;
extern bool *_pUnsafeMode = &_defaultUnsafeMode;

extern jl::Queue *dbContextList = NULL;


/**doc t:header
$MODULE_HEADER
**/

/**doc t:footer
$MODULE_FOOTER
**/

EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	jsval unsafeModePtrVal;
	J_CHK( GetConfigurationValue(cx, NAME_CONFIGURATION_UNSAFE_MODE_PTR, &unsafeModePtrVal) );
	if ( unsafeModePtrVal != JSVAL_VOID )
		_pUnsafeMode = (bool*)JSVAL_TO_PRIVATE(unsafeModePtrVal);

	if ( sqlite3_enable_shared_cache(true) != SQLITE_OK ) {
		
		J_REPORT_ERROR( "Unable to enable shared cache." );
	}

	
	dbContextList = jl::QueueConstruct();

	J_CHK( INIT_CLASS( SqliteError ) );
	J_CHK( INIT_CLASS( Result ) );
	J_CHK( INIT_CLASS( Database ) );

	return JS_TRUE;
}


EXTERN_C DLLEXPORT void ModuleRelease (JSContext *cx) {

	REMOVE_CLASS( SqliteError );
	REMOVE_CLASS( Result );
	REMOVE_CLASS( Database );
}

EXTERN_C DLLEXPORT void ModuleFree () {

	while ( !QueueIsEmpty(dbContextList) )
		free(QueuePop(dbContextList));
	QueueDestruct(dbContextList);
}


#ifdef XP_WIN
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

	if ( fdwReason == DLL_PROCESS_ATTACH )
		DisableThreadLibraryCalls(hinstDLL);
	return TRUE;
}
#endif // XP_WIN
