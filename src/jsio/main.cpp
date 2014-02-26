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
#include <jslibsModule.h>

DECLARE_CLASS( IoError )
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

bool
ModuleInit(JSContext *cx, JS::HandleObject obj) {

	JLDisableThreadNotifications();

	JL_ASSERT(jl::Host::getHost(cx).checkCompatId(JL_HOST_VERSIONID), E_MODULE, E_NOTCOMPATIBLE, E_HOST );

	if ( instanceCount == 0 && !PR_Initialized() ) {

		PR_Init(PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 0); // NSPR ignores threads of type PR_SYSTEM_THREAD when determining when a call to PR_Cleanup should return.
		PR_DisableClockInterrupts();
	}
	PR_AtomicIncrement(&instanceCount);

	JsioPrivate *mpv;
	mpv = (JsioPrivate*)jl_calloc(sizeof(JsioPrivate), 1);
	JL_ASSERT_ALLOC( mpv );
	jl::Host::getHost(cx).moduleManager().modulePrivate(moduleId()) = mpv;

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

	return true;
	JL_BAD;
}

bool ModuleRelease(JSContext *cx) {

	jl::Host &host = jl::Host::getHost(cx);
	JsioPrivate *mpv = (JsioPrivate*)host.moduleManager().modulePrivate(moduleId());

	if ( host.hostRuntime().skipCleanup() ) // do not cleanup in unsafe mode.
		return true;

	if ( mpv->peCancel != NULL )
		PR_DestroyPollableEvent(mpv->peCancel);

	jl_free(mpv);


	return true;
//	JL_BAD;
}

void ModuleFree(bool skipCleanup) {

	PR_AtomicDecrement(&instanceCount);
	if ( !skipCleanup && instanceCount == 0 && PR_Initialized() ) {

		PR_Cleanup(); // doc. PR_Cleanup must be called by the primordial thread near the end of the main function.
		// (TBD) manage PR_Cleanup's return value
	}
}
