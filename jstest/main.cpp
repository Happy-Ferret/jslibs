#include "stdafx.h"

#define XP_WIN
#include <jsapi.h>
#include "../common/jshelper.h"

#include "jstest.h"
//#include "jsobj.h"

/* recycle array objects instead of GC them
Hello,

In my application, I use a huge number of Array (as vector) like this: body1.position = [1,2,3];
This make the GC running very often, and causes big lags in my application.
Is it possible to tell SpiderMonkey to reuse unreferenced array objects instead of create new ones ?

Franck.

*/

	//	Here is a graphical representation of the bit allocation of a jsval value:
	//	H------------------------------L
	//	xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx1 31-bit integer
	//	ppppppppppppppppppppppppppppp000 29-bit pointer to a JSObject
	//	ppppppppppppppppppppppppppppp010 29-bit pointer to a 64-bit jsdouble,
	//	ppppppppppppppppppppppppppppp100 29-bit pointer to a JSString object
	//	xxxxxxxxxxxxxxxxxxxxxxxxxxxxx110 29-bit integer for JSBool usage

	//	The 29-bit addresses are are assigned and tracked by JavaScript Garbage
	//	Collector (GC). This means that the smallest memory block GC allocates is 8
	//	bytes;
	//	that is the pointers are aligned at 64-bit boundaries. 


/*

if ( !JSVAL_IS_NULL(mCbval) )
	JS_AddRoot( mCx, &mCbval );

jsdouble *d = JS_NewDouble( cx, f[i] );
if (d && JS_AddRoot( cx, d ))
	jsvec[i] = DOUBLE_TO_JSVAL(d);


if (JSVAL_IS_GCTHING(argv[0])) JS_AddRoot(priv->js_ctx, &argv[0]);

*/

jsval pool[1000];


DEFINE_FUNCTION( V ) {

// You are looking for JS_IsAboutToBeFinalized. This needs to be called in a GC callback when status==JSGC_MARK_END.


//	JS_LockGCThing


//	JSObject *o = JS_NewArrayObject(cx, 0, NULL);
	JSObject *o = JS_NewObject(cx, NULL, NULL, NULL);

	JSBool st;
	jsuint len;
	st = JS_HasArrayLength(cx, o, &len);
//	JS_ClearScope(cx, o); // if recycled only
	jsval value = INT_TO_JSVAL(12345);
	st = JS_SetElement(cx, o, 1, &value);

//	st = JS_DefineProperty(cx, o, (const char *)1, INT_TO_JSVAL(12345), NULL, NULL, JSPROP_INDEX);
	*rval = OBJECT_TO_JSVAL(o);


//	JS_SetProperty(cx, JS_GetGlobalObject(cx), "toto", &ov);
	


/*
	jsval dval;
	JS_NewDoubleValue(cx, 1.3f, &dval);

	if (JSVAL_IS_GCTHING(dval)) JS_AddRoot(cx, &dval);

	JS_ValueToNumber
		DOUBLE_TO_JSVAL
	jsdouble pd = JSVAL_TO_DOUBLE(dval);
	
	JS_GC(cx);
*/

	return JS_FALSE;
}


BEGIN_STATIC_FUNCTION_MAP
	FUNCTION( V )
END_MAP


JSGCCallback prevCallback;

JSBool newJSGCCallback(JSContext *cx, JSGCStatus status) {

	if ( status == JSGC_MARK_END ) {
	
//		
//		JS_IsAboutToBeFinalized(cx, 
		//gc_find_flags(thing);
		// JS_SetGCParameter(m_pRep, JSGC_MAX_MALLOC_BYTES, GC_INTERVAL);
		
	}
	return prevCallback(cx, status);
}


extern "C" __declspec(dllexport) JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	BIND_STATIC_FUNCTIONS(obj);


	
	




	prevCallback = JS_SetGCCallback( cx, newJSGCCallback );


//	InitClassTest( cx, obj );
	return JS_TRUE;
}

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved ) {

  switch (ul_reason_for_call) {

	  case DLL_PROCESS_ATTACH:
	  case DLL_THREAD_ATTACH:
	  case DLL_THREAD_DETACH:
	  case DLL_PROCESS_DETACH:
		  break;
  }
  return TRUE;
}
