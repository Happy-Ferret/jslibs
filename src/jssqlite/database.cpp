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

#include "../jslang/blobPub.h"

#include "error.h"
#include "result.h"
#include "database.h"

#include "../common/stack.h"

// #include <limits.h> // included by ../common/platform.h
#include <stdlib.h>

// (TBD) add User-defined Collation Sequences ( http://www.sqlite.org/datatype3.html )




DbContext* AddDbContext(sqlite3 *db) {

	DbContext *pv = (DbContext*)malloc(sizeof(DbContext));
	pv->db = db;
	QueueUnshift(dbContextList, pv);
	return pv;
}

DbContext* GetDbContext(sqlite3 *db) {

	for ( jl::QueueCell *it = jl::QueueBegin(dbContextList); it; it = jl::QueueNext(it) )
		if ( ((DbContext*)QueueGetData(it))->db == db )
			return (DbContext*)QueueGetData(it);
	return NULL;
}

void RemoveDbContext(sqlite3 *db) {

	for ( jl::QueueCell *it = jl::QueueBegin(dbContextList); it; it = jl::QueueNext(it) )
		if ( ((DbContext*)QueueGetData(it))->db == db ) {

			free(QueueGetData(it));
			QueueRemoveCell(dbContextList, it);
			return;
		}
}


/**doc fileIndex:top
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Database )


/**doc
$TOC_MEMBER $INAME
 $INAME( [fileName] [, flags] )
  Creates a new Database object.
  $H arguments
   $ARG $STR fileName: is the file name of the database, or an empty string for a temporary database.
    If omitted or $UNDEF, an in-memory database is created.
   $ARG $ENUM flags: can be one of the following constant:
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

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();

	int flags;
	if ( JL_ARG_ISDEF(2) )
		flags = JSVAL_TO_INT( JL_ARG(2) );
	else
		flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE; // default

	const char *fileName;
	if ( JL_ARG_ISDEF(1) )
		JL_CHK( JsvalToString(cx, &JL_ARG(1), &fileName) );
	else
		fileName = ":memory:";

	sqlite3 *db;
//	int status = sqlite3_open( fileName, &db ); // see. sqlite3_open_v2()
	int status;
	status = sqlite3_open_v2( fileName, &db, flags, NULL );

	AddDbContext(db)->obj = obj;

	if ( status != SQLITE_OK || db == NULL  )
		return SqliteThrowError( cx, status, sqlite3_errcode(db), sqlite3_errmsg(db) );

	sqlite3_extended_result_codes(db, true); // SQLite 3.3.8

	JS_SetReservedSlot(cx, obj, SLOT_SQLITE_DATABASE_STATEMENT_STACK, PRIVATE_TO_JSVAL(NULL));
	JL_SetPrivate( cx, obj, db );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FINALIZE() {

	int status;
	sqlite3 *db = (sqlite3 *)JL_GetPrivate( cx, obj );
	if ( db != NULL ) {

		void *stack;
		jsval v;

		//// finalize open database statements
		//
		//JS_GetReservedSlot(cx, obj, SLOT_SQLITE_DATABASE_STATEMENT_STACK, &v);
		//stack = JSVAL_TO_PRIVATE(v);
		//while ( !jl::StackIsEnd(&stack) ) {
		//	sqlite3_stmt *pStmt = (sqlite3_stmt*)jl::StackPop(&stack);
		//	status = sqlite3_clear_bindings( pStmt );
		// (TBD) usefull ?
		//	status = sqlite3_finalize( pStmt );
		//}


		// finalize open database statements
		JS_GetReservedSlot(cx, obj, SLOT_SQLITE_DATABASE_STATEMENT_STACK, &v);
		stack = JSVAL_TO_PRIVATE(v);
		while ( !jl::StackIsEnd(&stack) ) {

			sqlite3_stmt *pStmt = (sqlite3_stmt*)jl::StackPop(&stack);
			if ( pStmt == NULL ) // already finalized ( see Result:Close and Result:Finalize )
				continue;
			status = sqlite3_finalize( pStmt );
			if ( status != SQLITE_OK ) {
				// (TBD) report the error ?
			}
		}

		RemoveDbContext(db);
		// close the database
		status = sqlite3_close( db ); // All prepared statements must finalized before sqlite3_close() is called or else the close will fail with a return code of SQLITE_BUSY.
		if ( status != SQLITE_OK )
			JS_ReportError( cx, "unable to finalize the database (error:%d) ", status );
		JL_SetPrivate( cx, obj, NULL );
	}
}


/**doc
=== Methods ===
**/


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Close the database and all its opened [Result] objects.
  $H note
   It is recommended to close all [Result] objects before closing the database.
