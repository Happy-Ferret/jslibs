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

#include "descriptor.h"

BEGIN_STATIC

DEFINE_FUNCTION( Poll ) {

	PRInt32 result;

	// NSPR Poll Method:
	//   http://www.mozilla.org/projects/nspr/tech-notes/poll-method.html

	// http://developer.mozilla.org/en/docs/PR_Poll

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_ARRAY( J_ARG(1) );

	JSIdArray *idArray = JS_Enumerate( cx, JSVAL_TO_OBJECT(J_ARG(1)) ); // make a kind of auto-ptr for this

	PRIntervalTime pr_timeout;
	if ( J_ARG_ISDEF(2) ) {

		uint32 tmp;
		RT_JSVAL_TO_UINT32( J_ARG(2), tmp );
		pr_timeout = PR_MillisecondsToInterval(tmp);
	} else {

		pr_timeout = PR_INTERVAL_NO_TIMEOUT;
	}

	if ( idArray->length == 0 ) { // optimization

		JS_DestroyIdArray(cx, idArray);
		result = PR_Poll( NULL, 0, pr_timeout ); // we can replace this by a delay, but the function is Pool, not Sleep
		if ( result == -1 )
			return ThrowIoError(cx);
		*rval = JSVAL_ZERO;
		return JS_TRUE;
	}

	PRPollDesc staticPollDesc[32];
	PRPollDesc *pollDesc = staticPollDesc; // Optimization to avoid dynamic allocation when it is possible
	
	if ( idArray->length > (signed)(sizeof(staticPollDesc) / sizeof(staticPollDesc[0])) )
		pollDesc = (PRPollDesc*) malloc(idArray->length * sizeof(PRPollDesc));

	jsint i;
	for ( i = 0; i < idArray->length; i++ ) {

		jsval propVal;
		RT_CHECK_CALL( JS_IdToValue(cx, idArray->vector[i], &propVal ));
		RT_CHECK_CALL( JS_GetElement(cx, JSVAL_TO_OBJECT(J_ARG(1)), JSVAL_TO_INT(propVal), &propVal ));
		RT_ASSERT_OBJECT( propVal );
		JSObject *fdObj = JSVAL_TO_OBJECT( propVal );
		RT_ASSERT( InheritFrom(cx, fdObj, &classDescriptor), RT_ERROR_INVALID_CLASS );
		PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, fdObj );
//		RT_ASSERT_RESOURCE( fd ); // fd == NULL is supported !

		pollDesc[i].fd = fd; // fd is A pointer to a PRFileDesc object representing a socket or a pollable event.  This field can be set to NULL to indicate to PR_Poll that this PRFileDesc object should be ignored.
		pollDesc[i].in_flags = 0;
		pollDesc[i].out_flags = 0;
		jsval prop;

		if ( JS_GetProperty( cx, fdObj, "writable", &prop ) == JS_FALSE )
			goto failed;
		if ( prop != JSVAL_VOID )
			pollDesc[i].in_flags |= PR_POLL_WRITE;

		if ( JS_GetProperty( cx, fdObj, "readable", &prop ) == JS_FALSE )
			goto failed;
		if ( prop != JSVAL_VOID )
			pollDesc[i].in_flags |= PR_POLL_READ;

		if ( JS_GetProperty( cx, fdObj, "hangup", &prop ) == JS_FALSE )
			goto failed;
		if ( prop != JSVAL_VOID )
			pollDesc[i].in_flags |= PR_POLL_HUP;

		if ( JS_GetProperty( cx, fdObj, "exception", &prop ) == JS_FALSE )
			goto failed;
		if ( prop != JSVAL_VOID )
			pollDesc[i].in_flags |= PR_POLL_EXCEPT;

		if ( JS_GetProperty( cx, fdObj, "error", &prop ) == JS_FALSE )
			goto failed;
		if ( prop != JSVAL_VOID )
			pollDesc[i].in_flags |= PR_POLL_ERR;
	}

	result = PR_Poll( pollDesc, idArray->length, pr_timeout );
	if ( result == -1 ) {  // failed. see PR_GetError()

		ThrowIoError(cx); // returns later
		goto failed;
	}

	if ( result > 0 ) { // has event(s)

		for ( i = 0; i < idArray->length; i++ ) {

			jsval arrayItem;
			RT_CHECK_CALL( JS_IdToValue(cx, idArray->vector[i], &arrayItem) );
			RT_CHECK_CALL( JS_GetElement(cx, JSVAL_TO_OBJECT(J_ARG(1)), JSVAL_TO_INT(arrayItem), &arrayItem) );
			if ( arrayItem == JSVAL_VOID ) // socket has been removed from the list while js func "poll()" is runing
				continue;
			JSObject *fdObj = JSVAL_TO_OBJECT( arrayItem ); //JS_ValueToObject
			jsval prop, ret;
			PRInt16 outFlag = pollDesc[i].out_flags;
			jsval cbArgv[2] = { arrayItem, (outFlag & PR_POLL_HUP) ? JSVAL_TRUE : JSVAL_FALSE }; // fd, hup_flag

			if ( outFlag & PR_POLL_ERR ) {
				
				JS_GetProperty( cx, fdObj, "error", &prop );
				if ( JS_TypeOfValue( cx, prop ) == JSTYPE_FUNCTION )
					if ( JS_CallFunctionValue( cx, fdObj, prop, sizeof(cbArgv)/sizeof(*cbArgv), cbArgv, &ret ) == JS_FALSE ) // JS_CallFunction() DO NOT WORK !!!
						goto failed;
			}
			if (JS_IsExceptionPending(cx))
				goto failed;

			if ( outFlag & PR_POLL_EXCEPT ) {

				JS_GetProperty( cx, fdObj, "exception", &prop );
				if (JS_TypeOfValue( cx, prop ) == JSTYPE_FUNCTION )
					if ( JS_CallFunctionValue( cx, fdObj, prop, sizeof(cbArgv)/sizeof(*cbArgv), cbArgv, &ret ) == JS_FALSE ) // JS_CallFunction() DO NOT WORK !!!
						goto failed;
			}
			if (JS_IsExceptionPending(cx))
				goto failed;

			if ( outFlag & PR_POLL_HUP ) {
				
				JS_GetProperty( cx, fdObj, "hangup", &prop );
				if ( JS_TypeOfValue( cx, prop ) == JSTYPE_FUNCTION )
					if ( JS_CallFunctionValue( cx, fdObj, prop, sizeof(cbArgv)/sizeof(*cbArgv), cbArgv, &ret ) == JS_FALSE ) // JS_CallFunction() DO NOT WORK !!!
						goto failed;
			}
			if (JS_IsExceptionPending(cx))
				goto failed;

			if ( outFlag & PR_POLL_READ ) {
				
				JS_GetProperty( cx, fdObj, "readable", &prop );
				if ( JS_TypeOfValue( cx, prop ) == JSTYPE_FUNCTION )
					if ( JS_CallFunctionValue( cx, fdObj, prop, sizeof(cbArgv)/sizeof(*cbArgv), cbArgv, &ret ) == JS_FALSE ) // JS_CallFunction() DO NOT WORK !!!
						goto failed;
			}
			if (JS_IsExceptionPending(cx))
				goto failed;

			if ( outFlag & PR_POLL_WRITE ) {
			
				JS_GetProperty( cx, fdObj, "writable", &prop );
				if ( JS_TypeOfValue( cx, prop ) == JSTYPE_FUNCTION )
					if ( JS_CallFunctionValue( cx, fdObj, prop, sizeof(cbArgv)/sizeof(*cbArgv), cbArgv, &ret ) == JS_FALSE ) // JS_CallFunction() DO NOT WORK !!!
						goto failed;
			}
			if (JS_IsExceptionPending(cx))
				goto failed;
		}
	}

	*rval = INT_TO_JSVAL( result );
	if ( pollDesc != staticPollDesc )
		free(pollDesc);
	JS_DestroyIdArray(cx, idArray);
	return JS_TRUE;

