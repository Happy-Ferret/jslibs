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

struct ReleaseModule : jl::Observer<const jl::EventAfterDestroyRuntime> {
	jl::HostRuntime &_hostRuntime;
	JsioPrivate *_mpv;
	
	ReleaseModule(jl::HostRuntime &hostRuntime, JsioPrivate *mpv) :
		_hostRuntime(hostRuntime),
		_mpv(mpv) {
	}

	bool operator()( EventType &ev ) {
		
		ASSERT( _hostRuntime );

		if ( !_hostRuntime.skipCleanup() ) {
	
			if ( _mpv->peCancel != NULL )
				PR_DestroyPollableEvent(_mpv->peCancel);

			jl_free(_mpv);

			if ( PR_AtomicDecrement(&instanceCount) == 0 ) {

				ASSERT( PR_Initialized() );
				PR_Cleanup(); // doc. PR_Cleanup must be called by the primordial thread near the end of the main function.
				// (TBD) manage PR_Cleanup's return value
			}
		}

		return true;
	}
};

bool
ModuleInit(JSContext *cx, JS::HandleObject obj) {

//	JLDisableThreadNotifications();

	JL_ASSERT(jl::Host::getJLHost(cx)->checkCompatId(JL_HOST_VERSIONID), E_MODULE, E_NOTCOMPATIBLE, E_HOST );

	//ASSERT( !PR_Initialized() ); // is already initialized by the JS engine 

	if ( PR_AtomicIncrement(&instanceCount) == 1 ) {

		if ( PR_Initialized() ) {
		
			PR_AtomicIncrement(&instanceCount);
		} else {

			PR_Init(PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 0); // NSPR ignores threads of type PR_SYSTEM_THREAD when determining when a call to PR_Cleanup should return.

			// PR_DisableClockInterrupts();  is this compatible with the JS engine ?
		}
	}

	JsioPrivate *mpv;
	mpv = (JsioPrivate*)jl_calloc(sizeof(JsioPrivate), 1);
	JL_ASSERT_ALLOC( mpv );
	jl::Host::getJLHost(cx)->moduleManager().modulePrivate(moduleId()) = mpv;

	jl::HostRuntime &hostRuntime = jl::HostRuntime::getJLRuntime(cx);
	hostRuntime.addObserver(new ReleaseModule(hostRuntime, mpv)); // frees mpv after rt and cx has been destroyed

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
