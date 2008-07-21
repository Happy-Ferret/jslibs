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

#include <pprio.h> // nspr/include/nspr/private

#include "descriptor.h"
#include "file.h"


PRIntn FileOpenFlagsFromString( const char *strFlags, int length ) {
	
	if ( length == 0 || length > 2 )
		return 0;
	if ( length == 2 && strFlags[1] != '+' )
		return 0;

	char c = strFlags[0];
	bool plus = length == 2 && strFlags[1] == '+';

	PRIntn flags;
	if ( c == 'r' )
		flags = plus ? PR_RDWR : PR_RDONLY;
	else 
		if ( c == 'w' )
			flags = plus ? PR_CREATE_FILE | PR_TRUNCATE | PR_RDWR : PR_CREATE_FILE | PR_TRUNCATE | PR_WRONLY;
		else 
			if ( c == 'a' )
				flags = plus ? PR_CREATE_FILE | PR_APPEND | PR_RDWR : PR_CREATE_FILE | PR_APPEND | PR_WRONLY;
			else
				flags = 0;
	return flags;
}

/**doc
$CLASS_HEADER Descriptor
**/
BEGIN_CLASS( File )

DEFINE_FINALIZE() {

	FinalizeDescriptor(cx, obj); // defined in descriptor.cpp
}

/**doc
 * $INAME( fileName )
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN(1);
	JS_SetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, J_ARG(1) );
	JS_SetPrivate(cx, obj, NULL); // (TBD) optional ?
	ReserveStreamReadInterface(cx, obj);
	return JS_TRUE;
}

/**doc
=== Methods ===
**/

/**doc
 * $THIS $INAME( flags [, mode] )
  Open a file for reading, writing, or both.
  = =
  _flags_ is either a combinaison of open mode constants or a string that contains fopen like flags (+, r, w, a).
  = =
  The functions returns the file object itself (this), this allows to write things like: `new File('foo.txt').Open('r').Read();`
**/
DEFINE_FUNCTION( Open ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT( JS_GetPrivate( cx, obj ) == NULL, "File is already open." );

	jsval jsvalFileName;
	JS_GetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, &jsvalFileName );
	J_S_ASSERT_DEFINED( jsvalFileName );
	const char *fileName;
	J_CHK( JsvalToString(cx, jsvalFileName, &fileName) );

	PRIntn flags;
	if ( JSVAL_IS_INT( J_ARG(1) ) ) {
	
		flags = JSVAL_TO_INT( J_ARG(1) );
	} else {

		const char *strFlags;
		size_t len;
		J_CHK( JsvalToStringAndLength(cx, J_ARG(1), &strFlags, &len) );
		flags = FileOpenFlagsFromString(strFlags, len);
	}

	PRIntn mode;
	if ( J_ARG_ISDEF(2) ) {
		
		J_JSVAL_TO_INT32( J_ARG(2), mode );
	} else {

		mode = PR_IRUSR + PR_IWUSR; // read write permission, owner
	}

	PRFileDesc *fd = PR_Open( fileName, flags, mode ); // The mode parameter is currently applicable only on Unix platforms.
	if ( fd == NULL )
		return ThrowIoError(cx);
	JS_SetPrivate( cx, obj, fd );
	
//	J_CHK( SetStreamReadInterface(cx, obj, NativeInterfaceStreamRead) );

	J_CHK( ReserveStreamReadInterface(cx, obj) ); // this reserves the NativeInterface, then it can be switched on/off safely (see Descriptor::Close)
	J_CHK( SetStreamReadInterface(cx, obj, NativeInterfaceStreamRead) );

	*rval = OBJECT_TO_JSVAL(obj); // allows to write f.Open(...).Read()
	return JS_TRUE;
}


