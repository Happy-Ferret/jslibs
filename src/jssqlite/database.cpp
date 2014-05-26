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

#include "error.h"
#include "result.h"
#include "database.h"



/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
 $H note
  You cannot construct this class.
**/
BEGIN_CLASS( BlobStream )

enum {
	SLOT_DATABASE
};

struct Private {
	sqlite3_blob *pBlob;
	int position;
};

DEFINE_FINALIZE() {

	if ( jl::Host::getHost(fop->runtime()).hostRuntime().skipCleanup() )
		return;

	Private *pv = (Private*)js::GetObjectPrivate(obj);
	if ( pv != NULL ) {
		
//		sqlite3_blob_close(pv->pBlob); // closed
		jl_free(pv);
	}
}

/**doc
$TOC_MEMBER $INAME
 $VAL $INAME()
**/
DEFINE_FUNCTION( close ) {

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(0);

	Private *pv;
	pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	DatabasePrivate *dbpv;

	{

	JS::RootedValue v(cx);
	JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_DATABASE, &v) );
	JL_ASSERT( v.isObject() );
	
	{
	
	JS::RootedObject tmp(cx, &v.toObject());
	dbpv = (DatabasePrivate*)JL_GetPrivate(tmp);
	JL_ASSERT_OBJECT_STATE(dbpv, JL_GetClassName(tmp) );

	}

	}

	if ( sqlite3_blob_close(pv->pBlob) != SQLITE_OK )
		JL_CHK( SqliteThrowError(cx, dbpv->db) );

	jl::StackRemove(&dbpv->blobList, pv->pBlob);

	JL_CHK( JL_SetReservedSlot(JL_OBJ, SLOT_RESULT_DATABASE, JL_UNDEFINED()) );

	jl_free(pv);

	JL_SetPrivate(JL_OBJ, NULL);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VAL $INAME( [amount] )
**/
DEFINE_FUNCTION( read ) {

	jl::BufPartial buffer;

	JL_DEFINE_ARGS;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(0, 1);

	Private *pv;
	pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	int blobSize;
	blobSize = sqlite3_blob_bytes(pv->pBlob);

	int available;
	available = blobSize - pv->position;

	int amount;
	if ( JL_ARG_ISDEF(1) ) {
		
		JL_CHK( jl::getValue(cx, JL_ARG(1), &amount) );

		if ( amount == 0 && available > 0 ) { // not EOF

			//JL_CHK( JL_NewEmptyBuffer(cx, JL_RVAL) );
			JL_CHK( BlobCreateEmpty(cx, JL_RVAL) );
			return true;
		}

		amount = jl::min(amount, available);
	} else {

		amount = available;
	}

	if ( available == 0 ) { // EOF

		JL_RVAL.setUndefined();
		return true;
	}

	//uint8_t *buffer;
	//buffer = JL_NewBuffer(cx, amount, JL_RVAL);
	//JL_CHK( buffer );

	buffer.alloc(amount);
	JL_ASSERT_ALLOC(buffer);

	int st = sqlite3_blob_read(pv->pBlob, buffer.data(), amount, pv->position);
	if ( st != SQLITE_OK )
		JL_CHK( SqliteThrowErrorStatus(cx, st) );
	pv->position += amount;
	JL_CHK( BlobCreate(cx, buffer, JL_RVAL) );

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VAL $INAME( data )
**/
DEFINE_FUNCTION( write ) {

	jl::BufString data;

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(1);

	JL_CHK( jl::getValue(cx, JL_ARG(1), &data) );

	Private *pv;
	pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	// doc: Use the UPDATE SQL command to change the size of a blob.
	// see sqlite3_bind_zeroblob() and sqlite3_result_zeroblob()

	int st = sqlite3_blob_write(pv->pBlob, data.toData<const uint8_t*>(), data.length(), pv->position);
	if ( st != SQLITE_OK )
		JL_CHK( SqliteThrowErrorStatus(cx, st) );

	pv->position += data.length();

	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( available ) {

	JL_IGNORE(id);

	Private *pv;
	pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	int available = sqlite3_blob_bytes(pv->pBlob) - pv->position;
	JL_CHK( jl::setValue(cx, vp, available) );

	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( position ) {

	JL_IGNORE(id);

	Private *pv;
	pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	JL_CHK( jl::setValue(cx, vp, pv->position) );

	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( position ) {

	JL_IGNORE(id, strict);

	Private *pv;
	pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	JL_CHK( jl::getValue(cx, vp, &pv->position) );

	pv->position = jl::minmax(pv->position, 0, sqlite3_blob_bytes(pv->pBlob));

	return true;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))

	HAS_RESERVED_SLOTS(1)
	HAS_PRIVATE
	HAS_FINALIZE

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( available )
		PROPERTY( position )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION( read )
		FUNCTION( write )
		FUNCTION( close )
	END_FUNCTION_SPEC

END_CLASS






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
  db.exec('create table t1 (a,b,c);');
  }}}
