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
#include "jabber.h"

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


JSBool JidToJsval( JSContext *cx, const JID *jid, jsval *rval ) {

	JSObject *jidObj = JS_NewObject(cx, NULL, NULL, NULL);
	*rval = OBJECT_TO_JSVAL(jidObj);
	J_CHK( SetPropertyString(cx, jidObj, "bare", jid->bare().c_str()) );
	J_CHK( SetPropertyString(cx, jidObj, "full", jid->full().c_str()) );
	J_CHK( SetPropertyString(cx, jidObj, "server", jid->server().c_str()) );
	J_CHK( SetPropertyString(cx, jidObj, "username", jid->username().c_str()) );
	J_CHK( SetPropertyString(cx, jidObj, "resource", jid->resource().c_str()) );
	return JS_TRUE;
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
		if ( !JsvalIsFunction(_cx, fval) ) {

			JS_ReportError(_cx, "onLog is not a function.");
			return;
		}

		jsval argv[] = { INT_TO_JSVAL(level), INT_TO_JSVAL(area), NULL };
		StringToJsval(_cx, message.c_str(), &argv[2]);
		JS_CallFunctionValue(_cx, _obj, fval, COUNTOF(argv), argv, &rval); // errors will be managed later by JS_IsExceptionPending(cx)
	}

	bool onTLSConnect( const CertInfo& info ) {

		jsval fval, rval;
		if ( !JS_GetProperty(_cx, _obj, "onTLSConnect", &fval) || JSVAL_IS_VOID( fval ) )
			return true; // by default, accepts the certificate

		if ( !JsvalIsFunction(_cx, fval) ) {

			JS_ReportError(_cx, "onTLSConnect is not a function.");
			return false;
		}

		JSObject *infoObj = JS_NewObject(_cx, NULL, NULL, NULL);
		SetPropertyBool(_cx, _obj, "chain", info.chain);
		SetPropertyString(_cx, _obj, "issuer", info.issuer.c_str());
		SetPropertyString(_cx, _obj, "server", info.server.c_str());
		SetPropertyInt(_cx, _obj, "dateFrom", info.date_from);
		SetPropertyInt(_cx, _obj, "dateTo", info.date_to);
		SetPropertyString(_cx, _obj, "protocol", info.protocol.c_str());
		SetPropertyString(_cx, _obj, "cipher", info.cipher.c_str());
		SetPropertyString(_cx, _obj, "mac", info.mac.c_str());
		SetPropertyString(_cx, _obj, "compression", info.compression.c_str());

		jsval argv[] = { INT_TO_JSVAL(infoObj) };
		JS_CallFunctionValue(_cx, _obj, fval, COUNTOF(argv), argv, &rval); // errors will be managed later by JS_IsExceptionPending(cx)

		bool res;
		JsvalToBool(_cx, rval, &res);
		return res;
	}

	void onConnect() {

		jsval fval, rval;
		if ( !JS_GetProperty(_cx, _obj, "onConnect", &fval) || JSVAL_IS_VOID( fval ) )
			return;
		if ( !JsvalIsFunction(_cx, fval) ) {

			JS_ReportError(_cx, "onConnect is not a function.");
			return;
		}
		JS_CallFunctionValue(_cx, _obj, fval, 0, NULL, &rval); // errors will be managed later by JS_IsExceptionPending(cx)
	}

	void onDisconnect(ConnectionError cErr) {

		jsval fval, rval;
		if ( !JS_GetProperty(_cx, _obj, "onDisconnect", &fval) || JSVAL_IS_VOID( fval ) )
			return;
		if ( !JsvalIsFunction(_cx, fval) ) {

			JS_ReportError(_cx, "onDisconnect is not a function.");
			return;
		}
		jsval argv[] = { INT_TO_JSVAL(cErr) };
		JS_CallFunctionValue(_cx, _obj, fval, COUNTOF(argv), argv, &rval); // errors will be managed later by JS_IsExceptionPending(cx)
	}

	void handleMessage( Stanza *stanza, MessageSession *session ) {

		jsval fval, rval;
		if ( !JS_GetProperty(_cx, _obj, "onMessage", &fval) || JSVAL_IS_VOID( fval ) )
			return;
		if ( !JsvalIsFunction(_cx, fval) ) {

			JS_ReportError(_cx, "onMessage is not a function.");
			return;
		}

//		JS_EnterLocalRootScope(_cx);
		jsval fromVal, body;
		JidToJsval(_cx, &stanza->from(), &fromVal);
		StringToJsval(_cx, stanza->body().c_str(), &body);
		jsval argv[] = { fromVal, body };
		JS_CallFunctionValue(_cx, _obj, fval, COUNTOF(argv), argv, &rval); // errors will be managed later by JS_IsExceptionPending(cx)
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

		jsval fval, rval;
		if ( !JS_GetProperty(_cx, _obj, "onRosterPresence", &fval) || JSVAL_IS_VOID( fval ) )
			return;
		if ( !JsvalIsFunction(_cx, fval) ) {

			JS_ReportError(_cx, "onRosterPresence is not a function.");
			return;
		}

		jsval fromVal, presenceVal, msgVal;
		JidToJsval(_cx, &JID(item.jid()), &fromVal);
		IntToJsval(_cx, presence, &presenceVal);
		StringToJsval(_cx, msg.c_str(), &msgVal);

		jsval argv[] = { fromVal, presenceVal, msgVal };
		JS_CallFunctionValue(_cx, _obj, fval, COUNTOF(argv), argv, &rval); // errors will be managed later by JS_IsExceptionPending(cx)
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
**/
BEGIN_CLASS( Jabber )


DEFINE_FINALIZE() {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	if ( pv ) {

		delete pv->client;
		delete pv->handlers;
		JS_free(cx, pv);
	}
}


/**doc
 * $INAME( jid, password )
  Constructs a new unconnected jabber client.
  $H arguments
   $ARG string jid
   $ARG string password
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN(2);
	Private *pv = (Private*)JS_malloc(cx, sizeof(Private));
	J_S_ASSERT_ALLOC( pv );
	J_CHK( JS_SetPrivate(cx, obj, pv) );
	const char *jid, *password;
	J_CHK( JsvalToString(cx, &J_ARG(1), &jid) );
	J_CHK( JsvalToString(cx, &J_ARG(2), &password) );
	pv->handlers = new Handlers(obj);
	pv->client = new Client(JID(jid), password);
	pv->client->logInstance().registerLogHandler(LogLevelDebug, LogAreaAll, pv->handlers); // LogLevelDebug
	pv->client->registerConnectionListener( pv->handlers );
	pv->client->rosterManager()->registerRosterListener( pv->handlers, true );
	pv->client->registerMessageHandler( pv->handlers );
	return JS_TRUE;
}


/**doc
 * $VAL $INAME( serverName [ , port ] )
  Constructs a new unconnected jabber client.
  $H arguments
   $ARG string serverName: the XMPP server to connect to.
   $ARG string port: the port to connect to.
  $H return value
   A connected socket ID that can be used in a Poll() call. Or $UNDEF if no connection is established.
**/
DEFINE_FUNCTION( Connect ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	J_S_ASSERT_ARG_MIN(1);
	const char *serverName;
	J_CHK( JsvalToString(cx, &J_ARG(1), &serverName) );
	pv->client->setServer( serverName );
	if ( J_ARG_ISDEF(2) ) {

		int port;
		JsvalToInt(cx, J_ARG(2), &port);
		pv->client->setPort( port);
	}
	pv->handlers->_cx = cx;
	pv->client->connect(false); // the function returnes immediately after the connection has been established.
	pv->handlers->_cx = NULL;
	if ( JS_IsExceptionPending(cx) )
		return JS_FALSE;

	//bool usingCompression = pv->client->compression();

	ConnectionTCPClient *connection = dynamic_cast<ConnectionTCPClient*>( pv->client->connectionImpl() );
	if ( !connection )
		return JS_TRUE;

	int sock = connection->socket(); // return The socket of the active connection, or -1 if no connection is established.
	if ( sock == -1 )
		return JS_TRUE;

	J_CHK( IntToJsval(cx, sock, J_RVAL) );
	return JS_TRUE;
}


/**doc
 * $VOID $INAME()
  Disconnects from the server.
**/
DEFINE_FUNCTION( Disconnect ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	pv->handlers->_cx = cx;
	pv->client->disconnect();
	pv->handlers->_cx = NULL;

	if ( JS_IsExceptionPending(cx) )
		return JS_FALSE;

	return JS_TRUE;
}


/**doc
 * $VAL $INAME()
  Receive data from the socket and to feed the parser and call the event functions.
**/
DEFINE_FUNCTION( Process ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	pv->handlers->_cx = cx;
	ConnectionError cErr = pv->client->recv();
	pv->handlers->_cx = NULL;

	if ( JS_IsExceptionPending(cx) )
		return JS_FALSE;

	J_CHK( IntToJsval(cx, (int)cErr, rval) );
	return JS_TRUE;
}



/**doc
 * $VOID $INAME( to, body )
  Send a given message.
  $H arguments
   $ARG string to: the destination of the message.
   $ARG string body: the body of the message.
**/
DEFINE_FUNCTION( SendMessage ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	J_S_ASSERT_ARG_MIN(2);

	const char *to, *body;
	J_CHK( JsvalToString(cx, &J_ARG(1), &to) );
	J_CHK( JsvalToString(cx, &J_ARG(2), &body) );

	Tag *message = new Tag( "message" );
	message->addAttribute( "type", "chat" );
	new Tag( message, "body", body );

	message->addAttribute( "from", pv->client->jid().full() );
	message->addAttribute( "to", to);
	message->addAttribute( "id", pv->client->getID() );

	pv->client->send( message );
	return JS_TRUE;
}


/*
DEFINE_FUNCTION( RosterItem ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	J_S_ASSERT_ARG_MIN(1);

	return JS_TRUE;
}
*/

/**doc
 * $VAL $INAME
  The socket ID of the connection or $UNDEF if no connection is established.
**/
DEFINE_PROPERTY( socket ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	ConnectionTCPClient *connection = dynamic_cast<ConnectionTCPClient*>( pv->client->connectionImpl() );
	if ( !connection ) {

		*vp = JSVAL_VOID;
		return JS_TRUE;
	}
	int sock = connection->socket(); // return The socket of the active connection, or -1 if no connection is established.
	if ( sock == -1 ) {

		*vp = JSVAL_VOID;
		return JS_TRUE;
	}
	J_CHK( IntToJsval(cx, sock, vp) );
	return JS_TRUE;
}


/**doc
 * $STR $INAME
  Sets/Gets the current status message.
**/
DEFINE_PROPERTY_GETTER( status ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	J_CHK( StringToJsval(cx, pv->client->status().c_str(), vp) );
	return JS_TRUE;
}


DEFINE_PROPERTY_SETTER( status ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	const char *status;
	J_CHK( JsvalToString(cx, vp, &status) );
	pv->client->setPresence(pv->client->presence(), pv->client->priority(), status);
	return JS_TRUE;
}


/**doc
 * $ENUM $INAME
  Sets/Gets the current presence. either Jabber.PresenceUnknown, Jabber.PresenceAvailable, Jabber.PresenceChat, Jabber.PresenceAway, Jabber.PresenceDnd, Jabber.PresenceXa, Jabber.PresenceUnavailable,
**/
DEFINE_PROPERTY_GETTER( presence ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	Presence presence = pv->client->presence();
	J_CHK( IntToJsval(cx, presence, vp) );
	return JS_TRUE;
}

DEFINE_PROPERTY_SETTER( presence ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	int presence;
	J_CHK( JsvalToInt(cx, *vp, &presence) );
	pv->client->setPresence((Presence)presence, pv->client->priority(), pv->client->status());
	return JS_TRUE;
}

/*
DEFINE_PROPERTY( roster ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

//	JSObject *rosterObj = JS_NewObjectWithGivenProto(cx, NULL, NULL, NULL);
	Roster *roster = pv->client->rosterManager()->roster();
	JSObject *rosterList = JS_NewArrayObject(cx, roster->size(), NULL);
	int i = 0;
	for ( Roster::const_iterator it = roster->begin(); it != roster->end(); it++ ) {

		jsval rosterItem;
		J_CHK( StringToJsval(cx, (*it).first.c_str(), &rosterItem ) );
		J_CHK( JS_SetElement(cx, rosterList, i, &rosterItem) );
		i++;
	}
	return JS_TRUE;
}
*/


/**doc
 * $ENUM $INAME $READONLY
  is the total number of bytes received.
**/
DEFINE_PROPERTY( connectionTotalIn ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	int totalIn, totalOut;
	pv->client->connectionImpl()->getStatistics( totalIn, totalOut);
	J_CHK( IntToJsval(cx, totalIn, vp) );
	return JS_TRUE;
}


/**doc
 * $ENUM $INAME $READONLY
  is the total number of bytes sent.
**/
DEFINE_PROPERTY( connectionTotalOut ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	int totalIn, totalOut;
	pv->client->connectionImpl()->getStatistics( totalIn, totalOut);
	J_CHK( IntToJsval(cx, totalOut, vp) );
	return JS_TRUE;
}

/**doc
=== callback functions ===
  These functions are called on the current Jabber object.
  * $VOID onLog( logLevel, logArea, logMessage )
   Called for log messages. _logLevel_ is either: Jabber.LogLevelDebug, Jabber.LogLevelWarning, Jabber.LogLevelError
  * $BOOL onTLSConnect( infoObject )
   This function is called when the connection was TLS/SSL secured. Return true if cert credentials are accepted, false otherwise. If false is returned the connection is terminated.
   _infoObject_ has the following properties: chain, issuer, server, dateFrom, dateTo, protocol, cipher, mac, compression
   by default, if the function is not defined, this accepts the certificate.
  * $VOID onConnect()
   Called on a successful connections. It will be called either after all authentication is finished if username/password were supplied, or after a connection has been established if no credentials were supplied.
  * $VOID onMessage( from, body )
   Called when an event has been raised by the remote contact.
  * $VOID onRosterPresence( from, presence, message )
   Called on every status change of an item in the roster.
**/

CONFIGURE_CLASS

	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( Connect )
		FUNCTION( Disconnect )
		FUNCTION( Process )
		FUNCTION( SendMessage )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
//		PROPERTY_READ( roster )
		PROPERTY( presence )
		PROPERTY( status )
		PROPERTY_READ( connectionTotalIn )
		PROPERTY_READ( connectionTotalOut )
	END_PROPERTY_SPEC

	BEGIN_CONST_INTEGER_SPEC
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
	END_CONST_INTEGER_SPEC

END_CLASS