/**doc
 * $INT $INAME( [ offset = 0 [, whence = File.`SEEK_SET` ] ] )
  Moves read-write file offset.
**/
DEFINE_FUNCTION( Seek ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	J_S_ASSERT( fd != NULL, "File is closed." );

	PRInt64 offset;
	if ( J_ARG_ISDEF(1) ) {

		jsdouble doubleOffset;
		JS_ValueToNumber( cx, J_ARG(1), &doubleOffset );
		offset = (PRInt64)doubleOffset;
	} else
		offset = 0; // default is arg is missing

	PRSeekWhence whence;
	if ( J_ARG_ISDEF(2) ) {

		int32 tmp;
		JS_ValueToInt32( cx, J_ARG(2), &tmp );
		whence = (PRSeekWhence)tmp;
	} else
		whence = PR_SEEK_CUR; // default is arg is missing

	PRInt64 ret = PR_Seek64( fd, offset, whence );
	if ( ret == -1 )
		return ThrowIoError(cx);

	jsdouble newLocation = ret;
	JS_NewDoubleValue( cx, newLocation, rval );
	return JS_TRUE;
}


/**doc
 * $INT $INAME()
  Delete a file from the filesystem.
  The operation may fail if the file is open.
**/
DEFINE_FUNCTION( Delete ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	J_S_ASSERT( fd == NULL, "Cannot delete an open file." );
	jsval jsvalFileName;
	JS_GetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, &jsvalFileName );
	J_S_ASSERT_DEFINED( jsvalFileName );
	const char *fileName;
	J_CHK( JsvalToString(cx, jsvalFileName, &fileName) );
	if ( PR_Delete(fileName) != PR_SUCCESS )
		return ThrowIoError(cx);
	return JS_TRUE;
}


/**doc
 * $VOID $INAME( state )
  Lock or unlock a file for exclusive access.
  _state_ can be <true> or <false>.
**/
DEFINE_FUNCTION( Lock ) {

	J_S_ASSERT_ARG_MIN( 1 );
	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( fd );
	bool doLock;
	J_JSVAL_TO_BOOL( J_ARG(1), doLock );
	PRStatus st = doLock ? PR_LockFile(fd) : PR_UnlockFile(fd);
	if ( st != PR_SUCCESS )
		return ThrowIoError(cx);
	return JS_TRUE;
}

/**doc
 * $VOID $INAME( directory )
  Move the file to another directory.
**/
DEFINE_FUNCTION( Move ) {

	J_S_ASSERT_ARG_MIN( 1 );
	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	J_S_ASSERT( fd == NULL, "Cannot move an open file." );

	jsval jsvalFileName;
	JS_GetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, &jsvalFileName );
	J_S_ASSERT_DEFINED( jsvalFileName );
	const char *fileName;
	J_CHK( JsvalToString(cx, jsvalFileName, &fileName) );

	const char *destDirName;
	size_t destDirNameLength;
	J_CHK( JsvalToStringAndLength(cx, J_ARG(1), &destDirName, &destDirNameLength) );

	const char *fileNameOnly = strrchr(fileName, '/');
	if ( fileNameOnly == NULL )
		fileNameOnly = fileName;

	J_S_ASSERT( destDirNameLength + strlen(fileNameOnly) + 1 < PATH_MAX, "Invalid file name." );

	char destFileName[PATH_MAX];
	strcpy(destFileName, destDirName);
	strcat(destFileName, "/");
	strcat(destFileName, fileNameOnly);

	if ( PR_Rename(fileName, destFileName) != PR_SUCCESS )
		return ThrowIoError(cx);
	
	JSString *jsstr = JS_NewStringCopyZ(cx, destFileName);
	J_S_ASSERT_ALLOC( jsstr );

	J_CHK( JS_SetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, STRING_TO_JSVAL(jsstr) ) );
	
	return JS_TRUE;
}

/**doc
=== Properties ===
**/

/**doc
 * $INT $INAME
  Get or set the current position of the file pointer. Same as Seek() function used with SEEK_SET.
**/
DEFINE_PROPERTY( positionSetter ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( fd );
	PRInt64 offset;
	jsdouble doubleOffset;
	J_CHK( JS_ValueToNumber( cx, *vp, &doubleOffset ) );
	offset = (PRInt64)doubleOffset;
	PRInt64 ret = PR_Seek64( fd, offset, PR_SEEK_SET );
	if ( ret == -1 )
		return ThrowIoError(cx);
	return JS_TRUE;
}

