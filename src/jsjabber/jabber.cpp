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

#pragma warning( push )
#pragma warning(disable : 4800) // warning C4800: '???' : forcing value to bool 'true' or 'false' (performance warning)

#include <connectiontcpclient.h>
#include <connectionsocks5proxy.h>
#include <connectionhttpproxy.h>

#include <messagehandler.h>
//#include <connectionhandler.h>

#include <presencehandler.h>

#include <rostermanager.h>
#include <rosterlistener.h>

#include <connectionlistener.h>

#include <socks5bytestream.h>
#include <socks5bytestreamdatahandler.h>
#include <siprofilefthandler.h>

#include <client.h>
#pragma warning( pop )

using namespace gloox;

DECLARE_CLASS( Jabber )


bool JidToJsval( JSContext *cx, const JID *jid, jsval *rval ) {

	JSObject *jidObj = JL_NewObj(cx);
	*rval = OBJECT_TO_JSVAL(jidObj);
	JL_CHK( JL_NativeToProperty(cx, jidObj, "bare", jid->bare().c_str()) );
	JL_CHK( JL_NativeToProperty(cx, jidObj, "full", jid->full().c_str()) );
	JL_CHK( JL_NativeToProperty(cx, jidObj, "server", jid->server().c_str()) );
	JL_CHK( JL_NativeToProperty(cx, jidObj, "username", jid->username().c_str()) );
	JL_CHK( JL_NativeToProperty(cx, jidObj, "resource", jid->resource().c_str()) );
	return true;
	JL_BAD;
}


class Handlers :
	public ConnectionListener,
	public RosterListener,
	public MessageHandler,
