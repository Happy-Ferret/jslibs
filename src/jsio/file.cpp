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

#if defined(WIN)
	// see id function
	#include <winioctl.h>
#endif


#define DEFAULT_ACCESS_RIGHTS (PR_IRWXU | PR_IRWXG) // read, write, execute/search by owner & group ("770")

PRIntn FileOpenFlagsFromString( jl::BufString &str ) {

	size_t length = str.lengthOrZero();
	const char *strFlags = str.toData<const char*>();

	if ( length == 0 || length > 2 )
		return 0;

	if ( length == 2 && strFlags[1] != '+' )
		return 0;

	char c = strFlags[0];
	bool plus = length == 2 && strFlags[1] == '+';

	PRIntn flags;
	if ( c == 'r' ) {

		flags = plus ? PR_RDWR : PR_RDONLY;
	} else {

		if ( c == 'w' ) {

			flags = plus ? PR_CREATE_FILE | PR_TRUNCATE | PR_RDWR : PR_CREATE_FILE | PR_TRUNCATE | PR_WRONLY;
		} else {

			if ( c == 'a' ) {

				flags = plus ? PR_CREATE_FILE | PR_APPEND | PR_RDWR : PR_CREATE_FILE | PR_APPEND | PR_WRONLY;
			} else {

				flags = 0;
			}
		}
	}
	return flags;
}


PRIntn FileOpenModeFromString( jl::BufString &str ) {
	
	PRIntn mode = jl::atoi(str, 8);
	ASSERT( mode < (PR_IRWXU | PR_IRWXG | PR_IRWXO) );
	return mode;
}


/**doc
$CLASS_HEADER Descriptor
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( File )

DEFINE_FINALIZE() {
	
	FinalizeDescriptor(fop, obj); // defined in descriptor.cpp
}

/*
class Auto {
	int a;
	Auto(int a) : a(a) {}
	~Auto() {

	}


};
*/


/**doc
$TOC_MEMBER $INAME
 $INAME( fileName )
**/
DEFINE_CONSTRUCTOR() {

	JL_DEFINE_ARGS;
	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;
	JL_ASSERT_ARGC_RANGE(0,1);

	{
	JS::RootedValue filename(cx);
	if ( !JL_ARG_ISDEF(1) ) {
		
		char tempFileName[PATH_MAX];
		bool st = JLTemporaryFilename(tempFileName);
		JL_ASSERT( st, E_OS, E_INTERNAL );
		JL_CHK( jl::setValue(cx, &filename, tempFileName) );
	} else {
		
		filename.set(JL_ARG(1));
	}

	JL_CHK( JL_SetReservedSlot(JL_OBJ, SLOT_JSIO_FILE_NAME, filename) );

	JL_CHK( ReserveStreamReadInterface(cx, JL_OBJ) );

	ASSERT( JL_GetPrivate(JL_OBJ) == NULL ); // JL_SetPrivate( obj, NULL); // (TBD) optional ?
	}

	return true;
	JL_BAD;
}

/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME( [flags] [, mode] )
  Open a file for reading, writing, or both.
  $LF
  _flags_ is either a combinaison of open mode constants (see below) or a string that contains fopen like flags (+, r, w, a).
  If _flags_ is omitted, the file is open with CREATE_FILE + RDWR mode (r+).
  $LF
  _mode_ is an octal string (like '777') or a number that represents the mode.
  $LF
  The functions returns the file object itself (this), this allows to write things like: `new File('foo.txt').Open('r').Read();`
