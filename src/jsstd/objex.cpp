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


#define ADD_SLOT 0
#define DEL_SLOT 1
#define GET_SLOT 2
#define SET_SLOT 3
#define AUX_SLOT 4


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
 This class give the ability to spy properties changes. One can listen for add, del, set and set events on the object.
 It is also possible to store an hidden auxiliary value that can be access using ObjEx.Aux( _ObjEx object_ ) static function.
**/
BEGIN_CLASS( ObjEx )


inline JSBool NotifyObject( int slotIndex, JSContext *cx, JSObject *obj, jsid id, jsval *vp ) {

// (TBD) because in constructor we do JS_SetPrototype(cx, obj, NULL) to create a 'true' empty object, is the next lines useful ?

//	if ( JSVAL_IS_VOID(*vp) && strcmp( JL_GetStringBytes(JS_ValueToString(cx,id)), "__iterator__" ) == 0 ) // we don't want to override the iterator
//		return JS_TRUE;
	
	if ( id == JL_ATOMJSID(cx, iterator) )
		return JS_TRUE;

// (TBD) returns the current value too (cf.	JS_LookupProperty(cx, obj, )

/*
	JSObject *pobj;
	jsid propid;
	JL_CHK( JS_ValueToId(cx, id, &propid) );
	JSProperty *prop;
	JL_CHK( OBJ_LOOKUP_PROPERTY(cx, obj, propid, &pobj, &prop) );
	jsval prevValue;
	JL_CHK( JS_IdToValue(cx, prop->id, &prevValue) );
	OBJ_DROP_PROPERTY(cx, pobj, prop);
*/

//	jsval prevValue;
//	JL_CHK( JS_LookupPropertyWithFlags(cx, obj, JL_GetStringBytes(JS_ValueToString(cx, id)), JSRESOLVE_QUALIFIED|JSRESOLVE_ASSIGNING|JSRESOLVE_DECLARING, &prevValue) );

	jsval slot;
	JL_CHK( JL_GetReservedSlot( cx, obj, slotIndex , &slot ) );
	if ( JSVAL_IS_VOID(slot) )
		return JS_TRUE;
	jsval aux;
	JL_CHK( JL_GetReservedSlot( cx, obj, AUX_SLOT, &aux ) );
	jsval args[4]; // = { id, *vp, aux, INT_TO_JSVAL(slotIndex) }; // ( propertyName, propertyValue, auxObject, callbackIndex )
	// args[0] = id;
	JL_CHK( JS_IdToValue(cx, id, &args[0]) );
	args[1] = *vp;
	args[2] = aux;
	args[3] = INT_TO_JSVAL(slotIndex);
	{
	// at the moment, no GC protection is needed for argv and rval.
	JSBool st;
	st = JS_CallFunctionValue( cx, obj, slot, COUNTOF(args), args, vp );
	return st;
	}
	JL_BAD;
}

DEFINE_ADD_PROPERTY() {

	return NotifyObject( ADD_SLOT, cx, obj, id, vp );
}

DEFINE_DEL_PROPERTY() {

	return NotifyObject( DEL_SLOT, cx, obj, id, vp );
}

DEFINE_GET_PROPERTY() {

	return NotifyObject( GET_SLOT, cx, obj, id, vp );
}

DEFINE_SET_PROPERTY() {

	return NotifyObject( SET_SLOT, cx, obj, id, vp );
}

