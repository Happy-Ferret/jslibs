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
#include <cstring>

#include "../jslang/blobapi.h"

#include "error.h"
#include "result.h"
#include "database.h"

#include "../common/stack.h"

// #include <limits.h> // included by ../common/platform.h


JSBool SqliteToJsval( JSContext *cx, sqlite3_value *value, jsval *rval ) {

	int i;
	switch( sqlite3_value_type(value) ) {

		case SQLITE_INTEGER:
			i = sqlite3_value_int(value);
			if ( INT_FITS_IN_JSVAL(i) )
				*rval = INT_TO_JSVAL( i );
			else
				J_CHK( JS_NewNumberValue(cx, i, rval ) );
			break;
		case SQLITE_FLOAT:
			J_CHK( JS_NewNumberValue( cx, sqlite3_value_double(value), rval ) );
			break;
		case SQLITE_BLOB: {
				int length = sqlite3_value_bytes(value);
				void *data = JS_malloc(cx, length);
				J_S_ASSERT_ALLOC(data);
				memcpy(data, sqlite3_value_blob(value), length);
				J_CHK( J_NewBlob(cx, data, length, rval) );
			}
			break;
		case SQLITE_NULL:
			*rval = JSVAL_NULL;
			break;
		case SQLITE_TEXT:
			*rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,(const char *)sqlite3_value_text(value)));
			break;
		default:
			J_REPORT_ERROR( "Unable to convert data.");
	}
	return JS_TRUE;
}



// doc: The sqlite3_bind_*() routines must be called after sqlite3_prepare() or sqlite3_reset() and before sqlite3_step(). Bindings are not cleared by the sqlite3_reset() routine. Unbound parameters are interpreted as NULL.
JSBool SqliteSetupBindings( JSContext *cx, sqlite3_stmt *pStmt, JSObject *objAt, JSObject *objColon ) {
	
	int count = sqlite3_bind_parameter_count(pStmt);
	for ( int param = 1; param <= count; param++ ) {

		const char *name = sqlite3_bind_parameter_name( pStmt, param );
		// doc: Parameters of the form "?" have no name. ... If the value n is out of range or if the n-th parameter is nameless, then NULL is returned.

//		J_S_ASSERT( name != NULL, "Binding is out of range." ); // (TBD) better error message

		jsval val;
		
		if ( name != NULL ) {

			JSObject *obj = NULL;

			if ( objAt != NULL && name[0] == '@' )
				obj = objAt;

			if ( objColon != NULL && name[0] == ':' )
				obj = objColon;

			if ( obj == NULL )
				continue;

			JS_GetProperty(cx, obj, name+1, &val);
		} else { // ? is used in the SQL statement, then use objAt as an array and param as index

			JS_GetElement(cx, objAt, param-1, &val); // works with {0:2,1:2,2:2,length:3} and [2,2,2]
		}

		// sqlite3_bind_value( pStmt, param,
		// (TBD) how to use this
		switch ( JS_TypeOfValue(cx, val) ) {
			case JSTYPE_VOID:
			case JSTYPE_NULL: // http://www.sqlite.org/nulls.html
				sqlite3_bind_null(pStmt, param);
				break;
			case JSTYPE_BOOLEAN:
				sqlite3_bind_int(pStmt, param, JSVAL_TO_BOOLEAN(val) == JS_TRUE ? 1 : 0 );
				break;
			case JSTYPE_NUMBER:
				if ( JSVAL_IS_INT(val) ) {

					sqlite3_bind_int(pStmt, param, JSVAL_TO_INT(val));
				} else {

					jsdouble jd;
					JS_ValueToNumber(cx, val, &jd);
					if ( jd >= INT_MIN && jd <= INT_MAX && jd == (int)jd )
						sqlite3_bind_int( pStmt, param, (int)jd );
					else
						sqlite3_bind_double(pStmt, param, jd);
				}
				break;
			case JSTYPE_OBJECT: // beware: no break; because we use the JSTYPE_STRING's case JS_ValueToString conversion
				if ( JSVAL_IS_NULL(val) ) {

					sqlite3_bind_null(pStmt, param);
					break;
				}

				if ( JS_GET_CLASS(cx, JSVAL_TO_OBJECT(val)) == BlobJSClass(cx) ) { // beware: with SQLite, blob != text

					size_t length;
					const char *data;
					J_CHK( JsvalToStringAndLength(cx, &val, &data, &length) );
					sqlite3_bind_blob(pStmt, param, data, length, SQLITE_STATIC); // beware: assume that the string is not GC while SQLite is using it. else use SQLITE_TRANSIENT
					break;
				}
			case JSTYPE_XML:
			case JSTYPE_FUNCTION: // (TBD) call the function and pass its result to SQLite ?
			case JSTYPE_STRING: {

				//JSString *jsstr = JS_ValueToString(cx, val);
				// (TBD) GC protect (root) jsstr
				const char *str;
				size_t strLen;
				J_CHK( JsvalToStringAndLength(cx, &val, &str, &strLen) );
				sqlite3_bind_text(pStmt, param, str, strLen, SQLITE_STATIC); // beware: assume that the string is not GC while SQLite is using it. else use SQLITE_TRANSIENT
				}
				break;
			default:
				J_REPORT_ERROR("Unsupported data type"); // (TBD) better error message
		}
	}
	return JS_TRUE;
}


