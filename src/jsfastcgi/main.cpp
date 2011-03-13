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
#include <jslibsModule.cpp>

DECLARE_STATIC()

//#include "fastcgi.h"


EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj, uint32_t id) {

//	_unsafeMode = JL_GetHostPrivate(cx)->unsafeMode;

	JL_CHK( InitJslibsModule(cx, id)  );

//	INIT_CLASS( FastCGI );
	INIT_STATIC();

	return JS_TRUE;
	JL_BAD;
}
