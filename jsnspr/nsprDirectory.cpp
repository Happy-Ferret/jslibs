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

#include "nsprError.h"
#include "nsprDirectory.h"

BEGIN_CLASS( Directory )

DEFINE_FINALIZE() {

	PRDir *dd = (PRDir *)JS_GetPrivate( cx, obj );
	if ( dd != NULL ) {

		PRStatus status = PR_CloseDir( dd ); // what to do on error ??
		if ( status != PR_SUCCESS )
			JS_ReportError( cx, "a directory descriptor cannot be closed while Finalize" );
		JS_SetPrivate( cx, obj, NULL );
	}
}

DEFINE_CONSTRUCTOR() {

	if ( !JS_IsConstructing(cx) ) {

		JS_ReportError( cx, "need to be construct" );
		return JS_FALSE;
	}

	if ( argc < 1 ) {

		JS_ReportError( cx, "missing argument" );
		return JS_FALSE;
	}

	JS_SetReservedSlot( cx, obj, 0, argv[0] );
	return JS_TRUE;
}


DEFINE_FUNCTION( Open ) {

	jsval jsvalDirectoryName;
	JS_GetReservedSlot( cx, obj, 0, &jsvalDirectoryName );
	RT_ASSERT_DEFINED( jsvalDirectoryName );
	char *directoryName;
	RT_JSVAL_TO_STRING(jsvalDirectoryName, directoryName);

	PRDir *dd = PR_OpenDir( directoryName );

	if ( dd == NULL )
		return ThrowNSPRError( cx, PR_GetError() ); // ??? Doc do not say it is possible to read PR_GetError after an error on PR_OpenDir !!!
		// (TBD) check

	JS_SetPrivate( cx, obj, dd );
	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


DEFINE_FUNCTION( Close ) {

	PRDir *dd = (PRDir *)JS_GetPrivate( cx, obj );
	RT_ASSERT( dd != NULL, "directory is closed" );
	PRStatus status = PR_CloseDir( dd );
	if ( status != PR_SUCCESS )
		return ThrowNSPRError( cx, PR_GetError() );
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
			return ThrowNSPRError( cx, errorCode );
	}

	*rval = STRING_TO_JSVAL(JS_NewStringCopyZ( cx, dirEntry->name ));
	return JS_TRUE;
}

DEFINE_FUNCTION( Make ) {

	jsval jsvalDirectoryName;
	JS_GetReservedSlot( cx, obj, 0, &jsvalDirectoryName );
	RT_ASSERT_DEFINED( jsvalDirectoryName );
	char *directoryName;
	RT_JSVAL_TO_STRING(jsvalDirectoryName, directoryName);

	PRIntn mode = 0666;
	PRStatus status = PR_MkDir( directoryName, mode );
	if ( status != PR_SUCCESS )
		return ThrowNSPRError( cx, PR_GetError() );
	return JS_TRUE;
}

DEFINE_FUNCTION( Remove ) {

	jsval jsvalDirectoryName;
	JS_GetReservedSlot( cx, obj, 0, &jsvalDirectoryName );
	RT_ASSERT_DEFINED( jsvalDirectoryName );
	char *directoryName;
	RT_JSVAL_TO_STRING(jsvalDirectoryName, directoryName);

	PRStatus status = PR_RmDir( directoryName ); // PR_RmDir removes the directory specified by the pathname name. The directory must be empty. If the directory is not empty, PR_RmDir fails and PR_GetError returns the error code PR_DIRECTORY_NOT_EMPTY_ERROR.
	if ( status != PR_SUCCESS ) {

		PRErrorCode errorCode = PR_GetError();
		if ( errorCode == PR_DIRECTORY_NOT_EMPTY_ERROR )
			*rval = JSVAL_FALSE;
		else
			return ThrowNSPRError( cx, errorCode );
	} else {
			*rval = JSVAL_TRUE;
	}
	return JS_TRUE;
}


DEFINE_PROPERTY( exist ) {

	jsval jsvalDirectoryName;
	JS_GetReservedSlot( cx, obj, 0, &jsvalDirectoryName );
	RT_ASSERT_DEFINED( jsvalDirectoryName );
	char *directoryName;
	RT_JSVAL_TO_STRING(jsvalDirectoryName, directoryName);

	PRDir *dd = PR_OpenDir( directoryName );

	if ( dd == NULL ) {

		*vp = JSVAL_FALSE;
	} else {

		PRStatus status = PR_CloseDir(dd);
		if ( status != PR_SUCCESS )
			return ThrowNSPRError( cx, PR_GetError() ); // ??? Doc do not say it is possible to read PR_GetError after an error on PR_OpenDir !!!
		*vp = JSVAL_TRUE;
	}
	return JS_TRUE;
}

DEFINE_PROPERTY( name ) {

	JS_GetReservedSlot( cx, obj, 0, vp ); // (TBD) use the obj.name proprety directly instead of slot 0
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
