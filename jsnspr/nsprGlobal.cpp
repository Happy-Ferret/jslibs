#define XP_WIN
#include <jsapi.h>
#include <nspr.h>

#include "nsprError.h"
#include "nsprSocket.h"

JSBool Poll(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

  // http://developer.mozilla.org/en/docs/PR_Poll

	PRPollDesc pollDesc[1024];

	if ( argc < 1 ) { // || argc-1 > sizeof(pollDesc) / sizeof(PRPollDesc) )
		
		JS_ReportError( cx, "argument is missing" );
		return JS_FALSE;
	}

	if ( JS_IsArrayObject( cx, JSVAL_TO_OBJECT(argv[0]) ) != JS_TRUE ) { // first argument is an array ?

		JS_ReportError( cx, "Array object is required" );
		return JS_FALSE;
	}

	JSIdArray *idArray = JS_Enumerate( cx, JSVAL_TO_OBJECT(argv[0]) ); // make a kind of auto-ptr for this

	if ( idArray->length > sizeof(pollDesc) / sizeof(PRPollDesc) ) {

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

	uintN i, objCount = idArray->length;
	for ( i = 0; i < objCount; i++ ) {

		jsval propVal;
		JS_IdToValue( cx, idArray->vector[i], &propVal );
		JS_GetElement(cx, JSVAL_TO_OBJECT(argv[0]), JSVAL_TO_INT(propVal), &propVal );
		JSObject *o = JSVAL_TO_OBJECT( propVal ); //JS_ValueToObject
		*rval = OBJECT_TO_JSVAL( o ); // protect from GC

		PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, o );
		if ( fd == NULL ) {

			JS_ReportError( cx, "descriptor is NULL" );
			goto failed;
		}

		pollDesc[i].fd = fd;
		pollDesc[i].in_flags = 0;

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

	PRInt32 result = PR_Poll( pollDesc, objCount, pr_timeout );

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


JSBool g_IntervalNow(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	PRUint32 interval = PR_IntervalToMilliseconds( PR_IntervalNow() );
	JS_NewNumberValue( cx, interval, rval );
	return JS_TRUE;
}


JSBool g_Sleep(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	uint32 timeout;
	JSBool res = JS_ValueToECMAUint32( cx, argv[0], &timeout );
	PR_Sleep( PR_MillisecondsToInterval(timeout) );
	return JS_TRUE;
}


JSBool InitGlobal( JSContext *cx, JSObject *obj ) {

  JS_DefineFunction( cx, obj, "Poll", Poll, 1, 0 );
  JS_DefineFunction( cx, obj, "IntervalNow", g_IntervalNow, 0, 0 );
  JS_DefineFunction( cx, obj, "Sleep", g_Sleep, 1, 0 );
	return JS_TRUE;
}
