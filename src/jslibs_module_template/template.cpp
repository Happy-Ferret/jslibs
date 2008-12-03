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
#include "template.h"

/**doc
$CLASS_HEADER
 $SVN_REVISION $Revision$
**/
BEGIN_CLASS( Template ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

DEFINE_FINALIZE() { // called when the Garbage Collector is running if there are no remaing references to this object.
}

DEFINE_CONSTRUCTOR() { // Called when the object is constructed ( a = new Template() ) or activated ( a = Template() ). To distinguish the cases, use JS_IsConstructing() or use the J_S_ASSERT_CONSTRUCTING() macro.

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	return JS_TRUE;
	JL_BAD;
}

//DEFINE_FUNCTION( Call ) {
//	return JS_TRUE;
//}

//DEFINE_PROPERTY( prop ) {
//	return JS_TRUE;
//}

//DEFINE_FUNCTION( Func ) {
//	return JS_TRUE;
//}



//DEFINE_XDR() {
//	
//	if ( xdr->mode == JSXDR_ENCODE ) {
//
//		jsval tmp;
//		J_CHK( JS_GetReservedSlot(xdr->cx, *objp, 0, &tmp) );
//		JS_XDRValue(xdr, &tmp);
//		return JS_TRUE;
//	}
//
//	if ( xdr->mode == JSXDR_DECODE ) {
//
//		*objp = JS_NewObject(xdr->cx, _class, NULL, NULL);
//		jsval tmp;
//		JS_XDRValue(xdr, &tmp);
//		J_CHK( JS_SetReservedSlot(xdr->cx, *objp, 0, tmp) );
//		return JS_TRUE;
//	}
//
//	if ( xdr->mode == JSXDR_FREE ) {
//
//		return JS_TRUE;
//	}
//
//	JL_BAD;
//}



CONFIGURE_CLASS // This section containt the declaration and the configuration of the class

//	HAS_XDR


	REVISION(SvnRevToInt("$Revision$"))
	HAS_PRIVATE
//	HAS_RESERVED_SLOTS(1)

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
//		FUNCTION(Func)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
//		PROPERTY(prop)
	END_PROPERTY_SPEC

END_CLASS