//	public SIProfileFTHandler,
	public LogHandler {

public:
	Handlers( JSObject *obj ) : _obj(obj) {}
	JSContext *_cx;

private:
	JSObject *_obj;

	void handleLog( LogLevel level, LogArea area, const std::string& message ) {

		jsval fval, rval;
		if ( !JS_GetProperty(_cx, _obj, "onLog", &fval) || JSVAL_IS_VOID( fval ) )
			return;
		if ( !JL_ValueIsCallable(_cx, fval) ) {

			JS_ReportError(_cx, "onLog is not a function.");
			return;
		}

		jsval argv[3] = { INT_TO_JSVAL(level), INT_TO_JSVAL(area) /*, see below*/ };
		JL_NativeToJsval(_cx, message.c_str(), &argv[2]);
		JS_CallFunctionValue(_cx, _obj, fval, COUNTOF(argv), argv, &rval); // errors will be managed later by JL_IsExceptionPending(cx)
	}

	bool onTLSConnect( const CertInfo& info ) {

		jsval fval, rval;
		if ( !JS_GetProperty(_cx, _obj, "onTLSConnect", &fval) || JSVAL_IS_VOID( fval ) )
			return true; // by default, accepts the certificate

		if ( !JL_ValueIsCallable(_cx, fval) ) {

			JS_ReportError(_cx, "onTLSConnect is not a function.");
			return false;
		}

		JSObject *infoObj = JL_NewObj(_cx);
		JL_NativeToProperty(_cx, _obj, "chain", info.chain);
		JL_NativeToProperty(_cx, _obj, "issuer", info.issuer.c_str());
		JL_NativeToProperty(_cx, _obj, "server", info.server.c_str());
		JL_NativeToProperty(_cx, _obj, "dateFrom", info.date_from);
		JL_NativeToProperty(_cx, _obj, "dateTo", info.date_to);
		JL_NativeToProperty(_cx, _obj, "protocol", info.protocol.c_str());
		JL_NativeToProperty(_cx, _obj, "cipher", info.cipher.c_str());
		JL_NativeToProperty(_cx, _obj, "mac", info.mac.c_str());
		JL_NativeToProperty(_cx, _obj, "compression", info.compression.c_str());

		jsval argv[] = { OBJECT_TO_JSVAL(infoObj) };
		JS_CallFunctionValue(_cx, _obj, fval, COUNTOF(argv), argv, &rval); // errors will be managed later by JL_IsExceptionPending(cx)

		bool res;
		JL_JsvalToNative(_cx, rval, &res);
		return res;
	}

	void onConnect() {

		jsval fval, rval;
		if ( !JS_GetProperty(_cx, _obj, "onConnect", &fval) || JSVAL_IS_VOID( fval ) )
			return;
		if ( !JL_ValueIsCallable(_cx, fval) ) {

			JS_ReportError(_cx, "onConnect is not a function.");
			return;
		}
		JS_CallFunctionValue(_cx, _obj, fval, 0, NULL, &rval); // errors will be managed later by JL_IsExceptionPending(cx)
	}

	void onDisconnect(ConnectionError cErr) {

		jsval fval, rval;
		if ( !JS_GetProperty(_cx, _obj, "onDisconnect", &fval) || JSVAL_IS_VOID( fval ) )
			return;
		if ( !JL_ValueIsCallable(_cx, fval) ) {

			JS_ReportError(_cx, "onDisconnect is not a function.");
			return;
		}
		jsval argv[] = { INT_TO_JSVAL(cErr) };
		JS_CallFunctionValue(_cx, _obj, fval, COUNTOF(argv), argv, &rval); // errors will be managed later by JL_IsExceptionPending(cx)
	}

	void handleMessage( Stanza *stanza, MessageSession *session ) {

		JL_IGNORE(session);

		jsval fval, rval;
		if ( !JS_GetProperty(_cx, _obj, "onMessage", &fval) || JSVAL_IS_VOID( fval ) )
			return;
		if ( !JL_ValueIsCallable(_cx, fval) ) {

			JS_ReportError(_cx, "onMessage is not a function.");
			return;
		}

//		JS_EnterLocalRootScope(_cx);
		jsval fromVal, body;
		JidToJsval(_cx, &stanza->from(), &fromVal);
		JL_NativeToJsval(_cx, stanza->body().c_str(), &body);
		jsval argv[] = { fromVal, body };
		JS_CallFunctionValue(_cx, _obj, fval, COUNTOF(argv), argv, &rval); // errors will be managed later by JL_IsExceptionPending(cx)
//		js_LeaveLocalRootScope(_cx);
	}


// RosterListener
	void handleItemAdded( const JID& jid ) {}
	void handleItemSubscribed( const JID& jid ) {}
	void handleItemRemoved( const JID& jid ) {}
	void handleItemUpdated( const JID& jid ) {}
	void handleItemUnsubscribed( const JID& jid ) {}
	void handleRoster( const Roster& roster ) {}
	void handleSelfPresence( const RosterItem& item, const std::string& resource, Presence presence, const std::string& msg ) {}
   bool handleSubscriptionRequest( const JID& jid, const std::string& msg ) { return true; }
	bool handleUnsubscriptionRequest( const JID& jid, const std::string& msg ) { return true; }
	void handleNonrosterPresence( Stanza* stanza ) {}
	void handleRosterError( Stanza* stanza ) {}
	void handleRosterPresence( const RosterItem& item, const std::string& resource, Presence presence, const std::string& msg ) {

		jsval fval;
		if ( !JS_GetProperty(_cx, _obj, "onRosterPresence", &fval) || JSVAL_IS_VOID( fval ) )
			return;
		if ( !JL_ValueIsCallable(_cx, fval) ) {

			JS_ReportError(_cx, "onRosterPresence is not a function.");
			return;
		}

		jsval fromVal, presenceVal, msgVal, tmp;
		JidToJsval(_cx, &JID(item.jid()), &fromVal);
		JL_NativeToJsval(_cx, presence, &presenceVal);
		JL_NativeToJsval(_cx, msg.c_str(), &msgVal);

		jsval argv[] = { fromVal, presenceVal, msgVal };
		JS_CallFunctionValue(_cx, _obj, fval, COUNTOF(argv), argv, &tmp); // errors will be managed later by JL_IsExceptionPending(cx)
	}

// SIProfileFTHandler (file transfer)
/*
	void handleFTRequestError( Stanza* stanza, const std::string& sid ) {}

	void handleFTRequest( const JID& from, const std::string& id, const std::string& sid,
		const std::string& name, long size, const std::string& hash,
		const std::string& date, const std::string& mimetype,
		const std::string& desc, int stypes, long offset, long length ) {}

	void handleFTSOCKS5Bytestream( SOCKS5Bytestream* s5b ) {}
*/

};


