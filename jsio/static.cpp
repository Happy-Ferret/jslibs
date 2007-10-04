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

#include "static.h"

#include "error.h"
#include "descriptor.h"

BEGIN_STATIC

DEFINE_FUNCTION( Poll ) {

	uintN objCount;
	PRInt32 result;

	// NSPR Poll Method:
	//   http://www.mozilla.org/projects/nspr/tech-notes/poll-method.html

	// http://developer.mozilla.org/en/docs/PR_Poll

	PRPollDesc pollDesc[1024];

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_ARRAY( argv[0] );

	JSIdArray *idArray = JS_Enumerate( cx, JSVAL_TO_OBJECT(argv[0]) ); // make a kind of auto-ptr for this

	if ( idArray->length > (signed)(sizeof(pollDesc) / sizeof(PRPollDesc)) ) {

		JS_ReportError( cx, "Too many descriptors in Poll" );
		goto failed;
	}

	PRIntervalTime pr_timeout;
	if ( argc >= 2 && argv[1] != JSVAL_VOID ) {

		uint32 timeout;
		JS_ValueToECMAUint32( cx, argv[1], &timeout );
		pr_timeout = PR_MillisecondsToInterval(timeout);
	} else {

		pr_timeout = PR_INTERVAL_NO_TIMEOUT;
	}

	uintN i;
	objCount = idArray->length;
	for ( i = 0; i < objCount; i++ ) {

		jsval propVal;
		RT_CHECK_CALL( JS_IdToValue( cx, idArray->vector[i], &propVal ) );
		RT_CHECK_CALL( JS_GetElement(cx, JSVAL_TO_OBJECT(argv[0]), JSVAL_TO_INT(propVal), &propVal ) );
		RT_ASSERT_OBJECT( propVal );
		JSObject *fdObj = JSVAL_TO_OBJECT( propVal );
		RT_ASSERT( InheritFrom(cx, fdObj, &classDescriptor), RT_ERROR_INVALID_CLASS );
		PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, fdObj );
//		RT_ASSERT_RESOURCE( fd ); // fd == NULL is supported !

		pollDesc[i].fd = fd; // fd is A pointer to a PRFileDesc object representing a socket or a pollable event.  This field can be set to NULL to indicate to PR_Poll that this PRFileDesc object should be ignored.
		pollDesc[i].in_flags = 0;
		pollDesc[i].out_flags = 0;
	
		jsval prop;
		JS_GetProperty( cx, fdObj, "readable", &prop );
		if ( prop != JSVAL_VOID )
			pollDesc[i].in_flags |= PR_POLL_READ;

		JS_GetProperty( cx, fdObj, "writable", &prop );
		if ( prop != JSVAL_VOID )
			pollDesc[i].in_flags |= PR_POLL_WRITE;

		JS_GetProperty( cx, fdObj, "exception", &prop );
		if ( prop != JSVAL_VOID )
			pollDesc[i].in_flags |= PR_POLL_EXCEPT;

		JS_GetProperty( cx, fdObj, "error", &prop );
		if ( prop != JSVAL_VOID )
			pollDesc[i].in_flags |= PR_POLL_ERR;
	}

	result = PR_Poll( pollDesc, objCount, pr_timeout );
	if ( result == -1 ) {  // failed. see PR_GetError()

		ThrowIoError(cx);
		goto failed;
	}

	for ( i = 0; i < objCount; i++ ) {

		jsval arrayItem;
		RT_CHECK_CALL( JS_IdToValue(cx, idArray->vector[i], &arrayItem) );
		RT_CHECK_CALL( JS_GetElement(cx, JSVAL_TO_OBJECT(argv[0]), JSVAL_TO_INT(arrayItem), &arrayItem) );
		if ( arrayItem == JSVAL_VOID ) // socket has been removed from the list while js func "poll()" is runing
			continue;
		JSObject *fdObj = JSVAL_TO_OBJECT( arrayItem ); //JS_ValueToObject
		*rval = OBJECT_TO_JSVAL( fdObj ); // protect from GC

		jsval prop, ret;
		if ( result > 0 ) { // no timeout

			JS_GetProperty( cx, fdObj, "error", &prop );
			if ( pollDesc[i].out_flags & PR_POLL_ERR && JS_TypeOfValue( cx, prop ) == JSTYPE_FUNCTION )
				if ( JS_CallFunctionValue( cx, fdObj, prop, 1, rval, &ret ) == JS_FALSE ) // JS_CallFunction() DO NOT WORK !!!
					goto failed;

			JS_GetProperty( cx, fdObj, "exception", &prop );
			if ( pollDesc[i].out_flags & PR_POLL_EXCEPT && JS_TypeOfValue( cx, prop ) == JSTYPE_FUNCTION )
				if ( JS_CallFunctionValue( cx, fdObj, prop, 1, rval, &ret ) == JS_FALSE ) // JS_CallFunction() DO NOT WORK !!!
					goto failed;

/*
			JS_GetProperty( cx, fdObj, "nval", &prop );
			if ( pollDesc[i].out_flags & PR_POLL_NVAL && JS_TypeOfValue( cx, prop ) == JSTYPE_FUNCTION )
				if ( JS_CallFunctionValue( cx, fdObj, prop, 1, rval, &ret ) == JS_FALSE ) // JS_CallFunction() DO NOT WORK !!!
					goto failed;

			JS_GetProperty( cx, fdObj, "hup", &prop );
			if ( pollDesc[i].out_flags & PR_POLL_HUP && JS_TypeOfValue( cx, prop ) == JSTYPE_FUNCTION )
				if ( JS_CallFunctionValue( cx, fdObj, prop, 1, rval, &ret ) == JS_FALSE ) // JS_CallFunction() DO NOT WORK !!!
					goto failed;
*/

			JS_GetProperty( cx, fdObj, "writable", &prop );
			if ( pollDesc[i].out_flags & PR_POLL_WRITE && JS_TypeOfValue( cx, prop ) == JSTYPE_FUNCTION )
				if ( JS_CallFunctionValue( cx, fdObj, prop, 1, rval, &ret ) == JS_FALSE ) // JS_CallFunction() DO NOT WORK !!!
					goto failed;

			JS_GetProperty( cx, fdObj, "readable", &prop );
			if ( pollDesc[i].out_flags & PR_POLL_READ && JS_TypeOfValue( cx, prop ) == JSTYPE_FUNCTION )
				if ( JS_CallFunctionValue( cx, fdObj, prop, 1, rval, &ret ) == JS_FALSE ) // JS_CallFunction() DO NOT WORK !!!
					goto failed;

		}
	}

	*rval = INT_TO_JSVAL( result );
	JS_DestroyIdArray(cx, idArray);
	return JS_TRUE;