JSBool SqliteColumnToJsval( JSContext *cx, sqlite3_stmt *pStmt, int iCol, jsval *rval ) {

	J_CHK( SqliteToJsval(cx, sqlite3_column_value(pStmt, iCol), rval) );
	return JS_TRUE;
}

/**doc
$CLASS_HEADER
 A Result object is used to store a compiled SQL statement ready to be executed.
 = =
 When a statement has been prepared with Database.*Query* function, you need to execute it ( with *Step* function ) before any data can be read.
 However, some properties (like *columnCount*, ... ) can be read before the first *Step* has been done.
 $H note
  You cannot construct this class yourself.
**/
BEGIN_CLASS( Result )


DEFINE_FINALIZE() {

// beware: we cannot finalize the result here because there is no way
//         to get a reference to the database object that hold the statements stack.
//         When this function is call (by the GC), it is possible that the
//         database object (and its statement stack) has already be finalized !

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	if ( pStmt != NULL ) {

		//int status = sqlite3_finalize( pStmt ); // sqlite3_interrupt
		//if ( status != SQLITE_OK ) {
		//	// (TBD) do something ?
		//}
		JS_SetPrivate( cx, obj, NULL ); // (TBD) not needed
//		JS_SetReservedSlot(cx, obj, SLOT_RESULT_DATABASE, JSVAL_VOID); // beware: don't do JS_SetReservedSlot while GC !!
	}
}

/**doc
=== Methods ===
**/

/**doc
 * $VOID $INAME()
  Close the current Result object.
**/
DEFINE_FUNCTION( Close ) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( pStmt );
	JS_SetPrivate( cx, obj, NULL );

	jsval v;
	J_CHK( JS_GetReservedSlot(cx, obj, SLOT_RESULT_DATABASE, &v) );
	J_CHK( JS_GetReservedSlot(cx, JSVAL_TO_OBJECT(v), SLOT_SQLITE_DATABASE_STATEMENT_STACK, &v) );
	void *stack = JSVAL_TO_PRIVATE(v);
	StackReplaceData( &stack, pStmt, NULL );

	int status = sqlite3_finalize( pStmt );
	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, status, sqlite3_errcode(sqlite3_db_handle(pStmt)), sqlite3_errmsg(sqlite3_db_handle(pStmt)) );
	JS_SetReservedSlot(cx, obj, SLOT_RESULT_DATABASE, JSVAL_VOID);
	return JS_TRUE;
}


