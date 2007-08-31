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
#include "directory.h"

BEGIN_CLASS( Directory )

DEFINE_FINALIZE() {

	PRDir *dd = (PRDir *)JS_GetPrivate( cx, obj );
	if ( dd != NULL ) {

		if ( PR_CloseDir(dd) != PR_SUCCESS ) // what to do on error ??
			JS_ReportError( cx, "a directory descriptor cannot be closed while Finalize" );
		JS_SetPrivate( cx, obj, NULL );
	}
}


DEFINE_CONSTRUCTOR() {
	
	RT_ASSERT_CONSTRUCTING( _class );
	RT_ASSERT_ARGC( 1 );
	JS_SetReservedSlot( cx, obj, SLOT_JSIO_DIR_NAME, argv[0] );
	return JS_TRUE;
}


DEFINE_FUNCTION( Open ) {

	jsval jsvalDirectoryName;
	JS_GetReservedSlot( cx, obj, SLOT_JSIO_DIR_NAME, &jsvalDirectoryName );
	RT_ASSERT_DEFINED( jsvalDirectoryName );
	char *directoryName;
	RT_JSVAL_TO_STRING(jsvalDirectoryName, directoryName);

	PRDir *dd = PR_OpenDir( directoryName );
	if ( dd == NULL )
		ThrowIoError(cx);

	JS_SetPrivate( cx, obj, dd );
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Close ) {

	PRDir *dd = (PRDir *)JS_GetPrivate( cx, obj );
	RT_ASSERT( dd != NULL, "directory is closed" );
	if ( PR_CloseDir(dd) != PR_SUCCESS )
		return ThrowIoError(cx);
	JS_SetPrivate( cx, obj, NULL );
	return JS_TRUE;
}


DEFINE_FUNCTION( Read ) {

	PRDir *dd = (PRDir *)JS_GetPrivate( cx, obj );
	RT_ASSERT( dd != NULL, "directory is closed" );

	PRDirFlags flags = PR_SKIP_NONE;
	if ( argc >= 1 ) {
		int32 tmp;
		JS_ValueToInt32( cx, argv[0], &tmp );
		flags = (PRDirFlags)tmp;
	}

	PRDirEntry *dirEntry = PR_ReadDir( dd, flags );
	if ( dirEntry == NULL ) {

		PRErrorCode errorCode = PR_GetError();
		if ( errorCode == PR_NO_MORE_FILES_ERROR ) {
			*rval = JSVAL_VOID;
			return JS_TRUE;
		} else
			ThrowIoError(cx);
	}

	*rval = STRING_TO_JSVAL(JS_NewStringCopyZ( cx, dirEntry->name ));
	return JS_TRUE;
}

DEFINE_FUNCTION( Make ) {

	jsval jsvalDirectoryName;
	JS_GetReservedSlot( cx, obj, SLOT_JSIO_DIR_NAME, &jsvalDirectoryName );
	RT_ASSERT_DEFINED( jsvalDirectoryName );
	char *directoryName;
	RT_JSVAL_TO_STRING(jsvalDirectoryName, directoryName);
	PRIntn mode = 0666;
	if ( PR_MkDir(directoryName, mode) != PR_SUCCESS )
		ThrowIoError(cx);
	return JS_TRUE;
}

DEFINE_FUNCTION( Remove ) {

	jsval jsvalDirectoryName;
	JS_GetReservedSlot( cx, obj, SLOT_JSIO_DIR_NAME, &jsvalDirectoryName );
	RT_ASSERT_DEFINED( jsvalDirectoryName );
	char *directoryName;
	RT_JSVAL_TO_STRING(jsvalDirectoryName, directoryName);

	if ( PR_RmDir(directoryName) != PR_SUCCESS ) { // PR_RmDir removes the directory specified by the pathname name. The directory must be empty. If the directory is not empty, PR_RmDir fails and PR_GetError returns the error code PR_DIRECTORY_NOT_EMPTY_ERROR.

		PRErrorCode errorCode = PR_GetError();
		if ( errorCode == PR_DIRECTORY_NOT_EMPTY_ERROR )
			*rval = JSVAL_FALSE;
		else
			ThrowIoError(cx);
	} else {
			*rval = JSVAL_TRUE;
	}
	return JS_TRUE;
}


DEFINE_PROPERTY( exist ) {

	jsval jsvalDirectoryName;
	JS_GetReservedSlot( cx, obj, SLOT_JSIO_DIR_NAME, &jsvalDirectoryName );
	RT_ASSERT_DEFINED( jsvalDirectoryName );
	char *directoryName;
	RT_JSVAL_TO_STRING(jsvalDirectoryName, directoryName);

	PRDir *dd = PR_OpenDir( directoryName );

	if ( dd == NULL ) {

		*vp = JSVAL_FALSE;
	} else {

		if ( PR_CloseDir(dd) != PR_SUCCESS )
			ThrowIoError(cx); // ??? Doc do not say it is possible to read PR_GetError after an error on PR_OpenDir !!!
		*vp = JSVAL_TRUE;
	}
	return JS_TRUE;
}

DEFINE_PROPERTY( name ) {

	JS_GetReservedSlot( cx, obj, SLOT_JSIO_DIR_NAME, vp );
	return JS_TRUE;
}


CONFIGURE_CLASS

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

	BEGIN_CONST_DOUBLE_SPEC
		CONST_DOUBLE(SKIP_NONE   ,PR_SKIP_NONE )
		CONST_DOUBLE(SKIP_DOT    ,PR_SKIP_DOT )
		CONST_DOUBLE(SKIP_DOT_DOT,PR_SKIP_DOT_DOT )
		CONST_DOUBLE(SKIP_BOTH   ,PR_SKIP_BOTH )
		CONST_DOUBLE(SKIP_HIDDEN ,PR_SKIP_HIDDEN )
	END_CONST_DOUBLE_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS
