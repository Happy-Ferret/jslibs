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
#include "descriptor.h"
#include "file.h"

BEGIN_CLASS( File )

DEFINE_FINALIZE() {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd != NULL ) {
		
		jsval imported;
		JS_GetReservedSlot(cx, obj, SLOT_JSIO_DESCRIPTOR_IMPORTED, &imported);
		if ( imported != JSVAL_TRUE ) // Descriptor was inported, then do not close it
			PR_Close( fd );
		JS_SetPrivate( cx, obj, NULL );
	}
}


DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(_class);
	RT_ASSERT_ARGC(2);

	char *fileName;
	RT_JSVAL_TO_STRING( argv[0], fileName );

	PRIntn flags;
	RT_JSVAL_TO_INT32( argv[1], flags );

	PRIntn mode = PR_IRUSR + PR_IWUSR; // read write permission, owner

	PRFileDesc *fd = PR_Open(fileName, flags, mode);
	if ( fd == NULL )
		return ThrowIoError( cx, PR_GetError() );

	JS_SetPrivate(cx, obj, (void*)fd);

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}


CONFIGURE_CLASS
	
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_PROTOTYPE( prototypeDescriptor )
	HAS_RESERVED_SLOTS( 1 )
	HAS_PRIVATE

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
