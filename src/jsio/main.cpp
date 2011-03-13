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
#include <jlhelper.cpp>
#include <jslibsModule.cpp>


DECLARE_CLASS( Descriptor )
DECLARE_CLASS( Pipe )
DECLARE_CLASS( File )
DECLARE_CLASS( MemoryMapped )
DECLARE_CLASS( Socket )
DECLARE_CLASS( Directory )
DECLARE_CLASS( SharedMemory )
DECLARE_CLASS( Semaphore )
DECLARE_CLASS( Process )
DECLARE_STATIC()


static PRInt32 instanceCount = 0;

/**doc t:header
$MODULE_HEADER
 This module is based on Netscape Portable Runtime (NSPR) that provides a platform-neutral API for system level and libc like functions.
 NSPR API is used in the Mozilla client, many of Netscape/AOL/iPlanet's and other software offerings.
 $FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/

JSBool ModuleInit(JSContext *cx, JSObject *obj, uint32_t id) {

	JL_CHK( InitJslibsModule(cx, id)  );

	if ( instanceCount == 0 && !PR_Initialized() ) {

		PR_Init(PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 0); // NSPR ignores threads of type PR_SYSTEM_THREAD when determining when a call to PR_Cleanup should return.
		PR_DisableClockInterrupts();
	}
	PR_AtomicIncrement(&instanceCount);

	JL_CHKM( JL_SetModulePrivate(cx, _moduleId, jl_calloc(sizeof(JsioPrivate), 1)), E_MODULE, E_INIT );

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

JSBool ModuleRelease(JSContext *cx) {

	JsioPrivate *mpv = (JsioPrivate*)JL_GetModulePrivate(cx, _moduleId);

	if ( JL_GetHostPrivate(cx)->canSkipCleanup ) // do not cleanup in unsafe mode.
		return JS_TRUE;

	if ( mpv->peCancel != NULL )
		PR_DestroyPollableEvent(mpv->peCancel);

	jl_free(mpv);

	return JS_TRUE;
//	JL_BAD;
}

void ModuleFree() {

	PR_AtomicDecrement(&instanceCount);
	if ( instanceCount == 0 && PR_Initialized() ) {

		PR_Cleanup(); // doc. PR_Cleanup must be called by the primordial thread near the end of the main function.
		// (TBD) manage PR_Cleanup's return value
	}
}
