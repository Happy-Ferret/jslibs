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
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( File )

DEFINE_FINALIZE() {

	FinalizeDescriptor(cx, obj); // defined in descriptor.cpp
}

/**doc
$TOC_MEMBER $INAME
 $INAME( fileName )
**/
DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();
	JL_S_ASSERT_ARG_MIN(1);
	JL_CHK( JS_SetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, JL_ARG(1) ) );
	JL_SetPrivate(cx, obj, NULL); // (TBD) optional ?
	ReserveStreamReadInterface(cx, obj);
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( flags [, mode] )
  Open a file for reading, writing, or both.
  $LF
  _flags_ is either a combinaison of open mode constants or a string that contains fopen like flags (+, r, w, a).
  $LF
  The functions returns the file object itself (this), this allows to write things like: `new File('foo.txt').Open('r').Read();`
**/
DEFINE_FUNCTION( Open ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT( JL_GetPrivate( cx, obj ) == NULL, "File is already open." );

	PRIntn flags;
	if ( JSVAL_IS_INT( JL_ARG(1) ) ) {

		flags = JSVAL_TO_INT( JL_ARG(1) );
	} else {

		const char *strFlags;
		size_t len;
		JL_CHK( JsvalToStringAndLength(cx, &JL_ARG(1), &strFlags, &len) );
		flags = FileOpenFlagsFromString(strFlags, len);
	}

	PRIntn mode;
	if ( JL_ARG_ISDEF(2) ) {

		JL_CHK( JsvalToInt(cx, JL_ARG(2), &mode) );
	} else { // default

		mode = PR_IRUSR | PR_IWUSR | PR_IRGRP | PR_IWGRP; // read write permission for owner & group
	}

	jsval jsvalFileName;
	JS_GetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, &jsvalFileName );
	JL_S_ASSERT_DEFINED( jsvalFileName );
	const char *fileName;
	JL_CHK( JsvalToString(cx, &jsvalFileName, &fileName) );

	PRFileDesc *fd;
	fd = PR_Open( fileName, flags, mode ); // The mode parameter is currently applicable only on Unix platforms.
	if ( fd == NULL )
		return ThrowIoError(cx);
	JL_SetPrivate( cx, obj, fd );

//	JL_CHK( SetStreamReadInterface(cx, obj, NativeInterfaceStreamRead) );

//	JL_CHK( ReserveStreamReadInterface(cx, obj) ); // this reserves the NativeInterface, then it can be switched on/off safely (see Descriptor::Close)
	JL_CHK( SetStreamReadInterface(cx, obj, NativeInterfaceStreamRead) );

	*rval = OBJECT_TO_JSVAL(obj); // allows to write f.Open(...).Read()
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( [ offset = 0 [, whence = File.`SEEK_SET` ] ] )
  Moves read-write file offset.