**/
DEFINE_FUNCTION( open ) {

	JS::RootedValue jsvalFileName(cx);
	jl::BufString fileName;

	JL_DEFINE_ARGS;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MAX(2);
	JL_ASSERT( JL_GetPrivate(JL_OBJ) == NULL, E_FILE, E_OPEN );

	PRIntn flags;
	if ( JL_ARG_ISDEF(1) ) {

		if ( JL_ARG(1).isNumber() ) {

			JL_CHK( jl::getValue(cx, JL_ARG(1), &flags) );
		} else {

			jl::BufString strFlags;
			JL_CHK( jl::getValue(cx, JL_ARG(1), &strFlags) );
			flags = FileOpenFlagsFromString(strFlags);
		}
	} else { // default

		flags = PR_CREATE_FILE | PR_RDWR;
	}

	PRIntn mode;
	if ( JL_ARG_ISDEF(2) ) {

		if ( JL_ARG(2).isNumber() ) {

			JL_CHK( jl::getValue(cx, JL_ARG(2), &mode) );
		} else {

			jl::BufString strMode;
			JL_CHK( jl::getValue(cx, JL_ARG(2), &strMode) );
			mode = FileOpenModeFromString(strMode);
		}
	}
	else { // default

		mode = DEFAULT_ACCESS_RIGHTS;
	}

	JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_JSIO_FILE_NAME, &jsvalFileName) );
	JL_ASSERT_THIS_OBJECT_STATE( !jsvalFileName.isUndefined() );
	JL_CHK( jl::getValue(cx, jsvalFileName, &fileName) );

	PRFileDesc *fd;
	fd = PR_OpenFile(fileName, flags, mode); // PR_OpenFile has the same prototype as PR_Open but implements the specified file mode where possible.

	if ( fd == NULL )
		return ThrowIoError(cx);
	JL_SetPrivate(JL_OBJ, fd);

	JL_CHK( SetStreamReadInterface(cx, JL_OBJ, NativeInterfaceStreamRead) );

	JL_RVAL.setObject(*JL_OBJ); // allows to write f.Open(...).Read()
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME( [ offset = 0 [, whence = File.`SEEK_SET` ] ] )
  Moves read-write file offset.
 $H note
  max offset is 2^53
**/
DEFINE_FUNCTION( seek ) {

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT( fd, E_THISOPERATION, E_INVALID, E_SEP, E_NAME(JL_THIS_CLASS_NAME), E_CLOSED );

	PRInt64 offset;
	if ( JL_ARG_ISDEF(1) ) {

		JL_ASSERT_ARG_IS_INTEGER_NUMBER(1);
		double doubleOffset;
		JL_CHK( jl::getValue( cx, JL_ARG(1), &doubleOffset ) );
		offset = (PRInt64)doubleOffset;
	} else
		offset = 0; // default is arg is missing

	PRSeekWhence whence;
	if ( JL_ARG_ISDEF(2) ) {

		int32_t tmp;
		JS::ToInt32( cx, JL_ARG(2), &tmp );
		whence = (PRSeekWhence)tmp;
	} else
		whence = PR_SEEK_CUR; // default is arg is missing

	PRInt64 ret;
	ret = PR_Seek64( fd, offset, whence );
	if ( ret == -1 )
		return ThrowIoError(cx);

	JL_ASSERT( jl::isInBounds<double>(ret), E_VALUE, E_PRECISION );

	JL_RVAL.setDouble((double)ret);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Delete a file from the filesystem.
  The operation may fail if the file is open.
**/
DEFINE_FUNCTION( delete ) {

	JL_IGNORE( argc );

	JS::RootedValue jsvalFileName(cx);
	jl::BufString str;

	JL_DEFINE_ARGS;
	JL_ASSERT_THIS_INSTANCE();

	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT( !fd, E_THISOPERATION, E_INVALID, E_SEP, E_NAME(JL_THIS_CLASS_NAME), E_OPEN );

	JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_JSIO_FILE_NAME, &jsvalFileName) );
	JL_ASSERT_THIS_OBJECT_STATE( !jsvalFileName.isUndefined() );
	JL_CHK( jl::getValue(cx, jsvalFileName, &str) );
	if ( PR_Delete(str) != PR_SUCCESS )
		return ThrowIoError(cx);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( state )
  Lock or unlock a file for exclusive access.
  _state_ can be _true_ or _false_.
**/
DEFINE_FUNCTION( lock ) {

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 1 );

	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( fd );
	bool doLock;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &doLock) );
	PRStatus st;
	st = doLock ? PR_LockFile(fd) : PR_UnlockFile(fd);
	if ( st != PR_SUCCESS )
		return ThrowIoError(cx);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( directory )
  Move the file to another directory.
