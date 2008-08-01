/* ***** BEGIN LICENSE BLOCK *****
 * Version: GNU GPL 2.0
 *
 * The contents of this file are subject to the
 * GNU General Public License Version 2.0; you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * http://www.gnu.org/licenses/gpl.html
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * ***** END LICENSE BLOCK ***** */

#include "stdafx.h"

#include "../jslang/blobapi.h"

#include "error.h"
#include "result.h"
#include "database.h"

#include "../common/stack.h"

// #include <limits.h> // included by ../common/platform.h
#include <stdlib.h>

// (TBD) add User-defined Collation Sequences ( http://www.sqlite.org/datatype3.html )

/**doc fileIndex:top
$CLASS_HEADER
**/
BEGIN_CLASS( Database )


/**doc
 * $INAME( [fileName] [, flags] )
  Creates a new Database object.
  $H arguments
   $ARG string fileName: is the file name of the database, or an empty string for a temporary database.
    If omitted or <undefined>, an in-memory database is created.
   $ARG enum flags: can be one of the following constant:
    * $CONST READONLY
    * $CONST READWRITE
    * $CONST CREATE
    * $CONST DELETEONCLOSE
    * $CONST EXCLUSIVE
    * $CONST MAIN_DB
    * $CONST TEMP_DB
    * $CONST TRANSIENT_DB
    * $CONST MAIN_JOURNAL
    * $CONST TEMP_JOURNAL
    * $CONST SUBJOURNAL
    * $CONST MASTER_JOURNAL
  $H example
  {{{
  var db = new Database();
  db.Exec('create table t1 (a,b,c);');
  }}}
**/
DEFINE_CONSTRUCTOR() {

//	int isThreadSafe = sqlite3_threadsafe();

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();

	int flags;
	if ( J_ARG_ISDEF(2) )
		flags = JSVAL_TO_INT( J_ARG(2) );
	else
		flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE; // default

	const char *fileName;
	if ( J_ARG_ISDEF(1) )
		J_CHK( JsvalToString(cx, J_ARG(1), &fileName) );
	else
		fileName = ":memory:";

	sqlite3 *db;
//	int status = sqlite3_open( fileName, &db ); // see. sqlite3_open_v2()
	int status = sqlite3_open_v2( fileName, &db, flags, NULL );


	if ( status != SQLITE_OK || db == NULL  )
		return SqliteThrowError( cx, status, sqlite3_errcode(db), sqlite3_errmsg(db) );

	sqlite3_extended_result_codes(db, true); // SQLite 3.3.8

	JS_SetReservedSlot(cx, obj, SLOT_SQLITE_DATABASE_FUNCTION_CALL_STACK, PRIVATE_TO_JSVAL(NULL));
	JS_SetReservedSlot(cx, obj, SLOT_SQLITE_DATABASE_STATEMENT_STACK, PRIVATE_TO_JSVAL(NULL));
	JS_SetPrivate( cx, obj, db );
	return JS_TRUE;
}


DEFINE_FINALIZE() {

	int status;
	sqlite3 *db = (sqlite3 *)JS_GetPrivate( cx, obj );
	if ( db != NULL ) {

		void *stack;
		jsval v;

		//// finalize open database statements
		//
		//JS_GetReservedSlot(cx, obj, SLOT_SQLITE_DATABASE_STATEMENT_STACK, &v);
		//stack = JSVAL_TO_PRIVATE(v);
		//while ( !StackIsEnd(&stack) ) {
		//	sqlite3_stmt *pStmt = (sqlite3_stmt*)StackPop(&stack);
		//	status = sqlite3_clear_bindings( pStmt );
		// (TBD) usefull ?
		//	status = sqlite3_finalize( pStmt );
		//}

		// free the data allocated for function context
		JS_GetReservedSlot(cx, obj, SLOT_SQLITE_DATABASE_FUNCTION_CALL_STACK, &v);
		stack = JSVAL_TO_PRIVATE(v);
		StackFreeContent(&stack); // StackLength( &stack );


		// finalize open database statements
		JS_GetReservedSlot(cx, obj, SLOT_SQLITE_DATABASE_STATEMENT_STACK, &v);
		stack = JSVAL_TO_PRIVATE(v);
		while ( !StackIsEnd(&stack) ) {

			sqlite3_stmt *pStmt = (sqlite3_stmt*)StackPop(&stack);
			if ( pStmt == NULL ) // already finalized ( see Result:Close and Result:Finalize )
				continue;
			status = sqlite3_finalize( pStmt );
			if ( status != SQLITE_OK ) {
				// (TBD) report the error ?
			}
		}

		// close the database
		status = sqlite3_close( db ); // All prepared statements must finalized before sqlite3_close() is called or else the close will fail with a return code of SQLITE_BUSY.
		if ( status != SQLITE_OK )
			JS_ReportError( cx, "unable to finalize the database (error:%d) ", status );
		JS_SetPrivate( cx, obj, NULL );
	}
}


