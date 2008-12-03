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

#include "descriptor.h"
#include "pipe.h"
#include "file.h"
#include "memoryMapped.h"
#include "socket.h"
#include "directory.h"
#include "sharedMemory.h"
#include "semaphore.h"
#include "process.h"
#include "static.h"

extern bool _unsafeMode = false;

static PRInt32 instanceCount = 0;

/**doc t:header
$MODULE_HEADER
 This module is based on Netscape Portable Runtime (NSPR) that provides a platform-neutral API for system level and libc like functions.
 NSPR API is used in the Mozilla client, many of Netscape/AOL/iPlanet's and other software offerings.
**/

/**doc t:footer
$MODULE_FOOTER
**/

EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	if ( instanceCount == 0 && !PR_Initialized() )
		PR_Init(PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 0); // NSPR ignores threads of type PR_SYSTEM_THREAD when determining when a call to PR_Cleanup should return. 
	PR_AtomicIncrement(&instanceCount);

	_unsafeMode = GetHostPrivate(cx)->unsafeMode;

	INIT_CLASS( IoError );
	INIT_CLASS( Descriptor );
	INIT_CLASS( Pipe );
	INIT_CLASS( File );
	INIT_CLASS( MemoryMapped );
	INIT_CLASS( Socket );
	INIT_CLASS( Directory );
	INIT_CLASS( SharedMemory );
	INIT_CLASS( Semaphore );
	INIT_CLASS( Process );
	INIT_STATIC();
	return JS_TRUE;
	JL_BAD;
}

EXTERN_C DLLEXPORT JSBool ModuleRelease(JSContext *cx) {

	return JS_FALSE;
}

EXTERN_C DLLEXPORT void ModuleFree() {

	PR_AtomicDecrement(&instanceCount);
	if ( instanceCount == 0 && PR_Initialized() )
		PR_Cleanup();
}

#ifdef XP_WIN
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

	if ( fdwReason == DLL_PROCESS_ATTACH )
		DisableThreadLibraryCalls(hinstDLL);
	return TRUE;
}
#endif // XP_WIN
