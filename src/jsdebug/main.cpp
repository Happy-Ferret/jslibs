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
#include <js/OldDebugAPI.h> // JS_DefineDebuggerObject


DECLARE_STATIC()


/**doc t:header
$MODULE_HEADER
 Various debug tools.
$FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/

bool
ModuleInit(JSContext *cx, JS::HandleObject obj) {

	JL_CHK( JS_DefineDebuggerObject(cx, obj) ); // doc: https://developer.mozilla.org/en/SpiderMonkey/JS_Debugger_API_Guide

	JLDisableThreadNotifications();

	JL_ASSERT(jl::Host::getJLHost(cx).checkCompatId(JL_HOST_VERSIONID), E_MODULE, E_NOTCOMPATIBLE, E_HOST );

	INIT_STATIC();

	return true;
	JL_BAD;
}
