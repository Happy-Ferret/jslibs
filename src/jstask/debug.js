// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

loadModule('jstask');
loadModule('jsstd');


var t = new Task(function(test){

	return Blob("123");
});

t.Request('test');

ProcessEvents(t.Events(), EndSignalEvents());

print( uneval(t.Response()) );


Halt();



var t = new Task(function(data, idx){
	
	idx || loadModule('jsstd');
	Sleep(Math.random()*100);
	return data+1;
});

t.onResponse = function(t) {

	var v = t.Response();
	t.Request( v );
	print(v, '\n');
}

t.Request(0);

while (!endSignal)
	ProcessEvents(t.Events(), EndSignalEvents());


Halt();





loadModule('jsio');


myTask.Request();
myTask.Response();

var a = 123;
print( Expand('$(a)') );


/*
for ( var i = 0; i < 5; i++ ) {

	var myTask = new Task(function() {
		
		loadModule('jsio');
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
	print( res, '\n' );
}
*/