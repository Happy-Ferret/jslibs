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

#include <string.h>

#include "directory.h"


#define _SKIP_FILE 32
#define _SKIP_DIRECTORY 64
#define _SKIP_OTHER 128

/**doc
$CLASS_HEADER
 $SVN_REVISION $Revision$
 This class manages directory I/O Functions.
**/
BEGIN_CLASS( Directory )

DEFINE_FINALIZE() {

	PRDir *dd = (PRDir *)JS_GetPrivate( cx, obj );
	if ( dd != NULL ) {

		if ( PR_CloseDir(dd) != PR_SUCCESS ) // what to do on error ??
			JS_ReportError( cx, "a directory descriptor cannot be closed while Finalize" );
		JS_SetPrivate( cx, obj, NULL );
	}
}

/**doc
 * $INAME( directoryName )
  Creates a new Directory object.
  $H arguments
   $ARG string directoryName
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN( 1 );
	JS_SetReservedSlot( cx, obj, SLOT_JSIO_DIR_NAME, J_ARG(1) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Methods ===
**/

/**doc
 * $THIS $INAME()
  Open the directory.
**/
DEFINE_FUNCTION( Open ) {

	jsval jsvalDirectoryName;
	JS_GetReservedSlot( cx, obj, SLOT_JSIO_DIR_NAME, &jsvalDirectoryName );
	J_S_ASSERT_DEFINED( jsvalDirectoryName );
	const char *directoryName;
	J_CHK( JsvalToString(cx, &jsvalDirectoryName, &directoryName) );

	PRDir *dd;
	dd = PR_OpenDir( directoryName );
	if ( dd == NULL )
		return ThrowIoError(cx);

	JS_SetPrivate( cx, obj, dd );
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $VOID $INAME()
  Close the specified directory.
**/
DEFINE_FUNCTION( Close ) {

	PRDir *dd = (PRDir *)JS_GetPrivate( cx, obj );
	J_S_ASSERT( dd != NULL, "directory is closed" );
	if ( PR_CloseDir(dd) != PR_SUCCESS )
		return ThrowIoError(cx);
	JS_SetPrivate( cx, obj, NULL );
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $STR $INAME( [, flags = Directory.SKIP_NONE ] )
   Reads an item of the current directory and go to the next.
  $H arguments
   $ARG enum flags: specifies how special files are processed.
  $H return value
   returns a single directory item.
**/
DEFINE_FUNCTION( Read ) {

	PRDir *dd = (PRDir *)JS_GetPrivate( cx, obj );
	J_S_ASSERT( dd != NULL, "directory is closed" );

	PRDirFlags flags;
	flags = PR_SKIP_NONE;
	if ( J_ARG_ISDEF(1) ) {

		int32 tmp;
		JS_ValueToInt32( cx, J_ARG(1), &tmp );
		flags = (PRDirFlags)tmp;
	}

	PRDirEntry *dirEntry;
	dirEntry = PR_ReadDir( dd, flags );
	if ( dirEntry == NULL ) {

		PRErrorCode errorCode = PR_GetError();
		if ( errorCode == PR_NO_MORE_FILES_ERROR ) {

			*rval = JSVAL_VOID;
			return JS_TRUE;
		} else
			return ThrowIoError(cx);
	}

	*rval = STRING_TO_JSVAL(JS_NewStringCopyZ( cx, dirEntry->name ));
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $VOID $INAME()
  Create the directory given in the constructor.
**/
DEFINE_FUNCTION( Make ) {

	jsval jsvalDirectoryName;
	JS_GetReservedSlot( cx, obj, SLOT_JSIO_DIR_NAME, &jsvalDirectoryName );
	J_S_ASSERT_DEFINED( jsvalDirectoryName );
	const char *directoryName;
	J_CHK( JsvalToString(cx, &jsvalDirectoryName, &directoryName) );
	PRIntn mode;
	mode = 0766; // the permissions need to be set to 766 (linux uses the eXecute bit on directory as permission to allow access to a directory).
	if ( PR_MkDir(directoryName, mode) != PR_SUCCESS )
		return ThrowIoError(cx);
	return JS_TRUE;
	JL_BAD;
}

/**doc
 * $BOOL $INAME()
  Removes the directory given in the constructor.
  $H return value
   returns _false_ If the directory is not empty else it returns _true_.
**/
DEFINE_FUNCTION( Remove ) {

	jsval jsvalDirectoryName;
	JS_GetReservedSlot( cx, obj, SLOT_JSIO_DIR_NAME, &jsvalDirectoryName );
	J_S_ASSERT_DEFINED( jsvalDirectoryName );
	const char *directoryName;
	J_CHK( JsvalToString(cx, &jsvalDirectoryName, &directoryName) );

	if ( PR_RmDir(directoryName) != PR_SUCCESS ) { // PR_RmDir removes the directory specified by the pathname name. The directory must be empty. If the directory is not empty, PR_RmDir fails and PR_GetError returns the error code PR_DIRECTORY_NOT_EMPTY_ERROR.

		PRErrorCode errorCode = PR_GetError();
		if ( errorCode == PR_DIRECTORY_NOT_EMPTY_ERROR )
			*rval = JSVAL_FALSE;
		else
			return ThrowIoError(cx);
	} else {
			*rval = JSVAL_TRUE;
	}
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Properties ===
**/

/**doc
 * $INAME $READONLY
  Check if the directory exists.
**/
DEFINE_PROPERTY( exist ) {

	jsval jsvalDirectoryName;
	JS_GetReservedSlot( cx, obj, SLOT_JSIO_DIR_NAME, &jsvalDirectoryName );
	J_S_ASSERT_DEFINED( jsvalDirectoryName );
	const char *directoryName;
	J_CHK( JsvalToString(cx, &jsvalDirectoryName, &directoryName) );

	PRDir *dd;
	dd = PR_OpenDir( directoryName );

	if ( dd == NULL ) {

		*vp = JSVAL_FALSE;
	} else {

		if ( PR_CloseDir(dd) != PR_SUCCESS )
			return ThrowIoError(cx); // ??? Doc do not say it is possible to read PR_GetError after an error on PR_OpenDir !!!
		*vp = JSVAL_TRUE;
	}
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $INAME $READONLY
  Returns the name of the directory.
**/
DEFINE_PROPERTY( name ) {

	JS_GetReservedSlot( cx, obj, SLOT_JSIO_DIR_NAME, vp );
	return JS_TRUE;
}


/**doc
=== Static functions ===
**/

/**doc
 * $TYPE Array $INAME( dirName [, flags = Directory.SKIP_DOT] )
  Read all entries of a directory at once.
  $H arguments
   $ARG string dirName: is the path of the directory.
   $ARG enum flags: specifies how special files are processed.
  $H return value
   returns all entries in the directory _name_.
  $H note
   This function supports additional flags: Directory.`SKIP_FILE`, Directory.`SKIP_DIRECTORY`, Directory.`SKIP_OTHER`
  $H example
  {{{
  LoadModule('jsstd');
  LoadModule('jsio');
  Print( Directory.List('.').join('\n'), '\n' );
  }}}
**/
DEFINE_FUNCTION( List ) {

	J_S_ASSERT_ARG_MIN( 1 );
	const char *directoryName;
	size_t directoryNameLength;
	J_CHK( JsvalToStringAndLength(cx, &J_ARG(1), &directoryName, &directoryNameLength) );
	J_S_ASSERT( directoryNameLength < PATH_MAX, "Path too long" );
	PRDir *dd;
	dd = PR_OpenDir( directoryName );
	if ( dd == NULL )
		return ThrowIoError(cx);

	PRDirFlags flags;
	flags = PR_SKIP_DOT;
	if ( J_ARG_ISDEF( 2 ) ) {

		int32 tmp;
		J_CHK( JS_ValueToInt32( cx, J_ARG(2), &tmp ) );
		flags = (PRDirFlags)tmp;
	}

	JSObject *addrJsObj;
	addrJsObj = JS_NewArrayObject(cx, 0, NULL);
	J_S_ASSERT_ALLOC( addrJsObj );
	*rval = OBJECT_TO_JSVAL( addrJsObj );

	int index;
	index = 0;
	for (;;) {

		PRDirEntry *dirEntry = PR_ReadDir( dd, flags ); // & 0x0F
		if ( dirEntry == NULL ) {

			PRErrorCode errorCode = PR_GetError();
			if ( errorCode == PR_NO_MORE_FILES_ERROR )
				break;
			else
				return ThrowIoError(cx);
		}

		if ( flags & (_SKIP_FILE | _SKIP_DIRECTORY | _SKIP_OTHER) ) {

			PRFileInfo fileInfo;
			PRStatus status;

			char fileName[PATH_MAX];
			strcpy( fileName, directoryName );
			if ( directoryName[directoryNameLength-1] != '/' && directoryName[directoryNameLength-1] != '\\' )
				strcat( fileName, "/" );
			strcat( fileName, dirEntry->name );

			status = PR_GetFileInfo( fileName, &fileInfo );
			if ( status != PR_SUCCESS )
				return ThrowIoError(cx);

			if ( flags & _SKIP_FILE && fileInfo.type == PR_FILE_FILE ||
				  flags & _SKIP_DIRECTORY && fileInfo.type == PR_FILE_DIRECTORY ||
				  flags & _SKIP_OTHER && fileInfo.type == PR_FILE_OTHER )
				continue;
		}

		JSString *jsStr = JS_NewStringCopyZ( cx, dirEntry->name );
		J_S_ASSERT_ALLOC( jsStr );
		J_CHK(	JS_DefineElement(cx, addrJsObj, index++, STRING_TO_JSVAL(jsStr), NULL, NULL, JSPROP_ENUMERATE) );
	}

	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Constants ===
 * Directory.`SKIP_NONE`
  Do not skip any files.
 * Directory.`SKIP_DOT`
  Skip the directory entry "." representing the current directory.
 * Directory.`SKIP_DOT_DOT`
  Skip the directory entry ".." representing the parent directory.
 * Directory.`SKIP_BOTH`
  Skip both "." and ".." ( same as Directory.`SKIP_DOT` | Directory.`SKIP_DOT_DOT` )
 * Directory.`SKIP_HIDDEN`
  Skip hidden files. On Windows platforms and the Mac OS, this value identifies files with the "hidden" attribute set. On Unix platform, this value identifies files whose names begin with a period (".").
**/

CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( Open )
		FUNCTION( Close )
		FUNCTION( Read )
		FUNCTION( Make )
		FUNCTION( Remove )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( exist )
		PROPERTY_READ_STORE( name )
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( List )
	END_STATIC_FUNCTION_SPEC

	BEGIN_CONST_DOUBLE_SPEC
		CONST_DOUBLE(SKIP_NONE   ,PR_SKIP_NONE )
		CONST_DOUBLE(SKIP_DOT    ,PR_SKIP_DOT )
		CONST_DOUBLE(SKIP_DOT_DOT,PR_SKIP_DOT_DOT )
		CONST_DOUBLE(SKIP_BOTH   ,PR_SKIP_BOTH )
		CONST_DOUBLE(SKIP_HIDDEN ,PR_SKIP_HIDDEN )

		CONST_DOUBLE(SKIP_FILE, _SKIP_FILE )
		CONST_DOUBLE(SKIP_DIRECTORY, _SKIP_DIRECTORY )
		CONST_DOUBLE(SKIP_OTHER, _SKIP_OTHER )
	END_CONST_DOUBLE_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS

/**doc
=== Example ===
{{{
var dir = new Directory( 'c:/tmp' );
dir.Open();
for ( var entry; ( entry = dir.Read() ); ) {

   var file = new File(dir.name+'/'+entry);
   Print( entry + ' ('+ file.info.type +')', '\n');
}
}}}

=== Example ===
{{{
function RecursiveDir(path) {
   var testList = [];
   (function(path) {
      var dir = new Directory(path);
      dir.Open();
      for ( var entry; ( entry = dir.Read(Directory.SKIP_BOTH) ); ) {

         var file = new File(dir.name+'/'+entry);
         switch ( file.info.type ) {
            case File.FILE_DIRECTORY:
               arguments.callee(file.name);
               break;
            case File.FILE_FILE:
               testList.push(file.name);
               break;
         }
      }
      dir.Close();
   })(path);
   return testList;
}

Print( RecursiveDir('jshost').join('\n') );
}}}
**/