DEFINE_PROPERTY( positionGetter ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	J_S_ASSERT_RESOURCE( fd );
	PRInt64 ret = PR_Seek64( fd, 0, PR_SEEK_CUR );
	if ( ret == -1 )
		return ThrowIoError(cx);
	J_CHK( JS_NewNumberValue(cx, ret, vp) );
	return JS_TRUE;
}


/**doc
 * $STR $INAME
  Get or set the content of the file. If the file does not exist, content is <undefined>.
  Setting content with <undefined> deletes the file.
**/
DEFINE_PROPERTY( contentGetter ) { // (TBD) support BString

	J_S_ASSERT( (PRFileDesc *)JS_GetPrivate( cx, obj ) == NULL, "Cannot get content of an open file.");
	jsval jsvalFileName;
	JS_GetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, &jsvalFileName );
	J_S_ASSERT_DEFINED( jsvalFileName );
	const char *fileName;
	J_CHK( JsvalToString(cx, jsvalFileName, &fileName) );

	PRStatus status = PR_Access( fileName, PR_ACCESS_READ_OK ); // We want to read the whole file, then first check if the file is readable
	if ( status != PR_SUCCESS ) {

		*vp = JSVAL_VOID;
		return JS_TRUE;
	}

	PRFileDesc *fd = PR_OpenFile( fileName, PR_RDONLY, 0 ); // The mode parameter is currently applicable only on Unix platforms.
	if ( fd == NULL ) {

		PRErrorCode err = PR_GetError();
		if ( err == PR_FILE_NOT_FOUND_ERROR )
			return JS_TRUE; // property will return  undefined
		return ThrowIoErrorArg( cx, err, PR_GetOSError() );
	}

	PRInt32 available = PR_Available( fd ); // For a normal file, these are the bytes beyond the current file pointer.
	if ( available == -1 )
		return ThrowIoError(cx);
	char *buf = (char*)JS_malloc( cx, available +1 );
	J_S_ASSERT_ALLOC(buf);
	buf[available] = '\0';

	PRInt32 res = PR_Read( fd, buf, available );
	if ( res == -1 ) {

		JS_free( cx, buf );
		return ThrowIoError(cx);
	}
	if ( PR_Close( fd ) == PR_FAILURE )
		return ThrowIoError(cx);
	if ( res == 0 ) {

		JS_free( cx, buf );
		*vp = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}
	if ( MaybeRealloc( available, res ) ) { // should never occured

		buf = (char*)JS_realloc(cx, buf, res + 1); // realloc the string using its real size
		J_S_ASSERT_ALLOC(buf);
	}
	JSString *str = JS_NewString( cx, (char*)buf, res );
	J_S_ASSERT_ALLOC(str);
	*vp = STRING_TO_JSVAL(str);
	return JS_TRUE;
}


DEFINE_PROPERTY( contentSetter ) { // (TBD) support BString

//	J_S_ASSERT_DEFINED( *vp );
	J_S_ASSERT( (PRFileDesc *)JS_GetPrivate( cx, obj ) == NULL, "Cannot set content of an open file.");
	jsval jsvalFileName;
	JS_GetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, &jsvalFileName );
	J_S_ASSERT_DEFINED( jsvalFileName );
	const char *fileName;
	J_CHK( JsvalToString(cx, jsvalFileName, &fileName) );
	if ( *vp == JSVAL_VOID ) {

		if ( PR_Delete(fileName) != PR_SUCCESS ) {

			PRErrorCode err = PR_GetError();
			if ( err == PR_FILE_NOT_FOUND_ERROR )
				return JS_TRUE; // property will return  undefined
			return ThrowIoErrorArg( cx, err, PR_GetOSError() );
		}
		return JS_TRUE;
	}
	PRFileDesc *fd = PR_OpenFile( fileName, PR_CREATE_FILE | PR_TRUNCATE | PR_WRONLY, PR_IRUSR + PR_IWUSR ); // The mode parameter is currently applicable only on Unix platforms.
	if ( fd == NULL )
		return ThrowIoError(cx);
	const char *buf;
	size_t len;
	J_CHK( JsvalToStringAndLength(cx, *vp, &buf, &len) );
	PRInt32 bytesSent = PR_Write( fd, buf, len );
	if ( bytesSent == -1 )
		return ThrowIoError(cx);
	J_S_ASSERT( bytesSent == len, "unable to set content" );
	if ( PR_Close(fd) != PR_SUCCESS )
		return ThrowIoError(cx);
	return JS_TRUE;
}