**/
DEFINE_FUNCTION( move ) {

	JS::RootedValue jsvalFileName(cx);
	jl::BufString fileName, destDirName;
	
	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN( 1 );

	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT( !fd, E_THISOPERATION, E_INVALID, E_SEP, E_NAME(JL_THIS_CLASS_NAME), E_OPEN );

	JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_JSIO_FILE_NAME, &jsvalFileName) );
	JL_ASSERT_THIS_OBJECT_STATE( !jsvalFileName.isUndefined() );
	JL_CHK( jl::getValue(cx, jsvalFileName, &fileName) ); // warning: GC on the returned buffer !

//	const char *destDirName;
//	size_t destDirNameLength;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &destDirName, &destDirNameLength) ); // warning: GC on the returned buffer !
	JL_CHK( jl::getValue(cx, JL_ARG(1), &destDirName) ); // warning: GC on the returned buffer !

	const char *fileNameOnly;
	fileNameOnly = strrchr(fileName, '/');
	if ( fileNameOnly == NULL )
		fileNameOnly = fileName;

	JL_ASSERT( destDirName.length() + strlen(fileNameOnly) + 1 < PATH_MAX, E_ARG, E_NUM(1), E_MAX, E_NUM(PATH_MAX) );

	char destFileName[PATH_MAX];
	strcpy(destFileName, destDirName);
	strcat(destFileName, "/");
	strcat(destFileName, fileNameOnly);

	if ( PR_Rename(fileName, destFileName) != PR_SUCCESS )
		return ThrowIoError(cx);


	JSString *jsstr;
	jsstr = JS_NewStringCopyZ(cx, destFileName);
	JL_CHK( jsstr );
	jsvalFileName.setString(jsstr);

	JL_CHK( JL_SetReservedSlot(JL_OBJ, SLOT_JSIO_FILE_NAME, jsvalFileName) );

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}



/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Get or set the current position of the file pointer. Same as Seek() function used with SEEK_SET.
 $H note
  max position is 2^53
