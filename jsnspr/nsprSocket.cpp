#define XP_WIN
#include <jsapi.h>
#include <nspr.h>

#include "nsprError.h"
#include "nsprSocket.h"

void Socket_Finalize(JSContext *cx, JSObject *obj) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd != NULL ) { // check if not already closed
		PRStatus status = PR_Close( fd ); // what to do on error ??
		if ( status == PR_FAILURE )
			JS_ReportError( cx, "a socket descriptor cannot be closed while Finalize" );
	}
	JS_SetPrivate( cx, obj, NULL );
}


JSClass Socket_class = {
  "Socket", JSCLASS_HAS_PRIVATE,
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Socket_Finalize
};

JSBool Socket_construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	if ( !JS_IsConstructing(cx) ) {

		JS_ReportError( cx, "need to be construct" );
		return JS_FALSE;
	}

	PRFileDesc *fd = PR_NewTCPSocket(); // PR_NewTCPSocket();
	if ( fd == NULL )
		return ThrowNSPRError( cx, PR_GetError() );

	PRSocketOptionData sod = { PR_SockOpt_Nonblocking, PR_TRUE };
	PR_SetSocketOption(fd, &sod); // Make the socket nonblocking

	JS_SetPrivate( cx, obj, fd );

	return JS_TRUE;
}

// Graceful Shutdown, Linger Options, and Socket Closure
//   http://windowssdk.msdn.microsoft.com/en-us/library/ms738547.aspx
// PRLinger ... PR_Close
// 	http://developer.mozilla.org/en/docs/PRLinger
JSBool Socket_close(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd == NULL ) {

		JS_ReportError( cx, "descriptor is NULL" );
		return JS_FALSE;
	}

	PRStatus status;
/*
	status = PR_Shutdown( fd, PR_SHUTDOWN_BOTH );
	if (status == PR_FAILURE)
		return ThrowNSPRError( cx, PR_GetError() );
*/
	status = PR_Close( fd ); // cf. linger option
	if (status == PR_FAILURE) {

		PRErrorCode errorCode = PR_GetError();

//		if ( errorCode == PR_WOULD_BLOCK_ERROR ) {
//		} else
		return ThrowNSPRError( cx, PR_GetError() );
	}

	JS_SetPrivate( cx, obj, NULL );

	return JS_TRUE;
}



JSBool Socket_listen(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	PRStatus status;

	if ( argc < 2 ) {

		JS_ReportError( cx, "missing argument" );
		return JS_FALSE;
	}

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd == NULL ) {

		JS_ReportError( cx, "descriptor is NULL" );
		return JS_FALSE;
	}

	uint16 port;
	JS_ValueToUint16( cx, argv[1], &port );

	PRNetAddr addr;
	status = PR_InitializeNetAddr(PR_IpAddrAny, port, &addr);
	if ( status == PR_FAILURE )
		return ThrowNSPRError( cx, PR_GetError() );

	JSString *jsstr = JS_ValueToString( cx, argv[0] );
	argv[0] = STRING_TO_JSVAL( jsstr );
	char *hostName = JS_GetStringBytes( jsstr );

	if ( *hostName != 0 ) {

		status = PR_StringToNetAddr( hostName, &addr ); // see PR_GetHostByName
		if ( status == PR_FAILURE )
			return ThrowNSPRError( cx, PR_GetError() );
	}

	status = PR_Bind(fd, &addr);
	if ( status == PR_FAILURE )
		return ThrowNSPRError( cx, PR_GetError() );

	status = PR_Listen(fd, 32);
	if ( status == PR_FAILURE )
		return ThrowNSPRError( cx, PR_GetError() );

	return JS_TRUE;
}


JSBool Socket_accept(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd == NULL ) {

		JS_ReportError( cx, "descriptor is NULL" );
		return JS_FALSE;
	}

	PRFileDesc *newFd = PR_Accept( fd, NULL, PR_INTERVAL_NO_WAIT );

	if ( newFd == NULL )
		return ThrowNSPRError( cx, PR_GetError() );

	JSObject *object = JS_NewObject( cx, &Socket_class, NULL, NULL );
	JS_SetPrivate( cx, object, newFd );
	*rval = OBJECT_TO_JSVAL( object );
	return JS_TRUE;
}


