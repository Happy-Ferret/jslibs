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


JSBool SqliteToJsval( JSContext *cx, sqlite3_value *value, jsval *rval ) {

	switch( sqlite3_value_type(value) ) {

		case SQLITE_INTEGER:
			JL_CHK( JL_NativeToJsval(cx, sqlite3_value_int(value), rval) );
			break;
		case SQLITE_FLOAT:
			JL_CHK( JL_NativeToJsval(cx, sqlite3_value_double(value), rval) );
			break;
		case SQLITE_BLOB:
			JL_CHK( JL_NewBlobCopyN(cx, sqlite3_value_blob(value), sqlite3_value_bytes(value), rval) );
			break;
		case SQLITE_NULL:
			*rval = JSVAL_NULL;
			break;
		case SQLITE_TEXT:
			*rval = STRING_TO_JSVAL(JS_NewStringCopyN(cx, (const char *)sqlite3_value_text(value), sqlite3_value_bytes(value)));
			//*rval = STRING_TO_JSVAL(JS_NewUCStringCopyZ(cx, (const jschar*)sqlite3_value_text16(value)));
			break;
		default:
			JL_REPORT_ERROR_NUM(cx, JLSMSG_TYPE_ERROR, "unsupported database data type" );
	}
	return JS_TRUE;
	JL_BAD;
}



