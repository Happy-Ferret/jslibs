#include "stdafx.h"
#include "jstest.h"

#include <stdio.h>

/* TIPS

	Prototypes are not constructed by default. If you want your class's
	prototype object to be constructed (where the constructor you pass to
	JS_InitClass, the |JSNative constructor| parameter, gives the |obj|
	passed to it private data, use the JSCLASS_CONSTRUCT_PROTOTYPE .


JSClass.construct
-----------------

	'Archetype' is a JSClass defined natively.
	// this works fine
	var dog_type = new Archetype("dog", ...);
	// need to make this work too, and be able to run a native function.
	var dog = new dog_type( .... );

	=> You need to initialize JSClass.construct

*/

BEGIN_CLASS

	void Finalize(JSContext *cx, JSObject *obj) {
	}

	JSBool ClassConstruct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

		printf("constructing...");
		if ( !JS_IsConstructing(cx) || JS_GetClass(obj) != thisClass ) {

			JSClass *o = JS_GetClass(obj);
			// error
			printf("error\n");
			return JS_FALSE;
		}

		printf("ok\n");
		return JS_TRUE;
	}

	JSBool Call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

		printf("call\n");
		return JS_TRUE;
	}


	JSBool prop(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

		printf("read prop\n");
		return JS_TRUE;
	}

	JSBool Func(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

		printf("call Func\n");
		return JS_TRUE;
	}

	BEGIN_FUNCTION_MAP
		FUNCTION(Func)
	END_MAP
	BEGIN_PROPERTY_MAP
		READONLY(prop)
	END_MAP
	NO_STATIC_FUNCTION_MAP
	//BEGIN_STATIC_FUNCTION_MAP
	//END_MAP
	NO_STATIC_PROPERTY_MAP
	//BEGIN_STATIC_PROPERTY_MAP
	//END_MAP
	NO_OBJECT_CONSTRUCT
//	NO_CLASS_CONSTRUCT
//	NO_FINALIZE
//	NO_CALL
	NO_PROTOTYPE
	NO_CONSTANT_MAP
	NO_INITCLASSAUX

END_CLASS(Test, NO_PRIVATE, NO_RESERVED_SLOT)

