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
#include "jlhelper.cpp"

#include "com.h"

DECLARE_STATIC()

#include "error.h"
#include "icon.h"
#include "systray.h"
#include "console.h"

#include "jslibsModule.cpp"


/**doc t:header
$MODULE_HEADER
$FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/

EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj, uint32_t id) {

	JL_CHK( InitJslibsModule(cx, id)  );

	INIT_CLASS( WinError );
	INIT_STATIC();
	INIT_CLASS( Icon );
	INIT_CLASS( Systray );
	INIT_CLASS( Console );

	HRESULT hr;
	hr = CoInitialize(NULL);
	JL_S_ASSERT( SUCCEEDED(hr), "Unable to initialize COM sub system." );
	INIT_CLASS( ComVariant );
	INIT_CLASS( ComEnum );
	INIT_CLASS( ComDispatch );

	return JS_TRUE;
	JL_BAD;
}


EXTERN_C DLLEXPORT JSBool ModuleRelease(JSContext *cx) {

	return JS_TRUE;
}

EXTERN_C DLLEXPORT void ModuleFree() {

	CoUninitialize();
}