JSBool Socket_connect(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	PRStatus status;

	if ( argc < 2 ) {

		JS_ReportError( cx, "missing argument" );
		return JS_FALSE;
	}

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd == NULL ) {

		JS_ReportError( cx, "descriptor is NULL" );
		return JS_FALSE;
	}

	JSString *jsstr = JS_ValueToString( cx, argv[0] );
	argv[0] = STRING_TO_JSVAL( jsstr );
	char *hostName = JS_GetStringBytes( jsstr );

	uint16 intval;
	JS_ValueToUint16( cx, argv[1], &intval );
	PRUint16 port = intval;

	char netdb_buf[PR_NETDB_BUF_SIZE];
	PRHostEnt he;
	status = PR_GetHostByName( hostName, netdb_buf, sizeof(netdb_buf), &he);

	if ( status == PR_FAILURE )
		return ThrowNSPRError( cx, PR_GetError() );

	PRNetAddr addr;
	PRIntn next_index = 0;

	next_index = PR_EnumerateHostEnt(next_index, &he, port, &addr); // data is valid until return is -1
	if ( next_index == -1 )
		return ThrowNSPRError( cx, PR_GetError() );

	status = PR_Connect( fd, &addr , /*PRIntervalTime*/ PR_INTERVAL_NO_WAIT ); // timeout is ignored in nonblocking mode
	if ( status == PR_FAILURE ) {

		PRErrorCode errorCode = PR_GetError();

		if ( errorCode != PR_IN_PROGRESS_ERROR ) // not nonblocking-error
			return ThrowNSPRError( cx, errorCode );
	}
	// see PR_ConnectContinue
	return JS_TRUE;
}


JSBool Socket_send(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	if ( argc < 1 ) {

		JS_ReportError( cx, "missing argument" );
		return JS_FALSE;
	}

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd == NULL ) {

		JS_ReportError( cx, "descriptor is NULL" );
		return JS_FALSE;
	}

	JSString *jsstr = JS_ValueToString( cx, argv[0] );
	argv[0] = STRING_TO_JSVAL( jsstr ); // protect from GC

	PRInt32 length = JS_GetStringLength( jsstr );
	void *data = JS_GetStringBytes( jsstr );

	PRInt32 byteSent = PR_Send( fd, data, length, 0 /*must always be zero*/, PR_INTERVAL_NO_WAIT ); // timeout is ignored in nonblocking mode

//	printf( "%d<%d ?", byteSent, length ); // 		PR_WOULD_BLOCK_ERROR;
	if ( byteSent == -1 )
		return ThrowNSPRError( cx, PR_GetError() );

	if ( byteSent < length ) {

		JS_ReportError( cx, "unable to send datas" );
		return JS_FALSE;
	}

	return JS_TRUE;
}


JSBool Socket_recv(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd == NULL ) {

		JS_ReportError( cx, "descriptor is NULL" );
		return JS_FALSE;
	}

	//If PR_Poll() reports that the socket is readable (i.e., PR_POLL_READ is set in out_flags),
	//and PR_Available() returns 0, this means that the socket connection is closed.
	//see http://www.mozilla.org/projects/nspr/tech-notes/nonblockingio.html


  // the following code is needed when IE is connected then closed. does not happens with Moz.
	// else PR_Recv will return with -1 and error -5962 ( PR_BUFFER_OVERFLOW_ERROR (WSAEMSGSIZE for win32) )
	PRInt32 available = PR_Available( fd );
	if (available == -1)
		return ThrowNSPRError( cx, PR_GetError() );

	if ( available == 0 ) { // connection is closed

		*rval = JS_GetEmptyStringValue( cx );
		return JS_TRUE;
	}

	void *buf = JS_malloc( cx, available );

	PRInt32 res = PR_Recv( fd, buf, available, 0 /*must always be zero*/, PR_INTERVAL_NO_WAIT );
	if (res == -1) { // failure. The reason for the failure can be obtained by calling PR_GetError.

		JS_free( cx, buf );
		return ThrowNSPRError( cx, PR_GetError() );
	}

	JSString *str = JS_NewString( cx, (char*)buf, res );
	if (str == NULL) {

		JS_ReportError( cx, "JS_NewString error" );
		return JS_FALSE;
	}

	*rval = STRING_TO_JSVAL(str);
	return JS_TRUE;
}


JSFunctionSpec Socket_FunctionSpec[] = { // *name, call, nargs, flags, extra
 { "Listen"     , Socket_listen     , 2, 0, 0 },
 { "Accept"     , Socket_accept     , 0, 0, 0 },
 { "Connect"    , Socket_connect    , 2, 0, 0 },
 { "Close"      , Socket_close      , 0, 0, 0 },
 { "Send"       , Socket_send       , 1, 0, 0 },
 { "Recv"       , Socket_recv       , 0, 0, 0 },
 { 0 }
};