**/
DEFINE_FUNCTION( Seek ) {

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT( fd != NULL, "File is closed." );

	PRInt64 offset;
	if ( JL_ARG_ISDEF(1) ) {

		jsdouble doubleOffset;
		JS_ValueToNumber( cx, JL_ARG(1), &doubleOffset );
		offset = (PRInt64)doubleOffset;
	} else
		offset = 0; // default is arg is missing

	PRSeekWhence whence;
	if ( JL_ARG_ISDEF(2) ) {

		int32 tmp;
		JS_ValueToInt32( cx, JL_ARG(2), &tmp );
		whence = (PRSeekWhence)tmp;
	} else
		whence = PR_SEEK_CUR; // default is arg is missing

	PRInt64 ret;
	ret = PR_Seek64( fd, offset, whence );
	if ( ret == -1 )
		return ThrowIoError(cx);

	jsdouble newLocation;
	newLocation = ret;
	JS_NewDoubleValue( cx, newLocation, rval );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Delete a file from the filesystem.
  The operation may fail if the file is open.
**/
DEFINE_FUNCTION( Delete ) {

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT( fd == NULL, "Cannot delete an open file." );
	jsval jsvalFileName;
	JS_GetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, &jsvalFileName );
	JL_S_ASSERT_DEFINED( jsvalFileName );
	const char *fileName;
	JL_CHK( JsvalToString(cx, &jsvalFileName, &fileName) );
	if ( PR_Delete(fileName) != PR_SUCCESS )
		return ThrowIoError(cx);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( state )
  Lock or unlock a file for exclusive access.
  _state_ can be _true_ or _false_.
**/
DEFINE_FUNCTION( Lock ) {

	JL_S_ASSERT_ARG_MIN( 1 );
	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT_RESOURCE( fd );
	bool doLock;
	JL_CHK( JsvalToBool(cx, JL_ARG(1), &doLock) );
	PRStatus st;
	st = doLock ? PR_LockFile(fd) : PR_UnlockFile(fd);
	if ( st != PR_SUCCESS )
		return ThrowIoError(cx);
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( directory )
  Move the file to another directory.
**/
DEFINE_FUNCTION( Move ) {

	JL_S_ASSERT_ARG_MIN( 1 );
	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT( fd == NULL, "Cannot move an open file." );

	jsval jsvalFileName;
	JS_GetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, &jsvalFileName );
	JL_S_ASSERT_DEFINED( jsvalFileName );
	const char *fileName;
	JL_CHK( JsvalToString(cx, &jsvalFileName, &fileName) ); // warning: GC on the returned buffer !

	const char *destDirName;
	size_t destDirNameLength;
	JL_CHK( JsvalToStringAndLength(cx, &JL_ARG(1), &destDirName, &destDirNameLength) ); // warning: GC on the returned buffer !

	const char *fileNameOnly;
	fileNameOnly = strrchr(fileName, '/');
	if ( fileNameOnly == NULL )
		fileNameOnly = fileName;

	JL_S_ASSERT( destDirNameLength + strlen(fileNameOnly) + 1 < PATH_MAX, "Invalid file name." );

	char destFileName[PATH_MAX];
	strcpy(destFileName, destDirName);
	strcat(destFileName, "/");
	strcat(destFileName, fileNameOnly);

	if ( PR_Rename(fileName, destFileName) != PR_SUCCESS )
		return ThrowIoError(cx);

	JSString *jsstr;
	jsstr = JS_NewStringCopyZ(cx, destFileName);
	JL_CHK( jsstr );

	JL_CHK( JS_SetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, STRING_TO_JSVAL(jsstr) ) );

	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Get or set the current position of the file pointer. Same as Seek() function used with SEEK_SET.
**/
DEFINE_PROPERTY( positionSetter ) {

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT( fd != NULL, "File is closed." );
	PRInt64 offset;
	jsdouble doubleOffset;
	JL_CHK( JS_ValueToNumber( cx, *vp, &doubleOffset ) );
	offset = (PRInt64)doubleOffset;
	PRInt64 ret;
	ret = PR_Seek64( fd, offset, PR_SEEK_SET );
	if ( ret == -1 )
		return ThrowIoError(cx);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( positionGetter ) {

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT( fd != NULL, "File is closed." );
	PRInt64 ret;
	ret  = PR_Seek64( fd, 0, PR_SEEK_CUR );
	if ( ret == -1 )
		return ThrowIoError(cx);
	JL_CHK( JS_NewNumberValue(cx, ret, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Get or set the content of the file. If the file does not exist, content is _undefined_.
  Setting content with _undefined_ deletes the file.
**/
DEFINE_PROPERTY( contentGetter ) {

	JL_S_ASSERT( (PRFileDesc*)JL_GetPrivate(cx, obj) == NULL, "Cannot get content of an open file.");
	jsval jsvalFileName;
	JL_CHK( JS_GetReservedSlot(cx, obj, SLOT_JSIO_FILE_NAME, &jsvalFileName) ); // (TBD) add somthing like J_SCHK instead
	JL_S_ASSERT_DEFINED( jsvalFileName );
	const char *fileName;
	JL_CHK( JsvalToString(cx, &jsvalFileName, &fileName) );

	PRStatus status;
	status = PR_Access(fileName, PR_ACCESS_READ_OK); // We want to read the whole file, then first check if the file is readable
	if (unlikely( status != PR_SUCCESS )) {

		*vp = JSVAL_VOID;
		return JS_TRUE;
	}

	PRFileDesc *fd;
	fd = PR_OpenFile(fileName, PR_RDONLY, 0); // The mode parameter is currently applicable only on Unix platforms.
	if (unlikely( fd == NULL )) {

		PRErrorCode err = PR_GetError();
		if ( err == PR_FILE_NOT_FOUND_ERROR ) {

			*vp = JSVAL_VOID;
			return JS_TRUE;
		}
		return ThrowIoErrorArg( cx, err, PR_GetOSError() );
	}

	PRInt32 available;
	available = PR_Available( fd ); // For a normal file, these are the bytes beyond the current file pointer.
	if (unlikely( available == -1 ))
		return ThrowIoError(cx);

	char *buf;
	buf = (char*)JS_malloc(cx, available +1);
	JL_CHKB(buf, bad_free);

	PRInt32 res;
	res = PR_Read(fd, buf, available);
	if (unlikely( res == -1 )) {

		JS_free(cx, buf);
		return ThrowIoError(cx);
	}

	if (unlikely( PR_Close(fd) != PR_SUCCESS )) {

		JS_free(cx, buf);
		return ThrowIoError(cx);
	}

	if (unlikely( res == 0 )) {

		JS_free(cx, buf);
		*vp = JS_GetEmptyStringValue(cx);
		return JS_TRUE;
	}

	if ( JL_MaybeRealloc( available, res ) ) { // should never occured

		buf = (char*)JS_realloc(cx, buf, res +1); // realloc the string using its real size
		JL_CHK(buf);
	}

	buf[res] = '\0';
	JL_CHKB( JL_NewBlob( cx, buf, res, vp ), bad_free );
	return JS_TRUE;

bad_free:
	JS_free(cx, buf);
bad:
	return JS_FALSE;
}


DEFINE_PROPERTY( contentSetter ) {

	JL_S_ASSERT( (PRFileDesc *)JL_GetPrivate( cx, obj ) == NULL, "Cannot set content of an open file.");
	jsval jsvalFileName;
	JS_GetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, &jsvalFileName );
	JL_S_ASSERT_DEFINED( jsvalFileName );
	const char *fileName;
	JL_CHK( JsvalToString(cx, &jsvalFileName, &fileName) );
	if ( JSVAL_IS_VOID( *vp ) ) {

		if ( PR_Delete(fileName) != PR_SUCCESS ) {

			PRErrorCode err = PR_GetError();
			if ( err == PR_FILE_NOT_FOUND_ERROR )
				return JS_TRUE; // property will return  undefined
			return ThrowIoErrorArg( cx, err, PR_GetOSError() );
		}
		return JS_TRUE;
	}
	PRFileDesc *fd;
	fd = PR_OpenFile( fileName, PR_CREATE_FILE | PR_TRUNCATE | PR_WRONLY, PR_IRUSR + PR_IWUSR ); // The mode parameter is currently applicable only on Unix platforms.
	if ( fd == NULL )
		return ThrowIoError(cx);
	const char *buf;
	size_t len;
	JL_CHK( JsvalToStringAndLength(cx, vp, &buf, &len) );
	PRInt32 bytesSent;
	bytesSent = PR_Write( fd, buf, len );
	if ( bytesSent == -1 )
		return ThrowIoError(cx);
	JL_S_ASSERT( bytesSent == len, "unable to set content" );
	if ( PR_Close(fd) != PR_SUCCESS )
		return ThrowIoError(cx);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Contains the name of the file. Changing this value will rename or move the file.
**/
DEFINE_PROPERTY( nameGetter ) {

	JS_GetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, vp );
	return JS_TRUE;
}

DEFINE_PROPERTY( nameSetter ) {

	JL_S_ASSERT_DEFINED( *vp );

	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
	JL_S_ASSERT( fd == NULL, "Cannot rename an open file.");
	jsval jsvalFileName;
	JS_GetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, &jsvalFileName );
	JL_S_ASSERT_DEFINED( jsvalFileName );
	const char *fromFileName, *toFileName;
	JL_CHK( JsvalToString(cx, &jsvalFileName, &fromFileName) ); // warning: GC on the returned buffer !
	JL_CHK( JsvalToString(cx, vp, &toFileName) ); // warning: GC on the returned buffer !
	if ( PR_Rename(fromFileName, toFileName) != PR_SUCCESS ) // if status == PR_FILE_EXISTS_ERROR ...
		return ThrowIoError(cx);
	JS_SetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, *vp );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
  Contains true if the file exists.
**/
DEFINE_PROPERTY( exist ) {

	jsval jsvalFileName;
//	JS_GetProperty( cx, obj, "fileName", &jsvalFileName );
	JS_GetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, &jsvalFileName );
	JL_S_ASSERT_DEFINED( jsvalFileName );
	const char *fileName;
	JL_CHK( JsvalToString(cx, &jsvalFileName, &fileName) );
	PRStatus status;
	status = PR_Access( fileName, PR_ACCESS_EXISTS );
	*vp = BOOLEAN_TO_JSVAL( status == PR_SUCCESS );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME $READONLY
  This property works with opened and closed files.
  Contains an object that has the following properties:
   * type : Type of file.
   * size : Size, in bytes, of file's contents.
   * creationTime : Creation time (elapsed milliseconds since 1970.1.1).
   * modifyTime : Last modification time (elapsed milliseconds since 1970.1.1).
  $H note
   Time is relative to midnight (00:00:00), January 1, 1970 Greenwich Mean Time (GMT).
  $H beware
   File time resolution depends of hte underlying filesystem.
   $LF
   eg. the resolution of create time on FAT is 10 milliseconds, while write time has a resolution of 2 seconds and access time has a resolution of 1 day.
  $H example
   {{{
   var f = new File('test.txt');
   f.content = '123';
   Print( new Date( f.info.modifyTime ) );
   }}}
**/
DEFINE_PROPERTY( info ) {

	PRFileInfo fileInfo;
	PRStatus status;

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate( cx, obj );
	if ( fd == NULL ) {

		jsval jsvalFileName;
		JS_GetReservedSlot( cx, obj, SLOT_JSIO_FILE_NAME, &jsvalFileName );
		JL_S_ASSERT_DEFINED( jsvalFileName );
		const char *fileName;
		JL_CHK( JsvalToString(cx, &jsvalFileName, &fileName) );

		status = PR_GetFileInfo( fileName, &fileInfo );
	} else
		status = PR_GetOpenFileInfo( fd, &fileInfo );

	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);

	JSObject *fileInfoObj;
	if ( JSVAL_IS_OBJECT(*vp) ) {

		fileInfoObj = JSVAL_TO_OBJECT(*vp);
	} else {

		fileInfoObj = JS_NewObject( cx, NULL, NULL, NULL );
		*vp = OBJECT_TO_JSVAL( fileInfoObj );
	}

	JL_CHK( JS_DefineProperty(cx, fileInfoObj, "type", INT_TO_JSVAL((int)fileInfo.type), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT | JSPROP_ENUMERATE) );

	JL_CHK( JS_DefineProperty(cx, fileInfoObj, "size", INT_TO_JSVAL(fileInfo.size), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT | JSPROP_ENUMERATE) );

	jsval dateValue;

	JL_CHK( JS_NewNumberValue(cx, fileInfo.creationTime / (jsdouble)1000, &dateValue) );
	JL_CHK( JS_DefineProperty(cx, fileInfoObj, "creationTime", dateValue, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT | JSPROP_ENUMERATE) );

	JL_CHK( JS_NewNumberValue(cx, fileInfo.modifyTime / (jsdouble)1000, &dateValue) );
	JL_CHK( JS_DefineProperty(cx, fileInfoObj, "modifyTime", dateValue, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT | JSPROP_ENUMERATE) );

	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Static properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $TYPE File *stdin* $READONLY
  Is a jsio::File that represents the standard input.

 $TYPE File *stdout* $READONLY
  Is a jsio::File that represents the standard output.

 $TYPE File *stderr* $READONLY
  Is a jsio::File that represents the standard error.
**/
DEFINE_PROPERTY( standard ) {

	if ( JSVAL_IS_VOID( *vp ) ) {

		int32 i;
		JS_ValueToInt32( cx, id, &i );

		JSObject *obj = JS_NewObject(cx, classFile, NULL, NULL ); // no need to use classDescriptor as proto.
		*vp = OBJECT_TO_JSVAL( obj );

		PRFileDesc *fd = PR_GetSpecialFD((PRSpecialFD)i); // beware: cast !
		if ( fd == NULL )
			return ThrowIoError(cx);
		JL_SetPrivate( cx, obj, fd );

		JS_SetReservedSlot(cx, obj, SLOT_JSIO_DESCRIPTOR_IMPORTED, JSVAL_TRUE); // avoid PR_Close
	}
	return JS_TRUE;
}

/**doc
=== Constants ===
**/

/**doc
$TOC_MEMBER $INAME
 Open mode
  $CONST RDONLY

  $CONST WRONLY

  $CONST RDWR

  $CONST CREATE_FILE

  $CONST APPEND

  $CONST TRUNCATE

  $CONST SYNC

  $CONST EXCL

 Seek mode
  $CONST SEEK_SET

  $CONST SEEK_CUR

  $CONST SEEK_END

 info.type
  $CONST FILE_FILE

  $CONST FILE_DIRECTORY

  $CONST FILE_OTHER

**/


/**doc
=== Native Interface ===
 * *NIStreamRead*
  Read the file as a stream.
**/


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
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
		PROPERTY_READ_STORE( info )
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
=== Example 1 ===
{{{
function Copy(fromFilename, toFilename) {

 var fromFile = new File(fromFilename).Open(File.RDONLY);
 var toFile = new File(toFilename).Open(File.WRONLY | File.CREATE_FILE | File.TRUNCATE);
 for ( var buf; buf = fromFile.Read(65536); )
  toFile.Write(buf);
 toFile.Close();
 fromFile.Close();
}
}}}

=== Example 2 ===
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
