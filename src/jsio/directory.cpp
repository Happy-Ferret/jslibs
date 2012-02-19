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

	PRDir *dd = (PRDir *)JL_GetPrivate( cx, obj );
	if ( dd != NULL ) {

		if ( PR_CloseDir(dd) != PR_SUCCESS ) {

			JL_WARN( E_NAME(JL_THIS_CLASS_NAME), E_FIN ); // "a directory descriptor cannot be closed while Finalize"
		}
	}

bad:
	return;
}

/**doc
$TOC_MEMBER $INAME
 $INAME( directoryName )
  Creates a new Directory object.
  $H arguments
   $ARG $STR directoryName
**/
DEFINE_CONSTRUCTOR() {

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_ASSERT_ARGC_MIN( 1 );
	JL_CHK( JL_SetReservedSlot( cx, obj, SLOT_JSIO_DIR_NAME, JL_ARG(1) ) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $THIS $INAME()
  Open the directory.
**/
DEFINE_FUNCTION( open ) {

	JLStr str;

	JL_DEFINE_FUNCTION_OBJ;

	jsval jsvalDirectoryName;
	JL_GetReservedSlot( cx, obj, SLOT_JSIO_DIR_NAME, &jsvalDirectoryName );
	JL_ASSERT_THIS_OBJECT_STATE( !JSVAL_IS_VOID(jsvalDirectoryName) );
//	const char *directoryName;
//	JL_CHK( JL_JsvalToNative(cx, jsvalDirectoryName, &directoryName) );
	JL_CHK( JL_JsvalToNative(cx, jsvalDirectoryName, &str) );

	PRDir *dd;
	dd = PR_OpenDir( str.GetConstStr() );
	if ( dd == NULL )
		return ThrowIoError(cx);

	JL_SetPrivate( cx, obj, dd );
	*JL_RVAL = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Close the specified directory.
**/
DEFINE_FUNCTION( close ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	*JL_RVAL = JSVAL_VOID;

	PRDir *dd = (PRDir *)JL_GetPrivate( cx, obj );
	JL_ASSERT_WARN( dd, E_NAME(JL_THIS_CLASS_NAME), E_CLOSED );
	if ( !dd )
		return JS_TRUE;

	if ( PR_CloseDir(dd) != PR_SUCCESS )
		return ThrowIoError(cx);
	JL_SetPrivate( cx, obj, NULL );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( [, flags = Directory.SKIP_NONE ] )
   Reads an item of the current directory and go to the next.
  $H arguments
   $ARG $ENUM flags: specifies how special files are processed.
  $H return value
   A single directory item.
**/
DEFINE_FUNCTION( read ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	PRDir *dd = (PRDir *)JL_GetPrivate( cx, obj );
	JL_ASSERT( dd != NULL, E_NAME(JL_THIS_CLASS_NAME), E_CLOSED );

	PRDirFlags flags;
	flags = PR_SKIP_NONE;
	if ( JL_ARG_ISDEF(1) ) {

		int32_t tmp;
		JS_ValueToInt32( cx, JL_ARG(1), &tmp );
		flags = (PRDirFlags)tmp;
	}

	PRDirEntry *dirEntry;
	dirEntry = PR_ReadDir( dd, flags );
	if ( dirEntry == NULL ) {

		PRErrorCode errorCode = PR_GetError();
		if ( errorCode == PR_NO_MORE_FILES_ERROR ) {

			*JL_RVAL = JSVAL_VOID;
			return JS_TRUE;
		} else
			return ThrowIoError(cx);
	}

	*JL_RVAL = STRING_TO_JSVAL(JS_NewStringCopyZ( cx, dirEntry->name ));
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Create the directory given in the constructor.
**/
DEFINE_FUNCTION( make ) {

	JLStr str;
	JL_DEFINE_FUNCTION_OBJ;

	jsval jsvalDirectoryName;
	JL_GetReservedSlot( cx, obj, SLOT_JSIO_DIR_NAME, &jsvalDirectoryName );
	JL_ASSERT_THIS_OBJECT_STATE( !JSVAL_IS_VOID(jsvalDirectoryName) );
	
//	const char *directoryName;
//	JL_CHK( JL_JsvalToNative(cx, jsvalDirectoryName, &directoryName) );
	JL_CHK( JL_JsvalToNative(cx, jsvalDirectoryName, &str) );

	PRIntn mode;
	mode = 0766; // the permissions need to be set to 766 (linux uses the eXecute bit on directory as permission to allow access to a directory).
	if ( PR_MkDir(str.GetConstStr(), mode) != PR_SUCCESS )
		return ThrowIoError(cx);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME()
  Removes the directory given in the constructor.
  $H return value
   returns _false_ If the directory is not empty else it returns _true_.
**/
DEFINE_FUNCTION( remove ) {

	JLStr str;
	JL_DEFINE_FUNCTION_OBJ;

	jsval jsvalDirectoryName;
	JL_GetReservedSlot( cx, obj, SLOT_JSIO_DIR_NAME, &jsvalDirectoryName );
	JL_ASSERT_THIS_OBJECT_STATE( !JSVAL_IS_VOID(jsvalDirectoryName) );
//	const char *directoryName;
//	JL_CHK( JL_JsvalToNative(cx, jsvalDirectoryName, &directoryName) );
	JL_CHK( JL_JsvalToNative(cx, jsvalDirectoryName, &str) );

	if ( PR_RmDir(str.GetConstStr()) != PR_SUCCESS ) { // PR_RmDir removes the directory specified by the pathname name. The directory must be empty. If the directory is not empty, PR_RmDir fails and PR_GetError returns the error code PR_DIRECTORY_NOT_EMPTY_ERROR.

		PRErrorCode errorCode = PR_GetError();
		if ( errorCode == PR_DIRECTORY_NOT_EMPTY_ERROR )
			*JL_RVAL = JSVAL_FALSE;
		else
			return ThrowIoError(cx);
	} else {

			*JL_RVAL = JSVAL_TRUE;
	}
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Check if the directory exists.
**/
DEFINE_PROPERTY_GETTER( exist ) {

	JLStr str;

	jsval jsvalDirectoryName;
	JL_GetReservedSlot( cx, obj, SLOT_JSIO_DIR_NAME, &jsvalDirectoryName );
	JL_ASSERT_THIS_OBJECT_STATE( !JSVAL_IS_VOID(jsvalDirectoryName) );
//	const char *directoryName;
//	JL_CHK( JL_JsvalToNative(cx, jsvalDirectoryName, &directoryName) );
	JL_CHK( JL_JsvalToNative(cx, jsvalDirectoryName, &str) );

	PRDir *dd;
	dd = PR_OpenDir(str.GetConstStr());

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
$TOC_MEMBER $INAME
 $INAME $READONLY
  Returns the name of the directory.
**/
DEFINE_PROPERTY_GETTER( name ) {

	JL_CHK( JL_GetReservedSlot( cx, obj, SLOT_JSIO_DIR_NAME, vp ) );
	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


/**doc
=== Static functions ===
**/

/**doc
$TOC_MEMBER $INAME
 $TYPE Array $INAME( dirName [, flags = Directory.SKIP_DOT] )
  Read all entries of a directory at once.
  $H arguments
   $ARG $STR dirName: is the path of the directory.
   $ARG $ENUM flags: specifies how special files are processed.
  $H return value
   All entries in the directory _name_.
  $H note
   This function supports additional flags: Directory.`SKIP_FILE`, Directory.`SKIP_DIRECTORY`, Directory.`SKIP_OTHER`
  $H example
  {{{
  LoadModule('jsstd');
  LoadModule('jsio');
  Print( Directory.List('.').join('\n'), '\n' );
  }}}
**/
DEFINE_FUNCTION( list ) {

	JLStr directoryName;
	PRDir *dd = NULL;
	JL_ASSERT_ARGC_MIN( 1 );
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &directoryName, &directoryNameLength) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &directoryName) );

	JL_ASSERT( directoryName.Length() < PATH_MAX, E_ARG, E_NUM(1), E_MAX, E_NUM(PATH_MAX) );

	dd = PR_OpenDir(directoryName.GetConstStr());
	JL_CHKB( dd, bad_throw);

	PRDirFlags flags;
	flags = PR_SKIP_DOT;
	if ( JL_ARG_ISDEF( 2 ) ) {

		int32_t tmp;
		JL_CHK( JS_ValueToInt32( cx, JL_ARG(2), &tmp ) );
		flags = (PRDirFlags)tmp;
	}

	JSObject *addrJsObj;
	addrJsObj = JS_NewArrayObject(cx, 0, NULL);
	JL_CHK( addrJsObj );
	*JL_RVAL = OBJECT_TO_JSVAL( addrJsObj );

	int index;
	index = 0;
	for (;;) {

		PRDirEntry *dirEntry = PR_ReadDir( dd, flags ); // & 0x0F
		if ( dirEntry == NULL ) {

			if ( PR_GetError() == PR_NO_MORE_FILES_ERROR ) // reaching the end of the directory
				break;
			goto bad_throw; // or when an error occurs.
		}

		if ( flags & (_SKIP_FILE | _SKIP_DIRECTORY | _SKIP_OTHER) ) {

			PRFileInfo fileInfo;
			PRStatus status;

			char fileName[PATH_MAX];
			strcpy( fileName, directoryName );
			if ( directoryName.GetConstStr()[directoryName.Length()-1] != '/' && directoryName.GetConstStr()[directoryName.Length()-1] != '\\' )
				strcat( fileName, "/" );
			strcat( fileName, dirEntry->name );

			status = PR_GetFileInfo( fileName, &fileInfo );
			JL_CHKB( status == PR_SUCCESS, bad_throw );

			if ( ((flags & _SKIP_FILE) && fileInfo.type == PR_FILE_FILE) ||
				  ((flags & _SKIP_DIRECTORY) && fileInfo.type == PR_FILE_DIRECTORY) ||
				  ((flags & _SKIP_OTHER) && fileInfo.type == PR_FILE_OTHER) )
				continue;
		}

		JSString *jsStr = JS_NewStringCopyZ( cx, dirEntry->name );
		JL_CHK( jsStr );
		JL_CHK( JS_DefineElement(cx, addrJsObj, index++, STRING_TO_JSVAL(jsStr), NULL, NULL, JSPROP_ENUMERATE) );
	}

	JL_CHKB( PR_CloseDir(dd) == PR_SUCCESS, bad_throw);
	return JS_TRUE;

bad_throw:
	if (dd)
		PR_CloseDir(dd);
	ThrowIoError(cx);
	JL_BAD;
}

/**doc
=== Constants ===
 $CONST SKIP_NONE
  Do not skip any files.
 $CONST SKIP_DOT
  Skip the directory entry "." representing the current directory.
 $CONST SKIP_DOT_DOT
  Skip the directory entry ".." representing the parent directory.
 $CONST SKIP_BOTH
  Skip both "." and ".." ( same as Directory.`SKIP_DOT` | Directory.`SKIP_DOT_DOT` )
 $CONST SKIP_HIDDEN
  Skip hidden files. On Windows platforms and the Mac OS, this value identifies files with the "hidden" attribute set. On Unix platform, this value identifies files whose names begin with a period (".").
**/

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( open )
		FUNCTION( close )
		FUNCTION( read )
		FUNCTION( make )
		FUNCTION( remove )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( exist )
		PROPERTY_GETTER( name )
	END_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( list )
	END_STATIC_FUNCTION_SPEC

	BEGIN_CONST_INTEGER_SPEC
		CONST_INTEGER(SKIP_NONE   ,PR_SKIP_NONE )
		CONST_INTEGER(SKIP_DOT    ,PR_SKIP_DOT )
		CONST_INTEGER(SKIP_DOT_DOT,PR_SKIP_DOT_DOT )
		CONST_INTEGER(SKIP_BOTH   ,PR_SKIP_BOTH )
		CONST_INTEGER(SKIP_HIDDEN ,PR_SKIP_HIDDEN )

		CONST_INTEGER(SKIP_FILE, _SKIP_FILE )
		CONST_INTEGER(SKIP_DIRECTORY, _SKIP_DIRECTORY )
		CONST_INTEGER(SKIP_OTHER, _SKIP_OTHER )
	END_CONST_INTEGER_SPEC

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
