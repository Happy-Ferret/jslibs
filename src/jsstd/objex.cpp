#include "stdafx.h"

#include <jscntxt.h>

#include "objex.h"

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


inline JSBool NotifyObject( int slotIndex, JSContext *cx, JSObject *obj, jsval id, jsval *vp ) {

// (TBD) because in constructor we do JS_SetPrototype(cx, obj, NULL) to create a 'true' empty object, is the next lines useful ?

//	if ( JSVAL_IS_VOID(*vp) && strcmp( JS_GetStringBytes(JS_ValueToString(cx,id)), "__iterator__" ) == 0 ) // we don't want to override the iterator
//		return JS_TRUE;
	jsid idid;
	JL_CHK( JS_ValueToId(cx, id, &idid) );
	if ( idid == ATOM_TO_JSID(cx->runtime->atomState.iteratorAtom) ) // (TBD) check if it is faster
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
//	JL_CHK( JS_LookupPropertyWithFlags(cx, obj, JS_GetStringBytes(JS_ValueToString(cx, id)), JSRESOLVE_QUALIFIED|JSRESOLVE_ASSIGNING|JSRESOLVE_DECLARING, &prevValue) );

	jsval slot;
	JL_CHK( JS_GetReservedSlot( cx, obj, slotIndex , &slot ) );
	if ( JSVAL_IS_VOID(slot) )
		return JS_TRUE;
	jsval aux;
	JL_CHK( JS_GetReservedSlot( cx, obj, AUX_SLOT, &aux ) );
	jsval args[4]; // = { id, *vp, aux, INT_TO_JSVAL(slotIndex) }; // ( propertyName, propertyValue, auxObject, callbackIndex )
	args[0] = id;
	args[1] = *vp;
	args[2] = aux;
	args[3] = INT_TO_JSVAL(slotIndex);
	// at the moment, no GC protection is needed for argv and rval.
	return JS_CallFunctionValue( cx, obj, slot, COUNTOF(args), args, vp );
	JL_BAD;
}

JSBool AddProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	return NotifyObject( ADD_SLOT, cx, obj, id, vp );
}

JSBool DelProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	return NotifyObject( DEL_SLOT, cx, obj, id, vp );
}

JSBool GetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	return NotifyObject( GET_SLOT, cx, obj, id, vp );
}

JSBool SetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

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

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();

	for ( uintN i = 0; i < JL_ARGC && i < 4; i++ )
		if ( JL_ARG_ISDEF(i+1) ) {

			JL_S_ASSERT_FUNCTION(JL_ARG(i+1));
			JL_CHK( JS_SetReservedSlot( cx, obj, i, JL_ARG(i+1) ) );
		}
	if ( JL_ARGC >= 5 ) // AUX object
		JL_CHK( JS_SetReservedSlot( cx, obj, AUX_SLOT, JL_ARG(5) ) );
	JL_CHK( JS_SetPrototype(cx, obj, NULL) ); // this creates an empty object ( without __proto__, __parent__, toString, ... )
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Static functions ===
**/


/**doc
$TOC_MEMBER $INAME
 $INAME( objex [, newAux] )
  Returns the _auxObject_ stored in the _objex_.
  If newAux is given, it replaces the current auxiliary value of _objex_.
**/
DEFINE_FUNCTION_FAST( Aux ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_OBJECT( JL_FARG(1) );
	JL_S_ASSERT_CLASS( JSVAL_TO_OBJECT(JL_FARG(1)), _class);

	JSObject *object;
	object = JSVAL_TO_OBJECT(JL_FARG(1));
	JL_S_ASSERT_CLASS( object, _class );
	JL_CHK( JS_GetReservedSlot( cx, object, AUX_SLOT, JL_FRVAL ) );
	if ( JL_FARG_ISDEF(2) )
	  JL_CHK( JS_SetReservedSlot( cx, object, AUX_SLOT, JL_FARG(2) ) );
	return JS_TRUE;
	JL_BAD;
}

//DEFINE_HAS_INSTANCE() { // see issue#52
//
//	*bp = !JSVAL_IS_PRIMITIVE(v) && OBJ_GET_CLASS(cx, JSVAL_TO_OBJECT(v)) == _class;
//	return JS_TRUE;
//}


CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))
	HAS_RESERVED_SLOTS(5)
	HAS_CONSTRUCTOR
//	HAS_HAS_INSTANCE

	HAS_ADD_PROPERTY
	HAS_DEL_PROPERTY
	HAS_GET_PROPERTY
	HAS_SET_PROPERTY

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_FAST(Aux)
	END_STATIC_FUNCTION_SPEC

END_CLASS

/**doc
=== Example ===
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

=== Example ===

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