// doc: The sqlite3_bind_*() routines must be called after sqlite3_prepare() or sqlite3_reset() and before sqlite3_step(). Bindings are not cleared by the sqlite3_reset() routine. Unbound parameters are interpreted as NULL.
JSBool SqliteSetupBindings( JSContext *cx, sqlite3_stmt *pStmt, JSObject *argObj, JSObject *curObj ) {

	jsval val;
	int anonParamIndex = 0;
	const char *name;

	int count = sqlite3_bind_parameter_count(pStmt);
	for ( int param = 1; param <= count; param++ ) { // The first host parameter has an index of 1, not 0.

		// doc: Parameters of the form "?" have no name. ... If the value n is out of range or if the n-th parameter is nameless, then NULL is returned.
		name = sqlite3_bind_parameter_name(pStmt, param);

		if ( name == NULL || name[0] == '?' ) {

			if ( argObj != NULL ) {

				JL_CHK( JS_GetElement(cx, argObj, anonParamIndex, &val) ); // works with {0:2,1:2,2:2,length:3} and [2,2,2]
				anonParamIndex++;
				goto next;
			}

			val = JSVAL_VOID;
			JL_REPORT_ERROR("Unavailable %d-th anonymous SQL parameter.", anonParamIndex);
			goto next;
		}

		if ( name[0] == '$' ) {

//			JL_CHK( JL_GetVariableValue(cx, name+1, &val) );
//			goto next;
			JL_REPORT_ERROR("Unsupported SQL parameter prefix (%s).", name);
		}

		if ( name[0] == '@' ) {

			if ( argObj != NULL )  {

				JL_CHK( JS_GetProperty(cx, argObj, name+1, &val) );
			} else {
			
				val = JSVAL_VOID;
				JL_REPORT_ERROR("Undefined %s SQL parameter.", name);
			}
			goto next;
		}
		if ( name[0] == ':' ) {

			if ( curObj != NULL ) {

				JL_CHK( JS_GetProperty(cx, curObj, name+1, &val) );
			} else {
			
				val = JSVAL_VOID;
				JL_REPORT_ERROR("Undefined %s SQL parameter.", name);
			}
			goto next;
		}

		JL_REPORT_ERROR("Unsupported SQL Parameter");
	next:

		int ret;
		// sqlite3_bind_value( pStmt, param,
		// (TBD) how to use this
		switch ( JS_TypeOfValue(cx, val) ) {

			case JSTYPE_VOID:
			case JSTYPE_NULL: // http://www.sqlite.org/nulls.html
				if ( sqlite3_bind_null(pStmt, param) != SQLITE_OK )
					return SqliteThrowError(cx, sqlite3_db_handle(pStmt));
				break;
			case JSTYPE_BOOLEAN:
				if ( sqlite3_bind_int(pStmt, param, JSVAL_TO_BOOLEAN(val) == JS_TRUE ? 1 : 0 ) != SQLITE_OK )
					return SqliteThrowError(cx, sqlite3_db_handle(pStmt));
				break;
			case JSTYPE_NUMBER:
				if ( JSVAL_IS_INT(val) ) {

					ret = sqlite3_bind_int(pStmt, param, JSVAL_TO_INT(val));
				} else {

					jsdouble jd;
					JL_CHK( JL_JsvalToNative(cx, val, &jd) );
					if ( jd >= INT_MIN && jd <= INT_MAX && jd == (int)jd )
						ret = sqlite3_bind_int( pStmt, param, (int)jd );
					else
						ret = sqlite3_bind_double(pStmt, param, jd);
				}
				if ( ret != SQLITE_OK )
					return SqliteThrowError(cx, sqlite3_db_handle(pStmt));
				break;
			case JSTYPE_OBJECT: // beware: no break; because we use the JSTYPE_STRING's case JS_ValueToString conversion
				if ( JSVAL_IS_NULL(val) ) {

					if ( sqlite3_bind_null(pStmt, param) != SQLITE_OK )
						return SqliteThrowError(cx, sqlite3_db_handle(pStmt));
					break;
				}
//				if ( JL_GetClass(JSVAL_TO_OBJECT(val)) == JL_GetCachedClassProto(JL_GetHostPrivate(cx), "Blob")->clasp ) { // beware: with SQLite, blob != text
				if ( JL_JsvalIsBlob(cx, val) ) {

					JLStr data;
					JL_CHK( JL_JsvalToNative(cx, val, &data) );
					if ( sqlite3_bind_blob(pStmt, param, data.GetConstStr(), data.Length(), SQLITE_STATIC) != SQLITE_OK ) // beware: assume that the string is not GC while SQLite is using it. else use SQLITE_TRANSIENT
						return SqliteThrowError(cx, sqlite3_db_handle(pStmt));
					break;
				}
			case JSTYPE_XML:
			case JSTYPE_FUNCTION: // (TBD) call the function and pass its result to SQLite ?
			case JSTYPE_STRING: {

				JLStr str;
				JL_CHK( JL_JsvalToNative(cx, val, &str) );

				if ( sqlite3_bind_text(pStmt, param, str.GetConstStr(), str.Length(), SQLITE_STATIC) != SQLITE_OK ) // beware: assume that the string is not GC while SQLite is using it. else use SQLITE_TRANSIENT
					return SqliteThrowError(cx, sqlite3_db_handle(pStmt));
				}
				break;
			default:
				JL_REPORT_ERROR_NUM(cx, JLSMSG_TYPE_ERROR, "unsupported SQL parameter data type"); // (TBD) better error message
		}
	}
	return JS_TRUE;
	JL_BAD;
}


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
 A Result object is used to store a compiled SQL statement ready to be executed.$LF
 When a statement has been prepared with Database.*Query* function, you need to execute it ( with *Step* function ) before any data can be read.
 However, some properties (like *columnCount*, ... ) can be read before the first *Step* has been done.$LF
 A result has the ability to be iterated through a  _for each..in_  loop (_for..in_ loop is note supported).
 $H example 1
  {{{
  var db = new Database(); // in-memory database
  db.Exec('create table t1 (name,value);');
  db.Exec('insert into t1 (name,value) values ("red","#F00")');
  db.Exec('insert into t1 (name,value) values ("green","#0F0")');
  db.Exec('insert into t1 (name,value) values ("blue","#00F")');
  
  for each ( row in db.Query('SELECT * from t1') )
   Print( row.name + ' = ' + row.value, '\n' );
  }}}
  prints:
  {{{
  red = #F00
  green = #0F0
  blue = #00F
  }}}

 $H example 2
  {{{
  Print( [ color.name for each ( color in db.Query('SELECT * from t1') ) ] ); // prints: red,green,blue
  }}}

 $H note
  You cannot construct this class.
**/
BEGIN_CLASS( Result )


DEFINE_FINALIZE() {

	if ( JL_GetHostPrivate(cx)->canSkipCleanup )
		return;

	sqlite3_stmt *pStmt = (sqlite3_stmt*)JL_GetPrivate(cx, obj);
	if ( pStmt != NULL ) {

/* unable to do this because the database may have already been finalized.
		jsval dbVal;
		JL_GetReservedSlot(cx, obj, SLOT_RESULT_DATABASE, &dbVal);
		if ( !JSVAL_IS_VOID(dbVal) ) {

			DatabasePrivate *dbpv = (DatabasePrivate*)JL_GetPrivate(cx, JSVAL_TO_OBJECT(dbVal));
			jl::StackRemove(&dbpv->stmtList, pStmt);
		}

		int status = sqlite3_finalize(pStmt);
		if ( status != SQLITE_OK )
			JS_ReportError(cx, "Unable to finalize the statement (%d) ", status);
*/
	}
}

