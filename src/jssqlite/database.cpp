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


// (TBD) add User-defined Collation Sequences ( http://www.sqlite.org/datatype3.html )


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

	DatabasePrivate *pv = NULL;

	JLStr fileName;

	JL_S_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	//	int isThreadSafe = sqlite3_threadsafe();
	
	int flags;
	if ( JL_ARG_ISDEF(2) )
		flags = JSVAL_TO_INT( JL_ARG(2) );
	else
		flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE; // default

//	flags |= SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE;

	if ( JL_ARG_ISDEF(1) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &fileName) );
	else
		fileName = JLStr(":memory:", true);

	pv = (DatabasePrivate*)JS_malloc(cx, sizeof(DatabasePrivate));
	JL_CHK(pv);

	if ( sqlite3_open_v2(fileName, &pv->db, flags, NULL) != SQLITE_OK )
		JL_CHK( SqliteThrowError(cx, pv->db) );

	sqlite3_extended_result_codes(pv->db, true); // SQLite 3.3.8
	sqlite3_limit(pv->db, SQLITE_LIMIT_FUNCTION_ARG, MAX_FUNCTION_ARG);

	jl::StackInit(&pv->fctpvList);
	jl::StackInit(&pv->stmtList);
	JL_SetPrivate(cx, obj, pv);
	return JS_TRUE;
bad:
	jl_free(pv); // jl_free(NULL) is legal
	return JS_FALSE;
}


DEFINE_FINALIZE() {

	if ( JL_GetHostPrivate(cx)->canSkipCleanup )
		return;

	DatabasePrivate *pv = (DatabasePrivate*)JL_GetPrivate(cx, obj);
	if ( pv == NULL )
		return;

//	jsval v;

	//// finalize open database statements
	//
	//JL_GetReservedSlot(cx, obj, SLOT_SQLITE_DATABASE_STATEMENT_STACK, &v);
	//stack = JSVAL_TO_PRIVATE(v);
	//while ( !jl::StackIsEnd(&stack) ) {
	//	sqlite3_stmt *pStmt = (sqlite3_stmt*)jl::StackPop(&stack);
	//	status = sqlite3_clear_bindings( pStmt );
	// (TBD) usefull ?
	//	status = sqlite3_finalize( pStmt );
	//}

	sqlite3_interrupt(pv->db);

	while ( !jl::StackIsEnd(&pv->fctpvList) )
		jl_free( jl::StackPop( &pv->fctpvList ) );

	while ( !jl::StackIsEnd(&pv->stmtList) )
		sqlite3_finalize( (sqlite3_stmt*)jl::StackPop( &pv->stmtList ) );

/* crash
	for ( sqlite3_stmt *pStmt = sqlite3_next_stmt(pv->db, NULL); pStmt; pStmt = sqlite3_next_stmt(pv->db, pStmt) ) {

		status = sqlite3_finalize(pStmt);
		if ( status != SQLITE_OK )
			JS_ReportError(cx, "Unable to finalize the statement (%d) ", status );
	}
*/

	// close the database
	// All prepared statements must finalized before sqlite3_close() is called or else the close will fail with a return code of SQLITE_BUSY.
	if ( sqlite3_close(pv->db) != SQLITE_OK )
		JS_ReportError( cx, "%s (%d)", sqlite3_errmsg(pv->db), sqlite3_extended_errcode(pv->db) );
//	JL_SetPrivate( cx, obj, NULL );

	JS_free(cx, pv);
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

	JL_DEFINE_FUNCTION_OBJ;

	DatabasePrivate *pv = (DatabasePrivate*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	JL_SetPrivate(cx, obj, NULL);

	sqlite3_interrupt(pv->db);

	while ( !jl::StackIsEnd(&pv->fctpvList) )
		jl_free( jl::StackPop( &pv->fctpvList ) );

	while ( !jl::StackIsEnd(&pv->stmtList) )
		sqlite3_finalize( (sqlite3_stmt*)jl::StackPop( &pv->stmtList ) );

/* chash
	for ( sqlite3_stmt *pStmt = sqlite3_next_stmt(pv->db, NULL); pStmt; pStmt = sqlite3_next_stmt(pv->db, pStmt) ) {

		status = sqlite3_finalize(pStmt);
		if ( status != SQLITE_OK )
			JS_ReportError(cx, "Unable to finalize the statement (error:%d) ", status );	
	}
*/

	// close the database
	// All prepared statements must finalized before sqlite3_close() is called or else the close will fail with a return code of SQLITE_BUSY.
	if ( sqlite3_close( pv->db ) != SQLITE_OK )
		JL_CHK( SqliteThrowError(cx, pv->db) );

	jl_free(pv);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;

bad:
	jl_free(pv);
	return JS_FALSE;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE Result $INAME( sqlStr [, map ] )
  Evaluates a SQL string and returns a Result object ready to be executed.
  $H arguments
   $ARG $STR sqlStr: The SQL query string. ?NNN :VVV and @VVV parameters are accepted.
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
    var res = db.Query('SELECT ? + ?', [4,5]);
    Print( res.Row().toSource() ); // Prints: [9]
    }}}
  $H return value
   A new Result object.
  $H beware
   There are some limitation in variable bindings. For example, they cannot be used to specify a table name.
   `db.Query('SELECT * FROM ?', ['myTable']);` will failed with this exception: `SQLite error 1: near "?": syntax error`
  $H example 1
  {{{
  ...
  var result = db.Query('SELECT name FROM table WHERE id=:userId' );
  result.userId = 1341;
  result.Step();
  Print( result.Col(0) );
  }}}
  $H example 2
  {{{
  var result = db.Query('SELECT name FROM table WHERE id=@userId', { userId: 1341 } );
  Print( result.Col(0) );
  }}}
  $H example 3
  {{{
  var result = db.Query('SELECT ? FROM table WHERE id=?', ['name', 1341] ); // array-like objects {0:'name', 1:1341, length:2} also work.
  Print( result.Col(0) );
  }}}
