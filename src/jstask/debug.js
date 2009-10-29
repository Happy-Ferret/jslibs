// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

LoadModule('jstask');
LoadModule('jsstd');
LoadModule('jsio');

var i = 0;
while ( !endSignal && i++ < 300 ) {

	new Task(function(){});
}



Halt();



myTask.Request();
myTask.Response();

var a = 123;
Print( Expand('$(a)') );


/*
for ( var i = 0; i < 5; i++ ) {

	var myTask = new Task(function() {
		
		LoadModule('jsio');
		var serverSocket = new Socket();
		serverSocket.reuseAddr = true;
		serverSocket.Bind(8099, '127.0.0.1');
		serverSocket.Listen();

		serverSocket.readable = function(s) {

			s.Accept().Write('hello');
			s.Close();
		}
		
		Poll([serverSocket], 1000);
		serverSocket.Close();
	});
	
	myTask.Request();

	var client = new Socket();
	client.Bind(0, '192.168.0.11');
	client.Connect('127.0.0.1', 8099);
	var res = client.Read(5);
	Print( res, '\n' );
}
*/