**/
DEFINE_CONSTRUCTOR() {

	DatabasePrivate *pv = NULL;
	jl::BufString fileName;

	JL_DEFINE_ARGS;
	JL_ASSERT_CONSTRUCTING();

	{
	JL_DEFINE_CONSTRUCTOR_OBJ;

	//	int isThreadSafe = sqlite3_threadsafe();
	
	int flags;
	if ( JL_ARG_ISDEF(2) )
		flags = JL_ARG(2).toInt32();
	else
		flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE; // default

//	flags |= SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE;

	if ( JL_ARG_ISDEF(1) )
		JL_CHK( jl::getValue(cx, JL_ARG(1), &fileName) );
	else
		fileName.get(":memory:");

	pv = (DatabasePrivate*)JS_malloc(cx, sizeof(DatabasePrivate));
	JL_CHK(pv);
	pv->db = NULL;

	if ( sqlite3_open_v2(fileName, &pv->db, flags, NULL) != SQLITE_OK )
		JL_CHK( SqliteThrowError(cx, pv->db) );

	sqlite3_extended_result_codes(pv->db, true); // SQLite 3.3.8
	sqlite3_limit(pv->db, SQLITE_LIMIT_FUNCTION_ARG, MAX_FUNCTION_ARG);

	jl::StackInit(&pv->fctpvList);
	jl::StackInit(&pv->stmtList);
	jl::StackInit(&pv->blobList);

	JL_SetPrivate(JL_OBJ, pv);
	return true;
	}

bad:
	if ( pv ) {

		if ( pv->db )
			sqlite3_close(pv->db);
		JS_free(cx, pv);
	}
	return false;
}


