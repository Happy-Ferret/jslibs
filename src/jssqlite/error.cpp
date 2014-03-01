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
#include <jsvalserializer.h>

/**doc fileIndex:bottom
$CLASS_HEADER
$SVN_REVISION $Revision$
 Its aim is to be throw as an exception on any SQLite runtime error.
 $H note
  You cannot construct this class.$LF
**/
BEGIN_CLASS( SqliteError )

/**doc
=== Properties ===
**/


/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( code ) {

	JL_IGNORE( id, cx );

	return JL_GetReservedSlot(  obj, SLOT_SQLITE_ERROR_CODE, vp );
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
**/

ALWAYS_INLINE const char *
SqliteConstString( int errorCode ) {

	switch ( errorCode ) {
		case SQLITE_OK: return "SQLITE_OK";
		case SQLITE_ERROR: return "SQLITE_ERROR";
		case SQLITE_INTERNAL: return "SQLITE_INTERNAL";
		case SQLITE_PERM: return "SQLITE_PERM";
		case SQLITE_ABORT: return "SQLITE_ABORT";
		case SQLITE_BUSY: return "SQLITE_BUSY";
		case SQLITE_LOCKED: return "SQLITE_LOCKED";
		case SQLITE_NOMEM: return "SQLITE_NOMEM";
		case SQLITE_READONLY: return "SQLITE_READONLY";
		case SQLITE_INTERRUPT: return "SQLITE_INTERRUPT";
		case SQLITE_IOERR: return "SQLITE_IOERR";
		case SQLITE_CORRUPT: return "SQLITE_CORRUPT";
		case SQLITE_NOTFOUND: return "SQLITE_NOTFOUND";
		case SQLITE_FULL: return "SQLITE_FULL";
		case SQLITE_CANTOPEN: return "SQLITE_CANTOPEN";
		case SQLITE_PROTOCOL: return "SQLITE_PROTOCOL";
		case SQLITE_EMPTY: return "SQLITE_EMPTY";
		case SQLITE_SCHEMA: return "SQLITE_SCHEMA";
		case SQLITE_TOOBIG: return "SQLITE_TOOBIG";
		case SQLITE_CONSTRAINT: return "SQLITE_CONSTRAINT";
		case SQLITE_MISMATCH: return "SQLITE_MISMATCH";
		case SQLITE_MISUSE: return "SQLITE_MISUSE";
		case SQLITE_NOLFS: return "SQLITE_NOLFS";
		case SQLITE_AUTH: return "SQLITE_AUTH";
		case SQLITE_FORMAT: return "SQLITE_FORMAT";
		case SQLITE_RANGE: return "SQLITE_RANGE";
		case SQLITE_NOTADB: return "SQLITE_NOTADB";
		case SQLITE_ROW: return "SQLITE_ROW";
		case SQLITE_DONE: return "SQLITE_DONE";

		// Extended Result Codes
		case SQLITE_IOERR_READ: return "SQLITE_IOERR_READ";
		case SQLITE_IOERR_SHORT_READ: return "SQLITE_IOERR_SHORT_READ";
		case SQLITE_IOERR_WRITE: return "SQLITE_IOERR_WRITE";
		case SQLITE_IOERR_FSYNC: return "SQLITE_IOERR_FSYNC";
		case SQLITE_IOERR_DIR_FSYNC: return "SQLITE_IOERR_DIR_FSYNC";
		case SQLITE_IOERR_TRUNCATE: return "SQLITE_IOERR_TRUNCATE";
		case SQLITE_IOERR_FSTAT: return "SQLITE_IOERR_FSTAT";
		case SQLITE_IOERR_UNLOCK: return "SQLITE_IOERR_UNLOCK";
		case SQLITE_IOERR_RDLOCK: return "SQLITE_IOERR_RDLOCK";
		case SQLITE_IOERR_DELETE: return "SQLITE_IOERR_DELETE";
		case SQLITE_IOERR_BLOCKED: return "SQLITE_IOERR_BLOCKED";
		case SQLITE_IOERR_NOMEM: return "SQLITE_IOERR_NOMEM";
		case SQLITE_IOERR_ACCESS: return "SQLITE_IOERR_ACCESS";
		case SQLITE_IOERR_CHECKRESERVEDLOCK: return "SQLITE_IOERR_CHECKRESERVEDLOCK";
		case SQLITE_IOERR_LOCK: return "SQLITE_IOERR_LOCK";
		case SQLITE_IOERR_CLOSE: return "SQLITE_IOERR_CLOSE";
		case SQLITE_IOERR_DIR_CLOSE: return "SQLITE_IOERR_DIR_CLOSE";
		case SQLITE_IOERR_SHMOPEN: return "SQLITE_IOERR_SHMOPEN";
		case SQLITE_IOERR_SHMSIZE: return "SQLITE_IOERR_SHMSIZE";
		case SQLITE_IOERR_SHMLOCK: return "SQLITE_IOERR_SHMLOCK";
		case SQLITE_IOERR_SHMMAP: return "SQLITE_IOERR_SHMMAP";
		case SQLITE_IOERR_SEEK: return "SQLITE_IOERR_SEEK";
		case SQLITE_LOCKED_SHAREDCACHE: return "SQLITE_LOCKED_SHAREDCACHE";
		case SQLITE_BUSY_RECOVERY: return "SQLITE_BUSY_RECOVERY";
		case SQLITE_CANTOPEN_NOTEMPDIR: return "SQLITE_CANTOPEN_NOTEMPDIR";
		case SQLITE_CORRUPT_VTAB: return "SQLITE_CORRUPT_VTAB";
		case SQLITE_READONLY_RECOVERY: return "SQLITE_READONLY_RECOVERY";
		case SQLITE_READONLY_CANTLOCK: return "SQLITE_READONLY_CANTLOCK";
		case SQLITE_ABORT_ROLLBACK: return "SQLITE_ABORT_ROLLBACK";
		default:;
	}
	return "UNKNOWN_ERROR";
}

DEFINE_PROPERTY_GETTER( const ) {

	JL_IGNORE( id );

	JL_GetReservedSlot(  obj, SLOT_SQLITE_ERROR_CODE, vp );
	if ( vp.isUndefined() )
		return true;
	int errorCode = vp.toInt32();
	JSString *str = JS_NewStringCopyZ( cx, SqliteConstString(errorCode) );
	vp.setString( str );
	return true;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( text ) {

	JL_IGNORE( id, cx );

	return JL_GetReservedSlot(  obj, SLOT_SQLITE_ERROR_TEXT, vp );
}

DEFINE_FUNCTION( toString ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
	JL_DEFINE_FUNCTION_OBJ;
	
//	JS::RootedObject hobj(cx, JS_OBJ);
//	JS::RootedValue hrval(cx, JL_RVAL);

	return _textGetter(cx, JL_OBJ, JSID_EMPTYHANDLE, JL_RVAL);
	JL_BAD;
}



DEFINE_FUNCTION( _serialize ) {

	JL_DEFINE_ARGS;
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_TYPE( jl::JsvalIsSerializer(cx, JL_ARG(1)), 1, "Serializer" );

	jl::Serializer *ser;
	ser = jl::JsvalToSerializer(cx, JL_ARG(1));

	JL_CHK( JS_GetPropertyById(cx, JL_OBJ, JLID(cx, fileName), JL_RVAL) );
	JL_CHK( ser->Write(cx, JL_RVAL) );
	JL_CHK( JS_GetPropertyById(cx, JL_OBJ, JLID(cx, lineNumber), JL_RVAL) );
	JL_CHK( ser->Write(cx, JL_RVAL) );
	JL_CHK( JL_GetReservedSlot( JL_OBJ, SLOT_SQLITE_ERROR_CODE, JL_RVAL) );
	JL_CHK( ser->Write(cx, JL_RVAL) );
	JL_CHK( JL_GetReservedSlot( JL_OBJ, SLOT_SQLITE_ERROR_TEXT, JL_RVAL) );
	JL_CHK( ser->Write(cx, JL_RVAL) );

	return true;
	JL_BAD;
}


DEFINE_FUNCTION( _unserialize ) {

	JL_DEFINE_ARGS;
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_TYPE( jl::JsvalIsUnserializer(cx, JL_ARG(1)), 1, "Unserializer" );

	jl::Unserializer *unser;
	unser = jl::JsvalToUnserializer(cx, JL_ARG(1));

	JL_CHK( unser->Read(cx, JL_RVAL) );
	JL_CHK( JS_SetPropertyById(cx, JL_OBJ, JLID(cx, fileName), JL_RVAL) );
	JL_CHK( unser->Read(cx, JL_RVAL) );
	JL_CHK( JS_SetPropertyById(cx, JL_OBJ, JLID(cx, lineNumber), JL_RVAL) );
	JL_CHK( unser->Read(cx, JL_RVAL) );
	JL_CHK( JL_SetReservedSlot( JL_OBJ, SLOT_SQLITE_ERROR_CODE, JL_RVAL) );
	JL_CHK( unser->Read(cx, JL_RVAL) );
	JL_CHK( JL_SetReservedSlot( JL_OBJ, SLOT_SQLITE_ERROR_TEXT, JL_RVAL) );

	return true;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))
	HAS_RESERVED_SLOTS(2)

	IS_UNCONSTRUCTIBLE

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( code )
		PROPERTY_GETTER( const )
		PROPERTY_GETTER( text )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION(toString)
		FUNCTION_ARGC(_serialize, 1)
		FUNCTION_ARGC(_unserialize, 1)
	END_FUNCTION_SPEC

END_CLASS


NEVER_INLINE bool FASTCALL
SqliteThrowErrorStatus( JSContext *cx, int status ) {

	JS::RootedObject errorObj(cx, JL_NewObjectWithGivenProto( cx, JL_CLASS(SqliteError), JL_CLASS_PROTOTYPE(cx, SqliteError) )); // (TBD) understand why classSqliteError must have a constructor to be throwed in an exception
	JS::RootedValue errorVal(cx, JS::ObjectValue(*errorObj)); // (TBD) understand why classSqliteError must have a constructor to be throwed in an exception
	
	JS_SetPendingException(cx, errorVal);

	JS::RootedValue tmp(cx, JS::NumberValue(status));
	JL_CHK(JL_SetReservedSlot(errorObj, SLOT_SQLITE_ERROR_CODE, tmp));
	tmp.setString(JS_NewStringCopyZ(cx, "???"));
	JL_CHK(JL_SetReservedSlot(errorObj, SLOT_SQLITE_ERROR_TEXT, tmp));
	
	JL_SAFE( JL_ExceptionSetScriptLocation(cx, &errorObj) );
	return false;
	JL_BAD;
}


NEVER_INLINE bool FASTCALL
SqliteThrowError( JSContext *cx, sqlite3 *db ) {
/*
	JSObject *error = JL_NewObjectWithGivenProto( cx, JL_CLASS(SqliteError), JL_CLASS_PROTOTYPE(cx, SqliteError) ); // (TBD) understand why classSqliteError must have a constructor to be throwed in an exception
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
	JL_CHK( JL_SetReservedSlot(  error, SLOT_SQLITE_ERROR_CODE, INT_TO_JSVAL(sqlite3_extended_errcode(db)) ) );
	JL_CHK( JL_SetReservedSlot(  error, SLOT_SQLITE_ERROR_TEXT, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, sqlite3_errmsg(db))) ) );
	JL_SAFE( JL_ExceptionSetScriptLocation(cx, error) );
*/
	JS::RootedObject errorObj(cx, JL_NewObjectWithGivenProto( cx, JL_CLASS(SqliteError), JL_CLASS_PROTOTYPE(cx, SqliteError) )); // (TBD) understand why classSqliteError must have a constructor to be throwed in an exception
	JS::RootedValue errorVal(cx, JS::ObjectValue(*errorObj)); // (TBD) understand why classSqliteError must have a constructor to be throwed in an exception
	
	JS_SetPendingException(cx, errorVal);

	JS::RootedValue tmp(cx, JS::NumberValue(sqlite3_extended_errcode(db)));
	JL_CHK(JL_SetReservedSlot(errorObj, SLOT_SQLITE_ERROR_CODE, tmp));
	tmp.setString(JS_NewStringCopyZ(cx, sqlite3_errmsg(db)));
	JL_CHK(JL_SetReservedSlot(errorObj, SLOT_SQLITE_ERROR_TEXT, tmp));
	
	JL_SAFE( JL_ExceptionSetScriptLocation(cx, &errorObj) );


	return false;
	JL_BAD;
}

/**doc
=== Exemple ===
{{{
try {

  db.exec('yfiqwygqiwye'); // generate an error

} catch ( ex if ex instanceof SqliteError ) {

   print( 'SqliteError: ' + ex.text + '('+ex.code+')', '\n' );
} catch( ex ) {

   throw ex;
}
}}}
**/




/*

Sure, you can JS_GetProperty(cx, global, "Error", &v) to get the constructor, then JS_GetProperty(cx, JSVAL_TO_OBJECT(v), "prototype", &v), then JL_GetClass(JSVAL_TO_OBJECT(v)).  Error and type checking elided, as usual.

*/
