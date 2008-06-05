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

#include "../jslang/bstringapi.h"

#include "error.h"
#include "result.h"
#include "database.h"

#include "../common/stack.h"

// #include <limits.h> // included by ../common/platform.h
#include <stdlib.h>

// (TBD) add User-defined Collation Sequences ( http://www.sqlite.org/datatype3.html )


BEGIN_CLASS( Database )


DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();

	const char *fileName;
	if ( J_ARG_ISDEF(1) ) {

		RT_JSVAL_TO_STRING( argv[0], fileName );
	} else {

		fileName = ":memory:";
	}

	sqlite3 *db;
	int status = sqlite3_open( fileName, &db ); // see. sqlite3_open_v2()
	if ( status != SQLITE_OK )
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


DEFINE_FUNCTION( Close ) {

	sqlite3 *db = (sqlite3 *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( db );
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
	RT_CHECK_CALL( JS_GetReservedSlot(cx, obj, SLOT_SQLITE_DATABASE_STATEMENT_STACK, &v) );
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


DEFINE_FUNCTION( Query ) {

	RT_ASSERT_ARGC( 1 );

	sqlite3 *db = (sqlite3*)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( db );

	const char *sqlQuery;
	RT_JSVAL_TO_STRING( argv[0], sqlQuery );

	const char *szTail;
	sqlite3_stmt *pStmt;
	int status = sqlite3_prepare( db, sqlQuery, -1, &pStmt, &szTail ); // If the next argument, "nBytes", is less than zero, then zSql is read up to the first nul terminator.
	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, status, sqlite3_errcode(db), sqlite3_errmsg(db) );
	RT_ASSERT_1( *szTail == '\0', "too many SQL statements (%s)", szTail ); // for the moment, do not support multiple statements

	if ( pStmt == NULL ) // if there is an error, *ppStmt may be set to NULL. If the input text contained no SQL (if the input is and empty string or a comment) then *ppStmt is set to NULL.
		REPORT_ERROR( "Invalid SQL string." );

	// remember the statement for later finalization
	jsval v;
	JS_GetReservedSlot(cx, obj, SLOT_SQLITE_DATABASE_STATEMENT_STACK, &v);
	void *stack = JSVAL_TO_PRIVATE(v);
	StackPush( &stack, pStmt );
	JS_SetReservedSlot(cx, obj, SLOT_SQLITE_DATABASE_STATEMENT_STACK, PRIVATE_TO_JSVAL(stack));

	// create the Result (statement) object
	JSObject *dbStatement = JS_NewObject( cx, &classResult, NULL, NULL );
	JS_SetPrivate( cx, dbStatement, pStmt );
	JS_SetReservedSlot(cx, dbStatement, SLOT_RESULT_DATABASE, OBJECT_TO_JSVAL( obj )); // link to avoid GC
	// (TBD) enhance
	*rval = OBJECT_TO_JSVAL( dbStatement );

	if ( argc >= 2 && argv[1] != JSVAL_VOID && JSVAL_IS_OBJECT(argv[1]) )
		JS_SetReservedSlot(cx, dbStatement, SLOT_RESULT_QUERY_ARGUMENT_OBJECT, argv[1]);


	return JS_TRUE;
}



DEFINE_FUNCTION( Exec ) {

	RT_ASSERT_ARGC( 1 );
	sqlite3 *db = (sqlite3*)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( db );

	const char *sqlQuery;
	RT_JSVAL_TO_STRING( argv[0], sqlQuery );

	const char *szTail;
	sqlite3_stmt *pStmt;
	int status;
	status = sqlite3_prepare( db, sqlQuery, -1, &pStmt, &szTail ); // If the next argument, "nBytes", is less than zero, then zSql is read up to the first nul terminator.
	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, status, sqlite3_errcode(db), sqlite3_errmsg(db) );
	RT_ASSERT_1( *szTail == '\0', "Too many SQL statements (%s).", szTail ); // for the moment, do not support multiple statements
	// (TBD) support multiple statements

	if ( argc >= 2 && argv[1] != JSVAL_VOID && JSVAL_IS_OBJECT(argv[1]) )
		RT_CHECK_CALL( SqliteSetupBindings(cx, pStmt, JSVAL_TO_OBJECT(argv[1]) , NULL ) ); // "@" : the the argument passed to Exec(), ":" nothing
	status = sqlite3_step( pStmt ); // The return value will be either SQLITE_BUSY, SQLITE_DONE, SQLITE_ROW, SQLITE_ERROR, or 	SQLITE_MISUSE.

	if ( JS_IsExceptionPending(cx) )
		return JS_FALSE;

	switch (status) {
		case SQLITE_ERROR:
			return SqliteThrowError( cx, status, sqlite3_errcode(sqlite3_db_handle( pStmt )), sqlite3_errmsg(sqlite3_db_handle( pStmt )));
		case SQLITE_MISUSE: // means that the this routine was called inappropriately. Perhaps it was called on a virtual machine that had already been finalized or on one that had previously returned SQLITE_ERROR or SQLITE_DONE. Or it could be the case that a database connection is being used by a different thread than the one it was created it.
			REPORT_ERROR( "This routine was called inappropriately." );
		case SQLITE_DONE: // means that the statement has finished executing successfully. sqlite3_step() should not be called again on this virtual machine without first calling sqlite3_reset() to reset the virtual machine back to its initial state.
			//			REPORT_ERROR( "No result found." );
			*rval = JSVAL_VOID;
			break;
		case SQLITE_ROW:
			RT_CHECK_CALL( SqliteColumnToJsval(cx, pStmt, 0, rval) );
			break;
	}

	status = sqlite3_finalize( pStmt );
	if ( status != SQLITE_OK )
		return SqliteThrowError( cx, status, sqlite3_errcode(sqlite3_db_handle(pStmt)), sqlite3_errmsg(sqlite3_db_handle(pStmt)) );

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
			if ( JS_GET_CLASS(cx, JSVAL_TO_OBJECT(rval)) == BStringJSClass(cx) ) { // beware: with SQLite, blob != text

				JSObject *bstringObject = JSVAL_TO_OBJECT(rval);
				void *data = BStringData(cx, bstringObject);
				//J_S_ASSERT( data != NULL, "Invalid BString object.")
				int length = BStringLength(cx, bstringObject);
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


DEFINE_SET_PROPERTY() {

	if ( JSVAL_IS_OBJECT(*vp) && *vp != JSVAL_NULL && JS_ObjectIsFunction(cx, JSVAL_TO_OBJECT(*vp) ) ) {

		sqlite3 *db = (sqlite3 *)JS_GetPrivate( cx, obj );
		RT_ASSERT_RESOURCE( db );

		const char *fName;
		RT_JSVAL_TO_STRING( id, fName );

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

		int status = sqlite3_create_function(db, fName, -1, SQLITE_ANY /*SQLITE_UTF8*/, data, sqlite_function_call, NULL, NULL);
		if ( status != SQLITE_OK )
			return SqliteThrowError( cx, status, sqlite3_errcode(db), sqlite3_errmsg(db) );
	}
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

	HAS_SET_PROPERTY
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(2)

END_CLASS