**/
DEFINE_PROPERTY_SETTER( position ) {

	JL_IGNORE( strict, id );

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate( obj );
	JL_ASSERT( fd, E_THISOPERATION, E_INVALID, E_SEP, E_NAME(JL_THIS_CLASS_NAME), E_CLOSED );

	JL_ASSERT_IS_INTEGER_NUMBER(vp, "");
	PRInt64 offset;
	double doubleOffset;
	JL_CHK( jl::getValue( cx, vp, &doubleOffset ) );
	offset = (PRInt64)doubleOffset;
	PRInt64 ret;
	ret = PR_Seek64( fd, offset, PR_SEEK_SET );
	if ( ret == -1 )
		return ThrowIoError(cx);
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( position ) {

	JL_DEFINE_PROP_ARGS;

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate( obj );
	JL_ASSERT( fd, E_THISOPERATION, E_INVALID, E_SEP, E_NAME(JL_THIS_CLASS_NAME), E_CLOSED );

	PRInt64 ret;
	ret = PR_Seek64( fd, 0, PR_SEEK_CUR );
	if ( ret == -1 )
		return ThrowIoError(cx);
//	JL_CHK( JL_NewNumberValue(cx, (double)ret, vp) );
//	return true;
	return jl::setValue(cx, JL_RVAL, ret);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Get or set the content of the file. If the file does not exist or is not readable, content is _undefined_.
  Setting content with _undefined_ deletes the file.
**/
DEFINE_PROPERTY_GETTER( content ) {

	JL_DEFINE_PROP_ARGS;

	JS::RootedValue jsvalFileName(cx);
	jl::BufBase buf;
	jl::BufString fileName;

	JL_ASSERT( !JL_GetPrivate(JL_OBJ), E_THISOPERATION, E_INVALID, E_SEP, E_NAME(JL_THIS_CLASS_NAME), E_OPEN );
	JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_JSIO_FILE_NAME, &jsvalFileName) ); // (TBD) add somthing like J_SCHK instead
	JL_ASSERT_THIS_OBJECT_STATE( !jsvalFileName.isUndefined() );
	JL_CHK( jl::getValue(cx, jsvalFileName, &fileName) );

/*
	PRStatus status;
	status = PR_Access(fileName, PR_ACCESS_READ_OK); // We want to read the whole file, then first check if the file is readable
	if (unlikely( status != PR_SUCCESS )) {

		*vp = JSVAL_VOID;
		return true;
	}
*/

	PRFileDesc *fd;
	fd = PR_Open(fileName, PR_RDONLY, 0);
	if (unlikely( fd == NULL )) {

		PRErrorCode err = PR_GetError();
		if ( err == PR_FILE_NOT_FOUND_ERROR ) {

			vp.setUndefined();
			return true;
		}
		return ThrowIoErrorArg(cx, err, PR_GetOSError());
	}

	PRInt64 available;
	available = PR_Available64(fd); // For a normal file, these are the bytes beyond the current file pointer.

	if ( available == 0 ) {

		PR_Close(fd);
		JL_CHK( BlobCreateEmpty(cx, vp) );
		return true;
	}

	if (unlikely( available == -1 )) {
		
		PR_Close(fd);
		return ThrowIoError(cx);
	}

	if ( available > PR_INT32_MAX ) {
		
		PR_Close(fd);
		JL_ERR( E_DATASIZE, E_MAX, E_STR("2^32") );
	}

	JL_CHKM( jl::isInBounds<size_t>(available), E_FILE, E_TOOBIG );
	buf.alloc((size_t)available);
	JL_ASSERT_ALLOC( buf );
	
	PRInt32 res;
	res = PR_Read(fd, buf.data(), buf.allocSize());
	if (unlikely( res == -1 )) {

		ThrowIoError(cx);
		PR_Close(fd);
		JL_CHK( false );
	}

	if (unlikely( PR_Close(fd) != PR_SUCCESS )) {

		JL_CHK( ThrowIoError(cx) );
	}

	//JL_CHK( JL_NewBufferGetOwnership(cx, buf, res, vp) );
	buf.setUsed(res);
	JL_CHK( BlobCreate(cx, buf, vp) );
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_SETTER( content ) {

	JL_IGNORE( strict, id );

	jl::BufString fileName;
	jl::BufString buf;
	JS::RootedValue jsvalFileName(cx);

	JL_CHK( JL_GetReservedSlot( obj, SLOT_JSIO_FILE_NAME, &jsvalFileName) );
	JL_ASSERT( !JL_GetPrivate(obj), E_THISOPERATION, E_INVALID, E_SEP, E_NAME(JL_THIS_CLASS_NAME), E_OPEN );
	JL_ASSERT_THIS_OBJECT_STATE( !jsvalFileName.isUndefined() );
	JL_CHK( jl::getValue(cx, jsvalFileName, &fileName) );

	if ( vp.isUndefined() ) {

		if ( PR_Delete(fileName) != PR_SUCCESS ) {

			PRErrorCode err = PR_GetError();
			if ( err == PR_FILE_NOT_FOUND_ERROR ) {

				return true; // property will return *undefined*
			}
			return ThrowIoErrorArg( cx, err, PR_GetOSError() );
		}
		return true;
	}

	PRFileDesc *fd;
	fd = PR_OpenFile(fileName, PR_CREATE_FILE | PR_TRUNCATE | PR_SYNC | PR_WRONLY, DEFAULT_ACCESS_RIGHTS); // The mode parameter is currently applicable only on Unix platforms.
	if ( fd == NULL )
		return ThrowIoError(cx);

	JL_CHK( jl::getValue(cx, vp, &buf) );
	PRInt32 bytesSent;

	ASSERT( buf.length() <= PR_INT32_MAX );
	bytesSent = PR_Write(fd, buf.toData<const uint8_t*>(), (PRInt32)buf.length());
	if ( bytesSent == -1 ) {
		
		PR_Close(fd);
		return ThrowIoError(cx);
	}

	//JL_ASSERT( bytesSent == (int)buf.length(), E_FILE, E_NAME(fileName), E_WRITE );
	if ( bytesSent != (int)buf.length() ) {

		JL_ERR( E_FILE, E_NAME(fileName), E_WRITE );
	}

	if ( PR_Close(fd) != PR_SUCCESS )
		return ThrowIoError(cx);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Contains the name of the file. Changing this value will rename or move the file.
**/
DEFINE_PROPERTY_GETTER( name ) {

	JL_IGNORE( id, cx );

	return JL_GetReservedSlot(  obj, SLOT_JSIO_FILE_NAME, vp );
}

DEFINE_PROPERTY_SETTER( name ) {

	JL_DEFINE_PROP_ARGS;

	JS::RootedValue jsvalFileName(cx);
	jl::BufString fromFileName, toFileName;

	PRFileDesc *fd;
	fd = (PRFileDesc *)JL_GetPrivate(JL_OBJ);

	JL_ASSERT( !fd, E_THISOPERATION, E_INVALID, E_SEP, E_NAME(JL_THIS_CLASS_NAME), E_OPEN );
	JL_ASSERT( !vp.isUndefined(), E_VALUE, E_DEFINED );

	JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_JSIO_FILE_NAME, &jsvalFileName) );
	JL_ASSERT_THIS_OBJECT_STATE( !jsvalFileName.isUndefined() );

	JL_CHK( jl::getValue(cx, jsvalFileName, &fromFileName) ); // warning: GC on the returned buffer !
	JL_CHK( jl::getValue(cx, vp, &toFileName) ); // warning: GC on the returned buffer !
	if ( PR_Rename(fromFileName, toFileName) != PR_SUCCESS ) // if status == PR_FILE_EXISTS_ERROR ...
		return ThrowIoError(cx);
	JL_CHK( JL_SetReservedSlot(JL_OBJ, SLOT_JSIO_FILE_NAME, vp) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
  is true if the file exists.
**/
DEFINE_PROPERTY_GETTER( exist ) {

	JL_DEFINE_PROP_ARGS;

	jl::BufString fileName;
	JS::RootedValue jsvalFileName(cx);
	JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_JSIO_FILE_NAME, &jsvalFileName) );
	JL_ASSERT_THIS_OBJECT_STATE( !jsvalFileName.isUndefined() );
	JL_CHK( jl::getValue(cx, jsvalFileName, &fileName) );
	return jl::setValue(cx, JL_RVAL, PR_Access( fileName, PR_ACCESS_EXISTS ) == PR_SUCCESS);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
  is true if the file is writable.
**/
DEFINE_PROPERTY_GETTER( hasWriteAccess ) {
	
	JL_DEFINE_PROP_ARGS;

	jl::BufString fileName;
	JS::RootedValue jsvalFileName(cx);
	JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_JSIO_FILE_NAME, &jsvalFileName) );
	JL_ASSERT_THIS_OBJECT_STATE( !jsvalFileName.isUndefined() );
	JL_CHK( jl::getValue(cx, jsvalFileName, &fileName) );
	return jl::setValue(cx, JL_RVAL, PR_Access( fileName, PR_ACCESS_WRITE_OK ) == PR_SUCCESS);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
  is true if the file is readable.
