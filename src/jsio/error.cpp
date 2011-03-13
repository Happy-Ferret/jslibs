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


/**doc fileIndex:bottom **/

const char *ConstString( int errorCode ) {

	switch ( errorCode ) {
		case PR_OUT_OF_MEMORY_ERROR: return "PR_OUT_OF_MEMORY_ERROR";
		case PR_BAD_DESCRIPTOR_ERROR: return "PR_BAD_DESCRIPTOR_ERROR";
		case PR_WOULD_BLOCK_ERROR: return "PR_WOULD_BLOCK_ERROR";
		case PR_ACCESS_FAULT_ERROR: return "PR_ACCESS_FAULT_ERROR";
		case PR_INVALID_METHOD_ERROR: return "PR_INVALID_METHOD_ERROR";
		case PR_ILLEGAL_ACCESS_ERROR: return "PR_ILLEGAL_ACCESS_ERROR";
		case PR_UNKNOWN_ERROR: return "PR_UNKNOWN_ERROR";
		case PR_PENDING_INTERRUPT_ERROR: return "PR_PENDING_INTERRUPT_ERROR";
		case PR_NOT_IMPLEMENTED_ERROR: return "PR_NOT_IMPLEMENTED_ERROR";
		case PR_IO_ERROR: return "PR_IO_ERROR";
		case PR_IO_TIMEOUT_ERROR: return "PR_IO_TIMEOUT_ERROR";
		case PR_IO_PENDING_ERROR: return "PR_IO_PENDING_ERROR";
		case PR_DIRECTORY_OPEN_ERROR: return "PR_DIRECTORY_OPEN_ERROR";
		case PR_INVALID_ARGUMENT_ERROR: return "PR_INVALID_ARGUMENT_ERROR";
		case PR_ADDRESS_NOT_AVAILABLE_ERROR: return "PR_ADDRESS_NOT_AVAILABLE_ERROR";
		case PR_ADDRESS_NOT_SUPPORTED_ERROR: return "PR_ADDRESS_NOT_SUPPORTED_ERROR";
		case PR_IS_CONNECTED_ERROR: return "PR_IS_CONNECTED_ERROR";
		case PR_BAD_ADDRESS_ERROR: return "PR_BAD_ADDRESS_ERROR";
		case PR_ADDRESS_IN_USE_ERROR: return "PR_ADDRESS_IN_USE_ERROR";
		case PR_CONNECT_REFUSED_ERROR: return "PR_CONNECT_REFUSED_ERROR";
		case PR_NETWORK_UNREACHABLE_ERROR: return "PR_NETWORK_UNREACHABLE_ERROR";
		case PR_CONNECT_TIMEOUT_ERROR: return "PR_CONNECT_TIMEOUT_ERROR";
		case PR_NOT_CONNECTED_ERROR: return "PR_NOT_CONNECTED_ERROR";
		case PR_LOAD_LIBRARY_ERROR: return "PR_LOAD_LIBRARY_ERROR";
		case PR_UNLOAD_LIBRARY_ERROR: return "PR_UNLOAD_LIBRARY_ERROR";
		case PR_FIND_SYMBOL_ERROR: return "PR_FIND_SYMBOL_ERROR";
		case PR_INSUFFICIENT_RESOURCES_ERROR: return "PR_INSUFFICIENT_RESOURCES_ERROR";
		case PR_DIRECTORY_LOOKUP_ERROR: return "PR_DIRECTORY_LOOKUP_ERROR";
		case PR_TPD_RANGE_ERROR: return "PR_TPD_RANGE_ERROR";
		case PR_PROC_DESC_TABLE_FULL_ERROR: return "PR_PROC_DESC_TABLE_FULL_ERROR";
		case PR_SYS_DESC_TABLE_FULL_ERROR: return "PR_SYS_DESC_TABLE_FULL_ERROR";
		case PR_NOT_SOCKET_ERROR: return "PR_NOT_SOCKET_ERROR";
		case PR_NOT_TCP_SOCKET_ERROR: return "PR_NOT_TCP_SOCKET_ERROR";
		case PR_SOCKET_ADDRESS_IS_BOUND_ERROR: return "PR_SOCKET_ADDRESS_IS_BOUND_ERROR";
		case PR_NO_ACCESS_RIGHTS_ERROR: return "PR_NO_ACCESS_RIGHTS_ERROR";
		case PR_OPERATION_NOT_SUPPORTED_ERROR: return "PR_OPERATION_NOT_SUPPORTED_ERROR";
		case PR_PROTOCOL_NOT_SUPPORTED_ERROR: return "PR_PROTOCOL_NOT_SUPPORTED_ERROR";
		case PR_REMOTE_FILE_ERROR: return "PR_REMOTE_FILE_ERROR";
		case PR_BUFFER_OVERFLOW_ERROR: return "PR_BUFFER_OVERFLOW_ERROR";
		case PR_CONNECT_RESET_ERROR: return "PR_CONNECT_RESET_ERROR";
		case PR_RANGE_ERROR: return "PR_RANGE_ERROR";
		case PR_DEADLOCK_ERROR: return "PR_DEADLOCK_ERROR";
		case PR_FILE_IS_LOCKED_ERROR: return "PR_FILE_IS_LOCKED_ERROR";
		case PR_FILE_TOO_BIG_ERROR: return "PR_FILE_TOO_BIG_ERROR";
		case PR_NO_DEVICE_SPACE_ERROR: return "PR_NO_DEVICE_SPACE_ERROR";
		case PR_PIPE_ERROR: return "PR_PIPE_ERROR";
		case PR_NO_SEEK_DEVICE_ERROR: return "PR_NO_SEEK_DEVICE_ERROR";
		case PR_IS_DIRECTORY_ERROR: return "PR_IS_DIRECTORY_ERROR";
		case PR_LOOP_ERROR: return "PR_LOOP_ERROR";
		case PR_NAME_TOO_LONG_ERROR: return "PR_NAME_TOO_LONG_ERROR";
		case PR_FILE_NOT_FOUND_ERROR: return "PR_FILE_NOT_FOUND_ERROR";
		case PR_NOT_DIRECTORY_ERROR: return "PR_NOT_DIRECTORY_ERROR";
		case PR_READ_ONLY_FILESYSTEM_ERROR: return "PR_READ_ONLY_FILESYSTEM_ERROR";
		case PR_DIRECTORY_NOT_EMPTY_ERROR: return "PR_DIRECTORY_NOT_EMPTY_ERROR";
		case PR_FILESYSTEM_MOUNTED_ERROR: return "PR_FILESYSTEM_MOUNTED_ERROR";
		case PR_NOT_SAME_DEVICE_ERROR: return "PR_NOT_SAME_DEVICE_ERROR";
		case PR_DIRECTORY_CORRUPTED_ERROR: return "PR_DIRECTORY_CORRUPTED_ERROR";
		case PR_FILE_EXISTS_ERROR: return "PR_FILE_EXISTS_ERROR";
		case PR_MAX_DIRECTORY_ENTRIES_ERROR: return "PR_MAX_DIRECTORY_ENTRIES_ERROR";
		case PR_INVALID_DEVICE_STATE_ERROR: return "PR_INVALID_DEVICE_STATE_ERROR";
		case PR_DEVICE_IS_LOCKED_ERROR: return "PR_DEVICE_IS_LOCKED_ERROR";
		case PR_NO_MORE_FILES_ERROR: return "PR_NO_MORE_FILES_ERROR";
		case PR_END_OF_FILE_ERROR: return "PR_END_OF_FILE_ERROR";
		case PR_FILE_SEEK_ERROR: return "PR_FILE_SEEK_ERROR";
		case PR_FILE_IS_BUSY_ERROR: return "PR_FILE_IS_BUSY_ERROR";
		case PR_OPERATION_ABORTED_ERROR: return "PR_OPERATION_ABORTED_ERROR";
		case PR_IN_PROGRESS_ERROR: return "PR_IN_PROGRESS_ERROR";
		case PR_ALREADY_INITIATED_ERROR: return "PR_ALREADY_INITIATED_ERROR";
		case PR_GROUP_EMPTY_ERROR: return "PR_GROUP_EMPTY_ERROR";
		case PR_INVALID_STATE_ERROR: return "PR_INVALID_STATE_ERROR";
		case PR_NETWORK_DOWN_ERROR: return "PR_NETWORK_DOWN_ERROR";
		case PR_SOCKET_SHUTDOWN_ERROR: return "PR_SOCKET_SHUTDOWN_ERROR";
		case PR_CONNECT_ABORTED_ERROR: return "PR_CONNECT_ABORTED_ERROR";
		case PR_HOST_UNREACHABLE_ERROR: return "PR_HOST_UNREACHABLE_ERROR";
		case PR_LIBRARY_NOT_LOADED_ERROR: return "PR_LIBRARY_NOT_LOADED_ERROR";
		case PR_CALL_ONCE_ERROR: return "PR_CALL_ONCE_ERROR";
		case PR_MAX_ERROR: return "PR_MAX_ERROR";
	}
	return "UNKNOWN_ERROR";
}



