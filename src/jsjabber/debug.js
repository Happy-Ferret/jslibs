LoadModule('jsstd');
LoadModule('jsjabber');
LoadModule('jsio');

// create the file info.txt, and store ['username@gmail.com','********','talk.google.com']
var [jid, password, server] = eval(new File('info.txt').content.toString());


Print( 'jid: '+jid, '\n' );
Print( 'password: '+password, '\n' );
Print( 'server: '+server, '\n' );


var j = new Jabber(jid, password);

j.onLog = function( level, area, message ) {

	Print( 'LOG: '+message, '\n');
}

j.onConnect = function() {
	
	Print('onConnect', '\n');
}

j.onDisconnect = function() {
	
	Print('onDisconnect', '\n');
}

j.Connect(server);


Print( j.socket, '\n' );

var s = Descriptor.Import(j.socket, Descriptor.DESC_SOCKET_TCP);
s.nonblocking = true;

s.readable = function(s) {
	
	Print( 'readable', '\n');
	var res = j.Process();
	Print( 'precess result: '+res , '\n');
}

while ( !endSignal )
	Poll([s], 100);








