LoadModule('jsstd');
LoadModule('jsjabber');
LoadModule('jsio');

// create the file info.txt, and store ['username@gmail.com','********','talk.google.com']
var [jid, password, server] = eval(new File('info.txt').content.toString());

Print( 'jid: '+jid, '\n' );
Print( 'password: '+password, '\n' );
Print( 'server: '+server, '\n' );

//
// Configuration

var j = new Jabber(jid, password);

j.onLog = function( level, area, message ) {

	if ( level == Jabber.LogLevelWarning || level == Jabber.LogLevelError )
		 Print( 'LOG: '+message, '\n');
}

j.onConnect = function() {
	
	Print('onConnect', '\n');
	j.presence = Jabber.PresenceAvailable;
}

j.onDisconnect = function() {
	
	Print('onDisconnect', '\n');
}

j.onRosterPresence = function( fromVal, presenceVal, msgVal ) {

	Print( 'onRosterPresence: '+fromVal.full+'='+presenceVal+' '+msgVal, '\n');
}

j.onMessage = function( from, body ) {

	Print( 'onMessage: '+from.full+'='+body, '\n');
}

//
// Connection

var osSocket = j.Connect(server);
var s = Descriptor.Import(osSocket, Descriptor.DESC_SOCKET_TCP);

s.readable = function(s) {
	
	var res = j.Process();
	if ( res != Jabber.ConnNoError )
		Print( 'Error '+res+' while processing data', '\n' );
}

//
// Idle

while ( !endSignal ) {

	Poll([s], 1000);
}
