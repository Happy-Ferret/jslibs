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

BEGIN_CLASS( Descriptor )

DEFINE_FUNCTION( Close ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT( fd != NULL, "file is closed." );
	PRStatus st = PR_Close( fd );
	if ( st == PR_FAILURE )
		return ThrowIoError( cx, PR_GetError() );
	JS_SetPrivate( cx, obj, NULL );
	return JS_TRUE;
}

DEFINE_FUNCTION( Read ) {

	RT_ASSERT_ARGC( 1 );
	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );
	PRInt32 amount;
	RT_JSVAL_TO_INT32( argv[0], amount );
	char *buf = (char*)JS_malloc( cx, amount +1 );
	RT_ASSERT_ALLOC(buf);
	buf[amount] = 0; // (TBD) check if useful: PR_Read can read binary data !

	PRInt32 res = PR_Read( fd, buf, amount );

	if (res == -1) { // failure. The reason for the failure can be obtained by calling PR_GetError.

		JS_free( cx, buf );
		return ThrowIoError( cx, PR_GetError() );
	}

	if (res == 0) {

		JS_free( cx, buf );
		*rval = JS_GetEmptyStringValue(cx); // (TBD) check if it is realy faster.
		return JS_TRUE;
	}

	JSString *str = JS_NewString( cx, (char*)buf, res );
	RT_ASSERT_ALLOC(str);
	*rval = STRING_TO_JSVAL(str); // GC protection is ok with this ?
	return JS_TRUE;
}


DEFINE_FUNCTION( Write ) {

	RT_ASSERT_ARGC(1);

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	RT_ASSERT_RESOURCE( fd );

	char *str;
	int len;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], str, len );

	PRInt32 bytesSent = PR_Write( fd, str, len );

	if ( bytesSent == -1 )
		return ThrowIoError( cx, PR_GetError() );

	if ( bytesSent < len )
		*rval = STRING_TO_JSVAL( JS_NewDependentString(cx, JSVAL_TO_STRING( argv[0] ), bytesSent, len-bytesSent) );
	else
		*rval = JS_GetEmptyStringValue(cx);

	return JS_TRUE;
}


CONFIGURE_CLASS

	BEGIN_FUNCTION_SPEC
		FUNCTION( Close )
		FUNCTION( Read )
		FUNCTION( Write )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
	END_PROPERTY_SPEC

END_CLASS;
