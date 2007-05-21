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
#include "nsprSocket.h"

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
		JS_IdToValue( cx, idArray->vector[i], &propVal );
		JS_GetElement(cx, JSVAL_TO_OBJECT(argv[0]), JSVAL_TO_INT(propVal), &propVal );
		JSObject *o = JSVAL_TO_OBJECT( propVal ); //JS_ValueToObject
		*rval = OBJECT_TO_JSVAL( o ); // protect from GC
		// (TBD) is it useful ? I don't use JS_ValueToObject

		PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, o );

		// not needed, see below
//   if ( fd == NULL ) {
//
//			JS_ReportError( cx, "descriptor is NULL" );
//			goto failed;
//		}

		pollDesc[i].fd = fd; // fd is A pointer to a PRFileDesc object representing a socket or a pollable event.  This field can be set to NULL to indicate to PR_Poll that this PRFileDesc object should be ignored.
		pollDesc[i].in_flags = 0;
		pollDesc[i].out_flags = 0;

		jsval prop;

		JS_GetProperty( cx, o, "readable", &prop );
		if ( prop != JSVAL_VOID )
			pollDesc[i].in_flags |= PR_POLL_READ;

		JS_GetProperty( cx, o, "writable", &prop );
		if ( prop != JSVAL_VOID )
			pollDesc[i].in_flags |= PR_POLL_WRITE;

		JS_GetProperty( cx, o, "exception", &prop );
		if ( prop != JSVAL_VOID )
			pollDesc[i].in_flags |= PR_POLL_EXCEPT;
	}

	result = PR_Poll( pollDesc, objCount, pr_timeout );

	if ( result == -1 ) {  // failed. see PR_GetError()

		ThrowNSPRError( cx, PR_GetError() );
		goto failed;
	}

	for ( i = 0; i < objCount; i++ ) {

		jsval arrayItem;
		JS_IdToValue( cx, idArray->vector[i], &arrayItem );
		JS_GetElement(cx, JSVAL_TO_OBJECT(argv[0]), JSVAL_TO_INT(arrayItem), &arrayItem );
		if ( arrayItem == JSVAL_VOID ) // socket has been removed from the list while js func "poll()" is runing
			continue;
		JSObject *o = JSVAL_TO_OBJECT( arrayItem ); //JS_ValueToObject
		*rval = OBJECT_TO_JSVAL( o ); // protect from GC

//		printf( "is protected?%d ", JSVAL_IS_GCTHING( *rval ) ); // returns 1
//		*rval = propVal;

		jsval prop, ret;

		if ( result > 0 ) { // no timeout

			JS_GetProperty( cx, o, "readable", &prop );
			if ( pollDesc[i].out_flags & PR_POLL_READ && JS_TypeOfValue( cx, prop ) == JSTYPE_FUNCTION )
				if ( JS_CallFunctionValue( cx, o, prop, 1, rval, &ret ) == JS_FALSE ) // JS_CallFunction() DO NOT WORK !!!
					goto failed;

			JS_GetProperty( cx, o, "writable", &prop );
			if ( pollDesc[i].out_flags & PR_POLL_WRITE && JS_TypeOfValue( cx, prop ) == JSTYPE_FUNCTION )
				if ( JS_CallFunctionValue( cx, o, prop, 1, rval, &ret ) == JS_FALSE ) // JS_CallFunction() DO NOT WORK !!!
					goto failed;

			JS_GetProperty( cx, o, "exception", &prop );
			if ( pollDesc[i].out_flags & PR_POLL_EXCEPT && JS_TypeOfValue( cx, prop ) == JSTYPE_FUNCTION )
				if ( JS_CallFunctionValue( cx, o, prop, 1, rval, &ret ) == JS_FALSE ) // JS_CallFunction() DO NOT WORK !!!
					goto failed;

			// need PR_POLL_NVAL and PR_POLL_HUP ??
		}
	}

	JS_DestroyIdArray( cx, idArray );
	*rval = INT_TO_JSVAL( result );
	return JS_TRUE;

failed: // goto is the cheaper solution
	JS_DestroyIdArray( cx, idArray );
	return JS_FALSE;
}

DEFINE_FUNCTION( IsReadable ) {

	if ( argc < 1 ) {

		JS_ReportError( cx, "argument is missing" );
		return JS_FALSE;
	}

	JSObject *o = JSVAL_TO_OBJECT( argv[0] ); //JS_ValueToObject
	*rval = OBJECT_TO_JSVAL( o ); // (TBD) check if it is needed to protect from GC ?
	// (TBD) is it useful ? I don't use JS_ValueToObject
	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, o );
	PRPollDesc desc;
	desc.fd = fd;
	desc.in_flags = PR_POLL_READ;
	desc.out_flags = 0;

	PRInt32 result = PR_Poll( &desc, 1, PR_INTERVAL_NO_WAIT );
	if ( result == -1 ) // error
		return ThrowNSPRError( cx, PR_GetError() );

	if ( result == 1 && (desc.out_flags & PR_POLL_READ) != 0 ) {

		*rval = JSVAL_TRUE;
		return JS_TRUE;
	}
	*rval = JSVAL_FALSE;
	return JS_TRUE;
}

DEFINE_FUNCTION( IsWritable ) {

	if ( argc < 1 ) {

		JS_ReportError( cx, "argument is missing" );
		return JS_FALSE;
	}

	JSObject *o = JSVAL_TO_OBJECT( argv[0] ); //JS_ValueToObject
	*rval = OBJECT_TO_JSVAL( o ); // protect from GC
	// (TBD) is it useful ? I don't use JS_ValueToObject
	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, o );
	PRPollDesc desc;
	desc.fd = fd;
	desc.in_flags = PR_POLL_WRITE;
	desc.out_flags = 0;

	PRInt32 result = PR_Poll( &desc, 1, PR_INTERVAL_NO_WAIT );
	if ( result == -1 ) // error
		return ThrowNSPRError( cx, PR_GetError() );

	if ( result == 1 && (desc.out_flags & PR_POLL_WRITE) != 0 ) {

		*rval = JSVAL_TRUE;
		return JS_TRUE;
	}
	*rval = JSVAL_FALSE;
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

DEFINE_FUNCTION( NSPRVersion ) {

	JSString *jsstr = JS_NewStringCopyZ(cx,PR_VERSION);
	RT_ASSERT_ALLOC( jsstr );
	*rval = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
}

DEFINE_FUNCTION( GetRandomNoise ) {

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_INT( argv[0] );
	PRSize rndSize = JSVAL_TO_INT( argv[0] );
	void *buf = (void*)JS_malloc(cx, rndSize);
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
		FUNCTION( NSPRVersion )
	END_STATIC_FUNCTION_SPEC

END_STATIC

