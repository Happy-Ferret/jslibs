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

jl::Queue *dbContextList = NULL;


#include "../common/jslibsModule.cpp"


/**doc t:header
$MODULE_HEADER
$FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/

EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	JL_CHK( InitJslibsModule(cx) );

	if ( sqlite3_enable_shared_cache(true) != SQLITE_OK ) {

		JL_REPORT_ERROR( "Unable to enable shared cache." );
	}

	dbContextList = jl::QueueConstruct();

	INIT_CLASS( SqliteError );
	INIT_CLASS( Result );
	INIT_CLASS( Database );

	return JS_TRUE;
	JL_BAD;
}


EXTERN_C DLLEXPORT JSBool ModuleRelease(JSContext *cx) {

	REMOVE_CLASS( SqliteError );
	REMOVE_CLASS( Result );
	REMOVE_CLASS( Database );

	return JS_TRUE;
	JL_BAD;
}

EXTERN_C DLLEXPORT void ModuleFree() {

	while ( !QueueIsEmpty(dbContextList) )
		jl_free(QueuePop(dbContextList));
	QueueDestruct(dbContextList);
}