/**doc
=== Methods ===
**/


/**doc
 * $VOID $INAME()
  Close the database and all its opened [Result] objects.
  $H note
   It is recommended to close all [Result] objects before closing the database.
**/
DEFINE_FUNCTION( Close ) {

	sqlite3 *db = (sqlite3 *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( db );
	JS_SetPrivate( cx, obj, NULL );
	int status;
	sqlite3_interrupt(db);
	// free the data allocated for function context
	void *stack;
	jsval v;
	JS_GetReservedSlot(cx, obj, SLOT_SQLITE_DATABASE_FUNCTION_CALL_STACK, &v);
	stack = JSVAL_TO_PRIVATE(v);
	StackFreeContent( &stack );
	// finalize open database statements
	J_CHK( JS_GetReservedSlot(cx, obj, SLOT_SQLITE_DATABASE_STATEMENT_STACK, &v) );
	stack = JSVAL_TO_PRIVATE(v);
	while ( !StackIsEnd(&stack) ) {

		sqlite3_stmt *pStmt = (sqlite3_stmt*)StackPop(&stack);
		if ( pStmt == NULL ) // already finalized ( see Result:Close )
			continue;
		status = sqlite3_finalize( pStmt );
		if ( status != SQLITE_OK )
			return SqliteThrowError( cx, status, sqlite3_errcode(db), sqlite3_errmsg(db) );
	}
	// close the database
	status = sqlite3_close( db ); // All prepared statements must finalized before sqlite3_close() is called or else the close will fail with a return code of SQLITE_BUSY.
	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, status, sqlite3_errcode(db), sqlite3_errmsg(db) );
	return JS_TRUE;
}


/**doc
 * $TYPE Result $INAME( sqlStr [, map ] )
  Evaluates a SQL string and returns a Result object ready to be executed.
  $H arguments
   $ARG string sqlStr:
   $ARG Object map: _map_ is bind to the SQL statement and can be access using '@' char ( see. *Exec* ). If you create new properties on the [Result] object, you can access then in the _sqlStr_ using ':' char. '?' allows you access the _map_ as an array ( see examples ).
    $H example 1
    {{{
    var res = db.Query('SELECT :test1 + 5');
    res.test1 = 123;
    Print( res.Row().toSource() ); // Prints: [128]
    }}}
    $H example 2
    {{{
    var res = db.Query('SELECT @test2 + 5', {test2:6});
    Print( res.Row().toSource() ); // Prints: [11]
    }}}
    $H example 3
    {{{
    var res = db.Query('SELECT ? + ?', [ 4, 5 ]);
    Print( res.Row().toSource() ); // Prints: [9]
    }}}
  $H return value
   returns a new Result object.
  $H beware
   There are some limitation in variable bindings. For example, they cannot be used to specify a table name.
   `db.Query('SELECT * FROM ?', ['myTable']);` will failed with this exception: `SQLite error 1: near "?": syntax error`
  $H example 1
  {{{
  var result = db.Query('SELECT name FROM table WHERE id=:userId' );
  result.userId = 1341;
  Print( result.Col(0) );
  }}}
  $H example 2
  {{{
  var result = db.Query('SELECT name FROM table WHERE id=@userId', { userId:1341 } );
  Print( result.Col(0) );
  }}}
  $H example 3
  {{{
  var result = db.Query('SELECT ? FROM table WHERE id=?', ['name', 1341] ); // array-like {0:'name', 1:1341, length:2} works too.
  Print( result.Col(0) );
  }}}
**/
DEFINE_FUNCTION( Query ) {

	J_S_ASSERT_ARG_MIN( 1 );

	sqlite3 *db = (sqlite3*)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( db );

	const char *sqlQuery;
	size_t sqlQueryLength;
	J_CHK( JsvalToStringAndLength(cx, J_ARG(1), &sqlQuery, &sqlQueryLength) );

	const char *szTail;
	sqlite3_stmt *pStmt;
	int status = sqlite3_prepare_v2( db, sqlQuery, sqlQueryLength, &pStmt, &szTail ); // If the next argument, "nBytes", is less than zero, then zSql is read up to the first nul terminator.
	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, status, sqlite3_errcode(db), sqlite3_errmsg(db) );
	J_S_ASSERT( *szTail == '\0', "too many SQL statements." ); // for the moment, do not support multiple statements