DEFINE_FINALIZE() {

	if ( jl::Host::getHost(fop->runtime()).hostRuntime().skipCleanup() )
		return;

	DatabasePrivate *pv = (DatabasePrivate*)js::GetObjectPrivate(obj);
	if ( pv == NULL )
		return;

//	jsval v;

	//// finalize open database statements
	//
	//JL_GetReservedSlot( obj, SLOT_SQLITE_DATABASE_STATEMENT_STACK, &v);
	//stack = JSVAL_TO_PRIVATE(v);
	//while ( !jl::StackIsEnd(&stack) ) {
	//	sqlite3_stmt *pStmt = (sqlite3_stmt*)jl::StackPop(&stack);
	//	status = sqlite3_clear_bindings( pStmt );
	// (TBD) usefull ?
	//	status = sqlite3_finalize( pStmt );
	//}

	sqlite3_interrupt(pv->db);

	while ( !jl::StackIsEnd(&pv->fctpvList) ) {

		FunctionPrivate * fpv = reinterpret_cast<FunctionPrivate*>(jl::StackPop(&pv->fctpvList));
		fpv->FunctionPrivate::~FunctionPrivate();
		jl_free( fpv );
	}

	while ( !jl::StackIsEnd(&pv->stmtList) )
		sqlite3_finalize( (sqlite3_stmt*)jl::StackPop( &pv->stmtList ) );

	while ( !jl::StackIsEnd(&pv->blobList) )
		sqlite3_blob_close( (sqlite3_blob*)jl::StackPop( &pv->blobList ) );

/* crash
	for ( sqlite3_stmt *pStmt = sqlite3_next_stmt(pv->db, NULL); pStmt; pStmt = sqlite3_next_stmt(pv->db, pStmt) ) {

		status = sqlite3_finalize(pStmt);
		if ( status != SQLITE_OK )
			JS_ReportError(cx, "Unable to finalize the statement (%d) ", status );
	}
*/

	// close the database
	// All prepared statements must finalized before sqlite3_close() is called or else the close will fail with a return code of SQLITE_BUSY.
	int status = sqlite3_close(pv->db);
	JL_IGNORE( status );
	// JL_ASSERT_WARN( sqlite3_close(pv->db) == SQLITE_OK, E_NAME(JL_THIS_CLASS_NAME), E_FIN, E_DETAILS, E_STR(sqlite3_errmsg(pv->db)), E_ERRNO(sqlite3_extended_errcode(pv->db)) ); // (TBD) send to log !
//	JL_SetPrivate(  obj, NULL );

bad:
	JS_freeop(fop, pv);
	return;
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
DEFINE_FUNCTION( close ) {

	JL_IGNORE( argc );

	DatabasePrivate *pv = NULL;

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	pv = (DatabasePrivate*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_SetPrivate(JL_OBJ, NULL);

	sqlite3_interrupt(pv->db);

	while ( !jl::StackIsEnd(&pv->fctpvList) ) {

		FunctionPrivate * fpv = reinterpret_cast<FunctionPrivate*>(jl::StackPop(&pv->fctpvList));
		fpv->FunctionPrivate::~FunctionPrivate();
		jl_free( fpv );
	}

	while ( !jl::StackIsEnd(&pv->stmtList) )
		sqlite3_finalize( (sqlite3_stmt*)jl::StackPop( &pv->stmtList ) );

	while ( !jl::StackIsEnd(&pv->blobList) )
		sqlite3_blob_close( (sqlite3_blob*)jl::StackPop( &pv->blobList ) );

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

	JL_RVAL.setUndefined();
	return true;

bad:
	jl_free(pv);
	return false;
}

/**doc
$TOC_MEMBER $INAME
 $TYPE BlobStream $INAME( tableName, columnName, rowid [, readOnly = false] )
**/
DEFINE_FUNCTION( openBlobStream ) {

	jl::BufString tableName, columnName;
	sqlite3_int64 rowid;
	int flags;
	BlobStream::Private *blobStreamPv = NULL;

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(3, 4);

	DatabasePrivate *pv;
	pv = (DatabasePrivate*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	JL_CHK( jl::getValue(cx, JL_ARG(1), &tableName) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &columnName) );
	JL_CHK( jl::getValue(cx, JL_ARG(3), &rowid) );

	// doc: If the flags parameter is non-zero, then the BLOB is opened for read and write access. If it is zero, the BLOB is opened for read access. 
	if ( JL_ARG_ISDEF(4) )
		JL_CHK( jl::getValue(cx, JL_ARG(4), &flags) );
	else
		flags = 1;

	sqlite3_blob *pBlob;
	if ( sqlite3_blob_open(pv->db, "main", tableName, columnName, rowid, flags, &pBlob) != SQLITE_OK ) // (TBD) handle "temp" tables also.
		JL_CHK( SqliteThrowError(cx, pv->db) );

	jl::StackPush(&pv->blobList, pBlob);


	blobStreamPv = (BlobStream::Private*)jl_malloc(sizeof(BlobStream::Private));
	JL_ASSERT_ALLOC( blobStreamPv );

	blobStreamPv->pBlob = pBlob;
	blobStreamPv->position = 0;

	{

	JS::RootedObject blobStreamObj(cx, jl::newObjectWithGivenProto(cx, JL_CLASS(BlobStream), JL_CLASS_PROTOTYPE(cx, BlobStream)));
	JL_CHK( blobStreamObj );

	JL_SetPrivate(blobStreamObj, blobStreamPv);
	JL_CHK( JL_SetReservedSlot(blobStreamObj, BlobStream::SLOT_DATABASE, JL_OBJVAL) ); // link to avoid GC

	JL_RVAL.setObject(*blobStreamObj);
	
	}

	return true;

bad:
	if ( blobStreamPv ) {

		if ( blobStreamPv->pBlob )
			sqlite3_blob_close( blobStreamPv->pBlob );
		jl_free(blobStreamPv);
	}
	return false;
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
    var res = db.query('SELECT :test1 + 5');
    res.test1 = 123;
    print( res.row().toSource() ); // Prints: [128]
    }}}
    $H example 2
    {{{
    var res = db.query('SELECT @test2 + 5', {test2:6});
    print( res.row().toSource() ); // Prints: [11]
    }}}
    $H example 3
    {{{
    var res = db.query('SELECT ? + ?', [4,5]);
    print( res.row().toSource() ); // Prints: [9]
    }}}
  $H return value
   A new Result object.
  $H beware
   There are some limitation in variable bindings. For example, they cannot be used to specify a table name.
   `db.query('SELECT * FROM ?', ['myTable']);` will failed with this exception: `SQLite error 1: near "?": syntax error`
  $H example 1
  {{{
  ...
  var result = db.query('SELECT name FROM table WHERE id=:userId' );
  result.userId = 1341;
  result.step();
  print( result.col(0) );
  }}}
  $H example 2
  {{{
  var result = db.query('SELECT name FROM table WHERE id=@userId', { userId: 1341 } );
  print( result.col(0) );
  }}}
  $H example 3
  {{{
  var result = db.query('SELECT ? FROM table WHERE id=?', ['name', 1341] ); // array-like objects {0:'name', 1:1341, length:2} also work.
  print( result.col(0) );
  }}}
