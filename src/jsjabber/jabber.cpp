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
#include <messagehandler.h> 
//#include <connectionhandler.h>

#include <presencehandler.h>

#include <rostermanager.h>
#include <rosterlistener.h>

#include <connectionlistener.h>
//#include <messagesessionhandler.h>

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
	public LogHandler { // , public MessageSessionHandler

public:
	Handlers( JSObject *obj ) : _obj(obj) {}
	JSContext *_cx;

private:
	JSObject *_obj;

	void handleLog( LogLevel level, LogArea area, const std::string& message ) {
		
		jsval fval, rval;
		if ( !JS_GetProperty(_cx, _obj, "onLog", &fval) || fval == JSVAL_VOID )
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
		if ( !JS_GetProperty(_cx, _obj, "onTLSConnect", &fval) || fval == JSVAL_VOID )
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
		if ( !JS_GetProperty(_cx, _obj, "onConnect", &fval) || fval == JSVAL_VOID )
			return;
		if ( !JsvalIsFunction(_cx, fval) ) {
			
			JS_ReportError(_cx, "onConnect is not a function.");
			return;
		}
		JS_CallFunctionValue(_cx, _obj, fval, 0, NULL, &rval); // errors will be managed later by JS_IsExceptionPending(cx)
	}

	void onDisconnect(ConnectionError cErr) {

		jsval fval, rval;
		if ( !JS_GetProperty(_cx, _obj, "onDisconnect", &fval) || fval == JSVAL_VOID )
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
		if ( !JS_GetProperty(_cx, _obj, "onMessage", &fval) || fval == JSVAL_VOID )
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
		if ( !JS_GetProperty(_cx, _obj, "onRosterPresence", &fval) || fval == JSVAL_VOID )
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
//
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
//	pv->client->registerMessageSessionHandler( pv->handlers, 0 );
	return JS_TRUE;
}


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

	ConnectionTCPClient *connection = dynamic_cast<ConnectionTCPClient*>( pv->client->connectionImpl() );
	if ( !connection )
		return JS_TRUE;

	int sock = connection->socket(); // return The socket of the active connection, or -1 if no connection is established.
	if ( sock == -1 )
		return JS_TRUE;

	J_CHK( IntToJsval(cx, sock, J_RVAL) );
	return JS_TRUE;
}


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


CONFIGURE_CLASS

	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( Connect )
		FUNCTION( Process )
		FUNCTION( SendMessage )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
//		PROPERTY_READ( roster )
		PROPERTY( presence )
		PROPERTY( status )
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
