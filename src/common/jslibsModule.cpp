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

// (TBD) Should we create a new module for so few symbols ?

#include "../common/jsHelper.h"
#include "../common/jslibsModule.h"

#include <malloc.h>

// by default, we run in unsafe mode.
bool _unsafeMode = true;

// initialize with default allocators
jl_malloc_t jl_malloc = malloc;
jl_calloc_t jl_calloc = calloc;
jl_realloc_t jl_realloc = realloc;
jl_free_t jl_free = free;


JSBool InitJslibsModule( JSContext *cx ) {

	HostPrivate *pv = GetHostPrivate(cx);

	_unsafeMode = pv ? pv->unsafeMode : _unsafeMode;

	jl_malloc = pv && pv->malloc ? pv->malloc : jl_malloc; // ie. if we have a host and if the host has custom allocators.
	jl_calloc = pv && pv->calloc ? pv->calloc : jl_calloc;
	jl_realloc = pv && pv->realloc ? pv->realloc : jl_realloc;
	jl_free = pv && pv->free ? pv->free : jl_free;

	return JS_TRUE;
}

/* not needed
#if !defined NO_DllMain && defined XP_WIN && defined _LIB
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

	if ( fdwReason == DLL_PROCESS_ATTACH )
		DisableThreadLibraryCalls(hinstDLL);
	return TRUE;
}
#endif // XP_WIN
*/

/* MS doc:

	To illustrate this, consider the following example:

	   - .EXE is linked with MSVCRT.LIB
	   - DLL A is linked with LIBCMT.LIB
	   - DLL B is linked with CRTDLL.LIB

	If the .EXE creates a CRT file handle using _create() or _open(), this file handle may only be passed to _lseek(), _read(), _write(), _close(), etc. in the .EXE file. Do not pass this CRT file handle to either DLL. Do not pass a CRT file handle obtained from either DLL to the other DLL or to the .EXE.
	If DLL A allocates a block of memory with malloc(), only DLL A may call free(), _expand(), or realloc() to operate on that block. You cannot call malloc() from DLL A and try to free that block from the .EXE or from DLL B.
	NOTE: If all three modules were linked with CRTDLL.LIB or all three were linked with MSVCRT.LIb, these restrictions would not apply.
	 (source: http://support.microsoft.com/kb/94248)
*/
