#include "stdafx.h"

#define XP_WIN
#include <jsapi.h>
#include <sqlite3.h>

#include "sqliteClass.h"


void Database_Finalize(JSContext *cx, JSObject *obj) { 

//	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
//	JS_SetPrivate( cx, obj, NULL );

	sqlite3_close() 
}


JSClass Database_class = { 
  "Database", JSCLASS_HAS_PRIVATE, 
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, 
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Database_Finalize
};

JSBool Database_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	if ( !JS_IsConstructing(cx) ) {

		JS_ReportError( cx, "need to be construct" );
		return JS_FALSE;
	}

	if ( argc < 1 ) {

		JS_ReportError( cx, "missing argument" );
		return JS_FALSE;
	}

	char *fileName = JS_GetStringBytes( JSVAL_TO_STRING(argv[0]) );
	if ( fileName == NULL ) {

		JS_ReportError( cx, "invalid file name" );
		return JS_FALSE;
	}

	sqlite3 *db;
	int status = sqlite3_open( fileName, &db );

	if ( status != SQLITE_OK )
		return ThrowSqliteError( cx, sqlite3_errcode(&db), sqlite3_errmsg(&db) );

	JS_SetPrivate( cx, obj, db );

	return JS_TRUE;
}


JSFunctionSpec Database_FunctionSpec[] = { // *name, call, nargs, flags, extra
// { "Listen"     , Socket_listen     , 2, 0, 0 },
 { 0 }
};


JSPropertySpec Database_PropertySpec[] = { // *name, tinyid, flags, getter, setter
//  { "linger"   , PR_SockOpt_Linger, JSPROP_PERMANENT, Socket_getOption, Socket_setOption },
  { 0 }
};


JSObject *InitSqliteClass( JSContext *cx, JSObject *obj ) {
	
	return JS_InitClass( cx, obj, NULL, &Database_class, Database_construct, 1, Database_PropertySpec, Database_FunctionSpec, NULL, NULL );
}