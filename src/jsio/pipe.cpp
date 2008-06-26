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

#include "descriptor.h"
#include "pipe.h"

/**doc
$CLASS_HEADER
**/
BEGIN_CLASS( Pipe )

DEFINE_FINALIZE() {

	FinalizeDescriptor(cx, obj); // defined in descriptor.cpp
}

/*
DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING( _class );
	RT_ASSERT_ARGC(1);
	JS_SetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, J_ARG(1) );
	JS_SetPrivate(cx, obj, NULL); // (TBD) optional ?
	return JS_TRUE;
}
*/


CONFIGURE_CLASS

	HAS_PROTOTYPE( prototypeDescriptor )

//	HAS_CONSTRUCTOR
	HAS_FINALIZE

	HAS_PRIVATE
	HAS_RESERVED_SLOTS( 1 ) // SLOT_JSIO_DESCRIPTOR_IMPORTED

	BEGIN_FUNCTION_SPEC
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
	END_PROPERTY_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
	END_STATIC_PROPERTY_SPEC

	BEGIN_CONST_DOUBLE_SPEC
	END_CONST_DOUBLE_SPEC

END_CLASS
