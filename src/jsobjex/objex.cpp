#include "stdafx.h"

#include <jscntxt.h>

#include "objex.h"

#define ADD_SLOT 0
#define DEL_SLOT 1
#define GET_SLOT 2
#define SET_SLOT 3
#define AUX_SLOT 4

JSBool NotifyObject( int slotIndex, JSContext *cx, JSObject *obj, jsval id, jsval *vp ) {

	// (TBD) because in constructor we do JS_SetPrototype(cx, obj, NULL) to create a 'true' empty object, is the next line useful ?

//	if ( JSVAL_IS_VOID(*vp) && strcmp( JS_GetStringBytes(JS_ValueToString(cx,id)), "__iterator__" ) == 0 ) // we don't want to override the iterator
//		return JS_TRUE;

	jsid idid;
	JS_ValueToId(cx, id, &idid);
	if ( idid == ATOM_TO_JSID(cx->runtime->atomState.iteratorAtom) ) // (TBD) check if it is faster
		return JS_TRUE;

	jsval slot;
	JS_GetReservedSlot( cx, obj, slotIndex , &slot );
	if ( JSVAL_IS_VOID(slot) )
		return JS_TRUE;
	jsval aux;
	JS_GetReservedSlot( cx, obj, AUX_SLOT, &aux );
	jsval args[] = { id, *vp, aux, INT_TO_JSVAL(slotIndex) }; // ( propertyName, propertyValue, auxObject, callbackIndex )
	return JS_CallFunctionValue( cx, obj, slot, sizeof(args)/sizeof(jsval), args, vp );
}

JSBool objex_addProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	return NotifyObject( ADD_SLOT, cx, obj, id, vp );
}

JSBool objex_delProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	return NotifyObject( DEL_SLOT, cx, obj, id, vp );
}

JSBool objex_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	return NotifyObject( GET_SLOT, cx, obj, id, vp );
}

JSBool objex_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	return NotifyObject( SET_SLOT, cx, obj, id, vp );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass objex_class = { "ObjEx", JSCLASS_HAS_RESERVED_SLOTS(5) /*| JSCLASS_NEW_RESOLVE | JSCLASS_SHARE_ALL_PROPERTIES */,
  objex_addProperty, objex_delProperty, objex_getProperty, objex_setProperty,
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_CONSTRUCTOR() {

	if ( !JS_IsConstructing(cx) ) {

		JS_ReportError( cx, "need to be construct" );
		return JS_FALSE;
	}

	for ( uintN i=0; i<argc && i<4; i++ ) {

		if ( JS_TypeOfValue( cx, argv[i] ) != JSTYPE_FUNCTION && !JSVAL_IS_VOID(argv[i]) ) {

			JS_ReportError( cx, "function or undefined expected" );
			return JS_FALSE;
		}
		JS_SetReservedSlot( cx, obj, i, argv[i] );
	}

	if ( argc >= 5 ) // AUX object
		JS_SetReservedSlot( cx, obj, AUX_SLOT, argv[4] );

	JS_SetPrototype(cx, obj, NULL); // this creates an empty object ( without __proto__, __parent__, toString, ... )
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DEFINE_FUNCTION_FAST( Aux ) {

	if ( J_ARGC < 1 ) {

		JS_ReportError( cx, "missing argument" );
		return JS_FALSE;
	}

	if ( !JSVAL_IS_OBJECT(J_FARG(1)) || J_FARG(1) == JSVAL_NULL ) {

		JS_ReportError( cx, "object expected" );
		return JS_FALSE;
	}

	JSObject *object = JSVAL_TO_OBJECT(J_FARG(1));

	if ( JS_GET_CLASS(cx,object) != &objex_class  ) {

		JS_ReportError( cx, "%s object expected", objex_class.name );
		return JS_FALSE;
	}

  JS_GetReservedSlot( cx, object, AUX_SLOT, J_FRVAL );

	if ( J_ARGC >= 2 )
	  JS_SetReservedSlot( cx, object, AUX_SLOT, J_FARG(2) );

	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSFunctionSpec objex_static_FunctionSpec[] = {
	FUNCTION_FAST( Aux )
	JS_FS_END
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSObject *objexInitClass( JSContext *cx, JSObject *obj ) {

	return JS_InitClass( cx, obj, NULL, &objex_class, Constructor, 0, NULL, NULL, NULL, objex_static_FunctionSpec );
}

/**doc
----
== jsobjex::ObjEx class ==
 [http://code.google.com/p/jslibs/ home] *>* [JSLibs] *>* [jsobjex] *>* [ObjEx] - [http://jslibs.googlecode.com/svn/trunk/jsobjex/objex.cpp http://jslibs.googlecode.com/svn/wiki/source.png]

=== Description ===

 This class give the ability to spy properties changes. One can listen for add, del, set and set events on the object.
 = =
 It is possible to store an hidden auxiliary object that can be access using ObjEx.Aux( _ObjEx object_ ) static function.

=== Functions ===

 * ,,constructor,, *ObjEx*( _addCallback_, _delCallback_, _getCallback_, _setCallback_, _auxObject_ )
  ===== note: =
  _addCallback_, _delCallback_, _getCallback_, _setCallback_ can be undefined.

=== Static Functions ===

 * _value_ *Aux*( _ObjEx object_ )
  Returns the _auxObject_ stored in the _ObjEx object_. 

=== Events / Callbacks ===

 function _addCallback_, _delCallback_, _getCallback_, _setCallback_ are called according to the operation done on the object.
 = =
 arguments are: ( propertyName, propertyValue, auxObject, callbackIndex )
 
 * propertyName : the name of the property being handled
 * propertyValue : the value of the property being handled
 * auxObject : the _auxObject_ provided to the constructor
 * callbackIndex : an integer that has the folowing value: 0 for addCallback, 1 for delCallback, 2 for getCallback, 3 for setCallback.
 
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
{{{
adding foo = 123
}}}
Note that _addCallback_ is called only when the property is being added, in opposition to _setCallback_ that is called each time the value is changed.

=== Example ===

 http://jsircbot.googlecode.com/svn/trunk/dataObject.js

**/



/****************************************************************

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