/**doc
 * $STR $INAME
  Contains the name of the file. Changing this value will rename or move the file.
**/
DEFINE_PROPERTY( nameGetter ) {

	JS_GetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, vp );
	return JS_TRUE;
}

DEFINE_PROPERTY( nameSetter ) {

	J_S_ASSERT_DEFINED( *vp );

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	J_S_ASSERT( fd == NULL, "Cannot rename an open file.");
	jsval jsvalFileName;
	JS_GetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, &jsvalFileName );
	J_S_ASSERT_DEFINED( jsvalFileName );
	const char *fromFileName, *toFileName;
	J_CHK( JsvalToString(cx, jsvalFileName, &fromFileName) );
	J_CHK( JsvalToString(cx, *vp, &toFileName) );
	if ( PR_Rename(fromFileName, toFileName) != PR_SUCCESS ) // if status == PR_FILE_EXISTS_ERROR ...
		return ThrowIoError(cx);
	JS_SetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, *vp );
	return JS_TRUE;
}


/**doc
 * $BOOL $INAME $READONLY
  Contains true if the file exists.
**/
DEFINE_PROPERTY( exist ) {

	jsval jsvalFileName;
//	JS_GetProperty( cx, obj, "fileName", &jsvalFileName );
	JS_GetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, &jsvalFileName );
	J_S_ASSERT_DEFINED( jsvalFileName );
	const char *fileName;
	J_CHK( JsvalToString(cx, jsvalFileName, &fileName) );
	PRStatus status = PR_Access( fileName, PR_ACCESS_EXISTS );
	*vp = BOOLEAN_TO_JSVAL( status == PR_SUCCESS );
	return JS_TRUE;
}


/**doc
 * $OBJ $INAME $READONLY
  This property works with opened and closed files.
  Contains an object that has the following properties:
   * type : the type of the file
   * size : the size of the file
**/
DEFINE_PROPERTY( info ) {

	PRFileInfo fileInfo;
	PRStatus status;

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd == NULL ) {

		jsval jsvalFileName;
		JS_GetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, &jsvalFileName );
		J_S_ASSERT_DEFINED( jsvalFileName );
		const char *fileName;
		J_CHK( JsvalToString(cx, jsvalFileName, &fileName) );

		status = PR_GetFileInfo( fileName, &fileInfo );
	} else
		status = PR_GetOpenFileInfo( fd, &fileInfo );

	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);

	JSObject *fileTypeObj = JS_NewObject( cx, NULL, NULL, NULL );
	*vp = OBJECT_TO_JSVAL( fileTypeObj );

	// (TBD) these properties must be read-only !!
	jsval jsvalType = INT_TO_JSVAL((int)fileInfo.type);
	JS_SetProperty( cx, fileTypeObj, "type", &jsvalType );

	jsval jsvalSize = INT_TO_JSVAL(fileInfo.size);
	JS_SetProperty( cx, fileTypeObj, "size", &jsvalSize );

	jsval jsvalCreationTime = INT_TO_JSVAL(fileInfo.creationTime);
	JS_SetProperty( cx, fileTypeObj, "creationTime", &jsvalCreationTime );

	jsval jsvalModifyTime = INT_TO_JSVAL(fileInfo.modifyTime);
	JS_SetProperty( cx, fileTypeObj, "modifyTime", &jsvalModifyTime );

	return JS_TRUE;
}

/**doc
=== Static properties ===
**/

/**doc
 * $TYPE File *stdin* $READONLY
  Is a jsio::File that represents the standard input.
  
 * $TYPE File *stdout* $READONLY
  Is a jsio::File that represents the standard output.
  
 * $TYPE File *stderr* $READONLY
  Is a jsio::File that represents the standard error.
**/
DEFINE_PROPERTY( standard ) {

	if ( *vp == JSVAL_VOID ) {

		int32 i;
		JS_ValueToInt32( cx, id, &i );

		JSObject *obj = JS_NewObject(cx, &classFile, NULL, NULL ); // no need to use classDescriptor as proto.
		*vp = OBJECT_TO_JSVAL( obj );

		PRFileDesc *fd = PR_GetSpecialFD((PRSpecialFD)i); // beware: cast !
		if ( fd == NULL )
			return ThrowIoError(cx);
		JS_SetPrivate( cx, obj, fd );

		JS_SetReservedSlot(cx, obj, SLOT_JSIO_DESCRIPTOR_IMPORTED, JSVAL_TRUE); // avoid PR_Close
	}
	return JS_TRUE;
}

