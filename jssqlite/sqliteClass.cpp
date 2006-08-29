#include "stdafx.h"

#define XP_WIN
#include <jsapi.h>
#include <sqlite3.h>

#include "sqliteClass.h"
#include "sqliteError.h"

bool _safeMode = true;


extern JSClass Database_class;


void Result_Finalize(JSContext *cx, JSObject *obj) { 

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	if ( pStmt != NULL ) {

			int status = sqlite3_finalize( pStmt );
			if ( status != SQLITE_OK )
				JS_ReportError( cx, "unable to finalize the statement (%d)", status );
			JS_SetPrivate( cx, obj, NULL );
	}
}


JSClass Result_class = { 
  "Result", JSCLASS_HAS_PRIVATE, 
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, 
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Result_Finalize
};


JSBool Result_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	JS_ReportError( cx, "This class cannot be constructed" );
	return JS_FALSE;
}


JSBool Result_step(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	if ( pStmt == NULL ) {

		JS_ReportError( cx, "Invalid statment" );
		return JS_FALSE;
	}

	int status = sqlite3_step( pStmt ); // The return value will be either SQLITE_BUSY, SQLITE_DONE, SQLITE_ROW, SQLITE_ERROR, or SQLITE_MISUSE.
	switch (status) {
		case SQLITE_ERROR:
			return SqliteThrowError( cx, sqlite3_errcode(sqlite3_db_handle( pStmt )), sqlite3_errmsg(sqlite3_db_handle( pStmt )));
		case SQLITE_MISUSE: // means that the this routine was called inappropriately. Perhaps it was called on a virtual machine that had already been finalized or on one that had previously returned SQLITE_ERROR or SQLITE_DONE. Or it could be the case that a database connection is being used by a different thread than the one it was created it.
			JS_ReportError( cx, "this routine was called inappropriately" );
			return JS_FALSE;
		case SQLITE_DONE: // means that the statement has finished executing successfully. sqlite3_step() should not be called again on this virtual machine without first calling sqlite3_reset() to reset the virtual machine back to its initial state.
			*rval = JSVAL_FALSE;
			return JS_TRUE;
		case SQLITE_ROW: // SQLITE_ROW is returned each time a new row of data is ready for processing by the caller
			*rval = JSVAL_TRUE;
			return JS_TRUE;
		default:
			JS_ReportError( cx, "uncautch error (%d)", status );
			return JS_FALSE;
	}
}


JSBool Result_col(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	if ( pStmt == NULL ) {

		JS_ReportError( cx, "Invalid statment" );
		return JS_FALSE;
	}

	if ( argc < 1 ) {

		JS_ReportError( cx, "argument is missing" );
		return JS_FALSE;
	}

	int32 col;
	JS_ValueToInt32( cx, argv[0], &col );

	switch ( sqlite3_column_type( pStmt, col ) ) {
		case SQLITE_INTEGER:
			*rval = INT_TO_JSVAL( sqlite3_column_int( pStmt, col ) );
			break;
		case SQLITE_FLOAT:
			JS_NewNumberValue( cx, sqlite3_column_double( pStmt, col ), rval );
			break;
		case SQLITE_BLOB:
			*rval = STRING_TO_JSVAL( JS_NewStringCopyN( cx,(const char *)sqlite3_column_blob( pStmt, col ), sqlite3_column_bytes( pStmt, col ) ) );
			break;
		case SQLITE_NULL:
			*rval = JSVAL_NULL;
			break;
		case SQLITE_TEXT:
		default: // by default, if type is unknown, use TEXT cast
			*rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,(const char *)sqlite3_column_text( pStmt, col )));
	}
	return JS_TRUE;
}


