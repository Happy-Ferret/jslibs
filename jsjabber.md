<b>If something seems wrong or incomplete, please enter <a href='#commentform.md'>a comment at the bottom of this page</a>.</b><br /><br />- [source](http://jslibs.googlecode.com/svn/trunk/./src/jsjabber/) - [main](JSLibs.md) - [QA](http://jslibs.googlecode.com/svn/trunk/./src/jsjabber/qa.js) -
# jsjabber module #

> jsjabber is a wrapper to the gloox library.<br />
> gloox is a full-featured Jabber/XMPP client library.
> 



---

## jsjabber static members ##
- [top](#jsjabber_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsjabber/static.cpp?r=2555) -


---

## class jsjabber::Jabber ##
- [top](#jsjabber_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsjabber/jabber.cpp?r=2577) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( jid, password )
> > Constructs a new unconnected jabber client.
> > ##### arguments: #####
      1. <sub>string</sub> _jid_
      1. <sub>string</sub> _password_

### Static functions ###

#### <font color='white' size='1'><b>Connect</b></font> ####

> <sub>value</sub> <b>Connect</b>( serverName [[.md](.md) , port ] )
> > Constructs a new unconnected jabber client.
> > ##### arguments: #####
      1. <sub>string</sub> _serverName_: the XMPP server to connect to.
      1. <sub>string</sub> _port_: the port to connect to.
> > ##### return value: #####
> > > A connected socket ID that can be used in a Poll() call. Or _undefined_ if no connection is established.

#### <font color='white' size='1'><b>Disconnect</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Disconnect</b>()
> > Disconnects from the server.

#### <font color='white' size='1'><b>Process</b></font> ####

> <sub>value</sub> <b>Process</b>()
> > Receive data from the socket and to feed the parser and call the event functions.

#### <font color='white' size='1'><b>SendMessage</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>SendMessage</b>( to, body )
> > Send a given message.
> > ##### arguments: #####
      1. <sub>string</sub> _to_: the destination of the message.
      1. <sub>string</sub> _body_: the body of the message.

### Static properties ###

#### <font color='white' size='1'><b>socket</b></font> ####

> <sub>value</sub> <b>socket</b>
> > The socket ID of the connection or _undefined_ if no connection is established.

#### <font color='white' size='1'><b>status</b></font> ####

> <sub>string</sub> <b>status</b>
> > Sets/Gets the current status message.

#### <font color='white' size='1'><b>presence</b></font> ####

> <sub>enum</sub> <b>presence</b>
> > Sets/Gets the current presence. either Jabber.PresenceUnknown, Jabber.PresenceAvailable, Jabber.PresenceChat, Jabber.PresenceAway, Jabber.PresenceDnd, Jabber.PresenceXa, Jabber.PresenceUnavailable,

#### <font color='white' size='1'><b>connectionTotalIn</b></font> ####

> <sub>enum</sub> <b>connectionTotalIn</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is the total number of bytes received.

#### <font color='white' size='1'><b>connectionTotalOut</b></font> ####

> <sub>enum</sub> <b>connectionTotalOut</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is the total number of bytes sent.

### callback functions ###

> These functions are called on the current Jabber object.
  * <font color='gray' size='1'><sub>void</sub></font> **onLog**( logLevel, logArea, logMessage )
> > Called for log messages. _logLevel_ is either: Jabber.LogLevelDebug, Jabber.LogLevelWarning, Jabber.LogLevelError
  * <sub>boolean</sub> **onTLSConnect**( infoObject )
> > This function is called when the connection was TLS/SSL secured. Return true if cert credentials are accepted, false otherwise. If false is returned the connection is terminated.
> > _infoObject_ has the following properties: chain, issuer, server, dateFrom, dateTo, protocol, cipher, mac, compression
> > by default, if the function is not defined, this accepts the certificate.
  * <font color='gray' size='1'><sub>void</sub></font> **onConnect**()
> > Called on a successful connections. It will be called either after all authentication is finished if username/password were supplied, or after a connection has been established if no credentials were supplied.
  * <font color='gray' size='1'><sub>void</sub></font> **onMessage**( from, body )
> > Called when an event has been raised by the remote contact.
  * <font color='gray' size='1'><sub>void</sub></font> **onRosterPresence**( from, presence, message )
> > Called on every status change of an item in the roster.


---

- [top](#jsjabber_module.md) - [main](JSLibs.md) -
