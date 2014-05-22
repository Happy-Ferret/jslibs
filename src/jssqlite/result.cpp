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


bool SqliteToJsval( JSContext *cx, sqlite3_value *value, OUT JS::MutableHandleValue rval ) {

	switch( sqlite3_value_type(value) ) {

		case SQLITE_INTEGER:
			JL_CHK( jl::setValue(cx, rval, sqlite3_value_int(value)) );
			break;
		case SQLITE_FLOAT:
			JL_CHK( jl::setValue(cx, rval, sqlite3_value_double(value)) );
			break;
		case SQLITE_BLOB:
			//JL_CHK( JL_NewBufferCopyN(cx, sqlite3_value_blob(value), sqlite3_value_bytes(value), rval) );
			JL_CHK( BlobCreateCopy(cx, sqlite3_value_blob(value), sqlite3_value_bytes(value), rval) );
			break;
		case SQLITE_NULL:
			rval.setNull();
			break;
		case SQLITE_TEXT:
			rval.setString(JS_NewStringCopyN(cx, (const char *)sqlite3_value_text(value), sqlite3_value_bytes(value)));
			//*rval = STRING_TO_JSVAL(JS_NewUCStringCopyZ(cx, (const jschar*)sqlite3_value_text16(value)));
			break;
		default:
			JL_ERR( E_DATATYPE, E_NOTSUPPORTED );
	}
	return true;
	JL_BAD;
}