/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Close the current Result object.
**/
DEFINE_FUNCTION( Close ) {

	JL_DEFINE_FUNCTION_OBJ;
	sqlite3_stmt *pStmt = (sqlite3_stmt*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pStmt );

	jsval v;
	JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_RESULT_DATABASE, &v) );
	DatabasePrivate *dbpv;
	dbpv = (DatabasePrivate*)JL_GetPrivate(cx, JSVAL_TO_OBJECT(v));
	JL_S_ASSERT_RESOURCE(dbpv);

	if ( sqlite3_finalize(pStmt) != SQLITE_OK )
		JL_CHK( SqliteThrowError(cx, dbpv->db) );

	jl::StackRemove(&dbpv->stmtList, pStmt);
	JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_RESULT_DATABASE, JSVAL_VOID) );
	JL_SetPrivate(cx, obj, NULL);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/*
JSBool JssqliteStep( JSContext *cx, JSObject *obj, int *status ) {

	sqlite3_stmt *pStmt = (sqlite3_stmt*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pStmt );

	jsval dbVal;
	JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_RESULT_DATABASE, &dbVal) );
	DatabasePrivate *dbpv;
	dbpv = (DatabasePrivate*)JL_GetPrivate(cx, JSVAL_TO_OBJECT(dbVal));
	JL_S_ASSERT_RESOURCE(dbpv);

	sqlite3 *db;
	db = dbpv->db;
	JL_ASSERT( db == sqlite3_db_handle(pStmt) );

	// check if bindings are up to date
	jsval bindingUpToDate;
	JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_RESULT_BINDING_UP_TO_DATE, &bindingUpToDate) );

	if ( bindingUpToDate != JSVAL_TRUE ) {

		jsval queryArgument;
		JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_RESULT_QUERY_ARGUMENT_OBJECT, &queryArgument) );
		JL_CHK( SqliteSetupBindings(cx, pStmt, JSVAL_IS_PRIMITIVE( queryArgument ) ? NULL : JSVAL_TO_OBJECT( queryArgument ), obj) ); // ":" use result object. "@" is the object passed to Query()
		JL_CHK( JS_SetReservedSlot(cx, obj, SLOT_RESULT_BINDING_UP_TO_DATE, JSVAL_TRUE) );
		// doc: The sqlite3_bind_*() routines must be called AFTER sqlite3_prepare() or sqlite3_reset() and BEFORE sqlite3_step().
		//      Bindings are not cleared by the sqlite3_reset() routine. Unbound parameters are interpreted as NULL.
	}

	dbpv->tmpcx = cx;
	*status = sqlite3_step( pStmt ); // The return value will be either SQLITE_BUSY, SQLITE_DONE, SQLITE_ROW, SQLITE_ERROR, or SQLITE_MISUSE.
	dbpv->tmpcx = NULL;
	
	JL_CHK( !JL_IsExceptionPending(cx) );

	switch ( status ) {

		case SQLITE_ROW: // SQLITE_ROW is returned each time a new row of data is ready for processing by the caller
			*rval = JSVAL_TRUE;
			return JS_TRUE;
		case SQLITE_DONE: // means that the statement has finished executing successfully. sqlite3_step() should not be called again on this virtual machine without first calling sqlite3_reset() to reset the virtual machine back to its initial state.
			*rval = JSVAL_FALSE;
			return JS_TRUE;
		case SQLITE_MISUSE: // means that the this routine was called inappropriately. Perhaps it was called on a virtual machine that had already been finalized or on one that had previously returned SQLITE_ERROR or SQLITE_DONE. Or it could be the case that a database connection is being used by a different thread than the one it was created it.
			JL_REPORT_ERROR( "This routine was called inappropriately." );
//		case SQLITE_ERROR:
//		case SQLITE_SCHEMA: // (TBD) check for another error (doc. The database schema changed)
//			JL_CHK( SqliteThrowError(cx, db) );
	}
//	JL_REPORT_ERROR("invalid case (status:%d)", status );
	JL_CHK( SqliteThrowError(cx, db) );
	JL_BAD;

}
*/