JSBool Result_row(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	if ( pStmt == NULL ) {

		JS_ReportError( cx, "Invalid statment" );
		return JS_FALSE;
	}

	if ( Result_step( cx, obj, 0, NULL, rval ) == JS_FALSE ) // if something goes wrong in Result_step ( error report has already been set )
		return JS_FALSE;

	if ( *rval == JSVAL_FALSE ) { // the statement has finished executing successfully

		*rval = JSVAL_VOID; // return undefined
		return JS_TRUE;
	}

	// returns an array [ row1Data, row2Data, ... ] else return an object { row1Name:row1Data, row2Name:row2Data,  ... }
	JSBool namedRows = JS_FALSE;
	if ( argc >= 1 )
		JS_ValueToBoolean( cx, argv[0], &namedRows );

	JSObject *row = namedRows ? JS_NewObject(cx, NULL, NULL, NULL) : JS_NewArrayObject(cx, 0, NULL); // If length is 0, JS_NewArrayObject creates an array object of length 0 and ignores vector.
	*rval = OBJECT_TO_JSVAL(row); // now, row is protectef fom GC ??
	
	// If the previous call to sqlite3_step() returned SQLITE_DONE or an error code, 
	// then sqlite3_data_count() will return 0 whereas sqlite3_column_count() will continue to return the number of columns in the result set.
	int columnCount = sqlite3_data_count( pStmt ); // This routine returns 0 if pStmt is an SQL statement that does not return data (for example an UPDATE).

	jsval colJsValue, jsvCol;
	for ( int col = 0; col < columnCount; ++col ) {
		
		jsvCol = INT_TO_JSVAL(col);
		if ( Result_col( cx, obj, 1, &jsvCol, &colJsValue ) == JS_FALSE ) // if something goes wrong in Result_col ( error report has already been set )
			return JS_FALSE;

		if ( namedRows )
			JS_SetProperty( cx, row, sqlite3_column_name( pStmt, col ), &colJsValue );
		else
			JS_DefineElement( cx, row, col, colJsValue, NULL, NULL, JSPROP_ENUMERATE );
	}
	return JS_TRUE;
}


JSBool Result_reset(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	
	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	if ( pStmt == NULL ) {

		JS_ReportError( cx, "Invalid statment" );
		return JS_FALSE;
	}

	int status = sqlite3_reset(pStmt);
	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, sqlite3_errcode(sqlite3_db_handle(pStmt)), sqlite3_errmsg(sqlite3_db_handle(pStmt)) );
	return JS_TRUE;
}


JSBool Result_close(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	if ( pStmt == NULL ) {

		JS_ReportError( cx, "Invalid statment" );
		return JS_FALSE;
	}

	int status = sqlite3_finalize( pStmt );
	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, sqlite3_errcode(sqlite3_db_handle(pStmt)), sqlite3_errmsg(sqlite3_db_handle(pStmt)) );

	JS_SetPrivate( cx, obj, NULL );
	return JS_TRUE;
}


JSFunctionSpec Result_FunctionSpec[] = { // *name, call, nargs, flags, extra
 { "Row"   , Result_row   , 1, 0, 0 },
 { "Step"  , Result_step  , 0, 0, 0 },
 { "Reset" , Result_reset , 0, 0, 0 },
 { "Close" , Result_close , 0, 0, 0 },
 { "Col"   , Result_col   , 0, 0, 0 },
 { 0 }
};


JSBool Result_getter_columnCount(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	if ( pStmt == NULL ) {

		JS_ReportError( cx, "Invalid statment" );
		return JS_FALSE;
	}
	*vp = INT_TO_JSVAL(sqlite3_column_count(pStmt));
	return JS_TRUE;
}


JSBool Result_getter_columnNames(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	if ( pStmt == NULL ) {

		JS_ReportError( cx, "Invalid statment" );
		return JS_FALSE;
	}

	JSObject *columnNames = JS_NewArrayObject(cx, 0, NULL);
	*vp = OBJECT_TO_JSVAL( columnNames );

	int columnCount = sqlite3_column_count( pStmt ); // sqlite3_column_count AND NOT sqlite3_data_count because this function can be called before sqlite3_step

	jsval colJsValue;
	for ( int col = 0; col < columnCount; ++col ) {

		colJsValue = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,(const char *)sqlite3_column_name( pStmt, col ))); // sqlite3_column_name can be called BEFORE sqlite3_step
		JS_DefineElement(cx, columnNames, col, colJsValue, NULL, NULL, JSPROP_ENUMERATE);
	}

  return JS_TRUE;
}


