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

//#define MODULE_NAME "jsstd"
//static const char *_revision = "$Rev$";

#include "static.h"
#include "data.h"
#include "buffer.h"
#include "pack.h"


DEFINE_UNSAFE_MODE;


extern JSFunction *stdoutFunction;
JSFunction *stdoutFunction = NULL;


EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj) {

// read configuration
	jsval stdoutFunctionValue = GetConfigurationValue(cx, NAME_CONFIGURATION_STDOUT);
//	RT_ASSERT( stdoutFunctionValue != JSVAL_VOID, "Unable to read stdout function from configuration object." );
	stdoutFunction = JS_ValueToFunction(cx, stdoutFunctionValue); // returns NULL if the function is not defined

	SET_UNSAFE_MODE( GetConfigurationValue(cx, NAME_CONFIGURATION_UNSAFE_MODE ) == JSVAL_TRUE );

	INIT_STATIC();
	INIT_CLASS( Data );
	INIT_CLASS( Buffer );
	INIT_CLASS( Pack );

	return JS_TRUE;
}

EXTERN_C DLLEXPORT JSBool ModuleRelease(JSContext *cx) {

	return JS_FALSE;
}

EXTERN_C DLLEXPORT void ModuleFree() {
}
