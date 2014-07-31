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

#include <videoinput.h>



DECLARE_CLASS( VideoInput )

bool
ModuleInit( JSContext *cx, JS::HandleObject obj ) {

	JLDisableThreadNotifications();

	JL_ASSERT( jl::Host::getJLHost( cx ).checkCompatId( JL_HOST_VERSIONID ), E_MODULE, E_NOTCOMPATIBLE, E_HOST );

//	JL_ASSERT( vi == NULL, E_LIB, E_STR("videoinput"), E_INIT ); // "Invalid case: videoInput already initialized'"


	videoInput::setVerbose(false);
	videoInput *vi = new videoInput();
	JL_ASSERT_ALLOC( vi );
	jl::Host::getJLHost( cx ).moduleManager().modulePrivateT<videoInput*>( moduleId() ) = vi;

		
	struct ReleaseModule : jl::Events::Callback {
		jl::HostRuntime &_hostRuntime;
		ModulePrivate *_mpv;
	
		ReleaseModule(jl::HostRuntime &hostRuntime, ModulePrivate *mpv)
		: _hostRuntime(hostRuntime), _mpv(mpv) {
		}

		bool operator()() {
		
			ASSERT( _hostRuntime );
			if ( _hostRuntime.skipCleanup() )
				return;

			if ( _mpv != NULL )
				delete static_cast<videoInput*>(_mpv);


			return true;
		}
	};

	jl::HostRuntime &hostRuntime = jl::HostRuntime::getJLRuntime(cx);
	hostRuntime.addListener(jl::EventId::AFTER_DESTROY_RUNTIME, new ReleaseModule(hostRuntime, vi)); // frees mpv after rt and cx has been destroyed


	INIT_CLASS( VideoInput );

	return true;
	JL_BAD;
}