JSBool Socket_setOption( JSContext *cx, JSObject *obj, jsval id, jsval *vp ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd == NULL ) {

		JS_ReportError( cx, "descriptor is NULL" );
		return JS_FALSE;
	}
	PRSocketOptionData sod;
	sod.option = (PRSockOption)JSVAL_TO_INT( id );

	switch ( sod.option ) {
		case PR_SockOpt_Linger: { // http://developer.mozilla.org/en/docs/PRLinger
				uint32 timeout;
				JS_ValueToECMAUint32( cx, *vp, &timeout );
				if ( timeout >= 0 ) {
					sod.value.linger.polarity = PR_TRUE;
					sod.value.linger.linger = PR_MillisecondsToInterval(timeout);
				} else {
					sod.value.linger.polarity = PR_FALSE;
				}
			} break;
		case PR_SockOpt_NoDelay: {
			JSBool boolValue;
			JS_ValueToBoolean( cx, *vp, &boolValue );
			sod.value.no_delay = ( boolValue == JS_TRUE );
		} break;
		case PR_SockOpt_Reuseaddr: {
			JSBool boolValue;
			JS_ValueToBoolean( cx, *vp, &boolValue );
			sod.value.reuse_addr = ( boolValue == JS_TRUE );
		} break;
		case PR_SockOpt_Keepalive: {
			JSBool boolValue;
			JS_ValueToBoolean( cx, *vp, &boolValue );
			sod.value.keep_alive = ( boolValue == JS_TRUE );
		} break;
		case PR_SockOpt_RecvBufferSize: {
			uint32 size;
			JS_ValueToECMAUint32( cx, *vp, &size );
			sod.value.recv_buffer_size = size;
		} break;
		case PR_SockOpt_SendBufferSize: {
			uint32 size;
			JS_ValueToECMAUint32( cx, *vp, &size );
			sod.value.send_buffer_size = size;
		} break;

	}
	if ( PR_SetSocketOption(fd, &sod) == PR_FAILURE )
		return ThrowNSPRError( cx, PR_GetError() );
	return JS_TRUE;
}


JSBool Socket_getOption( JSContext *cx, JSObject *obj, jsval id, jsval *vp ) {

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd == NULL ) {

		JS_ReportError( cx, "descriptor is NULL" );
		return JS_FALSE;
	}
	PRSocketOptionData sod;
	sod.option = (PRSockOption)JSVAL_TO_INT( id );
	if ( PR_GetSocketOption(fd, &sod) == PR_FAILURE )
		return ThrowNSPRError( cx, PR_GetError() );
	switch ( sod.option ) {
		case PR_SockOpt_Linger:
			if ( sod.value.linger.polarity == PR_TRUE )
				JS_NewNumberValue( cx, PR_IntervalToMilliseconds(sod.value.linger.linger), vp );
			else
				*vp = JSVAL_ZERO;
			break;
		case PR_SockOpt_NoDelay:
			*vp = sod.value.no_delay == PR_TRUE ? JSVAL_TRUE : JSVAL_FALSE;
			break;
		case PR_SockOpt_Reuseaddr:
			*vp = sod.value.reuse_addr == PR_TRUE ? JSVAL_TRUE : JSVAL_FALSE;
			break;
		case PR_SockOpt_Keepalive:
			*vp = sod.value.keep_alive == PR_TRUE ? JSVAL_TRUE : JSVAL_FALSE;
			break;
		case PR_SockOpt_RecvBufferSize:
			*vp = INT_TO_JSVAL(sod.value.recv_buffer_size);
			break;
		case PR_SockOpt_SendBufferSize:
			*vp = INT_TO_JSVAL(sod.value.send_buffer_size);
			break;
	}
	return JS_TRUE;
}


JSBool Socket_getPeerName( JSContext *cx, JSObject *obj, jsval id, jsval *vp ) {

	PRStatus status;

	PRFileDesc *fd = (PRFileDesc *)JS_GetPrivate( cx, obj );
	if ( fd == NULL ) {

		JS_ReportError( cx, "descriptor is NULL" );
		return JS_FALSE;
	}

	PRNetAddr peerAddr;
	status = PR_GetPeerName( fd, &peerAddr );
	if ( status == PR_FAILURE )
		return ThrowNSPRError( cx, PR_GetError() );

	char buf[16]; // If addr is an IPv4 address, size needs to be at least 16. If addr is an IPv6 address, size needs to be at least 46.
	PR_NetAddrToString( &peerAddr, buf, sizeof(buf) );
	*vp = STRING_TO_JSVAL( JS_NewStringCopyZ( cx, buf ) );
	return JS_TRUE;
}


JSPropertySpec Socket_PropertySpec[] = { // *name, tinyid, flags, getter, setter
  { "linger"   , PR_SockOpt_Linger, JSPROP_PERMANENT, Socket_getOption, Socket_setOption },
  { "noDelay"  , PR_SockOpt_NoDelay, JSPROP_PERMANENT, Socket_getOption, Socket_setOption },
  { "reuseAddr", PR_SockOpt_Reuseaddr, JSPROP_PERMANENT, Socket_getOption, Socket_setOption },
  { "keepAlive", PR_SockOpt_Keepalive, JSPROP_PERMANENT, Socket_getOption, Socket_setOption },
  { "recvBufferSize", PR_SockOpt_RecvBufferSize, JSPROP_PERMANENT, Socket_getOption, Socket_setOption },
  { "sendBufferSize", PR_SockOpt_SendBufferSize, JSPROP_PERMANENT, Socket_getOption, Socket_setOption },
	{ "peerName", 0, JSPROP_PERMANENT|JSPROP_READONLY, Socket_getPeerName, NULL },
  { 0 }
};



JSObject *InitSocketClass( JSContext *cx, JSObject *obj ) {

	return JS_InitClass( cx, obj, NULL, &Socket_class, Socket_construct, 1, Socket_PropertySpec, Socket_FunctionSpec, NULL, NULL );
}