JSBool Result_getter_columnIndex(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	if ( pStmt == NULL ) {

		JS_ReportError( cx, "Invalid statment" );
		return JS_FALSE;
	}

	JSObject *columnIndexes = JS_NewObject( cx, NULL, NULL, NULL );
	*vp = OBJECT_TO_JSVAL( columnIndexes );

	int columnCount = sqlite3_column_count( pStmt );

	jsval colJsValue;
	for ( int col = 0; col < columnCount; ++col ) {

		colJsValue = INT_TO_JSVAL(col);
		JS_SetProperty( cx, columnIndexes, sqlite3_column_name( pStmt, col ), &colJsValue );
	}
  return JS_TRUE;
}


JSPropertySpec Result_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "columnCount"   , 0, JSPROP_PERMANENT|JSPROP_READONLY, Result_getter_columnCount, NULL },
	{ "columnNames"   , 0, JSPROP_PERMANENT|JSPROP_READONLY, Result_getter_columnNames, NULL },
	{ "columnIndexes" , 0, JSPROP_PERMANENT|JSPROP_READONLY, Result_getter_columnIndex, NULL },
  { 0 }
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Database_Finalize(JSContext *cx, JSObject *obj) { 

	sqlite3 *db = (sqlite3 *)JS_GetPrivate( cx, obj );
	if ( db != NULL ) {
		int status = sqlite3_close( db ); // All prepared statements must finalized before sqlite3_close() is called or else the close will fail with a return code of SQLITE_BUSY.
		if ( status != SQLITE_OK )
			JS_ReportError( cx, "unable to finalize the database (error:%d) ", status );
		JS_SetPrivate( cx, obj, NULL );
	}
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
	
	JSString *jssFileName = JS_ValueToString( cx, argv[0] );
	char *fileName = JS_GetStringBytes(jssFileName);
	if ( fileName == NULL ) {

		JS_ReportError( cx, "invalid file name" );
		return JS_FALSE;
	}

	sqlite3 *db;
	int status = sqlite3_open( fileName, &db );

	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, sqlite3_errcode(db), sqlite3_errmsg(db) );

	JS_SetPrivate( cx, obj, db );

	return JS_TRUE;
}

JSBool Database_query(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	if ( argc < 1 ) {

		JS_ReportError( cx, "missing argument" );
		return JS_FALSE;
	}

	char *sqlQuery = JS_GetStringBytes( JS_ValueToString( cx, argv[0] ) );

	sqlite3 *db = (sqlite3 *)JS_GetPrivate( cx, obj );
	if ( db == NULL ) {

		JS_ReportError( cx, "invalid database handler" );
		return JS_FALSE;
	}

	const char *szTail;
	sqlite3_stmt *pStmt;

	int status = sqlite3_prepare( db, sqlQuery, -1, &pStmt, &szTail ); // If the next argument, "nBytes", is less than zero, then zSql is read up to the first nul terminator. 

	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, sqlite3_errcode(db), sqlite3_errmsg(db) );

	if ( *szTail != '\0' ) { // for the moment, do not support multiple statements

		JS_ReportError( cx, "too many SQL statements (%s)", szTail );
		return JS_FALSE;
	}
	
	if ( pStmt == NULL ) { // if there is an error, *ppStmt may be set to NULL. If the input text contained no SQL (if the input is and empty string or a comment) then *ppStmt is set to NULL.
		// nothing to do here
	}
	
	JSObject *object = JS_NewObject( cx, &Result_class, NULL, obj ); // statement's parent is the database
	JS_SetPrivate( cx, object, pStmt );
	*rval = OBJECT_TO_JSVAL( object );

	return JS_TRUE;
}


