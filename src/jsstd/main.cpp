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

bool
ModuleInit(JSContext *cx, JS::HandleObject obj) {

	JLDisableThreadNotifications();

	JL_ASSERT(jl::Host::getHost(cx).checkCompatId(JL_HOST_VERSIONID), E_MODULE, E_NOTCOMPATIBLE, E_HOST );

	ModulePrivate *mpv;
	mpv = (ModulePrivate*)jl_malloc(sizeof(ModulePrivate));
	JL_ASSERT_ALLOC( mpv );
	jl::Host::getHost(cx).moduleManager().modulePrivate(moduleId()) = mpv;

	INIT_STATIC();

	return true;
	JL_BAD;
}


bool
ModuleRelease(JSContext *cx) {

	jl::Host &host = jl::Host::getHost(cx);
	if ( host.hostRuntime().skipCleanup() ) // do not cleanup in unsafe mode.
		return true;

	ModulePrivate *mpv = (ModulePrivate*)host.moduleManager().modulePrivate(moduleId());

	jl_free(mpv);

	return true;
}
