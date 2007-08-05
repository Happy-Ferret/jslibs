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

#define MODULE_NAME "jstemplate"
static const char *_revision = "$Rev$";

#include "jswindow.h"
#include "jsgl.h"
#include "jstransformation.h"

DEFINE_UNSAFE_MODE

extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	JS_DefineProperty(cx, obj, MODULE_NAME "_build", INT_TO_JSVAL(atoi(_revision+6)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ); // 6 is the size of "$Rev: "

	SET_UNSAFE_MODE( GetConfigurationValue(cx, "unsafeMode" ) == JSVAL_TRUE );

#ifdef XP_WIN
	INIT_CLASS( Window )
#endif

	INIT_CLASS( Transformation )
	INIT_CLASS( Gl )
	return JS_TRUE;
}
