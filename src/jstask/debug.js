var loadModule = host.loadModule;

// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  halt();
loadModule('jsstd'); exec('../common/tools.js'); runQATests('jstask'); throw 0; // -inlineOnly

loadModule('jstask');
loadModule('jsstd');
loadModule('jsio');



var t = new Task(function(test) {

	return {
		_serialize: function(serializer) {

			serializer.write(123);
		},
		_unserialize: function(unserializer) {
			
			return { a:unserializer.read() };
		}
	};

});


t.request('test');

processEvents(t.events(), host.endSignalEvents());

print( uneval(t.response()) );


throw 0;


var t = new Task(function(test) {

	wer();
});

t.request('test');

processEvents(t.events(), host.endSignalEvents());

print( uneval(t.response()));

throw 0;



var t = new Task(function(data, idx) {
	
	var loadModule = host.loadModule;
	idx || loadModule('jsstd');
	sleep(Math.random()*100);
	return data+1;
});

t.onResponse = function(t) {

	var v = t.response();
	t.request( v );
	print(v, '\n');
}

t.request(0);

while (!host.endSignal)
	processEvents(t.events(), host.endSignalEvents());


halt();





loadModule('jsio');


myTask.request();
myTask.response();

var a = 123;
print( expand('$(a)') );


/*
for ( var i = 0; i < 5; i++ ) {

	var myTask = new Task(function() {
		
		loadModule('jsio');
		var serverSocket = new Socket();
		serverSocket.reuseAddr = true;
		serverSocket.bind(8099, '127.0.0.1');
		serverSocket.listen();

		serverSocket.readable = function(s) {

			s.accept().write('hello');
			s.close();
		}
		
		poll([serverSocket], 1000);
		serverSocket.close();
	});
	
	myTask.request();

	var client = new Socket();
	client.bind(0, '192.168.0.11');
	client.connect('127.0.0.1', 8099);
	var res = client.read(5);
	print( res, '\n' );
}
*/