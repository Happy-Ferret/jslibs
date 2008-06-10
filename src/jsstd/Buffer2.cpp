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
#include <cstring>

#include "buffer.h"

#include "../common/queue.h"
#include "../jslang/bstringapi.h"

#define BUFFER2_SLOT_QUEUE 0

BEGIN_CLASS( Buffer2 )


DEFINE_FINALIZE() {

}


DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();

	JSObject *queue = JS_NewArrayObject(cx, 0, NULL);

//	JS_SetElement(cx, queue, 

	

	return JS_TRUE;
}

DEFINE_FUNCTION_FAST( Write ) {


	return JS_TRUE;
}

DEFINE_PROPERTY( length ) {

	return JS_TRUE;
}



CONFIGURE_CLASS

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST(Write)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(length)
	END_PROPERTY_SPEC

END_CLASS
