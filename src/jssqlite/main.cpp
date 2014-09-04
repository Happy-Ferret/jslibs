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
#include <jslibsModule.cpp>

#include "error.h"
#include "database.h"
#include "result.h"


NOALIAS void* xMalloc(int s) NOTHROW {
	return jl_malloc(s);
}
void xFree(void *p) NOTHROW {
	return jl_free(p);
}
NOALIAS void* xRealloc(void *p, int s) NOTHROW {
	return jl_realloc(p, s);
}
int xSize(void* p) NOTHROW {
	return (int)jl_msize(p);
}
int xRoundup(int s) NOTHROW {
	return s;
}
int xInit(void*) {
	return SQLITE_OK;
}
void xShutdown(void*) {
}

static const sqlite3_mem_methods mem = { xMalloc, xFree, xRealloc, xSize, xRoundup, xInit, xShutdown, NULL };

/**doc t:header
$MODULE_HEADER
$FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/

bool
ModuleInit(JSContext *cx, JS::HandleObject obj) {

/* crash case: see http://www.sqlite.org/cvstrac/tktview?tn=3251
	sqlite3_initialize();
	sqlite3 *db;
	sqlite3_open_v2(":memory:", &db, 0, NULL);
	char *query = "select 1";
	sqlite3_stmt *pStmt;
	sqlite3_prepare_v2(db, query, strlen(query), &pStmt, NULL);
	// ...
	for ( sqlite3_stmt *pStmt = sqlite3_next_stmt(db, NULL); pStmt; pStmt = sqlite3_next_stmt(db, pStmt) )
		sqlite3_finalize(pStmt); // pStmt is 0xfeefee at the 2nd loop
*/
 
	JLDisableThreadNotifications();

	JL_ASSERT(jl::Host::getJLHost(cx).checkCompatId(JL_HOST_VERSIONID), E_MODULE, E_NOTCOMPATIBLE, E_HOST );

	//JL_CHKM( sqlite3_config(SQLITE_CONFIG_SINGLETHREAD) == SQLITE_OK, "Unable to set the threading mode to Single-thread." ); // see SQLITE_THREADSAFE=0 define
	//JL_CHKM( sqlite3_config(SQLITE_CONFIG_SINGLETHREAD) == SQLITE_OK, "Unable to set the threading mode to Single-thread." ); // see SQLITE_THREADSAFE=0 define

/*
	int status;
	status = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, 0);
	if ( status != SQLITE_OK )
		JL_CHKM( SqliteThrowErrorStatus(cx, status) );
*/

	
//	JL_CHKM( sqlite3_config(SQLITE_CONFIG_MULTITHREAD) == SQLITE_OK, E_MODULE, E_INIT ); // doc: If SQLite is compiled with the SQLITE_THREADSAFE=0 compile-time option then it is not possible to set the Multi-thread threading mode and sqlite3_config() will return SQLITE_ERROR if called with the SQLITE_CONFIG_MULTITHREAD configuration option.

	JL_CHKM( sqlite3_config(SQLITE_CONFIG_MEMSTATUS, 0) == SQLITE_OK, E_MODULE, E_INIT ); // "Unable to disable memory stats."
	JL_CHKM( sqlite3_config(SQLITE_CONFIG_MALLOC, &mem) == SQLITE_OK, E_MODULE, E_INIT ); // "Unable to initialize memory manager."
	JL_CHKM( sqlite3_enable_shared_cache(true) == SQLITE_OK, E_MODULE, E_INIT ); // "Unable to enable shared cache."
	JL_CHKM( sqlite3_initialize() == SQLITE_OK, E_MODULE, E_INIT ); // "Unable to initialize sqlite."

	struct ReleaseModule : jl::Events::Callback {
		bool operator()() {
		
			sqlite3_shutdown();
			return true;
		}
	};

	jl::HostRuntime::getJLRuntime(cx).addListener(jl::AFTER_DESTROY_RUNTIME, new ReleaseModule()); // frees mpv after rt and cx has been destroyed


	INIT_CLASS( SqliteError );
	INIT_CLASS( Result );
	INIT_CLASS( BlobStream );
	INIT_CLASS( Database );

	return true;
	JL_BAD;
}