JSBool DoStep(JSContext *cx, JSObject *obj, jsval *rval) {


	sqlite3_stmt *pStmt = (sqlite3_stmt*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pStmt );

	jsval dbVal;
	JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_RESULT_DATABASE, &dbVal) );
	DatabasePrivate *dbpv;
	dbpv = (DatabasePrivate*)JL_GetPrivate(cx, JSVAL_TO_OBJECT(dbVal));
	JL_S_ASSERT_RESOURCE(dbpv);

	sqlite3 *db;
	db = dbpv->db;
	JL_ASSERT( db == sqlite3_db_handle(pStmt) );

	// check if bindings are up to date
	jsval bindingUpToDate;
	JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_RESULT_BINDING_UP_TO_DATE, &bindingUpToDate) );

	if ( bindingUpToDate != JSVAL_TRUE ) {

		jsval queryArgument;
		JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_RESULT_QUERY_ARGUMENT_OBJECT, &queryArgument) );
		JL_CHK( SqliteSetupBindings(cx, pStmt, JSVAL_IS_PRIMITIVE( queryArgument ) ? NULL : JSVAL_TO_OBJECT( queryArgument ), obj) ); // ":" use result object. "@" is the object passed to Query()
		JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_RESULT_BINDING_UP_TO_DATE, JSVAL_TRUE) );
		// doc: The sqlite3_bind_*() routines must be called AFTER sqlite3_prepare() or sqlite3_reset() and BEFORE sqlite3_step().
		//      Bindings are not cleared by the sqlite3_reset() routine. Unbound parameters are interpreted as NULL.
	}

	dbpv->tmpcx = cx;
	int status;
	status = sqlite3_step( pStmt ); // The return value will be either SQLITE_BUSY, SQLITE_DONE, SQLITE_ROW, SQLITE_ERROR, or SQLITE_MISUSE.
	dbpv->tmpcx = NULL;
	
	JL_CHK( !JL_IsExceptionPending(cx) );

	switch ( status ) {

		case SQLITE_ROW: // SQLITE_ROW is returned each time a new row of data is ready for processing by the caller
			*rval = JSVAL_TRUE;
			return JS_TRUE;
		case SQLITE_DONE: // means that the statement has finished executing successfully. sqlite3_step() should not be called again on this virtual machine without first calling sqlite3_reset() to reset the virtual machine back to its initial state.
			*rval = JSVAL_FALSE;
			return JS_TRUE;
		case SQLITE_MISUSE:
			// doc. means that the this routine was called inappropriately. Perhaps it was called on a virtual machine that had already been finalized or on one that had previously returned SQLITE_ERROR or SQLITE_DONE.
			//      Or it could be the case that a database connection is being used by a different thread than the one it was created it.
			// doc. If an interface fails with SQLITE_MISUSE, that means the interface was invoked incorrectly by the application. In that case, the error code and message may or may not be set.
			JL_CHK( SqliteThrowErrorStatus(cx, db, status) );
//		case SQLITE_ERROR:
//		case SQLITE_SCHEMA: // (TBD) check for another error (doc. The database schema changed)
//			JL_CHK( SqliteThrowError(cx, db) );
	}
//	JL_REPORT_ERROR("invalid case (status:%d)", status );

	JL_CHK( SqliteThrowError(cx, db) );
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Executes one step in the previously evaluated SQL statement.
  $H return value
   returns true if another row is ready. false if the last line has been reached.
**/
DEFINE_FUNCTION( Step ) {

	JL_DEFINE_FUNCTION_OBJ;
	return DoStep(cx, obj, JL_RVAL);
}


