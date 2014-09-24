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
#include "jslang.h"


DECLARE_CLASS( Handle )
DECLARE_CLASS( Blob )
DECLARE_CLASS( Stream )
DECLARE_CLASS( Serializer )
DECLARE_CLASS( Unserializer )
DECLARE_STATIC()

/**doc t:header
$MODULE_HEADER
 This module contains all common classes used by other jslibs modules.
 $H note
  This module is automatically loaded by jshost and jswinhost. Then loadModule call is not needed.
$FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/


struct ReleaseModule : jl::Callback {
	jl::HostRuntime &_hostRuntime;
	ModulePrivate *_mpv;
	
	ReleaseModule(jl::HostRuntime &hostRuntime, ModulePrivate *mpv)
	: _hostRuntime(hostRuntime), _mpv(mpv) {
	}

	bool operator()() {
		
		ASSERT( _hostRuntime );
		ASSERT( _mpv );

		for ( size_t i = 0; i < COUNTOF(_mpv->processEventThreadInfo); ++i ) {

			ProcessEventThreadInfo *ti = &_mpv->processEventThreadInfo[i];
			if ( ti->thread != JLThreadInvalidHandler ) {

				ti->isEnd = true;
				ASSERT( ti->startSem );
				JLSemaphoreRelease(ti->startSem);
				JLThreadWait(ti->thread);
			}

			if ( ti->startSem != JLInvalidSemaphoreHandler ) {

				JLSemaphoreFree(&ti->startSem);
			}
		}
		JLSemaphoreFree(&_mpv->processEventSignalEventSem);

		jl_free( _mpv );

		return true;
	}
};


bool
jslangModuleInit(JSContext *cx, JS::HandleObject obj) {

	ModulePrivate *mpv = (ModulePrivate*)jl_calloc(sizeof(ModulePrivate), 1);
	jl::Host::getJLHost(cx).moduleManager().modulePrivate(localModuleId(jslangModuleInit)) = mpv;
	mpv->processEventSignalEventSem = JLSemaphoreCreate(0);

	jl::HostRuntime &hostRuntime = jl::HostRuntime::getJLRuntime(cx);
	hostRuntime.addListener(jl::HostRuntimeEvents::AFTER_DESTROY_RUNTIME, new ReleaseModule(hostRuntime, mpv)); // frees mpv after rt and cx has been destroyed

	INIT_CLASS( Handle );
	INIT_CLASS( Blob );
	INIT_CLASS( Stream );
	INIT_CLASS( Serializer );
	INIT_CLASS( Unserializer );
	INIT_STATIC();

	return true;
	JL_BAD;
}