//	if ( pStmt == NULL ) // if there is an error, *ppStmt may be set to NULL. If the input text contained no SQL (if the input is and empty string or a comment) then *ppStmt is set to NULL.
//		J_REPORT_ERROR( "Invalid SQL string." );

	// remember the statement for later finalization
	jsval v;
	JS_GetReservedSlot(cx, obj, SLOT_SQLITE_DATABASE_STATEMENT_STACK, &v);
	void *stack = JSVAL_TO_PRIVATE(v);
	StackPush( &stack, pStmt );
	JS_SetReservedSlot(cx, obj, SLOT_SQLITE_DATABASE_STATEMENT_STACK, PRIVATE_TO_JSVAL(stack));

	// create the Result (statement) object
	JSObject *dbStatement = JS_NewObject( cx, classResult, NULL, NULL );
	JS_SetPrivate( cx, dbStatement, pStmt );
	JS_SetReservedSlot(cx, dbStatement, SLOT_RESULT_DATABASE, OBJECT_TO_JSVAL( obj )); // link to avoid GC
	// (TBD) enhance
	*rval = OBJECT_TO_JSVAL( dbStatement );

	if ( argc >= 2 && argv[1] != JSVAL_VOID && JSVAL_IS_OBJECT(argv[1]) )
		JS_SetReservedSlot(cx, dbStatement, SLOT_RESULT_QUERY_ARGUMENT_OBJECT, argv[1]);

	return JS_TRUE;
}



/**doc
 * $VAL $INAME( sqlStr [, map ] )
  Evaluates a SQL string and return the result in one operation.
  $H arguments
   $ARG string sqlStr: is the SQL statement.
   $ARG Object map: if given, this argument is bind (as a key:value variable map) to the SQL statement.
    $H example
    {{{
    db.Exec('PRAGMA user_version = @ver', { ver:5 } );
    }}}
  $H return value
   returns the first line and first column of the result.
  $H details
   [http://www.sqlite.org/capi3ref.html#sqlite3_bind_blob documentation]
  $H example
  {{{
  var version = db.Exec('PRAGMA user_version');
  db.Exec('PRAGMA user_version = 5');
  }}}
**/
DEFINE_FUNCTION( Exec ) {

	// see sqlite3_exec()

	J_S_ASSERT_ARG_MIN( 1 );
	sqlite3 *db = (sqlite3*)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( db );

	const char *sqlQuery;
//	J_JSVAL_TO_STRING( argv[0], sqlQuery );
	size_t sqlQueryLength;
	J_CHK( JsvalToStringAndLength(cx, J_ARG(1), &sqlQuery, &sqlQueryLength) );

	const char *szTail;
	sqlite3_stmt *pStmt;
	int status;
	status = sqlite3_prepare_v2( db, sqlQuery, sqlQueryLength, &pStmt, &szTail ); // If the next argument, "nBytes", is less than zero, then zSql is read up to the first nul terminator.
	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, status, sqlite3_errcode(db), sqlite3_errmsg(db) );
	J_S_ASSERT( *szTail == '\0', "Too many SQL statements." ); // for the moment, do not support multiple statements
