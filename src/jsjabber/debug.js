LoadModule('jsstd');
LoadModule('jsjabber');
LoadModule('jsio');

dlist = [];

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
	Print( 'roster: ' + [item for ( item in j.roster )].join(',') );
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

var jabberOsSocket = j.Connect(server);
var jabberSocket = Descriptor.Import(jabberOsSocket, Descriptor.DESC_SOCKET_TCP);
dlist.push(jabberSocket);

jabberSocket.readable = function() {
	
	var res = j.Process();
	if ( res != Jabber.ConnNoError )
		Print( 'Error '+res+' while processing data', '\n' );
}

/*
//
// Telnet server

var serverSocket = new Socket();
serverSocket.nonblocking = true;
serverSocket.readable = function(s) {

	var incomingClient = s.Accept();
	dlist.push(incomingClient);
	
	incomingClient.console = { code:'' };
	
	incomingClient.readable = function(s) {

		var data = s.Read();
		if ( !data ) {
		
			dlist.splice(dlist.indexOf(incomingClient), 1);
			return;
		}
		
		s.console.code += data;
		
		switch (data.charCodeAt(0)) {
		case 3:
			s.console.code = '';
			break;
		case 13:
		Print('enter');
			if ( IsStatementValid(s.console.code) ) {
				
				try {
				
					s.Write('\r\n'+eval(s.console.code)+'\r\n');
				} catch(ex) {
				
					s.Write(ex+'\r\n');
				}
				s.console.code = '';
			}
			break;			
		}
		//s.Write(data); // echo
	}
}

serverSocket.Bind( 21, '127.0.0.1' );
serverSocket.Listen();
dlist.push( serverSocket );
*/


//
// Idle

while ( !endSignal ) {

	Poll(dlist, 1000);
}
