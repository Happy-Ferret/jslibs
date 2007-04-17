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
#include "fastcgi.h"
/*
#include "../common/jsNativeInterface.h"
GetNativeInterface(cx, JSVAL_TO_OBJECT(argv[0]), NI_READ_RESOURCE, (FunctionPointer*)&desc->read, &desc->pv);
RT_ASSERT( desc->read != NULL && desc->pv != NULL, "Unable to GetNativeResource." );
*/


JSObject *CreateObjectFromPairs( JSContext *cx ) {


	return NULL;
}

/*
typedef struct State {
	int bufferLength

} State;
*/

BEGIN_CLASS( FastCGI )

DEFINE_FINALIZE() {
}

DEFINE_CONSTRUCTOR() {


	RT_ASSERT_CONSTRUCTING(_class);
	return JS_TRUE;
}


DEFINE_FUNCTION(Send) {
	
	RT_ASSERT_ARGC(1);
	unsigned char *buffer = (unsigned char *)JS_GetPrivate(cx, obj);




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