//	if ( pStmt == NULL ) // if there is an error, *ppStmt may be set to NULL. If the input text contained no SQL (if the input is and empty string or a comment) then *ppStmt is set to NULL.
//		J_REPORT_ERROR( "Invalid SQL string." );

	// (TBD) support multiple statements

	if ( argc >= 2 && argv[1] != JSVAL_VOID && JSVAL_IS_OBJECT(argv[1]) )
		J_CHK( SqliteSetupBindings(cx, pStmt, JSVAL_TO_OBJECT(argv[1]) , NULL ) ); // "@" : the the argument passed to Exec(), ":" nothing
	status = sqlite3_step( pStmt ); // Evaluates the statement. The return value will be either SQLITE_BUSY, SQLITE_DONE, SQLITE_ROW, SQLITE_ERROR, or 	SQLITE_MISUSE.

	if ( JS_IsExceptionPending(cx) )
		return JS_FALSE;

	switch (status) {
		case SQLITE_ERROR:
			return SqliteThrowError( cx, status, sqlite3_errcode(sqlite3_db_handle( pStmt )), sqlite3_errmsg(sqlite3_db_handle( pStmt )));
		case SQLITE_MISUSE: // means that the this routine was called inappropriately. Perhaps it was called on a virtual machine that had already been finalized or on one that had previously returned SQLITE_ERROR or SQLITE_DONE. Or it could be the case that a database connection is being used by a different thread than the one it was created it.
			J_REPORT_ERROR( "This routine was called inappropriately." );
		case SQLITE_DONE: // means that the statement has finished executing successfully. sqlite3_step() should not be called again on this virtual machine without first calling sqlite3_reset() to reset the virtual machine back to its initial state.
			*rval = JSVAL_VOID;
			break;
		case SQLITE_ROW:
			J_CHK( SqliteColumnToJsval(cx, pStmt, 0, rval) );
			break;
		default:
			J_REPORT_ERROR_1("invalid case (status:%d)", status );
	}

	status = sqlite3_finalize( pStmt );
	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, status, sqlite3_errcode(sqlite3_db_handle(pStmt)), sqlite3_errmsg(sqlite3_db_handle(pStmt)) );

	return JS_TRUE;
}

/**doc
=== Properties ===
**/

/**doc
 * $INAME $READONLY
  is the rowid of the most recent successful INSERT into the database from the database connection shown in the first argument. If no successful inserts have ever occurred on this database connection, zero is returned.
  $H details
   [http://www.sqlite.org/capi3ref.html#sqlite3_last_insert_rowid documentation]
**/
DEFINE_PROPERTY( lastInsertRowid ) {

	sqlite3 *db = (sqlite3 *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( db );
	JS_NewNumberValue( cx, sqlite3_last_insert_rowid(db), vp ); // use JS_NewNumberValue because sqlite3_last_insert_rowid returns int64
  return JS_TRUE;
}


/**doc
 * $INAME $READONLY
  is the number of database rows that were changed or inserted or deleted by the most recently completed SQL statement on the connection specified by the first parameter. Only changes that are directly specified by the INSERT, UPDATE, or DELETE statement are counted.
  $H details
   [http://www.sqlite.org/capi3ref.html#sqlite3_changes documentation]
**/
DEFINE_PROPERTY( changes ) {

	sqlite3 *db = (sqlite3 *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( db );

	// This function returns the number of database rows that were changed (or inserted or deleted) by the most recently completed INSERT, UPDATE, or DELETE statement.
	// Only changes that are directly specified by the INSERT, UPDATE, or DELETE statement are counted. Auxiliary changes caused by triggers are not counted. Use the sqlite3_total_changes() function to find the total number of changes including changes caused by triggers.
	//JS_NewNumberValue( cx, sqlite3_changes(db), vp );
	*vp = INT_TO_JSVAL( sqlite3_changes(db) );
  return JS_TRUE;
}


/**doc
=== Static Functions ===
**/

/**doc
 * *version* $READONLY
  Hold the current version of the database engine.
**/
DEFINE_PROPERTY( version ) {

	*vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,(const char *)sqlite3_libversion()));
  return JS_TRUE;
}

/**doc
 * *memoryUsed* $READONLY
**/
DEFINE_PROPERTY( memoryUsed ) {

	*vp = INT_TO_JSVAL( sqlite3_memory_used() );
	return JS_TRUE;
}


typedef struct {

	JSRuntime *rt;
	JSObject *object;
	JSFunction *function;
} SqliteFunctionCallUserData;


