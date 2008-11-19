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
#include "map.h"

#include <jsxdrapi.h>

//#include "../common/jsNativeInterface.h"
#include "../common/queue.h"


/**doc
$CLASS_HEADER
 Map is an unsorted associative container that associates strings with values.
 $H note
  No two elements have the same key.
 $H note
  Map can store any string value as a key (even reserved special strings like __proto__, __parent__, toString, ...).
 $H note
  Constructing the object is not mendatory. Calling Map() will return a new Map object;
**/

BEGIN_CLASS( Map )

/**doc
 * $INAME( [ obj ] )
  Creates a new Map object.
  $H arguments
   $ARG object obj: if given, _obj_ is converted into a Map object (Array is also an object). No recursion is made.
**/
DEFINE_CONSTRUCTOR() {
		
	if ( JS_IsConstructing(cx) == JS_TRUE ) {

		J_S_ASSERT_THIS_CLASS();
		J_CHK( JS_SetPrototype(cx, obj, NULL) );
	} else {

		//Doc: JS_NewObject, JS_NewObjectWithGivenProto behaves exactly the same, except that if proto is NULL, it creates an object with no prototype.
		obj = JS_NewObjectWithGivenProto(cx, _class, NULL, NULL);
		J_S_ASSERT_ALLOC( obj );
		*rval = OBJECT_TO_JSVAL(obj);
	}

	if ( J_ARG_ISDEF(1) ) {

		jsid id;
		jsval key, value;
		JSObject *srcObj; // (TBD) root ?
		J_CHK( JS_ValueToObject(cx, J_ARG(1), &srcObj) );
		JSObject *it = JS_NewPropertyIterator(cx, srcObj);
		J_S_ASSERT( it != NULL, "Unable to iterate the object." );
		for (;;) {

			J_CHK( JS_NextProperty(cx, it, &id) );
			if ( id != JSVAL_VOID ) {

				J_CHK( JS_IdToValue(cx, id, &key) );
				J_CHK( OBJ_GET_PROPERTY(cx, srcObj, id, &value) );
				J_CHK( OBJ_SET_PROPERTY(cx, obj, id, &value) ); 
			} else {

				break;
			}
		}	
	}
	return JS_TRUE;
	JL_BAD;
}


DEFINE_XDR() {
	
	jsid id;
	jsval key, value;

	if ( xdr->mode == JSXDR_ENCODE ) {
		
		JSObject *it = JS_NewPropertyIterator(xdr->cx, *objp);

		for (;;) {

			J_CHK( JS_NextProperty(xdr->cx, it, &id) );
			if ( id != JSVAL_VOID ) { // ... or JSVAL_VOID if there is no such property left to visit.

				J_CHK( JS_IdToValue(xdr->cx, id, &key) );
				J_CHK( OBJ_GET_PROPERTY(xdr->cx, *objp, id, &value) ); // returning false on error or exception, true on success.
				J_CHK( JS_XDRValue(xdr, &key) );
				J_CHK( JS_XDRValue(xdr, &value) );
			} else {

				jsval tmp = JSVAL_VOID;
				J_CHK( JS_XDRValue(xdr, &tmp) );
				break;
			}
		}
		return JS_TRUE;
	}
	
	if ( xdr->mode == JSXDR_DECODE ) {

		*objp = JS_NewObject(xdr->cx, _class, NULL, NULL);
		
		for (;;) {

			J_CHK( JS_XDRValue(xdr, &key) );
			if ( key != JSVAL_VOID ) {

				JS_ValueToId(xdr->cx, key, &id);
				J_CHK( JS_XDRValue(xdr, &value) );
				J_CHK( OBJ_SET_PROPERTY(xdr->cx, *objp, id, &value) ); 
			} else {

				break;
			}
		}
		return JS_TRUE;
	}

	if ( xdr->mode == JSXDR_FREE ) {

		// (TBD) nothing to free ?
		return JS_TRUE;
	}

	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS
	HAS_CONSTRUCTOR
	HAS_XDR
END_CLASS


/**doc
=== example 1 ===
 {{{
 var m = new Map();
 m.a = 1;
 m.b = 2;
 m.c = 3;

 Print( m.c ); // prints: 3
 }}}

=== example 2 ===
 {{{
 var m = new Map([1,2,3,4]);
 Print( [k+'='+v for ([k,v] in Iterator(m))] ); // prints: 3=4,2=3,1=2,0=1
 }}}
**/