**/
DEFINE_PROPERTY_GETTER( hasReadAccess ) {

	JL_DEFINE_PROP_ARGS;

	jl::BufString fileName;
	JS::RootedValue jsvalFileName(cx);
	JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_JSIO_FILE_NAME, &jsvalFileName) );
	JL_ASSERT_THIS_OBJECT_STATE( !jsvalFileName.isUndefined() );
	JL_CHK( jl::getValue(cx, jsvalFileName, &fileName) );
	return jl::setValue(cx, JL_RVAL, PR_Access( fileName, PR_ACCESS_READ_OK ) == PR_SUCCESS);
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
   print( new Date( f.info.modifyTime ) );
   }}}
**/
DEFINE_PROPERTY_GETTER( info ) {

	JL_DEFINE_PROP_ARGS;

	PRFileInfo fileInfo;
	PRStatus status;

	PRFileDesc *fd = (PRFileDesc *)JL_GetPrivate( obj );
	if ( fd == NULL ) {

		jl::BufString fileName;
		JS::RootedValue jsvalFileName(cx);
		JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_JSIO_FILE_NAME, &jsvalFileName) );
		JL_ASSERT_THIS_OBJECT_STATE( !jsvalFileName.isUndefined() );
		JL_CHK( jl::getValue(cx, jsvalFileName, &fileName) );
		status = PR_GetFileInfo( fileName, &fileInfo );
	} else {

		status = PR_GetOpenFileInfo( fd, &fileInfo );
	}

	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);
	
	{
	
	JS::RootedObject fileInfoObj(cx);
	if ( vp.isObject() ) {

		fileInfoObj = &vp.toObject();
	} else {

		fileInfoObj = JL_NewObj(cx);
		JL_ASSERT(fileInfoObj);
		vp.setObject( *fileInfoObj );
	}

	JL_CHK( jl::setProperty(cx, fileInfoObj, "type", (int)fileInfo.type) );
	JL_CHK( jl::setProperty(cx, fileInfoObj, "size", fileInfo.size) );
	JL_CHK( jl::setProperty(cx, fileInfoObj, "creationTime", fileInfo.creationTime / (double)1000) );
	JL_CHK( jl::setProperty(cx, fileInfoObj, "modifyTime", fileInfo.modifyTime / (double)1000) );
	}
	
