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
#include "buffer.h"

#include "../common/jsNativeInterface.h"


static bool NativeInterfaceReadBuffer( void *pv, unsigned char *buf, unsigned int *amount ) {
/*
	PRInt32 tmp = *amount;
	PRInt32 status = PR_Read( (PRFileDesc *)pv, buf, tmp );
	*amount = tmp;
	if ( status == -1 )
		return false;
	*amount = status;
*/

// 	SetNativeInterface(cx, obj, NI_READ_RESOURCE, (FunctionPointer)NativeInterfaceReadBuffer, fd);
	return true;
}

/*
			JS_GetProperty(cx, itemObject, "onunderflow", &itemVal);
			if ( itemVal != JSVAL_VOID && JS_TypeOfValue(cx, itemVal) == JSTYPE_FUNCTION )
				RT_CHECK_CALL( CallFunction(cx, obj, itemVal, &itemVal, 2, item, key) );
*/


BEGIN_CLASS( Buffer )

DEFINE_FINALIZE() {
}

DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING( _class );
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

//DEFINE_FUNCTION( Call ) {
//	return JS_TRUE;
//}

//DEFINE_PROPERTY( prop ) {
//	return JS_TRUE;
//}

//DEFINE_FUNCTION( Func ) {
//	return JS_TRUE;
//}

	BEGIN_FUNCTION_SPEC
//		FUNCTION(Func)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
//		PROPERTY(prop)
	END_PROPERTY_SPEC

	HAS_PRIVATE
//	HAS_RESERVED_SLOTS(1)

END_CLASS