JSBool Database_close(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	sqlite3 *db = (sqlite3 *)JS_GetPrivate( cx, obj );
	if ( db == NULL ) {

		JS_ReportError( cx, "invalid database handler" );
		return JS_FALSE;
	}

	int status = sqlite3_close( db ); // All prepared statements must finalized before sqlite3_close() is called or else the close will fail with a return code of SQLITE_BUSY.
	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, sqlite3_errcode(db), sqlite3_errmsg(db) );
			
	JS_SetPrivate( cx, obj, NULL );

	return JS_TRUE;
}


JSFunctionSpec Database_FunctionSpec[] = { // *name, call, nargs, flags, extra
 { "Query"     , Database_query     , 1, 0, 0 },
 { "Close"     , Database_close     , 0, 0, 0 },
 { 0 }
};

JSBool Database_getter_lastInsertRowid(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	sqlite3 *db = (sqlite3 *)JS_GetPrivate( cx, obj );
	if ( db == NULL ) {

		JS_ReportError( cx, "invalid database handler" );
		return JS_FALSE;
	}

	JS_NewNumberValue( cx, sqlite3_last_insert_rowid(db), vp ); // use JS_NewNumberValue because sqlite3_last_insert_rowid returns int64
  return JS_TRUE;
}



JSBool Database_getter_changes(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	sqlite3 *db = (sqlite3 *)JS_GetPrivate( cx, obj );
	if ( db == NULL ) {

		JS_ReportError( cx, "invalid database handler" );
		return JS_FALSE;
	}

	// This function returns the number of database rows that were changed (or inserted or deleted) by the most recently completed INSERT, UPDATE, or DELETE statement. 
	// Only changes that are directly specified by the INSERT, UPDATE, or DELETE statement are counted. Auxiliary changes caused by triggers are not counted. Use the sqlite3_total_changes() function to find the total number of changes including changes caused by triggers.
	//JS_NewNumberValue( cx, sqlite3_changes(db), vp );
	*vp = INT_TO_JSVAL( sqlite3_changes(db) );
  return JS_TRUE;
}


JSPropertySpec Database_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "lastInsertRowid" , 0, JSPROP_PERMANENT|JSPROP_READONLY, Database_getter_lastInsertRowid, NULL },
	{ "changes"         , 0, JSPROP_PERMANENT|JSPROP_READONLY, Database_getter_changes, NULL },
  { 0 }
};


JSBool Database_static_getter_version(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

	*vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,(const char *)sqlite3_libversion()));
  return JS_TRUE;
}



JSPropertySpec Database_static_PropertySpec[] = { // *name, tinyid, flags, getter, setter
	{ "version" , 0, JSPROP_PERMANENT|JSPROP_READONLY, Database_static_getter_version, NULL },
  { 0 }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



JSObject *SqliteInitClass( JSContext *cx, JSObject *obj ) {
	
	JS_InitClass( cx, obj, NULL, &Result_class, Database_construct, 1, Result_PropertySpec, Result_FunctionSpec, NULL, NULL );

	return JS_InitClass( cx, obj, NULL, &Database_class, Database_construct, 1, Database_PropertySpec, Database_FunctionSpec, Database_static_PropertySpec, NULL );
}

/*

SqLite API:
	http://www.sqlite.org/capi3ref.html


sqlite3_column_count
	Return the number of columns in the result set returned by the prepared SQL statement. This routine returns 0 if pStmt is an SQL statement that does not return data (for example an UPDATE).
sqlite3_data_count
	Return the number of values in the current row of the result set.
	After a call to sqlite3_step() that returns SQLITE_ROW, this routine will return the same value as the sqlite3_column_count() function. 
	After sqlite3_step() has returned an SQLITE_DONE, SQLITE_BUSY or error code, or before sqlite3_step() has been called on a prepared SQL statement, this routine returns zero. 
*/

