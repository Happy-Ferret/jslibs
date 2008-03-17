function( QAASSERT ) ({

	SharedMemory: function() {
		
		LoadModule('jsio');

		var mem = new SharedMemory( 'test.txt', 100 );
		mem.content = 'xxxxxx789';
		mem.Write('xxx456');
		mem.Write('123', 0);
		mem.Write('ABC', 9);
		
		var mem2 = new SharedMemory( 'test.txt', 100 );
		
		QAASSERT( mem2.Read() == '123456789ABC' );
		QAASSERT( mem2.content.length == 12 );
		QAASSERT( mem2.content == '123456789ABC' );
	},

	DirectoryTest: function() {
	
		var l = Directory.List('./.', Directory.SKIP_BOTH | Directory.SKIP_DIRECTORY | Directory.SKIP_OTHER );
	},

	NonBlockingTCPSocket: function() {

		LoadModule('jsstd');
		LoadModule('jsio');

		var count = 0;
		var dlist = [];
		var step = 0;

		var serverSocket = new Socket();
		serverSocket.nonblocking = true;
		serverSocket.exception = function(s) { QAASSERT( false, 'serverSocket.exception') }
		serverSocket.readable = function(s) {

			var incomingClient = s.Accept(); 
			dlist.push(incomingClient);
			incomingClient.readable = function(s) {
				
				count++;
				QAASSERT( s.Read() == '1234', 'read failed' );
			}
		}

		serverSocket.Bind( 9998, '127.0.0.1' );
		serverSocket.Listen();
		dlist.push(serverSocket);


		var clientSocket = new Socket();
		clientSocket.nonblocking = true;
		clientSocket.exception = function(s) { QAASSERT( false, 'client.exception') }
		clientSocket.writable = function(s) { s.Write('1234') }
		clientSocket.Connect( 'localhost', 9998 );
		dlist.push(clientSocket);

		for ( var i = 0; i < 10; i++ ) {

			Poll(dlist, 10);
		}
		
		QAASSERT( count == 100, 'incomplete data count: '+count );
	},


	NonBlockingUDPSocket: function() {

	
		var s2 = new Socket( Socket.UDP );
		s2.nonblocking = true;
		s2.Bind(9999);
		s2.readable = function(s) {

			var [data, ip, port] = s.RecvFrom();
			QAASSERT( data == '1234' );
		}

		var s1 = new Socket( Socket.UDP );
		s1.reuseAddr = true;
		s1.nonblocking = true;
		s1.Connect('127.0.0.1', 9999);

		var dlist = [s1,s2]; //descriptor list

		var step = 0;
		var i = 0;
		while(++i < 100) {

			Poll(dlist, 10);
			if ( !(++step % 4) ) {

				s1.Write('1234');
			}
		}
	},


	Interval: function() {

		var t0 = IntervalNow();
		Sleep(250);
		var t = IntervalNow() - t0;
		QAASSERT( t >= 250 && t < 270, 'Interval shift too high: '+t );
	}

})