// doc: The sqlite3_bind_*() routines must be called after sqlite3_prepare() or sqlite3_reset() and before sqlite3_step().
//      Bindings are not cleared by the sqlite3_reset() routine. Unbound parameters are interpreted as NULL.
bool SqliteSetupBindings( JSContext *cx, sqlite3_stmt *pStmt, JS::HandleValue argVal, JS::HandleObject curObj ) {

	JS::RootedValue val(cx);
	JS::RootedObject argObj(cx, argVal.toObjectOrNull());
	int anonParamIndex = 0;
	const char *name;

	int count = sqlite3_bind_parameter_count(pStmt);
	for ( int param = 1; param <= count; param++ ) { // The first host parameter has an index of 1, not 0.

		// doc: Parameters of the form "?" have no name. ... If the value n is out of range or if the n-th parameter is nameless, then NULL is returned.
		name = sqlite3_bind_parameter_name(pStmt, param);

		if ( name == NULL || name[0] == '?' ) {

			if ( argObj != NULL ) {

				JL_CHK( JL_GetElement(cx, argObj, anonParamIndex, &val) ); // works with {0:2,1:2,2:2,length:3} and [2,2,2]
				anonParamIndex++;
				goto next;
			}

			val = JSVAL_VOID;
			JL_ERR( E_PARAM, E_NUM(anonParamIndex), E_DEFINED );
			//JL_REPORT_ERROR_NUM( JLSMSG_LOGIC_ERROR, "invalid anonymous SQL parameter"); //JL_REPORT_ERROR("Unavailable %d-th anonymous SQL parameter.", anonParamIndex);
			goto next;
		}

		if ( name[0] == '$' ) {

//			JL_CHK( JL_GetVariableValue(cx, name+1, &val) );
//			goto next;
			//JL_REPORT_ERROR_NUM( JLSMSG_LOGIC_ERROR, "Unsupported SQL parameter prefix $"); //JL_REPORT_ERROR("Unsupported SQL parameter prefix (%s).", name);
			JL_ERR( E_PARAM, E_STR("$"), E_NOTSUPPORTED );
		}

		if ( name[0] == '@' ) {

			if ( argObj != NULL )  {

				JL_CHK( JS_GetProperty(cx, argObj, name+1, &val) );
			} else {
			
				val = JSVAL_VOID;
				//JL_REPORT_ERROR_NUM( JLSMSG_LOGIC_ERROR, "Undefined SQL parameter"); //JL_REPORT_ERROR("Undefined %s SQL parameter.", name);
				JL_ERR( E_PARAM, E_NAME(name), E_DEFINED );
			}
			goto next;
		}
		if ( name[0] == ':' ) {

			if ( curObj != NULL ) {

				JL_CHK( JS_GetProperty(cx, curObj, name+1, &val) );
			} else {
			
				val = JSVAL_VOID;
				//JL_REPORT_ERROR_NUM( JLSMSG_LOGIC_ERROR, "Undefined SQL parameter"); //JL_REPORT_ERROR("Undefined %s SQL parameter.", name);
				JL_ERR( E_PARAM, E_NAME(name), E_DEFINED );
			}
			goto next;
		}

		//JL_REPORT_ERROR_NUM( JLSMSG_LOGIC_ERROR, "SQL Parameter not supported"); // JL_REPORT_ERROR("Unsupported SQL Parameter");
		JL_ERR( E_PARAM, E_NOTSUPPORTED );

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
				if ( sqlite3_bind_int(pStmt, param, val.toBoolean() ? 1 : 0 ) != SQLITE_OK )
					return SqliteThrowError(cx, sqlite3_db_handle(pStmt));
				break;
			case JSTYPE_NUMBER:
				if ( JSVAL_IS_INT(val) ) {

					ret = sqlite3_bind_int(pStmt, param, val.toInt32());
				} else {

					double jd;
					JL_CHK( jl::getValue(cx, val, &jd) );
					if ( jd >= INT_MIN && jd <= INT_MAX && jd == (int)jd )
						ret = sqlite3_bind_int( pStmt, param, (int)jd );
					else
						ret = sqlite3_bind_double(pStmt, param, jd);
				}
				if ( ret != SQLITE_OK )
					return SqliteThrowError(cx, sqlite3_db_handle(pStmt));
				break;
			case JSTYPE_OBJECT: // beware: no break; because we use the JSTYPE_STRING's case JS::ToString conversion
				if ( val.isNull() ) {

					if ( sqlite3_bind_null(pStmt, param) != SQLITE_OK )
						return SqliteThrowError(cx, sqlite3_db_handle(pStmt));
					break;
				}
				if ( jl::isData(cx, val) ) {

					jl::BufString data;
					JL_CHK( jl::getValue(cx, val, &data) );
					if ( sqlite3_bind_blob(pStmt, param, data.toData<const uint8_t*>(), data.length(), SQLITE_STATIC) != SQLITE_OK ) // beware: assume that the string is not GC while SQLite is using it. else use SQLITE_TRANSIENT
						return SqliteThrowError(cx, sqlite3_db_handle(pStmt));
					break;
				}
			// case JSTYPE_XML:  has gone
			case JSTYPE_FUNCTION: // (TBD) call the function and pass its result to SQLite ?
			case JSTYPE_STRING: {

				jl::BufString str;
				JL_CHK( jl::getValue(cx, val, &str) );

				if ( sqlite3_bind_text(pStmt, param, str.toData<const char*>(), str.length(), SQLITE_STATIC) != SQLITE_OK ) // beware: assume that the string is not GC while SQLite is using it. else use SQLITE_TRANSIENT
					return SqliteThrowError(cx, sqlite3_db_handle(pStmt));
				}
				break;
			default:
				//JL_REPORT_ERROR_NUM( JLSMSG_TYPE_ERROR, "unsupported SQL parameter data type"); // (TBD) better error message
				JL_ERR( E_PARAMTYPE, E_NOTSUPPORTED );
		}
	}
	return true;
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
  db.exec('create table t1 (name,value);');
  db.exec('insert into t1 (name,value) values ("red","#F00")');
  db.exec('insert into t1 (name,value) values ("green","#0F0")');
  db.exec('insert into t1 (name,value) values ("blue","#00F")');
  
  for each ( row in db.query('SELECT * from t1') )
   print( row.name + ' = ' + row.value, '\n' );
  }}}
  prints:
  {{{
  red = #F00
  green = #0F0
  blue = #00F
  }}}

 $H example 2
  {{{
  print( [ color.name for each ( color in db.query('SELECT * from t1') ) ] ); // prints: red,green,blue
  }}}

 $H note
  You cannot construct this class.
**/
BEGIN_CLASS( Result )


