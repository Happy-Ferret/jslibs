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

//#include "../common/jsNativeInterface.h"
#include "../common/queue.h"


/**doc
$CLASS_HEADER
 Map is an unsorted associative container that associates strings with values.
 $H note
  Note that no two elements have the same key.
 $H note
  Map can store any string value as a key (event reserved special strings like __proto__, __parent__, toString, ...).
**/

BEGIN_CLASS( Map )

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
	return JS_TRUE;
}

CONFIGURE_CLASS
	HAS_CONSTRUCTOR
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
**/
