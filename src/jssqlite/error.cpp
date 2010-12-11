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
#include "../common/jsvalserializer.h"

#include "error.h"

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


/* see issue#52
DEFINE_CONSTRUCTOR() {

	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_REPORT_ERROR( "This object cannot be construct." ); // (TBD) remove constructor and define HAS_HAS_INSTANCE
	return JS_TRUE;
}
*/

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
**/
DEFINE_PROPERTY( code ) {

	return JL_GetReservedSlot( cx, obj, SLOT_SQLITE_ERROR_CODE, vp );
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
**/


const char *SqliteConstString( int errorCode ) {

	switch (errorCode) {
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
	}
	return "UNKNOWN_ERROR";
}

DEFINE_PROPERTY( const ) {

	JL_GetReservedSlot( cx, obj, SLOT_SQLITE_ERROR_CODE, vp );
	if ( JSVAL_IS_VOID(*vp) )
		return JS_TRUE;
	int errorCode = JSVAL_TO_INT(*vp);
	JSString *str = JS_NewStringCopyZ( cx, SqliteConstString(errorCode) );
	*vp = STRING_TO_JSVAL( str );
	return JS_TRUE;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
**/
DEFINE_PROPERTY( text ) {

	return JL_GetReservedSlot( cx, obj, SLOT_SQLITE_ERROR_TEXT, vp );
}

DEFINE_FUNCTION( toString ) {

	JL_DEFINE_FUNCTION_OBJ;
	return _text(cx, obj, JSID_EMPTY, JL_RVAL);
}

DEFINE_HAS_INSTANCE() { // see issue#52

	*bp = !JSVAL_IS_PRIMITIVE(*v) && JL_InheritFrom(cx, JSVAL_TO_OBJECT(*v), JL_THIS_CLASS);
	return JS_TRUE;
}


DEFINE_FUNCTION( _serialize ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT( jl::JsvalIsSerializer(cx, JL_ARG(1)), "Invalid serializer object." );
	jl::Serializer *ser;
	ser = jl::JsvalToSerializer(cx, JL_ARG(1));

	JL_CHK( JS_GetPropertyById(cx, JL_OBJ, JL_ATOMJSID(cx, fileName), JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );
	JL_CHK( JS_GetPropertyById(cx, JL_OBJ, JL_ATOMJSID(cx, lineNumber), JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );
	JL_CHK( JL_GetReservedSlot(cx, JL_OBJ, SLOT_SQLITE_ERROR_CODE, JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );
	JL_CHK( JL_GetReservedSlot(cx, JL_OBJ, SLOT_SQLITE_ERROR_TEXT, JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( _unserialize ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT( jl::JsvalIsUnserializer(cx, JL_ARG(1)), "Invalid unserializer object." );
	jl::Unserializer *unser;
	unser = jl::JsvalToUnserializer(cx, JL_ARG(1));

	JL_CHK( unser->Read(cx, *JL_RVAL) );
	JL_CHK( JS_SetPropertyById(cx, obj, JL_ATOMJSID(cx, fileName), JL_RVAL) );
	JL_CHK( unser->Read(cx, *JL_RVAL) );
	JL_CHK( JS_SetPropertyById(cx, obj, JL_ATOMJSID(cx, lineNumber), JL_RVAL) );
	JL_CHK( unser->Read(cx, *JL_RVAL) );
	JL_CHK( JL_SetReservedSlot(cx, JL_OBJ, SLOT_SQLITE_ERROR_CODE, *JL_RVAL) );
	JL_CHK( unser->Read(cx, *JL_RVAL) );
	JL_CHK( JL_SetReservedSlot(cx, JL_OBJ, SLOT_SQLITE_ERROR_TEXT, *JL_RVAL) );

	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))

//	HAS_CONSTRUCTOR // see issue#52
	HAS_HAS_INSTANCE // see issue#52

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( code )
		PROPERTY_READ( const )
		PROPERTY_READ( text )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION(toString)
		FUNCTION_ARGC(_serialize, 1)
		FUNCTION_ARGC(_unserialize, 1)
	END_FUNCTION_SPEC

	HAS_RESERVED_SLOTS(2)

END_CLASS


JSBool SqliteThrowErrorStatus( JSContext *cx, sqlite3 *db, int status ) {

	JSObject *error = JS_NewObjectWithGivenProto( cx, JL_CLASS(SqliteError), JL_PROTOTYPE(cx, SqliteError), NULL ); // (TBD) understand why classSqliteError must have a constructor to be throwed in an exception
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
	JL_CHK( JL_SetReservedSlot( cx, error, SLOT_SQLITE_ERROR_CODE, INT_TO_JSVAL(status) ) );
	JL_CHK( JL_SetReservedSlot( cx, error, SLOT_SQLITE_ERROR_TEXT, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, "???")) ) );
	JL_SAFE( JL_ExceptionSetScriptLocation(cx, error) );
	return JS_FALSE;
	JL_BAD;
}


JSBool SqliteThrowError( JSContext *cx, sqlite3 *db ) {

	JSObject *error = JS_NewObjectWithGivenProto( cx, JL_CLASS(SqliteError), JL_PROTOTYPE(cx, SqliteError), NULL ); // (TBD) understand why classSqliteError must have a constructor to be throwed in an exception
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
	JL_CHK( JL_SetReservedSlot( cx, error, SLOT_SQLITE_ERROR_CODE, INT_TO_JSVAL(sqlite3_extended_errcode(db)) ) );
	JL_CHK( JL_SetReservedSlot( cx, error, SLOT_SQLITE_ERROR_TEXT, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, sqlite3_errmsg(db))) ) );
	JL_SAFE( JL_ExceptionSetScriptLocation(cx, error) );
	return JS_FALSE;
	JL_BAD;
}

/**doc
=== Exemple ===
{{{
try {

  db.Exec('yfiqwygqiwye'); // generate an error

} catch ( ex if ex instanceof SqliteError ) {

   Print( 'SqliteError: ' + ex.text + '('+ex.code+')', '\n' );
} catch( ex ) {

   throw ex;
}
}}}
**/




/*

Sure, you can JS_GetProperty(cx, global, "Error", &v) to get the constructor, then JS_GetProperty(cx, JSVAL_TO_OBJECT(v), "prototype", &v), then JL_GetClass(JSVAL_TO_OBJECT(v)).  Error and type checking elided, as usual.

*/