/**doc
$TOC_MEMBER $INAME
 $VAL $INAME( colIndex )
  Returns the current value of the _colIndex_ ^th^ column.
  $H arguments
   $ARG $INT colIndex
**/
DEFINE_FUNCTION( Col ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG_MIN( 1 );

	sqlite3_stmt *pStmt;
	pStmt = (sqlite3_stmt*)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pStmt );
	int col;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &col) );
	JL_CHK( SqliteToJsval(cx, sqlite3_column_value(pStmt, col), JL_RVAL) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $VAL $INAME( [namedRows = false] )
  Executes one step of the the current SQL statement and returns the resulting row of data.
  $H arguments
   $ARG $BOOL namedRows: if true, the function returns an objet containing {columnName:value} pair. else it returns an array of value.
  $H note
   The *Step* function is internally called before each *Row* call.
**/
DEFINE_FUNCTION( Row ) {

	JL_DEFINE_FUNCTION_OBJ;
	sqlite3_stmt *pStmt = (sqlite3_stmt*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pStmt );

	JL_CHK( DoStep(cx, obj, JL_RVAL) ); // if something goes wrong in Result_step ( error report has already been set )
	if ( *JL_RVAL == JSVAL_FALSE ) { // the statement has finished executing successfully

		*JL_RVAL = JSVAL_VOID; // return undefined
		return JS_TRUE;
	}

	// returns an array [ row1Data, row2Data, ... ] else return an object { row1Name:row1Data, row2Name:row2Data,  ... }
	bool namedRows;
	if ( argc >= 1 )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &namedRows) );
	else
		namedRows = false; // default value

	// If the previous call to sqlite3_step() returned SQLITE_DONE or an error code, then sqlite3_data_count() will return 0 
	// whereas sqlite3_column_count() will continue to return the number of columns in the result set.
	int columnCount;
	columnCount = sqlite3_data_count(pStmt); // This routine returns 0 if pStmt is an SQL statement that does not return data (for example an UPDATE).

	JSObject *row;
	row = namedRows ? JS_NewObject(cx, NULL, NULL, NULL) : JS_NewArrayObject(cx, columnCount, NULL); // If length is 0, JS_NewArrayObject creates an array object of length 0 and ignores vector.
	*JL_RVAL = OBJECT_TO_JSVAL(row); // now, row is protectef fom GC ??
	jsval colJsValue;
	for ( int col = 0; col < columnCount; ++col ) {

		//JL_CHK( SqliteColumnToJsval(cx, pStmt, col, &colJsValue ) ); // if something goes wrong in SqliteColumnToJsval, error report has already been set.
		JL_CHK( SqliteToJsval(cx, sqlite3_column_value(pStmt, col), &colJsValue) );

		if ( namedRows )
			JL_CHK( JS_SetProperty(cx, row, sqlite3_column_name( pStmt, col ), &colJsValue) );
		else
			JL_CHK( JS_DefineElement(cx, row, col, colJsValue, NULL, NULL, JSPROP_ENUMERATE) );
	}
	return JS_TRUE;
	JL_BAD;
}



DEFINE_FUNCTION( next ) { // for details, see Row() function thet is the base of this function.

	JL_DEFINE_FUNCTION_OBJ;

	sqlite3_stmt *pStmt = (sqlite3_stmt*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pStmt );
	JL_CHK( DoStep(cx, obj, JL_RVAL) );

	if ( *JL_RVAL == JSVAL_FALSE ) // means SQLITE_DONE
		return JS_ThrowStopIteration(cx);

	JSObject *row;
	row = JS_NewObject(cx, NULL, NULL, NULL);
	*JL_RVAL = OBJECT_TO_JSVAL(row);
	int columnCount;
	columnCount = sqlite3_data_count(pStmt);
	jsval tmp;
	for ( int col = 0; col < columnCount; ++col ) {

		//JL_CHK( SqliteColumnToJsval(cx, pStmt, col, &tmp) );
		JL_CHK( SqliteToJsval(cx, sqlite3_column_value(pStmt, col), &tmp) );
		JL_CHK( JS_SetProperty(cx, row, sqlite3_column_name(pStmt, col), &tmp) );
	}
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Resets the current Result object to its initial state.
**/
DEFINE_FUNCTION( Reset ) {

	JL_DEFINE_FUNCTION_OBJ;

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pStmt );
	if ( sqlite3_reset(pStmt) != SQLITE_OK )
		return SqliteThrowError(cx, sqlite3_db_handle(pStmt));
	*JL_RVAL = JSVAL_VOID;
	return JL_SetReservedSlot(cx, obj, SLOT_RESULT_BINDING_UP_TO_DATE, JSVAL_FALSE); // invalidate current bindings
	JL_BAD;
}