//	return jl::StoreProperty(cx, obj, id, vp, false); // file info may change between dwo calls
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( id ) {

	JL_DEFINE_PROP_ARGS;

	PRFileDesc *fd = (PRFileDesc*)JL_GetPrivate( obj );
	JL_ASSERT_THIS_OBJECT_STATE( fd );

#if defined(WIN)

	HANDLE fileHandle = (HANDLE)PR_FileDesc2NativeHandle(fd);
	JL_ASSERT_THIS_OBJECT_STATE( fileHandle && fileHandle != INVALID_HANDLE_VALUE );
	FILE_OBJECTID_BUFFER buf;
	DWORD cbOut;
	if ( !DeviceIoControl(fileHandle, FSCTL_CREATE_OR_GET_OBJECT_ID, NULL, 0, &buf, sizeof(buf), &cbOut, NULL) ) // WinBase.h
		return JL_ThrowOSError(cx);
	return jl::setValue(cx, JL_RVAL, jl::strSpec((const char *)&buf.ObjectId, sizeof(buf.ObjectId)));

#elif defined(UNIX)

	int fileHandle;
	fileHandle = (int)PR_FileDesc2NativeHandle(fd);

	struct stat fileStat;

	if ( fstat(fileHandle, &fileStat) == -1 )
		return JL_ThrowOSError(cx);

	return jl::setValue(cx, JL_RVAL, jl::strSpec( (const char *)&fileStat.st_ino, sizeof(fileStat.st_ino) ));

#else

	JL_ERR( E_API, E_NOTIMPLEMENTED );

#endif

	return true;
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
DEFINE_PROPERTY_GETTER( standard ) {

	JL_IGNORE( obj );

	if ( vp.isUndefined() ) {

		int i;
		//JS::ToInt32( cx, id, &i );
		i = JSID_TO_INT(id);
		JS::RootedObject obj(cx, jl::newObjectWithGivenProto(cx, JL_CLASS(File), JL_CLASS_PROTOTYPE(cx, File))); // no need to use classDescriptor as proto.
		vp.setObject( *obj );

		PRFileDesc *fd = PR_GetSpecialFD((PRSpecialFD)i); // beware: cast !
		if ( fd == NULL )
			return ThrowIoError(cx);
		JL_SetPrivate(  obj, fd );
		JL_CHK( JL_SetReservedSlot(obj, SLOT_JSIO_DESCRIPTOR_IMPORTED, JL_TRUE()) ); // avoid PR_Close
	}
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER_SWITCH( stdin, standard, PR_StandardInput );
DEFINE_PROPERTY_GETTER_SWITCH( stdout, standard, PR_StandardOutput );
DEFINE_PROPERTY_GETTER_SWITCH( stderr, standard, PR_StandardError );

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

	REVISION(jl::SvnRevToInt("$Revision$"))
	HAS_PROTOTYPE( Descriptor )

	HAS_CONSTRUCTOR_ARGC(1)
	HAS_FINALIZE

	HAS_PRIVATE
	HAS_RESERVED_SLOTS( 3 ) // SLOT_JSIO_DESCRIPTOR_IMPORTED, SLOT_JSIO_DESCRIPTOR_TIMEOUT, SLOT_JSIO_FILE_NAME

	BEGIN_FUNCTION_SPEC
		FUNCTION( open )
		FUNCTION( seek )
		FUNCTION( delete )
		FUNCTION( lock )
		FUNCTION( move )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( name )
		PROPERTY( content )
		PROPERTY( position )
		PROPERTY_GETTER( exist )
		PROPERTY_GETTER( hasWriteAccess )
		PROPERTY_GETTER( hasReadAccess )
		PROPERTY_GETTER( info )
		PROPERTY_GETTER( id )
	END_PROPERTY_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
//		PROPERTY_CREATE( stdin  ,PR_StandardInput  ,JSPROP_PERMANENT|JSPROP_READONLY, standard, NULL ) // (TBD) change this
//		PROPERTY_CREATE( stdout ,PR_StandardOutput ,JSPROP_PERMANENT|JSPROP_READONLY, standard, NULL ) // (TBD) change this
//		PROPERTY_CREATE( stderr ,PR_StandardError  ,JSPROP_PERMANENT|JSPROP_READONLY, standard, NULL ) // (TBD) change this
		PROPERTY_GETTER( stdin )
		PROPERTY_GETTER( stdout )
		PROPERTY_GETTER( stderr )
	END_STATIC_PROPERTY_SPEC

	BEGIN_CONST
// PR_Open flags
		CONST_INTEGER( RDONLY      ,PR_RDONLY )
		CONST_INTEGER( WRONLY      ,PR_WRONLY )
		CONST_INTEGER( RDWR        ,PR_RDWR )
		CONST_INTEGER( CREATE_FILE ,PR_CREATE_FILE )
		CONST_INTEGER( APPEND      ,PR_APPEND )
		CONST_INTEGER( TRUNCATE    ,PR_TRUNCATE )
		CONST_INTEGER( SYNC        ,PR_SYNC )
		CONST_INTEGER( EXCL        ,PR_EXCL )
// PRSeekWhence enum
		CONST_INTEGER( SEEK_SET	,PR_SEEK_SET )
		CONST_INTEGER( SEEK_CUR	,PR_SEEK_CUR )
		CONST_INTEGER( SEEK_END	,PR_SEEK_END )
// PRFileType enum
		CONST_INTEGER( FILE_FILE      ,PR_FILE_FILE )
		CONST_INTEGER( FILE_DIRECTORY ,PR_FILE_DIRECTORY )
		CONST_INTEGER( FILE_OTHER     ,PR_FILE_OTHER )
	END_CONST

END_CLASS

/**doc
=== Example 1 ===
{{{
function copy(fromFilename, toFilename) {

 var fromFile = new File(fromFilename).open(File.RDONLY);
 var toFile = new File(toFilename).open(File.WRONLY | File.CREATE_FILE | File.TRUNCATE);
 for ( var buf; buf = fromFile.read(65536); )
  toFile.write(buf);
 toFile.close();
 fromFile.close();
}
}}}

=== Example 2 ===
{{{
loadModule('jsstd');
loadModule('jsio');

try {

   var file = new File('file_test.txt');
   if ( file.exist ) {
      file.open( File.RDONLY );
      print( 'file content:\n' + file.read() );
      file.close();
   }

} catch ( ex if ex instanceof IoError ) {

   print( 'IOError: ' + ex.text, '\n' );
} catch( ex ) {

   throw ex;
}
}}}
**/