/**doc
=== Constants ===
**/

/**doc
 * *Open* mode
  * File.`RDONLY`
  * File.`WRONLY`
  * File.`RDWR`
  * File.`CREATE_FILE`
  * File.`APPEND`
  * File.`TRUNCATE`
  * File.`SYNC`
  * File.`EXCL`
 * *Seek* mode
  * File.`SEEK_SET`
  * File.`SEEK_CUR`
  * File.`SEEK_END`
 * *info.type*
  * File.`FILE_FILE`
  * File.`FILE_DIRECTORY`
  * File.`FILE_OTHER`
**/


/**doc
=== Native Interface ===
 *NIStreamRead*: Read the file as a stream.
**/


CONFIGURE_CLASS

	HAS_PROTOTYPE( prototypeDescriptor )

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	HAS_PRIVATE
	HAS_RESERVED_SLOTS( 2 ) // SLOT_JSIO_DESCRIPTOR_IMPORTED, SLOT_JSIO_FILE_NAME

	BEGIN_FUNCTION_SPEC
		FUNCTION( Open )
		FUNCTION( Seek )
		FUNCTION( Delete )
		FUNCTION( Lock )
		FUNCTION( Move )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( name )
		PROPERTY( content )
		PROPERTY( position )
		PROPERTY_READ( exist )
		PROPERTY_READ( info )
	END_PROPERTY_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_CREATE( stdin , PR_StandardInput,  JSPROP_PERMANENT|JSPROP_READONLY, standard, NULL ) // (TBD) change this
		PROPERTY_CREATE( stdout, PR_StandardOutput, JSPROP_PERMANENT|JSPROP_READONLY, standard, NULL ) // (TBD) change this
		PROPERTY_CREATE( stderr, PR_StandardError,  JSPROP_PERMANENT|JSPROP_READONLY, standard, NULL ) // (TBD) change this
	END_STATIC_PROPERTY_SPEC

	BEGIN_CONST_DOUBLE_SPEC
// PR_Open flags
		CONST_DOUBLE( RDONLY			,PR_RDONLY )
		CONST_DOUBLE( WRONLY			,PR_WRONLY )
		CONST_DOUBLE( RDWR			,PR_RDWR )
		CONST_DOUBLE( CREATE_FILE	,PR_CREATE_FILE )
		CONST_DOUBLE( APPEND			,PR_APPEND )
		CONST_DOUBLE( TRUNCATE		,PR_TRUNCATE )
		CONST_DOUBLE( SYNC			,PR_SYNC )
		CONST_DOUBLE( EXCL			,PR_EXCL )
// PRSeekWhence enum
		CONST_DOUBLE( SEEK_SET			,PR_SEEK_SET )
		CONST_DOUBLE( SEEK_CUR			,PR_SEEK_CUR )
		CONST_DOUBLE( SEEK_END			,PR_SEEK_END )
// PRFileType enum
		CONST_DOUBLE( FILE_FILE			,PR_FILE_FILE )
		CONST_DOUBLE( FILE_DIRECTORY	,PR_FILE_DIRECTORY )
		CONST_DOUBLE( FILE_OTHER		,PR_FILE_OTHER )
	END_CONST_DOUBLE_SPEC

END_CLASS

/**doc
=== Example ===
{{{
LoadModule('jsstd');
LoadModule('jsio');

try {

   var file = new File('file_test.txt');
   if ( file.exist ) {
      file.Open( File.RDONLY );
      Print( 'file content:\n' + file.Read() );
      file.Close();
   }

} catch ( ex if ex instanceof IoError ) {
   
   Print( 'IOError: ' + ex.text, '\n' );
} catch( ex ) {
   
   throw ex;
}
}}}
**/
