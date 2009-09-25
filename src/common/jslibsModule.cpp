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

#include "../common/jslibsModule.h"
#include "../common/jsHelper.h"

extern bool _unsafeMode = false;

extern void* (*jl_malloc)( size_t size ) = NULL;
extern void* (*jl_calloc)( size_t num, size_t size ) = NULL;
extern void (*jl_free)( void *ptr ) = NULL;
extern void* (*jl_realloc)( void *ptr, size_t size ) = NULL;


JSBool InitJslibsModule( JSContext *cx ) {
	
	HostPrivate *pv = GetHostPrivate(cx);

	_unsafeMode = pv ? pv->unsafeMode : true;
	jl_malloc = pv && pv->malloc ? pv->malloc : malloc;
	jl_calloc = pv && pv->calloc ? pv->calloc : calloc;
	jl_free = pv && pv->free ? pv->free : free;
	jl_realloc = pv && pv->realloc ? pv->realloc : realloc;

	return JS_TRUE;
}


#ifdef XP_WIN
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

	if ( fdwReason == DLL_PROCESS_ATTACH )
		DisableThreadLibraryCalls(hinstDLL);
	return TRUE;
}
#endif // XP_WIN
