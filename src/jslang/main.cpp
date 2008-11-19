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
#include "id.h"


/**doc t:header
$MODULE_HEADER
 This module contains all common classes used by other jslibs modules.
 $H note
  This module is automatically loaded by jshost and jswinhost. Then LoadModule call is not needed.
**/

/**doc t:footer
$MODULE_FOOTER
**/

JSBool jslangInit(JSContext *cx, JSObject *obj) {

//	jsval unsafeModePtrVal;
//	J_CHK( GetConfigurationValue(cx, NAME_CONFIGURATION_UNSAFE_MODE_PTR, &unsafeModePtrVal) );
//	if ( !JSVAL_IS_VOID( unsafeModePtrVal ) )
//		_pUnsafeMode = (bool*)JSVAL_TO_PRIVATE(unsafeModePtrVal);


	JSObject *globalObject = JS_GetGlobalObject(cx);
	J_S_ASSERT( obj == globalObject, "This module must be load into the global namespace" );
//	obj = JS_GetGlobalObject(cx); // avoid LoadModule.call( foo, 'jslang' );

	INIT_CLASS( Id );
	INIT_CLASS( Blob );
	INIT_CLASS( Stream );
	INIT_STATIC();

	return JS_TRUE;
	JL_BAD;
}