/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
 You cannot construct this class.$LF
 Its aim is to throw as an exception on any NSPR runtime error.
**/
BEGIN_CLASS( IoError )


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
*/
DEFINE_PROPERTY_GETTER( code ) {

	return JL_GetReservedSlot( cx, obj, 0, vp );  // (TBD) use the obj.name proprety directly instead of slot 0 ?
}


/**doc
$TOC_MEMBER $INAME
 $STRING $INAME $READONLY
*/
DEFINE_PROPERTY_GETTER( const ) {

	JL_CHK( JL_GetReservedSlot(cx, obj, 0, vp) );
	if ( JSVAL_IS_VOID(*vp) )
		return JS_TRUE;
	int errorCode = JSVAL_TO_INT(*vp);
	JSString *str = JS_NewStringCopyZ( cx, ConstString(errorCode) );
	JL_CHK( str );
	*vp = STRING_TO_JSVAL( str );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
*/
DEFINE_PROPERTY_GETTER( os ) {

	return JL_GetReservedSlot( cx, obj, 1, vp );  // (TBD) use the obj.name proprety directly instead of slot 1 ?
}



/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
*/
DEFINE_PROPERTY_GETTER( text ) {

	JL_CHK( JL_GetReservedSlot(cx, obj, 0, vp) );  // (TBD) use the obj.name proprety directly instead of slot 0 ?
	if ( JSVAL_IS_VOID(*vp) )
		return JS_TRUE;
	PRErrorCode errorCode;
	errorCode = JSVAL_TO_INT(*vp);
	JSString *str;
	str = JS_NewStringCopyZ( cx, PR_ErrorToString( errorCode, PR_LANGUAGE_EN ) );
	*vp = STRING_TO_JSVAL( str );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION( toString ) {

	JL_DEFINE_FUNCTION_OBJ;
	return _textGetter(cx, obj, JSID_EMPTY, JL_RVAL);
}

DEFINE_HAS_INSTANCE() { // see issue#52

	*bp = !JSVAL_IS_PRIMITIVE(*v) && JL_InheritFrom(cx, JSVAL_TO_OBJECT(*v), JL_THIS_CLASS);
	return JS_TRUE;
}


DEFINE_FUNCTION( _serialize ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARG_COUNT(1);
	JL_ASSERT_ARG_TYPE( jl::JsvalIsSerializer(cx, JL_ARG(1)), 1, "Serializer" );

	jl::Serializer *ser;
	ser = jl::JsvalToSerializer(cx, JL_ARG(1));

	JL_CHK( JS_GetPropertyById(cx, JL_OBJ, JL_ATOMJSID(cx, fileName), JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );
	JL_CHK( JS_GetPropertyById(cx, JL_OBJ, JL_ATOMJSID(cx, lineNumber), JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );
	JL_CHK( JL_GetReservedSlot(cx, JL_OBJ, 0, JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );
	JL_CHK( JL_GetReservedSlot(cx, JL_OBJ, 1, JL_RVAL) );
	JL_CHK( ser->Write(cx, *JL_RVAL) );

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( _unserialize ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARG_COUNT(1);
	JL_ASSERT_ARG_TYPE( jl::JsvalIsUnserializer(cx, JL_ARG(1)), 1, "Unserializer" );

	jl::Unserializer *unser;
	unser = jl::JsvalToUnserializer(cx, JL_ARG(1));

	JL_CHK( unser->Read(cx, *JL_RVAL) );
	JL_CHK( JS_SetPropertyById(cx, obj, JL_ATOMJSID(cx, fileName), JL_RVAL) );
	JL_CHK( unser->Read(cx, *JL_RVAL) );
	JL_CHK( JS_SetPropertyById(cx, obj, JL_ATOMJSID(cx, lineNumber), JL_RVAL) );
	JL_CHK( unser->Read(cx, *JL_RVAL) );
	JL_CHK( JL_SetReservedSlot(cx, JL_OBJ, 0, *JL_RVAL) );
	JL_CHK( unser->Read(cx, *JL_RVAL) );
	JL_CHK( JL_SetReservedSlot(cx, JL_OBJ, 1, *JL_RVAL) );

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
		PROPERTY_READ( os )
		PROPERTY_READ( text )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION(toString)
		FUNCTION_ARGC(_serialize, 1)
		FUNCTION_ARGC(_unserialize, 1)
	END_FUNCTION_SPEC

	HAS_RESERVED_SLOTS(2)

END_CLASS



NEVER_INLINE JSBool FASTCALL
ThrowIoErrorArg( JSContext *cx, PRErrorCode errorCode, PRInt32 osError ) {

	JSObject *error = JS_NewObjectWithGivenProto( cx, JL_CLASS(IoError), JL_PROTOTYPE(cx, IoError), NULL );
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
	JL_CHK( JL_SetReservedSlot( cx, error, 0, INT_TO_JSVAL(errorCode) ) );
	JL_CHK( JL_SetReservedSlot( cx, error, 1, INT_TO_JSVAL(osError) ) );
	JL_SAFE( JL_ExceptionSetScriptLocation(cx, error) );
	return JS_FALSE;
	JL_BAD;
}