**/
DEFINE_FUNCTION( Close ) {

	sqlite3 *db = (sqlite3 *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( db );
	JL_SetPrivate( cx, obj, NULL );
	int status;
	sqlite3_interrupt(db);

	// finalize open database statements
	jsval v;
	JL_CHK( JS_GetReservedSlot(cx, obj, SLOT_SQLITE_DATABASE_STATEMENT_STACK, &v) );
	void *stack;
	stack = JSVAL_TO_PRIVATE(v);
	while ( !jl::StackIsEnd(&stack) ) {

		sqlite3_stmt *pStmt = (sqlite3_stmt*)jl::StackPop(&stack);
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
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE Result $INAME( sqlStr [, map ] )
  Evaluates a SQL string and returns a Result object ready to be executed.
  $H arguments
   $ARG $STR sqlStr:
   $ARG $OBJ map: _map_ is bind to the SQL statement and can be access using '@' char ( see. *Exec* ). If you create new properties on the [Result] object, you can access then in the _sqlStr_ using ':' char. '?' allows you access the _map_ as an array ( see examples ).
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
   A new Result object.
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
  var result = db.Query('SELECT ? FROM table WHERE id=?', ['name', 1341] ); // array-like objects {0:'name', 1:1341, length:2} works too.
  Print( result.Col(0) );
  }}}
**/
DEFINE_FUNCTION( Query ) {

	JL_S_ASSERT_ARG_MIN( 1 );

	sqlite3 *db;
	db = (sqlite3*)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( db );

	const char *sqlQuery;
	size_t sqlQueryLength;
	JL_CHK( JsvalToStringAndLength(cx, &JL_ARG(1), &sqlQuery, &sqlQueryLength) );

	const char *szTail;
	sqlite3_stmt *pStmt;
	int status;
	status = sqlite3_prepare_v2( db, sqlQuery, sqlQueryLength, &pStmt, &szTail ); // If the next argument, "nBytes", is less than zero, then zSql is read up to the first nul terminator.
	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, status, sqlite3_errcode(db), sqlite3_errmsg(db) );
	JL_S_ASSERT( *szTail == '\0', "too many SQL statements." ); // for the moment, do not support multiple statements
