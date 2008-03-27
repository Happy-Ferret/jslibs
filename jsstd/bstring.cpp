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
#include "bstring.h"

#include "../common/jsNativeInterface.h"


BEGIN_CLASS( BString )

DEFINE_FINALIZE() {
}

DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING( _class );
	RT_ASSERT_ARGC( 1 );
//	RT_ASSERT_OBJECT( J_ARG(1) );
//	RT_ASSERT_CLASS( JSVAL_TO_OBJECT( J_ARG(1) ), &classBuffer );
//	RT_CHECK_CALL( JS_SetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, J_ARG(1)) );
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( Add ) {
	
	RT_ASSERT_ARGC( 1 );
	char *pv = (char*)JS_GetPrivate(cx, J_FOBJ);
	RT_ASSERT_RESOURCE( pv );

	jsval val;
	RT_CHECK_CALL( JS_GetReservedSlot(cx, J_FOBJ, BSTRING_SLOT_LENGTH, &val) );

//	int length;



//	pv = realloc( pv, 

	return JS_TRUE;
}


DEFINE_RESOLVE() {
	
	return JS_TRUE;
}

CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_RESOLVE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST( Add )
//		FUNCTION_ALIAS(toString, Read) // ised when the buffer has to be transformed into a string
//		FUNCTION_ALIAS(valueOf, Read) // ised when the buffer has to be transformed into a string
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
//		PROPERTY_READ(length)
	END_PROPERTY_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS

