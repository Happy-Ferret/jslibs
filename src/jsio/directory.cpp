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

	JL_IGNORE(fop);

	PRDir *dd = (PRDir*)js::GetObjectPrivate( obj );
	if ( dd != NULL ) {

		if ( PR_CloseDir(dd) != PR_SUCCESS ) {

			// JL_WARN( E_NAME(JL_THIS_CLASS_NAME), E_FIN ); // "a directory descriptor cannot be closed while Finalize" // (TBD) send to log !
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

	JL_DEFINE_ARGS;
	JL_ASSERT_CONSTRUCTING();
	
	{
	
	JL_DEFINE_CONSTRUCTOR_OBJ;
	JL_ASSERT_ARGC_MIN( 1 );

	JL_CHK( JL_SetReservedSlot(JL_OBJ, SLOT_JSIO_DIR_NAME, JL_ARG(1)) );
	
	}

	return true;
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

	JLData str;

	JL_DEFINE_ARGS;
	
	JS::RootedValue jsvalDirectoryName(cx);
	JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_JSIO_DIR_NAME, &jsvalDirectoryName) );
	JL_ASSERT_THIS_OBJECT_STATE( !jsvalDirectoryName.isUndefined() );
//	const char *directoryName;
//	JL_CHK( jl::getValue(cx, jsvalDirectoryName, &directoryName) );
	JL_CHK( jl::getValue(cx, jsvalDirectoryName, &str) );

	PRDir *dd;
	dd = PR_OpenDir( str.GetConstStrZ() );
	if ( dd == NULL )
		return ThrowIoError(cx);

	JL_SetPrivate(JL_OBJ, dd);
	JL_RVAL.setObject(*JL_OBJ);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Close the specified directory.
**/
DEFINE_FUNCTION( close ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	JL_RVAL.setUndefined();

	PRDir *dd;
	dd = (PRDir *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_WARN( dd, E_NAME(JL_THIS_CLASS_NAME), E_CLOSED );
	if ( !dd )
		return true;

	if ( PR_CloseDir(dd) != PR_SUCCESS )
		return ThrowIoError(cx);
	JL_SetPrivate(JL_OBJ, NULL);

	return true;
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

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	PRDir *dd;
	dd = (PRDir *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT( dd != NULL, E_NAME(JL_THIS_CLASS_NAME), E_CLOSED );

	PRDirFlags flags;
	flags = PR_SKIP_NONE;
	if ( JL_ARG_ISDEF(1) ) {

		int32_t tmp;
		JS::ToInt32( cx, JL_ARG(1), &tmp );
		flags = (PRDirFlags)tmp;
	}

	PRDirEntry *dirEntry;
	dirEntry = PR_ReadDir( dd, flags );
	if ( dirEntry == NULL ) {

		PRErrorCode errorCode = PR_GetError();
		if ( errorCode == PR_NO_MORE_FILES_ERROR ) {

			JL_RVAL.setUndefined();
			return true;
		} else
			return ThrowIoError(cx);
	}

	JL_RVAL.setString(JS_NewStringCopyZ(cx, dirEntry->name));
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Create the directory given in the constructor.
**/
DEFINE_FUNCTION( make ) {

	JL_IGNORE( argc );

	JLData str;
	JL_DEFINE_ARGS;
	
	JS::RootedValue jsvalDirectoryName(cx);
	JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_JSIO_DIR_NAME, &jsvalDirectoryName) );
	JL_ASSERT_THIS_OBJECT_STATE( !jsvalDirectoryName.isUndefined() );
	
//	const char *directoryName;
//	JL_CHK( jl::getValue(cx, jsvalDirectoryName, &directoryName) );
	JL_CHK( jl::getValue(cx, jsvalDirectoryName, &str) );

	PRIntn mode;
	mode = 0766; // the permissions need to be set to 766 (linux uses the eXecute bit on directory as permission to allow access to a directory).
	if ( PR_MkDir(str.GetConstStrZ(), mode) != PR_SUCCESS )
		return ThrowIoError(cx);

	JL_RVAL.setUndefined();
	return true;
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

	JL_IGNORE( argc );

	JLData str;
	JL_DEFINE_ARGS;
	
	JS::RootedValue jsvalDirectoryName(cx);
	JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_JSIO_DIR_NAME, &jsvalDirectoryName) );
	JL_ASSERT_THIS_OBJECT_STATE( !jsvalDirectoryName.isUndefined() );
	JL_CHK( jl::getValue(cx, jsvalDirectoryName, &str) );

	if ( PR_RmDir(str) != PR_SUCCESS ) { // PR_RmDir removes the directory specified by the pathname name. The directory must be empty. If the directory is not empty, PR_RmDir fails and PR_GetError returns the error code PR_DIRECTORY_NOT_EMPTY_ERROR.

		PRErrorCode errorCode = PR_GetError();
		if ( errorCode == PR_DIRECTORY_NOT_EMPTY_ERROR || errorCode == PR_READ_ONLY_FILESYSTEM_ERROR ) {

			JL_RVAL.setBoolean(false);
		} else {

			return ThrowIoError(cx);
		}
	} else {

		JL_RVAL.setBoolean(true);
	}
	return true;
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

	JL_IGNORE( id );

	JLData str;

	JS::RootedValue jsvalDirectoryName(cx);
	JL_CHK( JL_GetReservedSlot(  obj, SLOT_JSIO_DIR_NAME, &jsvalDirectoryName ) );
	JL_ASSERT_THIS_OBJECT_STATE( !jsvalDirectoryName.isUndefined() );
	JL_CHK( jl::getValue(cx, jsvalDirectoryName, &str) );

	PRDir *dd;
	dd = PR_OpenDir(str);

	if ( dd == NULL ) {
		
		if ( PR_GetError() == PR_FILE_NOT_FOUND_ERROR ) {
		
			vp.setBoolean(false);
			return true;
		} else {

			return ThrowIoError(cx);
		}
	} else {

		if ( PR_CloseDir(dd) != PR_SUCCESS )
			return ThrowIoError(cx);
		vp.setBoolean(true);
	}
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Returns the name of the directory.
**/
DEFINE_PROPERTY_GETTER( name ) {

	JL_CHK( JL_GetReservedSlot(  obj, SLOT_JSIO_DIR_NAME, vp ) );
	JL_CHK( jl::StoreProperty(cx, obj, id, vp, false) );
	return true;
	JL_BAD;
}


/**doc
=== Static functions ===
**/

/**doc
$TOC_MEMBER $INAME
 $TYPE Array $INAME( dirName [, flags = Directory.SKIP_DOT] [, showDirectoryChar] )
  Read all entries of a directory at once.
  $H arguments
   $ARG $STR dirName: is the path of the directory.
   $ARG $ENUM flags: specifies how special files are processed.
   $ARG $BOOL showDirectoryChar: if true, the directory names are ended with the directory separator (directorySeparator).
  $H return value
   All entries in the directory _name_.
  $H note
   This function supports additional flags: Directory.`SKIP_FILE`, Directory.`SKIP_DIRECTORY`, Directory.`SKIP_OTHER`
  $H example 1
  {{{
  loadModule('jsstd');
  loadModule('jsio');
  print( Directory.list('.').join('\n'), '\n' );
  }}}

  $H example 2
{{{
loadModule('jsstd');
loadModule('jsio');

(function(path) {

  for ( var name of Directory.list(path, Directory.SKIP_BOTH, true) ) {

    if ( name.substr(-1) == directorySeparator )

      arguments.callee(path+name);
    else

      print(path+name, '\n');
  }
})('.'+directorySeparator);
}}}
**/
DEFINE_FUNCTION( list ) {

	JLData directoryName;
	PRDir *dd = NULL;
	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_RANGE(1, 3);

	JL_CHK( jl::getValue(cx, JL_ARG(1), &directoryName) );
	JL_ASSERT( directoryName.Length() < PATH_MAX, E_ARG, E_NUM(1), E_MAX, E_NUM(PATH_MAX) );

	PRDirFlags flags;
	S_ASSERT( sizeof(PRDirFlags) == sizeof(int) );
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( jl::getValue(cx, JL_ARG(2), (int*)&flags) );
	else
		flags = PR_SKIP_DOT;

	bool showDirectoryChar;
	if ( JL_ARG_ISDEF(3) )
		JL_CHK( jl::getValue(cx, JL_ARG(3), &showDirectoryChar) );
	else
		showDirectoryChar = false;

	{
	JS::RootedObject addrJsObj(cx, JS_NewArrayObject(cx, 0));
	JL_CHK( addrJsObj );
	JL_RVAL.setObject(*addrJsObj);

	char dirSepChar;
	dirSepChar = PR_GetDirectorySeparator();

	char entryName[PATH_MAX];
	int entryNameLength;

	dd = PR_OpenDir(directoryName);
	JL_CHKB( dd, bad_throw );

	for ( unsigned int index = 0; ; ++index ) {

		PRDirEntry *dirEntry = PR_ReadDir(dd, flags); // & 0x0F
		if ( dirEntry == NULL ) {

			if ( PR_GetError() == PR_NO_MORE_FILES_ERROR ) { // reaching the end of the directory
				
				// PR_SetError(0, 0); needed ?
				break;
			}
			goto bad_throw; // or when an error occurs.
		}

		entryNameLength = strlen(dirEntry->name);
		strcpy(entryName, dirEntry->name);

		if ( showDirectoryChar || flags & (_SKIP_FILE | _SKIP_DIRECTORY | _SKIP_OTHER) ) {

			// add dir separator if not present
			char fileName[PATH_MAX];
			char *tmp = fileName;
			directoryName.CopyTo(tmp);
			tmp += directoryName.Length();

			char lastDirNameChar = directoryName.GetCharAt(directoryName.Length()-1);
			if ( lastDirNameChar != '/' && lastDirNameChar != dirSepChar ) {

				*tmp = dirSepChar;
				++tmp;
			}
			strcpy(tmp, entryName);


			PRFileInfo fileInfo;
			PRStatus status;
			status = PR_GetFileInfo(fileName, &fileInfo);
			JL_CHKB( status == PR_SUCCESS, bad_throw );

			if ( flags & _SKIP_FILE && fileInfo.type == PR_FILE_FILE )
				continue;

			if ( flags & _SKIP_DIRECTORY && fileInfo.type == PR_FILE_DIRECTORY )
				continue;

			if ( flags & _SKIP_OTHER && fileInfo.type == PR_FILE_OTHER )
				continue;

			if ( fileInfo.type == PR_FILE_DIRECTORY ) {
				
				entryName[entryNameLength] = dirSepChar;
				entryName[++entryNameLength] = '\0';
			}
		}

		JL_CHK( jl::setElement(cx, addrJsObj, index, jl::strSpec(entryName, entryNameLength)) );
	}

	JL_CHKB( PR_CloseDir(dd) == PR_SUCCESS, bad_throw);

	}

	return true;

bad_throw:
	ThrowIoError(cx);
	if (dd)
		PR_CloseDir(dd);
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


/*
static bool getListNext(JSContext *cx, unsigned argc, jsval *vp) {

	return true;
}

DEFINE_FUNCTION( getList ) {

	JL_IGNORE( argc );

	JSObject *iterator = JL_NewObj(cx);
	JS_DefineFunctionById(cx, iterator, JLID(cx, next), getListNext, 0, 0); // JSPROP_PERMANENT | JSPROP_READONLY
	jl::setProperty(cx, iterator, 

	return true;
	JL_BAD;
}

JSObject *jl::CreateIterator(JSContext *cx, JSNative call, void *private) {
}
*/


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

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

	BEGIN_CONST
		CONST_INTEGER(SKIP_NONE   ,PR_SKIP_NONE )
		CONST_INTEGER(SKIP_DOT    ,PR_SKIP_DOT )
		CONST_INTEGER(SKIP_DOT_DOT,PR_SKIP_DOT_DOT )
		CONST_INTEGER(SKIP_BOTH   ,PR_SKIP_BOTH )
		CONST_INTEGER(SKIP_HIDDEN ,PR_SKIP_HIDDEN )

		CONST_INTEGER(SKIP_FILE, _SKIP_FILE )
		CONST_INTEGER(SKIP_DIRECTORY, _SKIP_DIRECTORY )
		CONST_INTEGER(SKIP_OTHER, _SKIP_OTHER )
	END_CONST

END_CLASS

/**doc
=== Example ===
{{{
var dir = new Directory( 'c:/tmp' );
dir.open();
for ( var entry; ( entry = dir.read() ); ) {

   var file = new File(dir.name+'/'+entry);
   print( entry + ' ('+ file.info.type +')', '\n');
}
}}}

=== Example ===
{{{
function recursiveDir(path) {

   var testList = [];
   (function(path) {

      var dir = new Directory(path);
      dir.open();
      for ( var entry; ( entry = dir.read(Directory.SKIP_BOTH) ); ) {

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
      dir.close();
   })(path);
   return testList;
}

print( recursiveDir('jshost').join('\n') );
}}}
**/
