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
#include "static.h"

static bool _defaultUnsafeMode = false;
extern bool *_pUnsafeMode = &_defaultUnsafeMode;

/**doc t:header
#summary jsio module
#labels doc
- [http://jslibs.googlecode.com/svn/trunk/jsio/ source] - [JSLibs main] -
= jsio module =
 This module is based on Netscape Portable Runtime (NSPR) that provides a platform-neutral API for system level and libc like functions.
 NSPR API is used in the Mozilla client, many of Netscape/AOL/iPlanet's and other software offerings.
**/

/**doc t:footer
----
- [http://jslibs.googlecode.com/svn/trunk/jsio/ source] - [#jsio_module top] - [JSLibs main] -
**/

EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	jsval unsafeModePtrVal;
	J_CHK( GetConfigurationValue(cx, NAME_CONFIGURATION_UNSAFE_MODE_PTR, &unsafeModePtrVal) );
	if ( unsafeModePtrVal != JSVAL_VOID )
		_pUnsafeMode = (bool*)JSVAL_TO_PRIVATE(unsafeModePtrVal);


	INIT_CLASS( IoError );
	INIT_CLASS( Descriptor );
	INIT_CLASS( Pipe );
	INIT_CLASS( File );
	INIT_CLASS( MemoryMapped );
	INIT_CLASS( Socket );
	INIT_CLASS( Directory );
	INIT_CLASS( SharedMemory );
	INIT_CLASS( Semaphore );
	INIT_STATIC();
	return JS_TRUE;
}

EXTERN_C DLLEXPORT JSBool ModuleRelease(JSContext *cx) {

	return JS_FALSE;
}

EXTERN_C DLLEXPORT void ModuleFree() {

	if ( PR_Initialized() == PR_TRUE )
		PR_Cleanup();
}

#ifdef XP_WIN
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

	if ( fdwReason == DLL_PROCESS_ATTACH )
		DisableThreadLibraryCalls(hinstDLL);
	return TRUE;
}
#endif // XP_WIN