**/
DEFINE_FUNCTION( query ) {

	jl::BufString sql;
	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN(1);

	DatabasePrivate *pv;
	pv = (DatabasePrivate*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	JL_CHK( jl::getValue(cx, JL_ARG(1), &sql) );

	const char *szTail;
	sqlite3_stmt *pStmt;

	// If the next argument, "nBytes", is less than zero, then zSql is read up to the first nul terminator.

	const char *sqlStr;
	sqlStr = sql.toData<const char *>();
	size_t sqlLen;
	sqlLen = sql.length();

	if ( sqlite3_prepare_v2(pv->db, sqlStr, sqlLen, &pStmt, &szTail) != SQLITE_OK )
		JL_CHK( SqliteThrowError(cx, pv->db) );
	ASSERT( pStmt != NULL );
	JL_ASSERT_WARN( szTail == sqlStr + sqlLen, E_STR("too many SQL statements") ); // (TBD) for the moment, do not support multiple statements

//	if ( pStmt == NULL ) // if there is an error, *ppStmt may be set to NULL. If the input text contained no SQL (if the input is and empty string or a comment) then *ppStmt is set to NULL.
//		JL_REPORT_ERROR( "Invalid SQL string." );

	jl::StackPush(&pv->stmtList, pStmt);

	{

	// create the Result (statement) object
	JS::RootedObject dbStatement(cx, jl::newObjectWithGivenProto(cx, JL_CLASS(Result), JL_CLASS_PROTOTYPE(cx, Result)));
	JL_CHK( dbStatement );
	JL_SetPrivate(dbStatement, pStmt);
	JL_CHK( JL_SetReservedSlot( dbStatement, SLOT_RESULT_DATABASE, JL_OBJVAL) ); // link to avoid GC
	// (TBD) enhance
	JL_RVAL.setObject(*dbStatement);

	if ( argc >= 2 && !JSVAL_IS_PRIMITIVE(JL_ARG(2)) )
		JL_CHK( JL_SetReservedSlot( dbStatement, SLOT_RESULT_QUERY_ARGUMENT_OBJECT, JL_ARG(2)) );

	return true;
	
	}

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
    db.exec('PRAGMA user_version = @ver', { ver:5 } );
    }}}
  $H return value
   returns the first line and first column of the result.
  $H details
   [http://www.sqlite.org/capi3ref.html#sqlite3_bind_blob sqlite documentation]
  $H example
  {{{
  var version = db.exec('PRAGMA user_version');
  db.exec('PRAGMA user_version = 5');
  }}}
**/
DEFINE_FUNCTION( exec ) {

	jl::BufString sql;

	sqlite3_stmt *pStmt = NULL;
	// see sqlite3_exec()
	
	JL_DEFINE_ARGS;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 1 );

	DatabasePrivate *pv;
	pv = (DatabasePrivate*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

//	const char *sqlQuery;
//	J_JSVAL_TO_STRING( argv[0], sqlQuery );
//	size_t sqlQueryLength;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &sqlQuery, &sqlQueryLength) );
	JL_CHK( jl::getValue(cx, JL_ARG(1), &sql) );

	const char *szTail;
	// If the next argument, "nBytes", is less than zero, then zSql is read up to the first nul terminator.

	const char *sqlStr;
	sqlStr = sql.toData<const char *>();
	size_t sqlLen;
	sqlLen = sql.length();

	if ( sqlite3_prepare_v2( pv->db, sqlStr, sqlLen, &pStmt, &szTail ) != SQLITE_OK )
		JL_CHK( SqliteThrowError(cx, pv->db) );
	ASSERT( pStmt != NULL );
	JL_ASSERT_WARN( szTail == sqlStr + sqlLen, E_STR("too many SQL statements") ); // for the moment, do not support multiple statements

//	if ( pStmt == NULL ) // if there is an error, *ppStmt may be set to NULL. If the input text contained no SQL (if the input is and empty string or a comment) then *ppStmt is set to NULL.
//		JL_REPORT_ERROR( "Invalid SQL string." );

	// (TBD) support multiple statements

	JL_CHK( SqliteSetupBindings(cx, pStmt, (JL_ARGC >= 2 && JL_ARG(2).isObject()) ? JL_ARG(2) : JS::NullHandleValue, JL_OBJ) ); // "@" : the the argument passed to exec(), ":" nothing

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
			JL_RVAL.setUndefined();
			break;
		case SQLITE_MISUSE: // means that the this routine was called inappropriately. Perhaps it was called on a virtual machine that had already been finalized or on one that had previously returned SQLITE_ERROR or SQLITE_DONE. Or it could be the case that a database connection is being used by a different thread than the one it was created it.
			JL_ERR( E_LIB, E_OPERATION, E_SEP, E_CALL, E_INVALID ); // "this routine was called inappropriately"
		default:
			JL_CHK( SqliteThrowError(cx, pv->db) );
	}

	if ( sqlite3_finalize(pStmt) != SQLITE_OK )
		JL_CHK( SqliteThrowError(cx, pv->db) );

	return true;

bad:
	if ( pStmt != NULL )
		sqlite3_finalize(pStmt);

	return false;
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
DEFINE_PROPERTY_GETTER( lastInsertRowid ) {

	JL_DEFINE_PROP_ARGS;
	JL_ASSERT_THIS_INSTANCE();

	DatabasePrivate *pv;
	pv = (DatabasePrivate*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	sqlite3_int64 lastId;
	lastId = sqlite3_last_insert_rowid(pv->db);
	return jl::setValue(cx, vp, lastId);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  is the number of database rows that were changed or inserted or deleted by the most recently completed SQL statement on the connection specified by the first parameter. Only changes that are directly specified by the INSERT, UPDATE, or DELETE statement are counted.
  $H details
   [http://www.sqlite.org/capi3ref.html#sqlite3_changes sqlite documentation]
**/
DEFINE_PROPERTY_GETTER( changes ) {

	JL_DEFINE_PROP_ARGS;
	JL_ASSERT_THIS_INSTANCE();

	DatabasePrivate *pv;
	pv = (DatabasePrivate*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	// This function returns the number of database rows that were changed (or inserted or deleted) by the most recently completed INSERT, UPDATE, or DELETE statement.
	// Only changes that are directly specified by the INSERT, UPDATE, or DELETE statement are counted. Auxiliary changes caused by triggers are not counted. Use the sqlite3_total_changes() function to find the total number of changes including changes caused by triggers.
	//JL_NewNumberValue( cx, sqlite3_changes(db), vp );
	vp.setInt32( sqlite3_changes(pv->db) ); // sqlite3_total_changes
	return true;
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
DEFINE_PROPERTY_GETTER( version ) {

	vp.setString(JS_NewStringCopyZ(cx, sqlite3_libversion()));
	return jl::StoreProperty(cx, obj, id, vp, true);
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the amount of memory currently checked out.
**/
DEFINE_PROPERTY_GETTER( memoryUsed ) {

	JL_IGNORE( id, obj );

	//	int val, tmp;
//	sqlite3_status(SQLITE_STATUS_MEMORY_USED, &val, &tmp, false);
//	if ( val ) {
	return jl::setValue(cx, vp, (size_t)sqlite3_memory_used());
}

void sqlite_function_call( sqlite3_context *sCx, int sArgc, sqlite3_value **sArgv ) {

	//S_ASSERT( JSVAL_NULL == 0 );
	//ASSERT( DOUBLE_TO_JSVAL(0).asRawBits() == 0 );

	FunctionPrivate *fpv = (FunctionPrivate*)sqlite3_user_data(sCx);
	JSContext *cx = fpv->dbpv->tmpcx;
	ASSERT( cx != NULL );

	//JS::AutoValueArray<1 + MAX_FUNCTION_ARG> argv(cx);
	JS::RootedValue rval(cx);
	JS::AutoValueVector argv(cx);
	argv.resize(sArgc);

	//jsval argv[1 + MAX_FUNCTION_ARG]; // = {0}; // argv[0] is rval
	//memset(argv, 0, sizeof(argv));
	//ASSERT( JSVAL_IS_PRIMITIVE(argv[0]) && JSVAL_IS_PRIMITIVE(argv[1 + MAX_FUNCTION_ARG -1]) );

	

	for ( int r = 0; r < sArgc; r++ ) {

		if ( !SqliteToJsval(cx, sArgv[r], argv.handleAt(r)) ) {

			//sqlite3_result_error(sCx, "Invalid argument type", -1 ); // (TBD) enhance error report
			sqlite3_result_error_code(sCx, SQLITE_MISMATCH); // (TBD) check this
			JL_CHK( false );
		}
	}


	if ( JS_CallFunctionValue(cx, fpv->obj, fpv->fval, argv, &rval) != true ) {

		sqlite3_result_error(sCx, "Function call error", -1 ); // (TBD) better error message
		JL_CHK( false );
	}

	// (TBD) how to use sqlite3_result_value
	switch ( JS_TypeOfValue(cx, rval) ) {

		case JSTYPE_VOID:
		case JSTYPE_NULL:
			sqlite3_result_null(sCx); // http://www.sqlite.org/nulls.html
			break;
		case JSTYPE_BOOLEAN:
			sqlite3_result_int(sCx, rval == JSVAL_TRUE ? 1 : 0 );
			break;
		case JSTYPE_NUMBER:
			if ( JSVAL_IS_INT(rval) ) {

				sqlite3_result_int(sCx, rval.toInt32());
			} else {

				double jd;
				JL_CHK( jl::getValue(cx, rval, &jd) );
				if ( jd >= INT_MIN && jd <= INT_MAX && jd == (int)jd )
					sqlite3_result_int(sCx, (int)jd);
				else
					sqlite3_result_double(sCx, jd);
			}
			break;
		case JSTYPE_OBJECT: // beware: no break; because we use the JSTYPE_STRING's case JS::ToString conversion
			if ( rval.isNull() ) {

				sqlite3_result_null(sCx);
				break;
			}

//			if ( JL_GetClass(JSVAL_TO_OBJECT(rval)) == JL_GetCachedClassProto(JL_GetHostPrivate(cx), "Blob")->clasp ) { // beware: with SQLite, blob != text
//			if ( JL_JsvalIsBlob(cx, rval) ) {
			if ( jl::isData(cx, rval) ) {

				//const char *data;
				//size_t length;
				//JL_CHKB( JL_JsvalToStringAndLength(cx, &rval, &data, &length), bad_unroot );
				jl::BufString data;
				JL_CHK( jl::getValue(cx, rval, &data) );
				sqlite3_result_blob(sCx, data.toData<const uint8_t*>(), data.length(), SQLITE_STATIC); // beware: assume that the string is not GC while SQLite is using it. else use SQLITE_TRANSIENT
				break;
			}
			// else:
		case JSTYPE_FUNCTION: // (TBD) call the function and pass its result to SQLite ?
		case JSTYPE_STRING: {

//			const char *str;
//			size_t len;
//			JL_CHKB( JL_JsvalToStringAndLength(cx, &rval, &str, &len), bad_unroot );

			jl::BufString str;
			JL_CHK( jl::getValue(cx, rval, &str) );
			sqlite3_result_text(sCx, str.toData<const char*>(), str.length(), SQLITE_STATIC); // beware: assume that the string is not GC while SQLite is using it. else use SQLITE_TRANSIENT // cf.  int sqlite3_bind_text16(sqlite3_stmt*, int, const void*, int n, void(*)(void*));
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
  print( db.exec('SELECT multBy10(123)') ); // prints: 1230
  }}}
**/
DEFINE_SET_PROPERTY() {

	JL_DEFINE_PROP_ARGS;

	if ( jl::isCallable(cx, vp) && JSID_IS_STRING(id) ) {
		
		JL_ASSERT_THIS_INSTANCE();

		DatabasePrivate *dbpv = (DatabasePrivate*)JL_GetPrivate(obj);
		JL_ASSERT_THIS_OBJECT_STATE(dbpv);

		FunctionPrivate *fpv = new (jl_malloc(sizeof(FunctionPrivate))) FunctionPrivate(cx);

		JL_ASSERT_ALLOC(fpv);
		fpv->fval = *vp.address();
		fpv->obj = obj;
		fpv->dbpv = dbpv;
		
		// sArgc: If this parameter is -1, then the SQL function or aggregate may take any number of arguments between 0 and the limit set by sqlite3_limit(SQLITE_LIMIT_FUNCTION_ARG).
//		int status = sqlite3_create_function(dbpv->db, fName, JS_GetFunctionArity(fun), SQLITE_ANY /*SQLITE_UTF8*/, (void*)fpv, sqlite_function_call, NULL, NULL);
		// if ( sqlite3_create_function16(dbpv->db, JS_GetStringChars(JSID_TO_STRING(id)), JS_GetFunctionArity(JS_ValueToFunction(cx, *vp)), SQLITE_UTF16, (void*)fpv, sqlite_function_call, NULL, NULL) != SQLITE_OK ) {
		if ( sqlite3_create_function16(dbpv->db, JS_GetStringCharsZ(cx, JSID_TO_STRING(id)), JS_GetFunctionArity(JS_ValueToFunction(cx, vp)), SQLITE_UTF16, (void*)fpv, sqlite_function_call, NULL, NULL) != SQLITE_OK ) {
			
			jl_free(fpv);
			JL_CHK( SqliteThrowError(cx, dbpv->db) );
		}
		jl::StackPush(&dbpv->fctpvList, fpv);
	}
	return true;
	JL_BAD;
}

/**doc
=== Note ===
 jslibs Blob object is interpreted as a blob database type.
**/

CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))
	HAS_PRIVATE

	HAS_SET_PROPERTY
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( query )
		FUNCTION( exec )
		FUNCTION( close )
		FUNCTION( openBlobStream )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( lastInsertRowid )
		PROPERTY_GETTER( changes )
	END_PROPERTY_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_GETTER( version )
		PROPERTY_GETTER( memoryUsed )
	END_STATIC_PROPERTY_SPEC

	BEGIN_CONST
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
	END_CONST

END_CLASS

/**doc
=== Examples ===
 $H example 1
 {{{
 print('database version: ' + Database.version ,'\n' );

 var obj = { foo:toString('qqwe\0\0fv1234', true) };
 print( 'testFunc = ' + db.exec('SELECT length(:foo)', obj  ) ,'\n' );
 }}}
 $H example 2
 {{{
 loadModule('jsstd');
 loadModule('jssqlite');

 try {

  var db = new Database();
  db.exec('create table t1 (a,b,c);');
  db.exec('insert into t1 (a,b,c) values (5,6,7)');
  db.exec('insert into t1 (a,b,c) values (2,3,4)');
  db.exec('insert into t1 (a,b,c) values ("a","b","c")');

  var res = db.query('SELECT a,c from t1');

  print( res.row().toSource(), '\n' );
  print( res.row().toSource(), '\n' );
  print( res.row().toSource(), '\n' );

  } catch ( ex if ex instanceof SqliteError ) {

   print( 'SQLite error '+ex.code+': '+ex.text+'\n' );
  }
 }}}
**/