void sqlite_function_call( sqlite3_context *sCx, int sArgc, sqlite3_value **sArgv ) {

	SqliteFunctionCallUserData *data = (SqliteFunctionCallUserData*)sqlite3_user_data(sCx);

	// need: sqlite3 *sqlite3_context_db_handle(sqlite3_context*); ??


	JSContext *iterp = NULL;
	JSContext *cx = JS_ContextIterator(data->rt, &iterp); // (TBD) change this for multithread
	JSFunction *fun = data->function;
	JSObject *obj = data->object;

	jsval argv[128];
	jsval rval;

//	J_S_ASSERT( sizeof(argv)/sizeof(*argv) <= sArgc, "Too many arguments.");

	int r = 0;
	for ( ; r < sArgc; r++ ) {

		JS_AddRoot(cx, &argv[r]);
		if ( SqliteToJsval( cx, sArgv[r], &argv[r] ) == JS_FALSE ) {

			sqlite3_result_error(sCx, "Invalid type", -1 ); // (TBD) enhance error report & remove roots on error
			goto bad;
		}
	}


	if ( JS_CallFunction(cx, obj, fun, sArgc, argv, &rval) == JS_FALSE ) {
/*
		if ( JS_IsExceptionPending(cx) ) {

			jsval ex;
			JS_GetPendingException(cx, &ex);
			JSErrorReport *err = JS_ErrorFromException(cx, ex);

			if ( err == NULL ) {
				return;
			}

		}
*/

		sqlite3_result_error(sCx, "Function call error", -1 ); // (TBD) better error message
		goto bad;
	}

	// (TBD) how to use sqlite3_result_value
	switch ( JS_TypeOfValue(cx, rval) ) {
		case JSTYPE_VOID:
		case JSTYPE_NULL:
			sqlite3_result_null(sCx); // http://www.sqlite.org/nulls.html
			break;
		case JSTYPE_BOOLEAN:
			sqlite3_result_int(sCx, JSVAL_TO_BOOLEAN(rval) == JS_TRUE ? 1 : 0 );
			break;
		case JSTYPE_NUMBER:
			if ( JSVAL_IS_INT(rval) ) {

				sqlite3_result_int(sCx, JSVAL_TO_INT(rval));
			} else {

				jsdouble jd;
				JS_ValueToNumber(cx, rval, &jd);
				if ( jd >= INT_MIN && jd <= INT_MAX && jd == (int)jd )
					sqlite3_result_int(sCx, (int)jd);
				else
					sqlite3_result_double(sCx, jd);
			}
			break;
		case JSTYPE_OBJECT: // beware: no break; because we use the JSTYPE_STRING's case JS_ValueToString conversion
			if ( JSVAL_IS_NULL(rval) ) {

				sqlite3_result_null(sCx);
				break;
			}
			if ( JS_GET_CLASS(cx, JSVAL_TO_OBJECT(rval)) == BlobJSClass(cx) ) { // beware: with SQLite, blob != text

				const char *data;
				size_t length;
				JsvalToStringAndLength(cx, rval, &data, &length);
				sqlite3_result_blob(sCx, data, length, SQLITE_STATIC); // beware: assume that the string is not GC while SQLite is using it. else use SQLITE_TRANSIENT
				break;
			}
		case JSTYPE_XML:
		case JSTYPE_FUNCTION: // (TBD) call the function and pass its result to SQLite ?
		case JSTYPE_STRING: {
			JSString *jsstr = JS_ValueToString(cx, rval);
			// (TBD) GC protect (root) jsstr
			sqlite3_result_text(sCx, JS_GetStringBytes(jsstr), J_STRING_LENGTH(jsstr), SQLITE_STATIC); // beware: assume that the string is not GC while SQLite is using it. else use SQLITE_TRANSIENT // cf.  int sqlite3_bind_text16(sqlite3_stmt*, int, const void*, int n, void(*)(void*));
			break;
		}
		default:
			sqlite3_result_error(sCx, "Unsupported data type", -1 ); // (TBD) better error message
	}

bad:
	for ( --r; r >= 0; --r )
		JS_RemoveRoot(cx, &argv[r]);
}