/**doc
$TOC_MEMBER $INAME
 $INAME( [addCallback], [delCallback], [getCallback], [setCallback] [, auxObject] )
  Constructs a ObjEx object.
  $H arguments
   $ARG $FUN addCallback: called when the property is added to the object.
   $ARG $FUN delCallback: called when the property is deleted form the object.
   $ARG $FUN getCallback: called when the property is read form the object.
   $ARG $FUN setCallback: called when the property is changed. This include when the property is added.
   $ARG $VAL auxObject:
  $H note
   addCallback, delCallback, getCallback, setCallback: can be set to the $UNDEF value.
  $H behavior
   addCallback, delCallback, getCallback, setCallback functions are called according to the operation done on the object.
   These callback functions are called with the following arguments:
    # $ARG $STR propertyName : the name of the property being handled.
    # $ARG $VAL propertyValue : the value of the property being handled.
    # $ARG $VAL auxObject : the _auxObject_ provided to the constructor.
    # $ARG $INT callbackIndex : an integer that has the folowing value: 0 for addCallback, 1 for delCallback, 2 for getCallback, 3 for setCallback.
  $H note
   addCallback callback function is called only when the property is being added, in opposition to _setCallback_ that is called each time the value is changed.
**/
DEFINE_CONSTRUCTOR() {

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_ASSERT_ARGC_MAX(5);


	for ( uintN i = 0; i < JL_ARGC && i < 4; i++ )
		if ( JL_ARG_ISDEF(i+1) ) {

			JL_ASSERT_ARG_IS_FUNCTION(i+1);
			JL_CHK( JL_SetReservedSlot( cx, obj, i, JL_ARG(i+1) ) );
		}
	if ( JL_ARGC >= 5 ) // AUX object
		JL_CHK( JL_SetReservedSlot( cx, obj, AUX_SLOT, JL_ARG(5) ) );
	JL_CHK( JS_SetPrototype(cx, obj, NULL) ); // this creates an empty object ( without __proto__, __parent__, toString, ... )
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Static functions ===
**/


/**doc
$TOC_MEMBER $INAME
 $VAL $INAME( objex [, newAux] )
  Returns the _auxObject_ stored in the _objex_.
  If newAux is given, it replaces the current auxiliary value of _objex_.
**/
DEFINE_FUNCTION( Aux ) {

	JL_ASSERT_ARGC_RANGE(1, 2);
	JL_ASSERT_ARG_IS_OBJECT(1);
	JL_ASSERT( JL_GetClass(JSVAL_TO_OBJECT(JL_ARG(1))) == JL_THIS_CLASS, E_ARG, E_NUM(1), E_INSTANCE, E_NAME(JL_THIS_CLASS_NAME) );

	JSObject *object;
	object = JSVAL_TO_OBJECT(JL_ARG(1));  // beware, obj1 has no proto ! (see JS_SetPrototype(cx, obj, NULL) in the constructor)
	JL_CHK( JL_GetReservedSlot( cx, object, AUX_SLOT, JL_RVAL ) );
	if ( JL_ARG_ISDEF(2) )
	  JL_CHK( JL_SetReservedSlot( cx, object, AUX_SLOT, JL_ARG(2) ) );
	return JS_TRUE;
	JL_BAD;

}

//DEFINE_HAS_INSTANCE() { // see issue#52
//
//	*bp = JL_IsClass(*v, JL_THIS_CLASS);
//	return JS_TRUE;
//}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_RESERVED_SLOTS(5)
	HAS_CONSTRUCTOR
//	HAS_HAS_INSTANCE

	HAS_ADD_PROPERTY
	HAS_DEL_PROPERTY
	HAS_GET_PROPERTY
	HAS_SET_PROPERTY

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION(Aux)
	END_STATIC_FUNCTION_SPEC

END_CLASS

/**doc
=== Example 1 ===
{{{
function addCallback( name, value ) {

   Print('adding ' + name + ' = ' + value);
}

var obj = new ObjEx( addCallback, undefined, undefined, undefined, null );

obj.foo = 123;
obj.foo = 456;
}}}
prints:
<pre>
adding foo = 123
</pre>

=== Example 2 ===
{{{
LoadModule('jsstd');
LoadModule('jsio');

function OnGet(name, value, aux) {

  if ( name == 'content' )
    return new File(aux).content;
  if ( name in this )
    return value;
  return this[name] = new ObjEx(undefined, undefined, OnGet, undefined, aux + '/' + name );
}
var root = new ObjEx(undefined, undefined, OnGet, undefined, 'c:');

Print( root.windows.system32.drivers.etc.hosts.content );
}}}


=== Other example ===

 http://jsircbot.googlecode.com/svn/trunk/dataObject.js

**/



/*

http://groups.google.fr/group/netscape.public.mozilla.jseng/browse_frm/thread/ac4f3a3713ec2c21/075fb093b7d39ab5?lnk=st&q=JSCLASS_SHARE_ALL_PROPERTIES&rnum=2&hl=fr#075fb093b7d39ab5
	Of course -- JSClass.getProperty and JSClass.setProperty can mirror
	changes from and to your std::map.  You can set the JSClass.flags bit
	JSCLASS_SHARE_ALL_PROPERTIES to avoid any parallel map being managed by
	SpiderMonkey.  Then for each get or set, the appropriate JSClass hook
	will be called and you can fetch or store using your map.
	/be

with JSCLASS_SHARE_ALL_PROPERTIES :
	o.a = 123;
	print(a.o)// displays 123

without JSCLASS_SHARE_ALL_PROPERTIES :
	o.a = 123;
	print(a.o)// displays undefined

*/