**/
DEFINE_FUNCTION( Query ) {

	JLStr sql;
	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG_MIN( 1 );

	DatabasePrivate *pv;
	pv = (DatabasePrivate*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);

//	const char *sqlQuery;
//	size_t sqlQueryLength;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &sqlQuery, &sqlQueryLength) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &sql) );


	const char *szTail;
	sqlite3_stmt *pStmt;

	// If the next argument, "nBytes", is less than zero, then zSql is read up to the first nul terminator.
	if ( sqlite3_prepare_v2(pv->db, sql.GetConstStr(), sql.Length(), &pStmt, &szTail) != SQLITE_OK )
		JL_CHK( SqliteThrowError(cx, pv->db) );

	JL_S_ASSERT( *szTail == '\0', "too many SQL statements." ); // for the moment, do not support multiple statements
//	if ( pStmt == NULL ) // if there is an error, *ppStmt may be set to NULL. If the input text contained no SQL (if the input is and empty string or a comment) then *ppStmt is set to NULL.
//		JL_REPORT_ERROR( "Invalid SQL string." );

	jl::StackPush(&pv->stmtList, pStmt);

	// create the Result (statement) object
	JSObject *dbStatement;
	
	dbStatement = JS_NewObjectWithGivenProto(cx, JL_CLASS(Result), JL_PROTOTYPE(cx, Result), NULL);
	JL_CHK( dbStatement );

	JL_SetPrivate(cx, dbStatement, pStmt);
	JL_CHK( JL_SetReservedSlot(cx, dbStatement, SLOT_RESULT_DATABASE, OBJECT_TO_JSVAL( obj )) ); // link to avoid GC
	// (TBD) enhance
	*JL_RVAL = OBJECT_TO_JSVAL( dbStatement );

	if ( argc >= 2 && !JSVAL_IS_PRIMITIVE(JL_ARG(2)) )
		JL_CHK( JL_SetReservedSlot(cx, dbStatement, SLOT_RESULT_QUERY_ARGUMENT_OBJECT, JL_ARG(2)) );

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

	JLStr sql;

	JL_DEFINE_FUNCTION_OBJ;
	
	sqlite3_stmt *pStmt = NULL;
	// see sqlite3_exec()

	JL_S_ASSERT_ARG_MIN( 1 );

	DatabasePrivate *pv;
	pv = (DatabasePrivate*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);

//	const char *sqlQuery;
//	J_JSVAL_TO_STRING( argv[0], sqlQuery );
//	size_t sqlQueryLength;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &sqlQuery, &sqlQueryLength) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &sql) );

	const char *szTail;
	// If the next argument, "nBytes", is less than zero, then zSql is read up to the first nul terminator.
	if ( sqlite3_prepare_v2( pv->db, sql.GetConstStr(), sql.Length(), &pStmt, &szTail ) != SQLITE_OK )
		JL_CHK( SqliteThrowError(cx, pv->db) );
	JL_S_ASSERT( *szTail == '\0', "Too many SQL statements." ); // for the moment, do not support multiple statements
//	if ( pStmt == NULL ) // if there is an error, *ppStmt may be set to NULL. If the input text contained no SQL (if the input is and empty string or a comment) then *ppStmt is set to NULL.
//		JL_REPORT_ERROR( "Invalid SQL string." );

	// (TBD) support multiple statements

	JL_CHK( SqliteSetupBindings(cx, pStmt, argc < 2 || JSVAL_IS_PRIMITIVE( JL_ARG(2) ) ? NULL : JSVAL_TO_OBJECT( JL_ARG(2) ), obj) ); // "@" : the the argument passed to Exec(), ":" nothing

	pv->tmpcx = cx;
	int status;
	status = sqlite3_step(pStmt); // Evaluates the statement. The return value will be either SQLITE_BUSY, SQLITE_DONE, SQLITE_ROW, SQLITE_ERROR, or 	SQLITE_MISUSE.
	pv->tmpcx = NULL;

	JL_CHK( !JL_IsExceptionPending(cx) );

	switch ( status ) {

		case SQLITE_ROW:
			//JL_CHK( SqliteColumnToJsval(cx, pStmt, 0, rval) );
			JL_CHK( SqliteToJsval(cx, sqlite3_column_value(pStmt, 0), JL_RVAL) );
			break;
		case SQLITE_DONE: // means that the statement has finished executing successfully. sqlite3_step() should not be called again on this virtual machine without first calling sqlite3_reset() to reset the virtual machine back to its initial state.
			*JL_RVAL = JSVAL_VOID;
			break;
		case SQLITE_MISUSE: // means that the this routine was called inappropriately. Perhaps it was called on a virtual machine that had already been finalized or on one that had previously returned SQLITE_ERROR or SQLITE_DONE. Or it could be the case that a database connection is being used by a different thread than the one it was created it.
			JL_REPORT_ERROR( "This routine was called inappropriately." );
		default:
			JL_CHK( SqliteThrowError(cx, pv->db) );
	}

	if ( sqlite3_finalize(pStmt) != SQLITE_OK )
		JL_CHK( SqliteThrowError(cx, pv->db) );

	return JS_TRUE;

bad:
	if ( pStmt != NULL )
		sqlite3_finalize(pStmt);

	return JS_FALSE;
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

	DatabasePrivate *pv;
	pv = (DatabasePrivate*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	sqlite3_int64 lastId;
	lastId = sqlite3_last_insert_rowid(pv->db);
	return JL_NativeToJsval(cx, lastId, vp);
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

	DatabasePrivate *pv;
	pv = (DatabasePrivate*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);

	// This function returns the number of database rows that were changed (or inserted or deleted) by the most recently completed INSERT, UPDATE, or DELETE statement.
	// Only changes that are directly specified by the INSERT, UPDATE, or DELETE statement are counted. Auxiliary changes caused by triggers are not counted. Use the sqlite3_total_changes() function to find the total number of changes including changes caused by triggers.
	//JL_NewNumberValue( cx, sqlite3_changes(db), vp );
	*vp = INT_TO_JSVAL( sqlite3_changes(pv->db) ); // sqlite3_total_changes
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

	*vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, sqlite3_libversion()));
  return JL_StoreProperty(cx, obj, id, vp, true);
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the amount of memory currently checked out.
**/
DEFINE_PROPERTY( memoryUsed ) {

	//	int val, tmp;
//	sqlite3_status(SQLITE_STATUS_MEMORY_USED, &val, &tmp, false);
//	if ( val ) {
	return JL_NativeToJsval(cx, (size_t)sqlite3_memory_used(), vp);
}

