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
#include "../common/jslibsModule.h"

DECLARE_STATIC()
DECLARE_CLASS( Map )
DECLARE_CLASS( Buffer )
DECLARE_CLASS( Pack )
DECLARE_CLASS( OperationLimit )
DECLARE_CLASS( Sandbox )
DECLARE_CLASS( ObjEx )

/**doc t:header
$MODULE_HEADER
$FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/

EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	JL_CHK( InitJslibsModule(cx) );

	INIT_STATIC();
	INIT_CLASS( Map );
	INIT_CLASS( OperationLimit ); // exception
	INIT_CLASS( Sandbox );
	INIT_CLASS( Buffer );
	INIT_CLASS( Pack );
	INIT_CLASS( ObjEx );
	return JS_TRUE;
	JL_BAD;
}

EXTERN_C DLLEXPORT JSBool ModuleRelease(JSContext *cx) {

	REMOVE_STATIC();
	REMOVE_CLASS( Map );
	REMOVE_CLASS( OperationLimit ); // exception
	REMOVE_CLASS( Sandbox );
	REMOVE_CLASS( Buffer );
	REMOVE_CLASS( Pack );
	REMOVE_CLASS( ObjEx );
	return JS_TRUE;
	JL_BAD;
}