struct Private {

	Client *client;
	Handlers *handlers;
};






/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3533 $
**/
BEGIN_CLASS( Jabber )


DEFINE_FINALIZE() {

	Private *pv = (Private*)JL_GetPrivate(obj);
	if ( !pv )
		return;

	delete pv->client;
	delete pv->handlers;
	JS_freeop(fop, pv);
}


/**doc
$TOC_MEMBER $INAME
 $INAME( jid, password )
  Constructs a new unconnected jabber client.
  $H arguments
   $ARG $STR jid
   $ARG $STR password
**/
DEFINE_CONSTRUCTOR() {

	Private *pv = NULL;
	JLData jid, password;

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_ASSERT_ARGC_MIN(2);

	pv = (Private*)JS_malloc(cx, sizeof(Private));
	JL_CHK( pv );
	pv->handlers = NULL;
	pv->client = NULL;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &jid) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &password) );
	pv->handlers = new Handlers(obj);
	pv->client = new Client(JID(jid.GetConstStrZ()), password.GetConstStrZ());
	pv->client->logInstance().registerLogHandler(LogLevelDebug, LogAreaAll, pv->handlers); // LogLevelDebug
	pv->client->registerConnectionListener( pv->handlers );
	pv->client->rosterManager()->registerRosterListener( pv->handlers, true );
	pv->client->registerMessageHandler( pv->handlers );

	JL_SetPrivate(obj, pv);
	return true;

bad:
	if ( pv ) { 
	
		delete pv->client;
		delete pv->handlers;
		JS_free(cx, pv);
	}
	return false;
}

/**doc
=== Static functions ===
**/


/**doc
$TOC_MEMBER $INAME
 $VAL $INAME( serverName [ , port ] )
  Constructs a new unconnected jabber client.
  $H arguments
   $ARG $STR serverName: the XMPP server to connect to.
   $ARG $STR port: the port to connect to.
  $H return value
   A connected socket ID that can be used in a Poll() call. Or $UNDEF if no connection is established.
**/
DEFINE_FUNCTION( connect ) {

	JLData serverName;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN(1);

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &serverName) );
	pv->client->setServer( serverName.GetConstStrZ() );
	if ( JL_ARG_ISDEF(2) ) {

		int port;
		JL_JsvalToNative(cx, JL_ARG(2), &port);
		pv->client->setPort( port);
	}
	pv->handlers->_cx = cx;
	pv->client->connect(false); // the function returnes immediately after the connection has been established.
	pv->handlers->_cx = NULL;
	if ( JL_IsExceptionPending(cx) )
		return false;

	//bool usingCompression = pv->client->compression();

	ConnectionTCPClient *connection = dynamic_cast<ConnectionTCPClient*>( pv->client->connectionImpl() ); // (TBD) TM
	if ( !connection )
		return true;

	int sock = connection->socket(); // return The socket of the active connection, or -1 if no connection is established.
	if ( sock == -1 )
		return true;

	JL_CHK( JL_NativeToJsval(cx, sock, JL_RVAL) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Disconnects from the server.
**/
DEFINE_FUNCTION( disconnect ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	pv->handlers->_cx = cx;
	pv->client->disconnect();
	pv->handlers->_cx = NULL;

	if ( JL_IsExceptionPending(cx) )
		return false;

	*JL_RVAL = JSVAL_VOID;
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VAL $INAME()
  Receive data from the socket and to feed the parser and call the event functions.
**/
DEFINE_FUNCTION( process ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	pv->handlers->_cx = cx;
	ConnectionError cErr = pv->client->recv();
	pv->handlers->_cx = NULL;

	if ( JL_IsExceptionPending(cx) )
		return false;

	JL_CHK( JL_NativeToJsval(cx, (int)cErr, JL_RVAL) );
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( to, body )
  Send a given message.
  $H arguments
   $ARG $STR to: the destination of the message.
   $ARG $STR body: the body of the message.
**/
DEFINE_FUNCTION( sendMessage ) {

	JLData to, body;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN(2);

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &to) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &body) );

	Tag *message = new Tag( "message" );
	message->addAttribute( "type", "chat" );
	new Tag( message, "body", body.GetConstStrZ() );

	message->addAttribute( "from", pv->client->jid().full() );
	message->addAttribute( "to", to.GetConstStrZ());
	message->addAttribute( "id", pv->client->getID() );

	pv->client->send( message );

	*JL_RVAL = JSVAL_VOID;
	return true;
	JL_BAD;
}


