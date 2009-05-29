LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jstask');


for ( var i = 0; i < 2; i++ ) {

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