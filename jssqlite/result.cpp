#include "stdafx.h"
#include "error.h"
#include "result.h"

BEGIN_CLASS( Result )

DEFINE_FINALIZE() {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	if ( pStmt != NULL ) {

		int status = sqlite3_finalize( pStmt );
		if ( status != SQLITE_OK ) {
			// [TBD] do something ?
		}
		JS_SetPrivate( cx, obj, NULL );
	}
}


DEFINE_FUNCTION( Close ) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( pStmt );
	int status = sqlite3_finalize( pStmt );
	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, sqlite3_errcode(sqlite3_db_handle(pStmt)), sqlite3_errmsg(sqlite3_db_handle(pStmt)) );
	JS_SetPrivate( cx, obj, NULL );
	return JS_TRUE;
}


DEFINE_FUNCTION( Step ) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( pStmt );

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
	}
	JS_ReportError( cx, "uncautch error (%d)", status );
	return JS_FALSE;
}


DEFINE_FUNCTION( Col ) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( pStmt );
	RT_ASSERT_ARGC( 1 );

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
			// use default case ( string )
		default: // by default, if type is unknown, use TEXT cast
			*rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,(const char *)sqlite3_column_text( pStmt, col )));
	}
	return JS_TRUE;
}


DEFINE_FUNCTION( Row ) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( pStmt );

	if ( Step( cx, obj, 0, NULL, rval ) == JS_FALSE ) // if something goes wrong in Result_step ( error report has already been set )
		return JS_FALSE;

	if ( *rval == JSVAL_FALSE ) { // the statement has finished executing successfully

		*rval = JSVAL_VOID; // return undefined
		return JS_TRUE;
	}

	// returns an array [ row1Data, row2Data, ... ] else return an object { row1Name:row1Data, row2Name:row2Data,  ... }
	JSBool namedRows = JS_FALSE; // default value
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
		if ( Col( cx, obj, 1, &jsvCol, &colJsValue ) == JS_FALSE ) // if something goes wrong in Result_col ( error report has already been set )
			return JS_FALSE;
		if ( namedRows )
			JS_SetProperty( cx, row, sqlite3_column_name( pStmt, col ), &colJsValue );
		else
			JS_DefineElement( cx, row, col, colJsValue, NULL, NULL, JSPROP_ENUMERATE );
	}
	return JS_TRUE;
}


DEFINE_FUNCTION( Reset ) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( pStmt );
	int status = sqlite3_reset(pStmt);
	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, sqlite3_errcode(sqlite3_db_handle(pStmt)), sqlite3_errmsg(sqlite3_db_handle(pStmt)) );
	return JS_TRUE;
}


DEFINE_PROPERTY( columnCount ) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( pStmt );
	*vp = INT_TO_JSVAL(sqlite3_column_count(pStmt));
	return JS_TRUE;
}


DEFINE_PROPERTY( columnNames ) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( pStmt );
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


DEFINE_PROPERTY( columnIndexes ) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( pStmt );
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

CONFIGURE_CLASS

//	HAS_CONSTRUCTOR
	HAS_FINALIZE
	
	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( columnCount )
		PROPERTY_READ( columnNames )
		PROPERTY_READ( columnIndexes )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION( Step )
		FUNCTION( Reset )
		FUNCTION( Close )
		FUNCTION( Col )
		FUNCTION( Row )
	END_FUNCTION_SPEC

	HAS_PRIVATE

END_CLASS
