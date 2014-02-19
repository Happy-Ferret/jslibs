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

#include "jslibsModule.h"
#include "jlhelper.h" // see InitJslibsModule()
#include "jlalloc.h"


// (TBD) Should we create a new module for so few symbols ?


// set moduleId as uninitialized.
DLLLOCAL uint32_t _moduleId = 0;


// initialize with default allocators.
jl_malloc_t jl_malloc = malloc;
jl_calloc_t jl_calloc = calloc;
jl_memalign_t jl_memalign = memalign;
jl_realloc_t jl_realloc = realloc;
jl_msize_t jl_msize = msize;
jl_free_t jl_free = free;


// external libraries are using these symbols.
EXTERN_C void* jl_malloc_fct( size_t size ) { return jl_malloc(size); }
EXTERN_C void* jl_calloc_fct( size_t num, size_t size ) { return jl_calloc(num, size); }
EXTERN_C void* jl_memalign_fct( size_t alignment, size_t size ) { return jl_memalign(alignment, size); }
EXTERN_C void* jl_realloc_fct( void *ptr, size_t size ) { return jl_realloc(ptr, size); }
EXTERN_C size_t jl_msize_fct( void *ptr ) { return jl_msize(ptr); }
EXTERN_C void jl_free_fct( void *ptr ) { jl_free(ptr); }


static bool FASTCALL
InitJslibsModule( JSContext *cx, uint32_t id ) {

	// printf("id=%u / &_moduleId=%p / _moduleId=%u\n", &_moduleId, _moduleId, id);
	
#ifdef XP_WIN
	// doc:
	//   Disables the DLL_THREAD_ATTACH and DLL_THREAD_DETACH notifications for the specified dynamic-link library (DLL).
	//   This can reduce the size of the working set for some applications.
	BOOL st = ::DisableThreadLibraryCalls(jl::GetCurrentModule());
	ASSERT(st);
#endif // XP_WIN

	jl::Host &host = jl::Host::getHost(cx);
	if ( !host.checkCompatId(JL_HOST_VERSIONID) )
		JL_ERR( E_MODULE, E_NOTCOMPATIBLE, E_HOST );

	//_unsafeMode = hpv ? hpv->unsafeMode : _unsafeMode;
	_unsafeMode = host.unsafeMode();

	ASSERT( _moduleId == 0 || _moduleId == id );

	if ( _moduleId == 0 )
		_moduleId = id;

	host.getAllocators(jl_malloc, jl_calloc, jl_memalign, jl_realloc, jl_msize, jl_free);

/*
	jl_malloc = hpv && hpv->alloc.malloc ? hpv->alloc.malloc : jl_malloc; // ie. if we have a host and if the host has custom allocators, else keep the current one.
	jl_calloc = hpv && hpv->alloc.calloc ? hpv->alloc.calloc : jl_calloc;
	jl_memalign = hpv && hpv->alloc.memalign ? hpv->alloc.memalign : jl_memalign;
	jl_realloc = hpv && hpv->alloc.realloc ? hpv->alloc.realloc : jl_realloc;
	jl_msize = hpv && hpv->alloc.msize ? hpv->alloc.msize : jl_msize;
	jl_free = hpv && hpv->alloc.free ? hpv->alloc.free : jl_free;
*/

	return true;
	JL_BAD;
}


/* see InitJslibsModule()
#if defined _WINDLL && !defined JL_NO_DLL_MAIN
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

	JL_IGNORE(lpvReserved);
	//DisableThreadLibraryCalls() doc: http://msdn.microsoft.com/en-us/library/ms682579(v=vs.85).aspx
	//beware:
	//  Do not call this function from a DLL that is linked to the static C run-time library (CRT).
	//  The static CRT requires DLL_THREAD_ATTACH and DLL_THREAD_DETATCH notifications to function properly.
	if ( fdwReason == DLL_PROCESS_ATTACH )
		DisableThreadLibraryCalls(hinstDLL);
	return TRUE;
}
#endif
*/


//MS doc:
//  To illustrate this, consider the following example:
//  - .EXE is linked with MSVCRT.LIB
//  - DLL A is linked with LIBCMT.LIB
//  - DLL B is linked with CRTDLL.LIB
//
//  If the .EXE creates a CRT file handle using _create() or _open(), this file handle may only be passed to _lseek(), _read(), _write(), _close(), etc. in the .EXE file. Do not pass this CRT file handle to either DLL. Do not pass a CRT file handle obtained from either DLL to the other DLL or to the .EXE.
//  If DLL A allocates a block of memory with malloc(), only DLL A may call free(), _expand(), or realloc() to operate on that block. You cannot call malloc() from DLL A and try to free that block from the .EXE or from DLL B.
//  NOTE: If all three modules were linked with CRTDLL.LIB or all three were linked with MSVCRT.LIb, these restrictions would not apply.
//   (source: http://support.microsoft.com/kb/94248)