failed: // goto is the cheaper solution
	JS_DestroyIdArray( cx, idArray );
	return JS_FALSE;
}

DEFINE_FUNCTION( IsReadable ) {

	RT_ASSERT_ARGC( 1 );

	JSObject *descriptorObj = JSVAL_TO_OBJECT( argv[0] );
	RT_ASSERT( InheritFrom(cx, descriptorObj, &classDescriptor), RT_ERROR_INVALID_CLASS );
	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, descriptorObj );
//	RT_ASSERT_RESOURCE( fd ); // fd == NULL is supported !

	PRIntervalTime prTimeout;
	if ( argc >= 2 ) {

		uint32 timeout;
		RT_JSVAL_TO_UINT32( argv[1], timeout );
		prTimeout = PR_MillisecondsToInterval(timeout);
	} else
		prTimeout = PR_INTERVAL_NO_TIMEOUT;

	PRPollDesc desc = { fd, PR_POLL_READ, 0 };
	PRInt32 result = PR_Poll( &desc, 1, prTimeout );
	if ( result == -1 ) // error
		return ThrowIoError(cx);
	*rval = ( result == 1 && (desc.out_flags & PR_POLL_READ) != 0 ) ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
}


DEFINE_FUNCTION( IsWritable ) {

	RT_ASSERT_ARGC( 1 );

	JSObject *descriptorObj = JSVAL_TO_OBJECT( argv[0] );
	RT_ASSERT( InheritFrom(cx, descriptorObj, &classDescriptor), RT_ERROR_INVALID_CLASS );
	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, descriptorObj );
