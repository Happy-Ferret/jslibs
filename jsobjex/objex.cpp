#include "stdafx.h"

#define XP_WIN
#include <jsapi.h>

#include "objex.h"

#define ADD_SLOT 0
#define DEL_SLOT 1
#define GET_SLOT 2
#define SET_SLOT 3
#define AUX_SLOT 4

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void objex_Finalize(JSContext *cx, JSObject *obj) {
}


JSBool objex_addProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

  jsval slot;
  JS_GetReservedSlot( cx, obj, ADD_SLOT, &slot );
	if ( JS_TypeOfValue( cx, slot ) != JSTYPE_FUNCTION )
		return JS_TRUE;
	jsval aux;
  JS_GetReservedSlot( cx, obj, AUX_SLOT, &aux );
	jsval args[] = { INT_TO_JSVAL(ADD_SLOT), OBJECT_TO_JSVAL(obj), id, *vp, aux };
	return JS_CallFunctionValue( cx, obj, slot, sizeof(args)/sizeof(jsval), args, vp );
}

JSBool objex_delProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

  jsval slot;
  JS_GetReservedSlot( cx, obj, DEL_SLOT, &slot );
	if ( JS_TypeOfValue( cx, slot ) != JSTYPE_FUNCTION )
		return JS_TRUE;
	jsval aux;
  JS_GetReservedSlot( cx, obj, AUX_SLOT, &aux );
	jsval args[] = { INT_TO_JSVAL(DEL_SLOT), OBJECT_TO_JSVAL(obj), id, *vp, aux };
	return JS_CallFunctionValue( cx, obj, slot, sizeof(args)/sizeof(jsval), args, vp );
}

JSBool objex_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

  jsval slot;
  JS_GetReservedSlot( cx, obj, GET_SLOT, &slot );
	if ( JS_TypeOfValue( cx, slot ) != JSTYPE_FUNCTION )
		return JS_TRUE;
	jsval aux;
  JS_GetReservedSlot( cx, obj, AUX_SLOT, &aux );
	jsval args[] = { INT_TO_JSVAL(GET_SLOT), OBJECT_TO_JSVAL(obj), id, *vp, aux };
	return JS_CallFunctionValue( cx, obj, slot, sizeof(args)/sizeof(jsval), args, vp );
}

JSBool objex_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

  jsval slot;
  JS_GetReservedSlot( cx, obj, SET_SLOT, &slot );
	if ( JS_TypeOfValue( cx, slot ) != JSTYPE_FUNCTION )
		return JS_TRUE;
	jsval aux;
  JS_GetReservedSlot( cx, obj, AUX_SLOT, &aux );
	jsval args[] = { INT_TO_JSVAL(SET_SLOT), OBJECT_TO_JSVAL(obj), id, *vp, aux };
	return JS_CallFunctionValue( cx, obj, slot, sizeof(args)/sizeof(jsval), args, vp );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSClass objex_class = { "objex", JSCLASS_HAS_RESERVED_SLOTS(5) /*| JSCLASS_NEW_RESOLVE | JSCLASS_SHARE_ALL_PROPERTIES */,
  objex_addProperty, objex_delProperty, objex_getProperty, objex_setProperty,
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, objex_Finalize
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSBool objex_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	if ( !JS_IsConstructing(cx) ) {

		JS_ReportError( cx, "need to be construct" );
		return JS_FALSE;
	}

	for ( int i=0; i<argc; i++)
		JS_SetReservedSlot( cx, obj, i, argv[i] );
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
JSObject *objexInitClass( JSContext *cx, JSObject *obj ) {

	return JS_InitClass( cx, obj, NULL, &objex_class, objex_construct, 0, NULL, NULL, NULL, NULL );
}


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