/*
DEFINE_FUNCTION( rosterItem ) {

	JL_ASSERT_ARGC_MIN(1);

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	return true;
	JL_BAD;
}
*/


/**doc
=== Static properties ===
**/


/**doc
$TOC_MEMBER $INAME
 $VAL $INAME
  The socket ID of the connection or $UNDEF if no connection is established.
**/
DEFINE_PROPERTY_GETTER( socket ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ConnectionTCPClient *connection = dynamic_cast<ConnectionTCPClient*>( pv->client->connectionImpl() ); // (TBD) TM
	if ( !connection ) {

		*vp = JSVAL_VOID;
		return true;
	}
	int sock = connection->socket(); // return The socket of the active connection, or -1 if no connection is established.
	if ( sock == -1 ) {

		*vp = JSVAL_VOID;
		return true;
	}
	JL_CHK( JL_NativeToJsval(cx, sock, vp) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Sets/Gets the current status message.
**/
DEFINE_PROPERTY_GETTER( status ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_CHK( JL_NativeToJsval(cx, pv->client->status().c_str(), vp) );
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_SETTER( status ) {

	JLData status;
	
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_CHK( JL_JsvalToNative(cx, *vp, &status) );
	pv->client->setPresence(pv->client->presence(), pv->client->priority(), status.GetConstStrZ());
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ENUM $INAME
  Sets/Gets the current presence. either Jabber.PresenceUnknown, Jabber.PresenceAvailable, Jabber.PresenceChat, Jabber.PresenceAway, Jabber.PresenceDnd, Jabber.PresenceXa, Jabber.PresenceUnavailable,
**/
DEFINE_PROPERTY_GETTER( presence ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	Presence presence = pv->client->presence();
	JL_CHK( JL_NativeToJsval(cx, presence, vp) );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( presence ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	int presence;
	JL_CHK( JL_JsvalToNative(cx, *vp, &presence) );
	pv->client->setPresence((Presence)presence, pv->client->priority(), pv->client->status());
	return true;
	JL_BAD;
}

/*
DEFINE_PROPERTY( roster ) {

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

//	JSObject *rosterObj = JL_NewProtolessObj(cx);
	Roster *roster = pv->client->rosterManager()->roster();
	JSObject *rosterList = JS_NewArrayObject(cx, roster->size(), NULL);
	int i = 0;
	for ( Roster::const_iterator it = roster->begin(); it != roster->end(); it++ ) {

		jsval rosterItem;
		JL_CHK( StringToJsval(cx, (*it).first.c_str(), &rosterItem ) );
		JL_CHK( JL_SetElement(cx, rosterList, i, &rosterItem) );
		i++;
	}
	return true;
	JL_BAD;
}
*/


/**doc
$TOC_MEMBER $INAME
 $ENUM $INAME $READONLY
  is the total number of bytes received.
**/
DEFINE_PROPERTY_GETTER( connectionTotalIn ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	int totalIn, totalOut;
	pv->client->connectionImpl()->getStatistics( totalIn, totalOut);
	JL_CHK( JL_NativeToJsval(cx, totalIn, vp) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ENUM $INAME $READONLY
  is the total number of bytes sent.
**/
DEFINE_PROPERTY_GETTER( connectionTotalOut ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	int totalIn, totalOut;
	pv->client->connectionImpl()->getStatistics( totalIn, totalOut);
	JL_CHK( JL_NativeToJsval(cx, totalOut, vp) );
	return true;
	JL_BAD;
}

/**doc
=== callback functions ===
  These functions are called on the current Jabber object.
  * $VOID *onLog*( logLevel, logArea, logMessage )
   Called for log messages. _logLevel_ is either: Jabber.LogLevelDebug, Jabber.LogLevelWarning, Jabber.LogLevelError
  * $BOOL *onTLSConnect*( infoObject )
   This function is called when the connection was TLS/SSL secured. Return true if cert credentials are accepted, false otherwise. If false is returned the connection is terminated.
   _infoObject_ has the following properties: chain, issuer, server, dateFrom, dateTo, protocol, cipher, mac, compression
   by default, if the function is not defined, this accepts the certificate.
  * $VOID *onConnect*()
   Called on a successful connections. It will be called either after all authentication is finished if username/password were supplied, or after a connection has been established if no credentials were supplied.
  * $VOID *onMessage*( from, body )
   Called when an event has been raised by the remote contact.
  * $VOID *onRosterPresence*( from, presence, message )
   Called on every status change of an item in the roster.
**/

CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3533 $"))
	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( connect )
		FUNCTION( disconnect )
		FUNCTION( process )
		FUNCTION( sendMessage )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
//		PROPERTY_GETTER( roster )
		PROPERTY( presence )
		PROPERTY( status )
		PROPERTY_GETTER( connectionTotalIn )
		PROPERTY_GETTER( connectionTotalOut )
		PROPERTY_GETTER( socket )
	END_PROPERTY_SPEC

	BEGIN_CONST
		CONST_INTEGER_SINGLE(LogLevelDebug)
		CONST_INTEGER_SINGLE(LogLevelWarning)
		CONST_INTEGER_SINGLE(LogLevelError)

		CONST_INTEGER_SINGLE(PresenceUnknown)
		CONST_INTEGER_SINGLE(PresenceAvailable)
		CONST_INTEGER_SINGLE(PresenceChat)
		CONST_INTEGER_SINGLE(PresenceAway)
		CONST_INTEGER_SINGLE(PresenceDnd)
		CONST_INTEGER_SINGLE(PresenceXa)
		CONST_INTEGER_SINGLE(PresenceUnavailable)

		CONST_INTEGER_SINGLE(ConnNoError)
		CONST_INTEGER_SINGLE(ConnStreamError)
		CONST_INTEGER_SINGLE(ConnStreamVersionError)
		CONST_INTEGER_SINGLE(ConnStreamClosed)
		CONST_INTEGER_SINGLE(ConnProxyAuthRequired)
		CONST_INTEGER_SINGLE(ConnProxyAuthFailed)
		CONST_INTEGER_SINGLE(ConnProxyNoSupportedAuth)
		CONST_INTEGER_SINGLE(ConnIoError)
		CONST_INTEGER_SINGLE(ConnParseError)
		CONST_INTEGER_SINGLE(ConnConnectionRefused)
		CONST_INTEGER_SINGLE(ConnDnsError)
		CONST_INTEGER_SINGLE(ConnOutOfMemory)
		CONST_INTEGER_SINGLE(ConnNoSupportedAuth)
		CONST_INTEGER_SINGLE(ConnTlsFailed)
		CONST_INTEGER_SINGLE(ConnTlsNotAvailable)
		CONST_INTEGER_SINGLE(ConnCompressionFailed)
		CONST_INTEGER_SINGLE(ConnAuthenticationFailed)
		CONST_INTEGER_SINGLE(ConnUserDisconnected)
		CONST_INTEGER_SINGLE(ConnNotConnected)
	END_CONST

END_CLASS