/**doc
=== Remarks ===
 * Add SQL functions implemented in JavaScript.
  Any function properties stored to a [Database] object can be used in the SQL string.
  ===== example: =
  {{{
  var db = new Database('myDatabase');
  db.multBy10 = function(a) { return a * 10 }
  Print( db.Exec('SELECT multBy10(123)') ); // prints: 1230
  }}}
**/
DEFINE_SET_PROPERTY() {

	if ( JSVAL_IS_OBJECT(*vp) && *vp != JSVAL_NULL && JS_ObjectIsFunction(cx, JSVAL_TO_OBJECT(*vp) ) ) {

		sqlite3 *db = (sqlite3 *)JS_GetPrivate( cx, obj );
		J_S_ASSERT_RESOURCE( db );

		SqliteFunctionCallUserData *data = (SqliteFunctionCallUserData*)malloc(sizeof(SqliteFunctionCallUserData)); // (TBD) store this allocated pointer in a stack to be freed later
		data->rt = JS_GetRuntime(cx);
		data->object = obj;
		data->function = JS_ValueToFunction(cx, *vp);

		// remember the data allocation
		jsval v;
		JS_GetReservedSlot(cx, obj, SLOT_SQLITE_DATABASE_FUNCTION_CALL_STACK, &v);
		void *stack = JSVAL_TO_PRIVATE(v);
		StackPush( &stack, data );
		JS_SetReservedSlot(cx, obj, SLOT_SQLITE_DATABASE_FUNCTION_CALL_STACK, PRIVATE_TO_JSVAL(stack));

		const char *fName;
		J_CHK( JsvalToString(cx, id, &fName) );

		int status = sqlite3_create_function(db, fName, -1, SQLITE_ANY /*SQLITE_UTF8*/, data, sqlite_function_call, NULL, NULL);
		if ( status != SQLITE_OK )
			return SqliteThrowError( cx, status, sqlite3_errcode(db), sqlite3_errmsg(db) );
	}
	return JS_TRUE;
}

/**doc
=== Note ===
 jslibs Blob object is interpreted as a blob database type.
**/

CONFIGURE_CLASS

	HAS_SET_PROPERTY
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(2)

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
		PROPERTY_READ( memoryUsed )
	END_STATIC_PROPERTY_SPEC

	BEGIN_CONST_INTEGER_SPEC
		CONST_INTEGER( READONLY       , SQLITE_OPEN_READONLY       )
		CONST_INTEGER( READWRITE      , SQLITE_OPEN_READWRITE      )
		CONST_INTEGER( CREATE         , SQLITE_OPEN_CREATE         )
		CONST_INTEGER( DELETEONCLOSE  , SQLITE_OPEN_DELETEONCLOSE  )
		CONST_INTEGER( EXCLUSIVE      , SQLITE_OPEN_EXCLUSIVE      )
		CONST_INTEGER( MAIN_DB        , SQLITE_OPEN_MAIN_DB        )
		CONST_INTEGER( TEMP_DB        , SQLITE_OPEN_TEMP_DB        )
		CONST_INTEGER( TRANSIENT_DB   , SQLITE_OPEN_TRANSIENT_DB   )
		CONST_INTEGER( MAIN_JOURNAL   , SQLITE_OPEN_MAIN_JOURNAL   )
		CONST_INTEGER( TEMP_JOURNAL   , SQLITE_OPEN_TEMP_JOURNAL   )
		CONST_INTEGER( SUBJOURNAL     , SQLITE_OPEN_SUBJOURNAL     )
		CONST_INTEGER( MASTER_JOURNAL , SQLITE_OPEN_MASTER_JOURNAL )
	END_CONST_INTEGER_SPEC

END_CLASS

/**doc
=== Examples ===
 $H example 1
 {{{
 Print('database version: ' + Database.version ,'\n' );

 var obj = { foo:Blob('qqwe\00\00fv1234') };
 Print( 'testFunc = ' + db.Exec('SELECT length(:foo)', obj  ) ,'\n' );
 }}}
 $H example 2
LoadModule('jsstd');
LoadModule('jssqlite');

try {

 var db = new Database();
 db.Exec('create table t1 (a,b,c);');
 db.Exec('insert into t1 (a,b,c) values (5,6,7)');
 db.Exec('insert into t1 (a,b,c) values (2,3,4)');
 db.Exec('insert into t1 (a,b,c) values ("a","b","c")');

 var res = db.Query('SELECT a,c from t1');

 Print( res.Row().toSource(), '\n' );
 Print( res.Row().toSource(), '\n' );
 Print( res.Row().toSource(), '\n' );

 } catch ( ex if ex instanceof SqliteError ) {

  Print( 'SQLite error '+ex.code+': '+ex.text+'\n' );
 }
 **/