/**doc
 * $BOOL $INAME()
  Executes one step in the previously evaluated SQL statement.
  $H return value
   returns true if another row is ready. false if the last line has been reached.
**/
DEFINE_FUNCTION( Step ) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( pStmt );

	sqlite3 *db = sqlite3_db_handle(pStmt);
	GetDbContext(db)->cx = cx; // update the JS context used to call functions (see sqlite_function_call)

	// check if bindings are up to date
	jsval bindingUpToDate;
	JS_GetReservedSlot(cx, obj, SLOT_RESULT_BINDING_UP_TO_DATE, &bindingUpToDate);
	if ( bindingUpToDate != JSVAL_TRUE ) {

		jsval queryArgument;
		JS_GetReservedSlot(cx, obj, SLOT_RESULT_QUERY_ARGUMENT_OBJECT, &queryArgument);
		JSObject *argObj = NULL;
		if ( queryArgument != JSVAL_VOID )
			argObj = JSVAL_TO_OBJECT(queryArgument);

		JS_SetReservedSlot(cx, obj, SLOT_RESULT_BINDING_UP_TO_DATE, BOOLEAN_TO_JSVAL(JS_TRUE));
		J_CHK( SqliteSetupBindings(cx, pStmt, argObj, obj ) ); // ":" use result object. "@" is the object passed to Query()
		// doc: The sqlite3_bind_*() routines must be called AFTER sqlite3_prepare() or sqlite3_reset() and BEFORE sqlite3_step().
		//      Bindings are not cleared by the sqlite3_reset() routine. Unbound parameters are interpreted as NULL.
	}

	int status = sqlite3_step( pStmt ); // The return value will be either SQLITE_BUSY, SQLITE_DONE, SQLITE_ROW, SQLITE_ERROR, or SQLITE_MISUSE.

	J_SAFE( GetDbContext(db)->cx = NULL );
	if ( JS_IsExceptionPending(cx) )
		return JS_FALSE;

	switch (status) {
		case SQLITE_ERROR:
		case SQLITE_SCHEMA:
			return SqliteThrowError( cx, status, sqlite3_errcode(sqlite3_db_handle( pStmt )), sqlite3_errmsg(sqlite3_db_handle( pStmt )));
		case SQLITE_MISUSE: // means that the this routine was called inappropriately. Perhaps it was called on a virtual machine that had already been finalized or on one that had previously returned SQLITE_ERROR or SQLITE_DONE. Or it could be the case that a database connection is being used by a different thread than the one it was created it.
			J_REPORT_ERROR( "this routine was called inappropriately" );
		case SQLITE_DONE: // means that the statement has finished executing successfully. sqlite3_step() should not be called again on this virtual machine without first calling sqlite3_reset() to reset the virtual machine back to its initial state.
			*rval = JSVAL_FALSE;
			return JS_TRUE;
		case SQLITE_ROW: // SQLITE_ROW is returned each time a new row of data is ready for processing by the caller
			*rval = JSVAL_TRUE;
			return JS_TRUE;
	}
	J_REPORT_ERROR_1("invalid case (status:%d)", status );
}


/**doc
 * $VAL $INAME( colIndex )
  Returns the current value of the _colIndex_ ^th^ column.
  $H arguments
   $ARG integer colIndex
**/
DEFINE_FUNCTION( Col ) {

	J_S_ASSERT_ARG_MIN( 1 );
	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( pStmt );
	int32 col;
	JS_ValueToInt32( cx, argv[0], &col );
	SqliteColumnToJsval( cx, pStmt, col, rval );
	return JS_TRUE;
}

/**doc
 * $VAL $INAME( [namedRows = false] )
  Executes one step of the the current SQL statement and returns the resulting row of data.
  $H arguments
   $ARG boolean namedRows: if true, the function returns an objet containing {columnName:value} pair. else it returns an array of value.
  $H note
   The *Step* function is internally called before each *Row* call.
**/
DEFINE_FUNCTION( Row ) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( pStmt );

	J_CHK( _Step( cx, obj, 0, NULL, rval ) ); // if something goes wrong in Result_step ( error report has already been set )

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
	jsval colJsValue;
	for ( int col = 0; col < columnCount; ++col ) {

		J_CHK( SqliteColumnToJsval(cx, pStmt, col, &colJsValue ) ); // if something goes wrong in SqliteColumnToJsval, error report has already been set.

		if ( namedRows )
			JS_SetProperty( cx, row, sqlite3_column_name( pStmt, col ), &colJsValue );
		else
			JS_DefineElement( cx, row, col, colJsValue, NULL, NULL, JSPROP_ENUMERATE );
	}
	return JS_TRUE;
}