//	if ( pStmt == NULL ) // if there is an error, *ppStmt may be set to NULL. If the input text contained no SQL (if the input is and empty string or a comment) then *ppStmt is set to NULL.
//		JL_REPORT_ERROR( "Invalid SQL string." );

	// remember the statement for later finalization
	jsval v;
	JS_GetReservedSlot(cx, obj, SLOT_SQLITE_DATABASE_STATEMENT_STACK, &v);
	void *stack;
	stack = JSVAL_TO_PRIVATE(v);
	jl::StackPush( &stack, pStmt );
	JS_SetReservedSlot(cx, obj, SLOT_SQLITE_DATABASE_STATEMENT_STACK, PRIVATE_TO_JSVAL(stack));

	// create the Result (statement) object
	JSObject *dbStatement;
	dbStatement = JS_NewObject( cx, classResult, NULL, NULL );
	JL_SetPrivate( cx, dbStatement, pStmt );
	JS_SetReservedSlot(cx, dbStatement, SLOT_RESULT_DATABASE, OBJECT_TO_JSVAL( obj )); // link to avoid GC
	// (TBD) enhance
	*rval = OBJECT_TO_JSVAL( dbStatement );

	if ( argc >= 2 && !JSVAL_IS_PRIMITIVE(argv[1]) )
		JS_SetReservedSlot(cx, dbStatement, SLOT_RESULT_QUERY_ARGUMENT_OBJECT, argv[1]);

	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $VAL $INAME( sqlStr [, map ] )
  Evaluates a SQL string and return the result in one operation.
  $H arguments
   $ARG $STR sqlStr: is the SQL statement.
   $ARG $OBJ map: if given, this argument is bind (as a key:value variable map) to the SQL statement.
    $H example
    {{{
    db.Exec('PRAGMA user_version = @ver', { ver:5 } );
    }}}
  $H return value
   returns the first line and first column of the result.
  $H details
   [http://www.sqlite.org/capi3ref.html#sqlite3_bind_blob sqlite documentation]
  $H example
  {{{
  var version = db.Exec('PRAGMA user_version');
  db.Exec('PRAGMA user_version = 5');
  }}}
**/
DEFINE_FUNCTION( Exec ) {

	// see sqlite3_exec()

	JL_S_ASSERT_ARG_MIN( 1 );
	sqlite3 *db;
	db = (sqlite3*)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( db );

	GetDbContext(db)->cx = cx; // update the JS context used to call functions (see sqlite_function_call)

	const char *sqlQuery;
//	J_JSVAL_TO_STRING( argv[0], sqlQuery );
	size_t sqlQueryLength;
	JL_CHK( JsvalToStringAndLength(cx, &JL_ARG(1), &sqlQuery, &sqlQueryLength) );

	const char *szTail;
	sqlite3_stmt *pStmt;
	int status;
	status = sqlite3_prepare_v2( db, sqlQuery, sqlQueryLength, &pStmt, &szTail ); // If the next argument, "nBytes", is less than zero, then zSql is read up to the first nul terminator.
	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, status, sqlite3_errcode(db), sqlite3_errmsg(db) );
	JL_S_ASSERT( *szTail == '\0', "Too many SQL statements." ); // for the moment, do not support multiple statements
//	if ( pStmt == NULL ) // if there is an error, *ppStmt may be set to NULL. If the input text contained no SQL (if the input is and empty string or a comment) then *ppStmt is set to NULL.
//		JL_REPORT_ERROR( "Invalid SQL string." );

	// (TBD) support multiple statements

	if ( argc >= 2 && !JSVAL_IS_VOID( argv[1] ) && JSVAL_IS_OBJECT(argv[1]) )
		JL_CHK( SqliteSetupBindings(cx, pStmt, JSVAL_TO_OBJECT(argv[1]) , NULL ) ); // "@" : the the argument passed to Exec(), ":" nothing
	status = sqlite3_step( pStmt ); // Evaluates the statement. The return value will be either SQLITE_BUSY, SQLITE_DONE, SQLITE_ROW, SQLITE_ERROR, or 	SQLITE_MISUSE.

	JL_SAFE( GetDbContext(db)->cx = NULL );
	if ( JS_IsExceptionPending(cx) )
		return JS_FALSE;

	switch (status) {
		case SQLITE_ERROR:
			return SqliteThrowError( cx, status, sqlite3_errcode(sqlite3_db_handle( pStmt )), sqlite3_errmsg(sqlite3_db_handle( pStmt )));
		case SQLITE_MISUSE: // means that the this routine was called inappropriately. Perhaps it was called on a virtual machine that had already been finalized or on one that had previously returned SQLITE_ERROR or SQLITE_DONE. Or it could be the case that a database connection is being used by a different thread than the one it was created it.
			JL_REPORT_ERROR( "This routine was called inappropriately." );
		case SQLITE_DONE: // means that the statement has finished executing successfully. sqlite3_step() should not be called again on this virtual machine without first calling sqlite3_reset() to reset the virtual machine back to its initial state.
			*rval = JSVAL_VOID;
			break;
		case SQLITE_ROW:
			JL_CHK( SqliteColumnToJsval(cx, pStmt, 0, rval) );
			break;
		default:
			JL_REPORT_ERROR("invalid case (status:%d)", status );
	}

	status = sqlite3_finalize( pStmt );
	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, status, sqlite3_errcode(sqlite3_db_handle(pStmt)), sqlite3_errmsg(sqlite3_db_handle(pStmt)) );

	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  is the rowid of the most recent successful INSERT into the database from the database connection shown in the first argument. If no successful inserts have ever occurred on this database connection, zero is returned.
  $H details
   [http://www.sqlite.org/capi3ref.html#sqlite3_last_insert_rowid sqlite documentation]
**/
DEFINE_PROPERTY( lastInsertRowid ) {

	sqlite3 *db = (sqlite3 *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( db );
	JS_NewNumberValue( cx, sqlite3_last_insert_rowid(db), vp ); // use JS_NewNumberValue because sqlite3_last_insert_rowid returns int64
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  is the number of database rows that were changed or inserted or deleted by the most recently completed SQL statement on the connection specified by the first parameter. Only changes that are directly specified by the INSERT, UPDATE, or DELETE statement are counted.
  $H details
   [http://www.sqlite.org/capi3ref.html#sqlite3_changes sqlite documentation]
**/
DEFINE_PROPERTY( changes ) {

	sqlite3 *db = (sqlite3 *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( db );

	// This function returns the number of database rows that were changed (or inserted or deleted) by the most recently completed INSERT, UPDATE, or DELETE statement.
	// Only changes that are directly specified by the INSERT, UPDATE, or DELETE statement are counted. Auxiliary changes caused by triggers are not counted. Use the sqlite3_total_changes() function to find the total number of changes including changes caused by triggers.
	//JS_NewNumberValue( cx, sqlite3_changes(db), vp );
	*vp = INT_TO_JSVAL( sqlite3_changes(db) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Static Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Hold the current version of the database engine.
**/
DEFINE_PROPERTY( version ) {

	*vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,(const char *)sqlite3_libversion()));
  return JS_TRUE;
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the amount of memory currently checked out.
**/
DEFINE_PROPERTY( memoryUsed ) {

	*vp = INT_TO_JSVAL( sqlite3_memory_used() );
	return JS_TRUE;
}

void sqlite_function_call( sqlite3_context *sCx, int sArgc, sqlite3_value **sArgv ) {

	jsval fVal = (jsval)sqlite3_user_data(sCx);
	DbContext *pv = GetDbContext(sqlite3_context_db_handle(sCx));
	JSContext *cx = pv->cx;

	jsval argv[64+1]; // argv[0] is rval

	if ( sArgc > COUNTOF(argv)-1 ) {

		sqlite3_result_error(sCx, "Too many arguments", -1 );
		return;
	}

	memset(argv, 0, sizeof(argv));
	JSTempValueRooter tvr;
	JS_PUSH_TEMP_ROOT(cx, COUNTOF(argv), argv, &tvr);
	
	for ( int r = 0; r < sArgc; r++ ) {

		if ( SqliteToJsval(cx, sArgv[r], &argv[r+1]) == JS_FALSE ) {

			sqlite3_result_error(sCx, "Invalid type", -1 ); // (TBD) enhance error report & remove roots on error
			goto bad;
		}
	}

	if ( JS_CallFunctionValue(cx, pv->obj, fVal, sArgc, argv+1, argv) != JS_TRUE ) {

		sqlite3_result_error(sCx, "Function call error", -1 ); // (TBD) better error message
		goto bad;
	}

	// (TBD) how to use sqlite3_result_value
	switch ( JS_TypeOfValue(cx, argv[0]) ) {
		case JSTYPE_VOID:
		case JSTYPE_NULL:
			sqlite3_result_null(sCx); // http://www.sqlite.org/nulls.html
			break;
		case JSTYPE_BOOLEAN:
			sqlite3_result_int(sCx, JSVAL_TO_BOOLEAN(argv[0]) == JS_TRUE ? 1 : 0 );
			break;
		case JSTYPE_NUMBER:
			if ( JSVAL_IS_INT(argv[0]) ) {

				sqlite3_result_int(sCx, JSVAL_TO_INT(argv[0]));
			} else {

				jsdouble jd;
				JL_CHK( JS_ValueToNumber(cx, argv[0], &jd) );
				if ( jd >= INT_MIN && jd <= INT_MAX && jd == (int)jd )
					sqlite3_result_int(sCx, (int)jd);
				else
					sqlite3_result_double(sCx, jd);
			}
			break;
		case JSTYPE_OBJECT: // beware: no break; because we use the JSTYPE_STRING's case JS_ValueToString conversion
			if ( JSVAL_IS_NULL(argv[0]) ) {

				sqlite3_result_null(sCx);
				break;
			}
			if ( JL_GetClass(JSVAL_TO_OBJECT(argv[0])) == BlobJSClass(cx) ) { // beware: with SQLite, blob != text

				const char *data;
				size_t length;
				JL_CHK( JsvalToStringAndLength(cx, &argv[0], &data, &length) );
				sqlite3_result_blob(sCx, data, length, SQLITE_STATIC); // beware: assume that the string is not GC while SQLite is using it. else use SQLITE_TRANSIENT
				break;
			}
		case JSTYPE_XML:
		case JSTYPE_FUNCTION: // (TBD) call the function and pass its result to SQLite ?
		case JSTYPE_STRING: {
			const char *str;
			size_t len;
			JL_CHK( JsvalToStringAndLength(cx, &argv[0], &str, &len) );
			sqlite3_result_text(sCx, str, len, SQLITE_STATIC); // beware: assume that the string is not GC while SQLite is using it. else use SQLITE_TRANSIENT // cf.  int sqlite3_bind_text16(sqlite3_stmt*, int, const void*, int n, void(*)(void*));
			break;
		}
		default:
			sqlite3_result_error(sCx, "Unsupported data type", -1 ); // (TBD) better error message
	}

bad:
	JS_POP_TEMP_ROOT(cx, &tvr);
}


/**doc
=== Remarks ===
 * Add SQL functions implemented in JavaScript.
  Any function properties stored to a [Database] object can be used in the SQL string.
  $H example
  {{{
  var db = new Database('myDatabase');
  db.multBy10 = function(a) { return a * 10 }
  Print( db.Exec('SELECT multBy10(123)') ); // prints: 1230
  }}}
**/
DEFINE_SET_PROPERTY() {

	if ( JsvalIsFunction(cx, *vp) ) {

		sqlite3 *db = (sqlite3 *)JL_GetPrivate( cx, obj );
		JL_S_ASSERT_RESOURCE( db );
		const char *fName;
		JL_CHK( JsvalToString(cx, &id, &fName) );
		int status = sqlite3_create_function(db, fName, -1, SQLITE_ANY /*SQLITE_UTF8*/, (void*)*vp, sqlite_function_call, NULL, NULL);
		if ( status != SQLITE_OK )
			return SqliteThrowError( cx, status, sqlite3_errcode(db), sqlite3_errmsg(db) );
	}
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Note ===
 jslibs Blob object is interpreted as a blob database type.
**/

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_SET_PROPERTY
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

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
 {{{
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
 }}}
**/
