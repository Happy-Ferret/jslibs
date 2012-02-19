// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

loadModule('jsstd');
loadModule('jsjabber');
loadModule('jsio');

dlist = [];

// create the file info.txt, and store ['username@gmail.com','********','talk.google.com']
var [jid, password, server] = eval(new File('info.txt').content.toString());

print( 'jid: '+jid, '\n' );
print( 'password: '+password, '\n' );
print( 'server: '+server, '\n' );

//
// Configuration

var j = new Jabber(jid, password);

j.onLog = function( level, area, message ) {

	if ( level == Jabber.LogLevelWarning || level == Jabber.LogLevelError )
		 print( 'LOG: '+message, '\n');
}

j.onConnect = function() {
	
	print('onConnect', '\n');
	print( 'roster: ' + [item for ( item in j.roster )].join(',') );
	j.presence = Jabber.PresenceAvailable;
}

j.onDisconnect = function() {
	
	print('onDisconnect', '\n');
}

j.onRosterPresence = function( fromVal, presenceVal, msgVal ) {

	print( 'onRosterPresence: '+fromVal.full+'='+presenceVal+' '+msgVal, '\n');
}

j.onMessage = function( from, body ) {

	print( 'onMessage: '+from.full+'='+body, '\n');
}

//
// Connection

var jabberOsSocket = j.Connect(server);
var jabberSocket = Descriptor.Import(jabberOsSocket, Descriptor.DESC_SOCKET_TCP);
dlist.push(jabberSocket);

jabberSocket.readable = function() {
	
	var res = j.Process();
	if ( res != Jabber.ConnNoError )
		print( 'Error '+res+' while processing data', '\n' );
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
		print('enter');
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
