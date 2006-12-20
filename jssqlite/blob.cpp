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
#include "blob.h"

BEGIN_CLASS( Blob )

DEFINE_CONSTRUCTOR() {

	RT_ASSERT_ARGC(1);
	if ( JS_IsConstructing(cx) == JS_FALSE ) { // supports this form (w/o new operator) : result.param1 = Blob('Hello World');

		obj = JS_NewObject(cx, &classBlob, NULL, NULL);
		RT_ASSERT( obj != NULL, "Blob construction failed." );
		*rval = OBJECT_TO_JSVAL(obj);
	}
	JS_SetReservedSlot(cx, obj, SLOT_BLOB_DATA, argv[0]);
	return JS_TRUE;
}

DEFINE_CONVERT() {

	JS_GetReservedSlot(cx, obj, SLOT_BLOB_DATA, vp); // (TBD) why we cannot use this: if ( type == JSTYPE_STRING )
	return JS_TRUE;
}

CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_CONVERT
	HAS_RESERVED_SLOTS(1)

END_CLASS
