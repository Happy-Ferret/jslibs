#include "stdafx.h"
#include "error.h"
#include "result.h"
#include "database.h"


BEGIN_CLASS( Database )


DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING( _class );
	RT_ASSERT_ARGC( 1 );
	char *fileName;
	RT_JSVAL_TO_STRING( argv[0], fileName );
	sqlite3 *db;
	int status = sqlite3_open( fileName, &db );
	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, sqlite3_errcode(db), sqlite3_errmsg(db) );

//	sqlite3_extended_result_codes(db, true); // [TBD] symbol not found ??

	JS_SetPrivate( cx, obj, db );
	return JS_TRUE;
}


DEFINE_FINALIZE() {

	sqlite3 *db = (sqlite3 *)JS_GetPrivate( cx, obj );
	if ( db != NULL ) {

		int status = sqlite3_close( db ); // All prepared statements must finalized before sqlite3_close() is called or else the close will fail with a return code of SQLITE_BUSY.
		if ( status != SQLITE_OK )
			JS_ReportError( cx, "unable to finalize the database (error:%d) ", status );
		JS_SetPrivate( cx, obj, NULL );
	}
}


DEFINE_FUNCTION( Query ) {
	
	RT_ASSERT_ARGC( 1 );

	sqlite3 *db = (sqlite3*)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( db );

	char *sqlQuery = JS_GetStringBytes( JS_ValueToString( cx, argv[0] ) );

	const char *szTail;
	sqlite3_stmt *pStmt;

	int status = sqlite3_prepare( db, sqlQuery, -1, &pStmt, &szTail ); // If the next argument, "nBytes", is less than zero, then zSql is read up to the first nul terminator. 

	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, sqlite3_errcode(db), sqlite3_errmsg(db) );

	RT_ASSERT_1( *szTail == '\0', "too many SQL statements (%s)", szTail ); // for the moment, do not support multiple statements

	if ( pStmt == NULL ) { // if there is an error, *ppStmt may be set to NULL. If the input text contained no SQL (if the input is and empty string or a comment) then *ppStmt is set to NULL.
		// nothing to do here [TBD] why ?
	}
	
	JSObject *dbStatement = JS_NewObject( cx, &classResult, NULL, NULL );
	JS_SetPrivate( cx, dbStatement, pStmt );
	JS_SetReservedSlot(cx, dbStatement, SLOT_RESULT_DATABASE, OBJECT_TO_JSVAL( obj )); // link to avoid GC [TBD] enhance
	*rval = OBJECT_TO_JSVAL( dbStatement );

	return JS_TRUE;
}



DEFINE_FUNCTION( Exec ) {

	RT_ASSERT_ARGC( 1 );
	sqlite3 *db = (sqlite3*)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( db );

	char *sqlQuery;
	RT_JSVAL_TO_STRING( argv[0], sqlQuery );

	const char *szTail;
	sqlite3_stmt *pStmt;
	int status;
	status = sqlite3_prepare( db, sqlQuery, -1, &pStmt, &szTail ); // If the next argument, "nBytes", is less than zero, then zSql is read up to the first nul terminator. 
	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, sqlite3_errcode(db), sqlite3_errmsg(db) );
	RT_ASSERT_1( *szTail == '\0', "too many SQL statements (%s)", szTail ); // for the moment, do not support multiple statements
	SqliteSetupBindings(cx, pStmt, NULL, JS_GetParent(cx, obj) );
	status = sqlite3_step( pStmt ); // The return value will be either SQLITE_BUSY, SQLITE_DONE, SQLITE_ROW, SQLITE_ERROR, or 	SQLITE_MISUSE.
	switch (status) {
		case SQLITE_ERROR:
			return SqliteThrowError( cx, sqlite3_errcode(sqlite3_db_handle( pStmt )), sqlite3_errmsg(sqlite3_db_handle( pStmt )));
		case SQLITE_MISUSE: // means that the this routine was called inappropriately. Perhaps it was called on a virtual machine that had already been finalized or on one that had previously returned SQLITE_ERROR or SQLITE_DONE. Or it could be the case that a database connection is being used by a different thread than the one it was created it.
			REPORT_ERROR( "this routine was called inappropriately" );
		case SQLITE_DONE: // means that the statement has finished executing successfully. sqlite3_step() should not be called again on this virtual machine without first calling sqlite3_reset() to reset the virtual machine back to its initial state.
			//			REPORT_ERROR( "No result found." );
			*rval = JSVAL_VOID;
			break;
		case SQLITE_ROW:
			{
			RT_ASSERT_RETURN( SqliteColumnToJsval(cx, pStmt, 0, rval) );
			status = sqlite3_finalize( pStmt );
			if ( status != SQLITE_OK )
				return SqliteThrowError( cx, sqlite3_errcode(sqlite3_db_handle(pStmt)), sqlite3_errmsg(sqlite3_db_handle(pStmt)) );
			}
			break;
	}
	return JS_TRUE;
}



DEFINE_FUNCTION( Close ) {
	
	sqlite3 *db = (sqlite3 *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( db );
	int status = sqlite3_close( db ); // All prepared statements must finalized before sqlite3_close() is called or else the close will fail with a return code of SQLITE_BUSY.
	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, sqlite3_errcode(db), sqlite3_errmsg(db) );
	JS_SetPrivate( cx, obj, NULL );
	return JS_TRUE;
}


DEFINE_PROPERTY( lastInsertRowid ) {

	sqlite3 *db = (sqlite3 *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( db );
	JS_NewNumberValue( cx, sqlite3_last_insert_rowid(db), vp ); // use JS_NewNumberValue because sqlite3_last_insert_rowid returns int64
  return JS_TRUE;
}


DEFINE_PROPERTY( changes ) {

	sqlite3 *db = (sqlite3 *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( db );

	// This function returns the number of database rows that were changed (or inserted or deleted) by the most recently completed INSERT, UPDATE, or DELETE statement. 
	// Only changes that are directly specified by the INSERT, UPDATE, or DELETE statement are counted. Auxiliary changes caused by triggers are not counted. Use the sqlite3_total_changes() function to find the total number of changes including changes caused by triggers.
	//JS_NewNumberValue( cx, sqlite3_changes(db), vp );
	*vp = INT_TO_JSVAL( sqlite3_changes(db) );
  return JS_TRUE;
}


DEFINE_PROPERTY( version ) {

	*vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,(const char *)sqlite3_libversion()));
  return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE
	
	BEGIN_FUNCTION_SPEC
		FUNCTION( Query )
		FUNCTION( Exec )
		FUNCTION( Close )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( lastInsertRowid )
		PROPERTY_READ( changes )
	END_PROPERTY_SPEC


	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ( version )
	END_STATIC_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS
