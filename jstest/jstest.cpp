#include "stdafx.h"

#define XP_WIN
#include <jsapi.h>
#include "../common/jshelper.h"

#include "jstest.h"

#include <stdio.h>

BEGIN_CLASS

	void Finalize(JSContext *cx, JSObject *obj) {
	}

	JSBool Construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

		if ( !JS_IsConstructing(cx) || JS_GetClass(obj) != thisClass ) {
			// error
//			return JS_FALSE;
			printf("construct error\n");	
		}

		printf("construct\n");
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
	BEGIN_STATIC_FUNCTION_MAP
	END_MAP
	BEGIN_STATIC_PROPERTY_MAP
	END_MAP
	//	NO_CONSTRUCT
	//	NO_FINALIZE
	//	NO_CALL
	NO_PROTOTYPE

END_CLASS(Test, NO_RESERVED_SLOTS)