void sqlite_function_call( sqlite3_context *sCx, int sArgc, sqlite3_value **sArgv ) {

	jsval argv[1 + MAX_FUNCTION_ARG] = {0}; // argv[0] is rval
	//	memset(argv, 0, sizeof(argv)); // set JSVAL_NULL
	JL_ASSERT( JSVAL_IS_PRIMITIVE(argv[0]) && JSVAL_IS_PRIMITIVE(argv[1 + MAX_FUNCTION_ARG -1]) );

	FunctionPrivate *fpv = (FunctionPrivate*)sqlite3_user_data(sCx);
	JSContext *cx = fpv->dbpv->tmpcx;
	JL_S_ASSERT(cx != NULL, "Invalid context.");

	for ( int r = 0; r < sArgc; r++ ) {

		if ( SqliteToJsval(cx, sArgv[r], &argv[r+1]) != JS_TRUE ) {

			//sqlite3_result_error(sCx, "Invalid argument type", -1 ); // (TBD) enhance error report
			sqlite3_result_error_code(sCx, SQLITE_MISMATCH); // (TBD) check this
			goto bad;
		}
	}

	if ( JS_CallFunctionValue(cx, fpv->obj, fpv->fval, sArgc, argv+1, argv) != JS_TRUE ) {

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
			sqlite3_result_int(sCx, argv[0] == JSVAL_TRUE ? 1 : 0 );
			break;
		case JSTYPE_NUMBER:
			if ( JSVAL_IS_INT(argv[0]) ) {

				sqlite3_result_int(sCx, JSVAL_TO_INT(argv[0]));
			} else {

				jsdouble jd;
				JL_CHK( JL_JsvalToNative(cx, argv[0], &jd) );
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

//			if ( JL_GetClass(JSVAL_TO_OBJECT(argv[0])) == JL_GetCachedClassProto(JL_GetHostPrivate(cx), "Blob")->clasp ) { // beware: with SQLite, blob != text
			if ( JL_JsvalIsBlob(cx, argv[0]) ) {

				//const char *data;
				//size_t length;
				//JL_CHKB( JL_JsvalToStringAndLength(cx, &argv[0], &data, &length), bad_unroot );
				JLStr data;
				JL_CHK( JL_JsvalToNative(cx, argv[0], &data) );
				sqlite3_result_blob(sCx, data.GetConstStr(), data.Length(), SQLITE_STATIC); // beware: assume that the string is not GC while SQLite is using it. else use SQLITE_TRANSIENT
				break;
			}
			// else:
		case JSTYPE_XML:
		case JSTYPE_FUNCTION: // (TBD) call the function and pass its result to SQLite ?
		case JSTYPE_STRING: {

//			const char *str;
//			size_t len;
//			JL_CHKB( JL_JsvalToStringAndLength(cx, &argv[0], &str, &len), bad_unroot );

			JLStr str;
			JL_CHK( JL_JsvalToNative(cx, argv[0], &str) );
			sqlite3_result_text(sCx, str.GetConstStr(), str.Length(), SQLITE_STATIC); // beware: assume that the string is not GC while SQLite is using it. else use SQLITE_TRANSIENT // cf.  int sqlite3_bind_text16(sqlite3_stmt*, int, const void*, int n, void(*)(void*));
			break;
		}
		default:
			//sqlite3_result_error(sCx, "Unsupported data type", -1); // (TBD) better error message
			sqlite3_result_error_code(sCx, SQLITE_MISMATCH); // (TBD) check this
	}

bad:
	return;
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

	if ( JL_IsFunction(cx, *vp) && JSID_IS_STRING(id) ) {
		
		JL_S_ASSERT_THIS_CLASS();

		DatabasePrivate *dbpv = (DatabasePrivate*)JL_GetPrivate(cx, obj);
		JL_S_ASSERT_RESOURCE(dbpv);

		FunctionPrivate *fpv = (FunctionPrivate*)jl_malloc(sizeof(FunctionPrivate));
		JL_S_ASSERT_ALLOC(fpv);
		fpv->fval = *vp;
		fpv->obj = obj;
		fpv->dbpv = dbpv;
		
		// sArgc: If this parameter is -1, then the SQL function or aggregate may take any number of arguments between 0 and the limit set by sqlite3_limit(SQLITE_LIMIT_FUNCTION_ARG).
//		int status = sqlite3_create_function(dbpv->db, fName, JS_GetFunctionArity(fun), SQLITE_ANY /*SQLITE_UTF8*/, (void*)fpv, sqlite_function_call, NULL, NULL);
		// if ( sqlite3_create_function16(dbpv->db, JS_GetStringChars(JSID_TO_STRING(id)), JS_GetFunctionArity(JS_ValueToFunction(cx, *vp)), SQLITE_UTF16, (void*)fpv, sqlite_function_call, NULL, NULL) != SQLITE_OK ) {
		if ( sqlite3_create_function16(dbpv->db, JS_GetStringCharsZ(cx, JSID_TO_STRING(id)), JS_GetFunctionArity(JS_ValueToFunction(cx, *vp)), SQLITE_UTF16, (void*)fpv, sqlite_function_call, NULL, NULL) != SQLITE_OK ) {
			
			jl_free(fpv);
			JL_CHK( SqliteThrowError(cx, dbpv->db) );
		}
		jl::StackPush(&dbpv->fctpvList, fpv);
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

 var obj = { foo:Blob('qqwe\0\0fv1234') };
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