/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  Hold the number of columns of the current [Result]
**/
DEFINE_PROPERTY_GETTER( columnCount ) {

	sqlite3_stmt *pStmt = (sqlite3_stmt*)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pStmt );
	*vp = INT_TO_JSVAL(sqlite3_column_count(pStmt));
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME $READONLY
  Hold an $ARRAY that contain the index:name of the columns.
  $H example
  {{{
  var db = new Database();
  db.Exec('create table t1 (a,b,c);');
  var res = db.Query('SELECT a,c from t1');
  Print( res.columnNames.toSource(), '\n' ); // prints: ["a", "c"]
  }}}
**/
DEFINE_PROPERTY_GETTER( columnNames ) {

//	jsid jid;
//	JL_CHK( JS_ValueToId(cx, id, &jid) );
//	const char * tmp = JL_GetStringBytes( JS_ValueToString(cx, id) );
	
	if ( *vp != JSVAL_VOID )
		return JS_TRUE;

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pStmt );
	JSObject *columnNames;
	columnNames = JS_NewArrayObject(cx, 0, NULL);
	*vp = OBJECT_TO_JSVAL( columnNames );
	int columnCount;
	columnCount = sqlite3_column_count( pStmt ); // sqlite3_column_count AND NOT sqlite3_data_count because this function can be called before sqlite3_step
	jsval colJsValue;
	for ( int col = 0; col < columnCount; ++col ) {

		//see. sqlite3_column_origin_name(pStmt, col);
		colJsValue = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,(const char *)sqlite3_column_name( pStmt, col ))); // sqlite3_column_name can be called BEFORE sqlite3_step
		JL_CHK( JS_DefineElement(cx, columnNames, col, colJsValue, NULL, NULL, JSPROP_ENUMERATE | JSPROP_PERMANENT) );
	}
	
	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME $READONLY
  Hold an $OBJ that contain the name:index of the columns.
  $H example
  {{{
  var db = new Database();
  db.Exec('create table t1 (a,b,c);');
  var res = db.Query('SELECT a,c from t1');
  Print( res.columnIndexes.toSource(), '\n' ); // prints: ({a:0, c:1})
  }}}
**/
DEFINE_PROPERTY_GETTER( columnIndexes ) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pStmt );
	JSObject *columnIndexes;
	columnIndexes = JS_NewObject( cx, NULL, NULL, NULL );
	*vp = OBJECT_TO_JSVAL( columnIndexes );
	int columnCount;
	columnCount = sqlite3_column_count( pStmt );
	jsval colJsValue;
	for ( int col = 0; col < columnCount; ++col ) {

		colJsValue = INT_TO_JSVAL(col);
		JL_CHK( JS_SetProperty( cx, columnIndexes, sqlite3_column_name( pStmt, col ), &colJsValue ) );
	}
	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME $READONLY
  Retrieve a saved copy of the original SQL text used to create result.
**/
DEFINE_PROPERTY_GETTER( sql ) {

//	if ( *vp != JSVAL_VOID )
//		return JS_TRUE;

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( pStmt );
	JL_CHK( JL_NativeToJsval(cx, sqlite3_sql(pStmt), vp) );

	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


DEFINE_DEL_PROPERTY() {

	return JL_SetReservedSlot(cx, obj, SLOT_RESULT_BINDING_UP_TO_DATE, JSVAL_FALSE); // invalidate current bindings
}

DEFINE_SET_PROPERTY() {

	return JL_SetReservedSlot(cx, obj, SLOT_RESULT_BINDING_UP_TO_DATE, JSVAL_FALSE); // invalidate current bindings
}

DEFINE_EQUALITY_OP() {

	*bp = JS_FALSE;
	return JS_TRUE;
}

DEFINE_ITERATOR_OBJECT() {

	JL_S_ASSERT( !keysonly, "Only for each..in loop is supported." );
	return obj;
bad:
	return NULL;
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(3)

	HAS_FINALIZE
	HAS_SET_PROPERTY
	HAS_DEL_PROPERTY
	HAS_ITERATOR_OBJECT
	HAS_EQUALITY_OP

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( columnCount )
		PROPERTY_READ( columnNames )
		PROPERTY_READ( columnIndexes )
		PROPERTY_READ( sql )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION( Step )
		FUNCTION( Reset )
		FUNCTION( Close )
		FUNCTION( Col )
		FUNCTION( Row )
		FUNCTION( next )
	END_FUNCTION_SPEC

END_CLASS