/**doc
 * $VOID $INAME()
  Resets the current Result object to its initial state.
**/
DEFINE_FUNCTION( Reset ) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( pStmt );
	int status = sqlite3_reset(pStmt);
	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, status, sqlite3_errcode(sqlite3_db_handle(pStmt)), sqlite3_errmsg(sqlite3_db_handle(pStmt)) );
	return JS_TRUE;
}


/**doc
=== Properties ===
**/

/**doc
 * $INT $INAME $READONLY
  Hold the number of columns of the current [Result]
**/
DEFINE_PROPERTY( columnCount ) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( pStmt );
	*vp = INT_TO_JSVAL(sqlite3_column_count(pStmt));
	return JS_TRUE;
}


/**doc
 * $TYPE Array $INAME $READONLY
  Hold an [http://developer.mozilla.org/en/docs/Core_JavaScript_1.5_Reference:Objects:Array Array] that contain the index:name of the columns.
  $H example
  {{{
  var db = new Database();
  db.Exec('create table t1 (a,b,c);');
  var res = db.Query('SELECT a,c from t1');
  Print( res.columnNames.toSource(), '\n' ); // prints: ["a", "c"]
  }}}
**/
DEFINE_PROPERTY( columnNames ) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( pStmt );
	JSObject *columnNames = JS_NewArrayObject(cx, 0, NULL);
	*vp = OBJECT_TO_JSVAL( columnNames );
	int columnCount = sqlite3_column_count( pStmt ); // sqlite3_column_count AND NOT sqlite3_data_count because this function can be called before sqlite3_step
	jsval colJsValue;
	for ( int col = 0; col < columnCount; ++col ) {

		colJsValue = STRING_TO_JSVAL(JS_NewStringCopyZ(cx,(const char *)sqlite3_column_name( pStmt, col ))); // sqlite3_column_name can be called BEFORE sqlite3_step
		JS_DefineElement(cx, columnNames, col, colJsValue, NULL, NULL, JSPROP_ENUMERATE | JSPROP_PERMANENT);
	}
	return JS_TRUE;
}


/**doc
 * $TYPE Object $INAME $READONLY
  Hold an [http://developer.mozilla.org/en/docs/Core_JavaScript_1.5_Reference:Objects:Object Object] that contain the name:index of the columns.
  $H example
  {{{
  var db = new Database();
  db.Exec('create table t1 (a,b,c);');
  var res = db.Query('SELECT a,c from t1');
  Print( res.columnIndexes.toSource(), '\n' ); // prints: ({a:0, c:1})
  }}}
**/
DEFINE_PROPERTY( columnIndexes ) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( pStmt );
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


/**doc
 * *expired* $READONLY $DEPRECATED
  Indicates if the SQL statement must be re-evaluated.
**/
DEFINE_PROPERTY( expired ) {

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( pStmt );
	*vp = sqlite3_expired(pStmt) ? JSVAL_TRUE : JSVAL_FALSE;
  return JS_TRUE;
}


DEFINE_DEL_PROPERTY() {

	JS_SetReservedSlot(cx, obj, SLOT_RESULT_BINDING_UP_TO_DATE, BOOLEAN_TO_JSVAL(JS_FALSE) ); // invalidate current bindings
	return JS_TRUE;
}

DEFINE_SET_PROPERTY() {

	JS_SetReservedSlot(cx, obj, SLOT_RESULT_BINDING_UP_TO_DATE, BOOLEAN_TO_JSVAL(JS_FALSE) ); // invalidate current bindings
	return JS_TRUE;
}

CONFIGURE_CLASS

//	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_SET_PROPERTY
	HAS_DEL_PROPERTY

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( columnCount )
		PROPERTY_READ( columnNames )
		PROPERTY_READ( columnIndexes )
		PROPERTY_READ( expired )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION( Step )
		FUNCTION( Reset )
		FUNCTION( Close )
		FUNCTION( Col )
		FUNCTION( Row )
	END_FUNCTION_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(3)

END_CLASS