DEFINE_FINALIZE() {

	if ( jl::Host::getHost(fop->runtime()).hostRuntime().skipCleanup() )
		return;

	sqlite3_stmt *pStmt = (sqlite3_stmt*)js::GetObjectPrivate(obj);
	if ( pStmt != NULL ) {

/* unable to do this because the database may have already been finalized.
		jsval dbVal;
		JL_GetReservedSlot(cx, obj, SLOT_RESULT_DATABASE, &dbVal);
		if ( !JSVAL_IS_VOID(dbVal) ) {

			DatabasePrivate *dbpv = (DatabasePrivate*)JL_GetPrivate(JSVAL_TO_OBJECT(dbVal));
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
DEFINE_FUNCTION( close ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;

		JL_ASSERT_THIS_INSTANCE();

	sqlite3_stmt *pStmt = (sqlite3_stmt*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pStmt );

	DatabasePrivate *dbpv;
	{
	JS::RootedValue v(cx);
	JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_RESULT_DATABASE, &v) );
	JL_ASSERT( v.isObject() );
	
	JS::RootedObject vobj(cx, &v.toObject());
	dbpv = (DatabasePrivate*)JL_GetPrivate(vobj);
	JL_ASSERT_OBJECT_STATE(dbpv, JL_GetClassName(vobj) );
	}

	if ( sqlite3_finalize(pStmt) != SQLITE_OK )
		JL_CHK( SqliteThrowError(cx, dbpv->db) );

	jl::StackRemove(&dbpv->stmtList, pStmt);
	JL_CHK( JL_SetReservedSlot(JL_OBJ, SLOT_RESULT_DATABASE, JL_UNDEFINED()) );
	JL_SetPrivate(JL_OBJ, NULL);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/*
bool JssqliteStep( JSContext *cx, JSObject *obj, int *status ) {

	sqlite3_stmt *pStmt = (sqlite3_stmt*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pStmt );

	jsval dbVal;
	JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_RESULT_DATABASE, &dbVal) );
	DatabasePrivate *dbpv;
	dbpv = (DatabasePrivate*)JL_GetPrivate(JSVAL_TO_OBJECT(dbVal));
	JL_ASSERT_THIS_OBJECT_STATE(dbpv);

	sqlite3 *db;
	db = dbpv->db;
	ASSERT( db == sqlite3_db_handle(pStmt) );

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
	*status = sqlite3_step( pStmt ); // The return value will be either SQLITE_BUSY, SQLITE_DONE, SQLITE_ROW, SQLITE_ERROR, or SQLITE_MISUSE.
	dbpv->tmpcx = NULL;
	
	JL_CHK( !JL_IsExceptionPending(cx) );

	switch ( status ) {

		case SQLITE_ROW: // SQLITE_ROW is returned each time a new row of data is ready for processing by the caller
			*rval = JSVAL_TRUE;
			return true;
		case SQLITE_DONE: // means that the statement has finished executing successfully. sqlite3_step() should not be called again on this virtual machine without first calling sqlite3_reset() to reset the virtual machine back to its initial state.
			*rval = JSVAL_FALSE;
			return true;
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

bool DoStep(JSContext *cx, JS::HandleObject obj, JS::MutableHandleValue rval) {

	sqlite3_stmt *pStmt = (sqlite3_stmt*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pStmt );

	DatabasePrivate *dbpv;
	{

	JS::RootedValue dbVal(cx);
	JL_CHK( JL_GetReservedSlot(obj, SLOT_RESULT_DATABASE, &dbVal) );
	JL_ASSERT( dbVal.isObject() );


	JS::RootedObject dbValObj(cx, &dbVal.toObject());

	dbpv = (DatabasePrivate*)JL_GetPrivate(dbValObj);
	JL_ASSERT_OBJECT_STATE(dbpv, JL_GetClassName(dbValObj));

	}

	sqlite3 *db;
	db = dbpv->db;
	ASSERT( db == sqlite3_db_handle(pStmt) );

	{

	// check if bindings are up to date
	JS::RootedValue bindingUpToDate(cx);
	JL_CHK( JL_GetReservedSlot(obj, SLOT_RESULT_BINDING_UP_TO_DATE, &bindingUpToDate) );

	if ( bindingUpToDate != JSVAL_TRUE ) {

		JS::RootedValue queryArgument(cx);
		JL_CHK( JL_GetReservedSlot(obj, SLOT_RESULT_QUERY_ARGUMENT_OBJECT, &queryArgument) );

		JS::RootedObject queryArgumentObj(cx);


		JL_CHK( SqliteSetupBindings(cx, pStmt, queryArgument.isObject() ? queryArgument : JS::NullHandleValue, obj) ); // ":" use result object. "@" is the object passed to Query()
		JL_CHK( JL_SetReservedSlot(obj, SLOT_RESULT_BINDING_UP_TO_DATE, JL_TRUE()) );
		// doc: The sqlite3_bind_*() routines must be called AFTER sqlite3_prepare() or sqlite3_reset() and BEFORE sqlite3_step().
		//      Bindings are not cleared by the sqlite3_reset() routine. Unbound parameters are interpreted as NULL.
	}

	}

	dbpv->tmpcx = cx;
	int status;
	status = sqlite3_step( pStmt ); // The return value will be either SQLITE_BUSY, SQLITE_DONE, SQLITE_ROW, SQLITE_ERROR, or SQLITE_MISUSE.
	dbpv->tmpcx = NULL;
	
	JL_CHK( !JL_IsExceptionPending(cx) );

	switch ( status ) {

		case SQLITE_ROW: // SQLITE_ROW is returned each time a new row of data is ready for processing by the caller
			rval.setBoolean(true);
			return true;
		case SQLITE_DONE: // means that the statement has finished executing successfully. sqlite3_step() should not be called again on this virtual machine without first calling sqlite3_reset() to reset the virtual machine back to its initial state.
			rval.setBoolean(false);
			return true;
		case SQLITE_MISUSE:
			// doc. means that the this routine was called inappropriately. Perhaps it was called on a virtual machine that had already been finalized or on one that had previously returned SQLITE_ERROR or SQLITE_DONE.
			//      Or it could be the case that a database connection is being used by a different thread than the one it was created it.
			// doc. If an interface fails with SQLITE_MISUSE, that means the interface was invoked incorrectly by the application. In that case, the error code and message may or may not be set.
			JL_CHK( SqliteThrowError(cx, db) );
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
DEFINE_FUNCTION( step ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	return DoStep(cx, JL_OBJ, JL_RVAL);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VAL $INAME( colIndex )
  Returns the current value of the _colIndex_ ^th^ column.
  $H arguments
   $ARG $INT colIndex
**/
DEFINE_FUNCTION( col ) {

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN(1);

	sqlite3_stmt *pStmt;
	pStmt = (sqlite3_stmt*)JL_GetPrivate( JL_OBJ );
	JL_ASSERT_THIS_OBJECT_STATE( pStmt );
	int col;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &col) );
	JL_CHK( SqliteToJsval(cx, sqlite3_column_value(pStmt, col), JL_RVAL) );
	return true;
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
DEFINE_FUNCTION( row ) {

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	sqlite3_stmt *pStmt = (sqlite3_stmt*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pStmt );

	JL_CHK( DoStep(cx, JL_OBJ, JL_RVAL) ); // if something goes wrong in Result_step ( error report has already been set )
	if ( JL_RVAL.isFalse() ) { // the statement has finished executing successfully

		JL_RVAL.setUndefined(); // return undefined
		return true;
	}

	// returns an array [ row1Data, row2Data, ... ] else return an object { row1Name:row1Data, row2Name:row2Data,  ... }
	bool namedRows;
	if ( argc >= 1 )
		JL_CHK( jl::getValue(cx, JL_ARG(1), &namedRows) );
	else
		namedRows = false; // default value

	// If the previous call to sqlite3_step() returned SQLITE_DONE or an error code, then sqlite3_data_count() will return 0 
	// whereas sqlite3_column_count() will continue to return the number of columns in the result set.
	int columnCount;
	columnCount = sqlite3_data_count(pStmt); // This routine returns 0 if pStmt is an SQL statement that does not return data (for example an UPDATE).

	{
	JS::RootedObject row(cx, namedRows ? JL_NewObj(cx) : JS_NewArrayObject(cx, columnCount)); // If length is 0, JS_NewArrayObject creates an array object of length 0 and ignores vector.
	JL_RVAL.setObject(*row); // now, row is protectef fom GC ??

	JS::RootedValue colJsValue(cx);
	for ( int col = 0; col < columnCount; ++col ) {

		//JL_CHK( SqliteColumnToJsval(cx, pStmt, col, &colJsValue ) ); // if something goes wrong in SqliteColumnToJsval, error report has already been set.
		JL_CHK( SqliteToJsval(cx, sqlite3_column_value(pStmt, col), &colJsValue) );

		if ( namedRows ) {

			JL_CHK( jl::setProperty(cx, row, sqlite3_column_name(pStmt, col), colJsValue) );
		} else {

			//JL_CHK( JS_DefineElement(cx, row, col, colJsValue, NULL, NULL, JSPROP_ENUMERATE) );
			JL_CHK( jl::setElement(cx, row, col, colJsValue) );
		}
	}

	}

	return true;
	JL_BAD;
}



DEFINE_FUNCTION( next ) { // for details, see Row() function thet is the base of this function.

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	sqlite3_stmt *pStmt = (sqlite3_stmt*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pStmt );
	JL_CHK( DoStep(cx, JL_OBJ, JL_RVAL) );

	{

	if ( JL_RVAL.isFalse() ) // means SQLITE_DONE
		return JS_ThrowStopIteration(cx);

	JS::RootedObject row(cx, JL_NewObj(cx));
	JL_RVAL.setObject(*row);
	int columnCount;
	columnCount = sqlite3_data_count(pStmt);
	
	JS::RootedValue tmp(cx);
	for ( int col = 0; col < columnCount; ++col ) {

		//JL_CHK( SqliteColumnToJsval(cx, pStmt, col, &tmp) );
		JL_CHK( SqliteToJsval(cx, sqlite3_column_value(pStmt, col), &tmp) );
		JL_CHK( JS_SetProperty(cx, row, sqlite3_column_name(pStmt, col), tmp) );
	}

	}

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Resets the current Result object to its initial state.
**/
DEFINE_FUNCTION( reset ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JL_GetPrivate( JL_OBJ );
	JL_ASSERT_THIS_OBJECT_STATE( pStmt );
	if ( sqlite3_reset(pStmt) != SQLITE_OK )
		return SqliteThrowError(cx, sqlite3_db_handle(pStmt));
	JL_RVAL.setUndefined();
	return JL_SetReservedSlot(JL_OBJ, SLOT_RESULT_BINDING_UP_TO_DATE, JL_FALSE()); // invalidate current bindings
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

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();

	sqlite3_stmt *pStmt = (sqlite3_stmt*)JL_GetPrivate( JL_OBJ );
	JL_ASSERT_THIS_OBJECT_STATE( pStmt );
	vp.setInt32(sqlite3_column_count(pStmt));
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME $READONLY
  Hold an $ARRAY that contain the index:name of the columns.
  $H example
  {{{
  var db = new Database();
  db.exec('create table t1 (a,b,c);');
  var res = db.query('SELECT a,c from t1');
  print( res.columnNames.toSource(), '\n' ); // prints: ["a", "c"]
  }}}
**/
DEFINE_PROPERTY_GETTER( columnNames ) {

//	jsid jid;
//	JL_CHK( JS_ValueToId(cx, id, &jid) );
//	const char * tmp = JL_GetStringBytes( JS::ToString(cx, id) );
	
	if ( !vp.isUndefined() )
		return true;

	JL_DEFINE_PROP_ARGS;
	JL_ASSERT_THIS_INSTANCE();

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JL_GetPrivate( JL_OBJ );
	JL_ASSERT_THIS_OBJECT_STATE( pStmt );
	
	{

	JS::RootedObject columnNames(cx, JS_NewArrayObject(cx, 0));
	vp.setObject( *columnNames );
	int columnCount;
	columnCount = sqlite3_column_count( pStmt ); // sqlite3_column_count AND NOT sqlite3_data_count because this function can be called before sqlite3_step
	
	JS::RootedValue colJsValue(cx);
	for ( int col = 0; col < columnCount; ++col ) {

		//see. sqlite3_column_origin_name(pStmt, col);
		colJsValue.setString( JS_NewStringCopyZ(cx,(const char *)sqlite3_column_name( pStmt, col )) ); // sqlite3_column_name can be called BEFORE sqlite3_step
		//JL_CHK( JS_DefineElement(cx, columnNames, col, colJsValue, NULL, NULL, JSPROP_ENUMERATE | JSPROP_PERMANENT) );
		JL_CHK( JL_SetElement(cx, columnNames, col, colJsValue) );
	}
	
	return jl::StoreProperty(cx, obj, id, vp, false);

	}

	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME $READONLY
  Hold an $OBJ that contain the name:index of the columns.
  $H example
  {{{
  var db = new Database();
  db.exec('create table t1 (a,b,c);');
  var res = db.query('SELECT a,c from t1');
  print( res.columnIndexes.toSource(), '\n' ); // prints: ({a:0, c:1})
  }}}
**/
DEFINE_PROPERTY_GETTER( columnIndexes ) {

	JL_DEFINE_PROP_ARGS;
	JL_ASSERT_THIS_INSTANCE();

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( pStmt );
	
	{

	JS::RootedObject columnIndexes(cx, JL_NewObj(cx));
	vp.setObject(*columnIndexes);
	int columnCount;
	columnCount = sqlite3_column_count( pStmt );
	
	JS::RootedValue colJsValue(cx);
	for ( int col = 0; col < columnCount; ++col ) {

		colJsValue.setInt32(col);
		JL_CHK( JS_SetProperty( cx, columnIndexes, sqlite3_column_name( pStmt, col ), colJsValue ) );
	}

	}

	return jl::StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME $READONLY
  Retrieve a saved copy of the original SQL text used to create result.
**/
DEFINE_PROPERTY_GETTER( sql ) {

//	if ( *vp != JSVAL_VOID )
//		return true;

	JL_DEFINE_PROP_ARGS;
	JL_ASSERT_THIS_INSTANCE();

	sqlite3_stmt *pStmt = (sqlite3_stmt *)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( pStmt );
	JL_CHK( jl::setValue(cx, vp, sqlite3_sql(pStmt)) );

	return jl::StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


DEFINE_DEL_PROPERTY() {

	JL_IGNORE( id, cx );
	
	*succeeded = true;
	return JL_SetReservedSlot(obj, SLOT_RESULT_BINDING_UP_TO_DATE, JL_FALSE()); // invalidate current bindings
}

DEFINE_SET_PROPERTY() {

	JL_IGNORE( id, vp, strict, cx );

	return JL_SetReservedSlot(obj, SLOT_RESULT_BINDING_UP_TO_DATE, JL_FALSE()); // invalidate current bindings
}


/**qa
	loadModule('jssqlite');
	var db = new Database();
	var res = db.exec('create table a (id integer primary key, x varchar)');
	db.exec('insert into a values (NULL, "aaa")');
	var res = db.query('select * from a');
	QA.ASSERTOP( res, '!=', res );
**/

/* ext.equality hook has gone
DEFINE_EQUALITY_OP() {

	JL_IGNORE( v, obj, cx );

	*bp = false;
	return true;
}
*/


DEFINE_ITERATOR_OBJECT() {

	JL_CHKM( !keysonly, E_NAME("for...in"), E_NOTSUPPORTED );
	return obj;
bad:
	return NULL;
}


DEFINE_FUNCTION( stdIteratorNext ) {

	JL_DEFINE_ARGS;
	bool done;

	JS::RootedValue result(cx);
	JL_CHK( JS_GetPropertyById(cx, JL_OBJ, JLID(cx, source), &result) );

	{

	JS::RootedObject item(cx, JL_NewObj(cx));
	JS::RootedObject resultObj(cx, &result.toObject());
	JS::RootedValue row(cx);
	if ( !jl::call(cx, resultObj, JLID(cx, next), &row) ) {

		JS::RootedValue ex(cx);
		JL_CHK( JS_GetPendingException(cx, &ex) );
		if ( JS_IsStopIteration(ex) ) { 
			
			JS_ClearPendingException(cx);
			done = true;
		} else {

			goto bad;
		}
	} else {
		
		done = false;
	}

	ASSERT( row.isUndefined() == done );

	JL_CHK( JS_DefinePropertyById(cx, item, JLID(cx, done), JS::BooleanValue(done), NULL, NULL, 0) );
	JL_CHK( JS_DefinePropertyById(cx, item, JLID(cx, value), row, NULL, NULL, 0) );
	JL_RVAL.setObject(*item);

	}

	//>>> []["@@iterator"]().next()
	//Object { done=true, value=undefined}
	//>>> [1]["@@iterator"]().next()
	//Object { value=1, done=false}

	return true;
	JL_BAD;
}


DEFINE_STD_ITERATOR() {

	JL_DEFINE_ARGS;

	JS::RootedObject itObj(cx, JL_NewObj(cx));
	JL_CHK( JS_DefineFunctionById(cx, itObj, JLID(cx, next), _stdIteratorNext, 0, 0) );
	JL_CHK( JS_DefinePropertyById(cx, itObj, JLID(cx, source), JL_OBJVAL, NULL, NULL, 0) );
	JL_RVAL.setObject(*itObj);

	return true;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(3)

	HAS_FINALIZE
	HAS_SET_PROPERTY
	HAS_DEL_PROPERTY
	HAS_ITERATOR_OBJECT
	HAS_STD_ITERATOR
	
	//HAS_EQUALITY_OP  ext.equality hook has gone

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( columnCount )
		PROPERTY_GETTER( columnNames )
		PROPERTY_GETTER( columnIndexes )
		PROPERTY_GETTER( sql )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION( step )
		FUNCTION( reset )
		FUNCTION( close )
		FUNCTION( col )
		FUNCTION( row )
		FUNCTION( next )
	END_FUNCTION_SPEC

END_CLASS
