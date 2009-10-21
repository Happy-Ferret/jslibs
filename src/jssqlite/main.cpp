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

void* xMalloc(int s) {
	return jl_malloc(s);
}
void xFree(void *p) {
	return jl_free(p);
}
void* xRealloc(void *p, int s) {
	return jl_realloc(p, s);
}
int xSize(void* p) {
	if ( p == NULL )
		return 0;
	return jl_msize(p);
}
int xRoundup(int s) {
	return s;
}
int xInit(void*) {
	return 0;
}
void xShutdown(void*) {
}


/**doc t:header
$MODULE_HEADER
$FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/

static sqlite3_mem_methods mem = { xMalloc, xFree, xRealloc, xSize, xRoundup, xInit, xShutdown, NULL };

EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj) {
	
	sqlite3_config(SQLITE_CONFIG_MALLOC, &mem);

	JL_CHK( InitJslibsModule(cx) );

	if ( sqlite3_config(SQLITE_CONFIG_MEMSTATUS, 0) != SQLITE_OK )
		JL_REPORT_ERROR( "Unable to disable memory stats." );


	if ( sqlite3_initialize() != SQLITE_OK )
		JL_REPORT_ERROR( "Unable to initialize sqlite." );

	//	sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, cnt);

	if ( sqlite3_enable_shared_cache(true) != SQLITE_OK )
		JL_REPORT_ERROR( "Unable to enable shared cache." );

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

	sqlite3_shutdown();

	while ( !QueueIsEmpty(dbContextList) )
		jl_free(QueuePop(dbContextList));
	QueueDestruct(dbContextList);
}
