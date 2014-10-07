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

#include "com.h"

#include "error.h"

DECLARE_CLASS( WinAudioError )
DECLARE_STATIC()
DECLARE_CLASS( Icon )
DECLARE_CLASS( Systray )
DECLARE_CLASS( Console )
DECLARE_CLASS( AudioIn )


/**doc t:header
$MODULE_HEADER
$FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/

bool ModuleInit(JSContext *cx, JS::HandleObject obj) {
	//JLDisableThreadNotifications();
	JL_ASSERT(jl::Host::getJLHost(cx)->checkCompatId(JL_HOST_VERSIONID), E_MODULE, E_NOTCOMPATIBLE, E_HOST );

	INIT_CLASS( WinError );
	INIT_CLASS( WinAudioError );
	INIT_STATIC();
	INIT_CLASS( Icon );
	INIT_CLASS( Systray );
	INIT_CLASS( Console );
	INIT_CLASS( AudioIn );

	HRESULT hr;
	hr = CoInitialize(NULL);
	JL_ASSERT( SUCCEEDED(hr), E_NAME("COM"), E_INIT );


	struct ReleaseModule : jl::Observer<const jl::EventAfterDestroyRuntime> {
		bool operator()( EventType &ev ) {
		
			// Closes the COM library on the current thread, unloads all DLLs loaded by the thread, frees any other resources that the thread maintains, and forces all RPC connections on the thread to close.
			CoUninitialize();
			return true;
		}
	};

	jl::HostRuntime::getJLRuntime(cx).addObserver(new ReleaseModule); // frees mpv after rt and cx has been destroyed


	INIT_CLASS( ComVariant );
	INIT_CLASS( ComEnum );
	INIT_CLASS( ComDispatch );

	return true;
	JL_BAD;
}
