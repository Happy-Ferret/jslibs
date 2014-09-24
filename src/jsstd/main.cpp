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
#include "jsstd.h"

#include <jslibsModule.h>

DECLARE_STATIC()

/**doc t:header
$MODULE_HEADER
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
		if ( !_hostRuntime.skipCleanup() )
			jl_free(_mpv);
		return true;
	}
};


bool
ModuleInit(JSContext *cx, JS::HandleObject obj) {

	JLDisableThreadNotifications();

	JL_ASSERT(jl::Host::getJLHost(cx).checkCompatId(JL_HOST_VERSIONID), E_MODULE, E_NOTCOMPATIBLE, E_HOST );

	ModulePrivate *mpv;
	mpv = (ModulePrivate*)jl_malloc(sizeof(ModulePrivate));
	JL_ASSERT_ALLOC( mpv );
	jl::Host::getJLHost(cx).moduleManager().modulePrivate(moduleId()) = mpv;

	jl::HostRuntime &hostRuntime = jl::HostRuntime::getJLRuntime(cx);
	hostRuntime.addListener(jl::HostRuntimeEvents::AFTER_DESTROY_RUNTIME, new ReleaseModule(hostRuntime, mpv)); // frees mpv after rt and cx has been destroyed

	INIT_STATIC();

	return true;
	JL_BAD;
}