failed: // goto is the cheaper solution
	if ( pollDesc != staticPollDesc )
		free(pollDesc);
	JS_DestroyIdArray( cx, idArray );
	return JS_FALSE;
}

DEFINE_FUNCTION( IsReadable ) {

	RT_ASSERT_ARGC( 1 );

	JSObject *descriptorObj = JSVAL_TO_OBJECT( J_ARG(1) );
	RT_ASSERT( InheritFrom(cx, descriptorObj, &classDescriptor), RT_ERROR_INVALID_CLASS );
	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, descriptorObj );
//	RT_ASSERT_RESOURCE( fd ); // fd == NULL is supported !

	PRIntervalTime prTimeout;
	if ( J_ARG_ISDEF(2) ) {

		uint32 timeout;
		RT_JSVAL_TO_UINT32( J_ARG(2), timeout );
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

	JSObject *descriptorObj = JSVAL_TO_OBJECT( J_ARG(1) );
	RT_ASSERT( InheritFrom(cx, descriptorObj, &classDescriptor), RT_ERROR_INVALID_CLASS );
	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, descriptorObj );
//	RT_ASSERT_RESOURCE( fd ); // fd == NULL is supported !

	PRIntervalTime prTimeout;
	if ( J_ARG_ISDEF(2) ) {

		uint32 timeout;
		RT_JSVAL_TO_UINT32( J_ARG(2), timeout );
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


DEFINE_FUNCTION_FAST( UIntervalNow ) {

	PRUint32 interval = PR_IntervalToMicroseconds( PR_IntervalNow() );
	JS_NewNumberValue( cx, interval, &JS_RVAL(cx, vp) );
	return JS_TRUE;
}


DEFINE_FUNCTION( Sleep ) {

	uint32 timeout;
	RT_CHECK_CALL( JS_ValueToECMAUint32( cx, J_ARG(1), &timeout ) );
	PR_Sleep( PR_MillisecondsToInterval(timeout) );
	return JS_TRUE;
}


DEFINE_FUNCTION( GetEnv ) {

	RT_ASSERT_ARGC(1)
	char *name;
	RT_JSVAL_TO_STRING( J_ARG(1), name );
	char* value = PR_GetEnv(name); // If the environment variable is not defined, the function returns NULL.
	if ( value != NULL ) { // this will cause an 'undefined' return value

//		JSString *jsstr = JS_NewExternalString(cx, (jschar*)value, strlen(value), JS_AddExternalStringFinalizer(NULL)); only works with unicode strings
		JSString *jsstr = JS_NewStringCopyZ(cx,value);
		RT_ASSERT_ALLOC( jsstr );
		*rval = STRING_TO_JSVAL(jsstr);
	}
	return JS_TRUE;
}


DEFINE_FUNCTION( GetRandomNoise ) {

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_INT( J_ARG(1) );
	PRSize rndSize = JSVAL_TO_INT( J_ARG(1) );
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
	RT_JSVAL_TO_UINT32( J_ARG(1), val );

	val = PR_ntohl(val);

	if ( 

	PR_htonll
	return JS_TRUE;
}

*/

DEFINE_PROPERTY( hostName ) {

	char tmp[SYS_INFO_BUFFER_LENGTH];
	PRStatus status = PR_GetSystemInfo( PR_SI_HOSTNAME, tmp, sizeof(tmp) );
	if ( status != PR_SUCCESS )
		return ThrowIoError(cx);
	JSString *jsstr = JS_NewStringCopyZ(cx,tmp);
	RT_ASSERT_ALLOC( jsstr );
	*vp = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
}


DEFINE_PROPERTY( physicalMemorySize ) {
	
	PRUint64 mem = PR_GetPhysicalMemorySize();
	RT_CHECK_CALL( JS_NewNumberValue(cx, (jsdouble)mem, vp) );
	return JS_TRUE;
}


DEFINE_PROPERTY( systemInfo ) {

	if ( *vp == JSVAL_VOID ) {

		char tmp[SYS_INFO_BUFFER_LENGTH];
		
		JSObject *info = JS_NewObject(cx, NULL, NULL, NULL);
		RT_ASSERT_ALLOC( info );
		*vp = OBJECT_TO_JSVAL( info );

		PRStatus status;
		jsval tmpVal;
		JSString *jsstr;

		// (TBD) these properties must be read-only !!

		status = PR_GetSystemInfo( PR_SI_ARCHITECTURE, tmp, sizeof(tmp) );
		if ( status != PR_SUCCESS )
			return ThrowIoError(cx);
		jsstr = JS_NewStringCopyZ(cx,tmp);
		RT_ASSERT_ALLOC( jsstr );
		tmpVal = STRING_TO_JSVAL(jsstr); 
		JS_SetProperty(cx, info, "architecture", &tmpVal);

		status = PR_GetSystemInfo( PR_SI_SYSNAME, tmp, sizeof(tmp) );
		if ( status != PR_SUCCESS )
			return ThrowIoError(cx);
		jsstr = JS_NewStringCopyZ(cx,tmp);
		RT_ASSERT_ALLOC( jsstr );
		tmpVal = STRING_TO_JSVAL(jsstr); 
		JS_SetProperty(cx, info, "name", &tmpVal);

		status = PR_GetSystemInfo( PR_SI_RELEASE, tmp, sizeof(tmp) );
		if ( status != PR_SUCCESS )
			return ThrowIoError(cx);
		jsstr = JS_NewStringCopyZ(cx,tmp);
		RT_ASSERT_ALLOC( jsstr );
		tmpVal = STRING_TO_JSVAL(jsstr); 
		JS_SetProperty(cx, info, "release", &tmpVal);
	}

	return JS_TRUE;
}


CONFIGURE_STATIC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( Poll ) // Do not turn it in FAST NATIVE because we need a stack frame for debuging
		FUNCTION( IsReadable )
		FUNCTION( IsWritable )
		FUNCTION( IntervalNow )
		FUNCTION_FAST( UIntervalNow )
		FUNCTION( Sleep )
		FUNCTION( GetEnv )
		FUNCTION( GetRandomNoise )
	END_STATIC_FUNCTION_SPEC
	
	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ( hostName )
		PROPERTY_READ( physicalMemorySize )
		PROPERTY_READ_STORE( systemInfo )
	END_STATIC_PROPERTY_SPEC

END_STATIC