//	RT_ASSERT_RESOURCE( fd ); // fd == NULL is supported !

	PRIntervalTime prTimeout;
	if ( argc >= 2 ) {

		uint32 timeout;
		RT_JSVAL_TO_UINT32( argv[1], timeout );
		prTimeout = PR_MillisecondsToInterval(timeout);
	} else
		prTimeout = PR_INTERVAL_NO_TIMEOUT;

	PRPollDesc desc = { fd, PR_POLL_WRITE, 0 };
	PRInt32 result = PR_Poll( &desc, 1, prTimeout );
	if ( result == -1 ) // error
		return ThrowIoError(cx);
	*rval = ( result == 1 && (desc.out_flags & PR_POLL_WRITE) != 0 ) ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
}


DEFINE_FUNCTION( IntervalNow ) {

	PRUint32 interval = PR_IntervalToMilliseconds( PR_IntervalNow() );
	JS_NewNumberValue( cx, interval, rval );
	return JS_TRUE;
}


DEFINE_FUNCTION( Sleep ) {

	uint32 timeout;
	RT_CHECK_CALL( JS_ValueToECMAUint32( cx, argv[0], &timeout ) );
	PR_Sleep( PR_MillisecondsToInterval(timeout) );
	return JS_TRUE;
}


DEFINE_FUNCTION( GetEnv ) {

	RT_ASSERT_ARGC(1)
	char *name;
	RT_JSVAL_TO_STRING( argv[0], name );
	char* value = PR_GetEnv(name); // If the environment variable is not defined, the function returns NULL.
	if ( value != NULL ) { // this will cause an 'undefined' return value

		JSString *jsstr = JS_NewStringCopyZ(cx,value);
		RT_ASSERT_ALLOC( jsstr );
		*rval = STRING_TO_JSVAL(jsstr);
	}
	return JS_TRUE;
}


DEFINE_FUNCTION( HostName ) {

	char tmp[1024];
	PRStatus status = PR_GetSystemInfo( PR_SI_HOSTNAME, tmp, sizeof(tmp) );
	if ( status != PR_SUCCESS )
		JS_ReportError( cx, "unable to GetSystemInfo" );
	JSString *jsstr = JS_NewStringCopyZ(cx,tmp);
	RT_ASSERT_ALLOC( jsstr );
	*rval = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
}


DEFINE_FUNCTION( GetRandomNoise ) {

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_INT( argv[0] );
	PRSize rndSize = JSVAL_TO_INT( argv[0] );
	void *buf = (void*)JS_malloc(cx, rndSize);
	RT_ASSERT_ALLOC( buf );
	PRSize size = PR_GetRandomNoise(buf, rndSize);
	if ( size <= 0 ) {

		JS_free(cx, buf);
		REPORT_ERROR( "PR_GetRandomNoise is not implemented on this platform." );
	}
	JSString *jsstr = JS_NewString(cx, (char*)buf, size);
	RT_ASSERT_ALLOC( jsstr );
	*rval = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
}

/*
**      PR_ntohs        16 bit conversion from network to host
**      PR_ntohl        32 bit conversion from network to host
**      PR_ntohll       64 bit conversion from network to host
**      PR_htons        16 bit conversion from host to network
**      PR_htonl        32 bit conversion from host to network
**      PR_ntonll       64 bit conversion from host to network


DEFINE_FUNCTION( hton ) {
	
	RT_ASSERT_ARGC( 1 );	

	PRUint32 val;
	RT_JSVAL_TO_UINT32( argv[0], val );

	val = PR_ntohl(val);

	if ( 

	PR_htonll
	return JS_TRUE;
}

*/

CONFIGURE_STATIC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( Poll )
		FUNCTION( IsReadable )
		FUNCTION( IsWritable )
		FUNCTION( IntervalNow )
		FUNCTION( Sleep )
		FUNCTION( GetEnv )
		FUNCTION( HostName )
		FUNCTION( GetRandomNoise )
	END_STATIC_FUNCTION_SPEC

END_STATIC

