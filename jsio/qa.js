function( QA ) ({



	DirectoryTest: function() {
	
		var l = Directory.List('./.', Directory.SKIP_BOTH | Directory.SKIP_DIRECTORY | Directory.SKIP_OTHER );
	},

	NonBlockingTCPSocket: function() {

		var count = 0;
		var dlist = [];
		var step = 0;

		var serverSocket = new Socket();
		serverSocket.reuseAddr = true;
		serverSocket.nonblocking = true;
		serverSocket.exception = function(s) { QA.ASSERT( true, false, 'serverSocket.exception') }
		serverSocket.readable = function(s) {

			var incomingClient = s.Accept(); 
			dlist.push(incomingClient);
			incomingClient.readable = function(s) {
				
				count++;
				QA.ASSERT( s.Read(), '1234', 'read data match' );
			}
		}

		serverSocket.Bind( 9998, '127.0.0.1' );
		
		QA.ASSERT( DumpObjectPrivate(serverSocket) != 0, true, 'descriptor' );
		
		serverSocket.Listen();

		dlist.push(serverSocket);


		var clientSocket = new Socket();
		clientSocket.nonblocking = true;
		clientSocket.exception = function(s) { QA.ASSERT( true, false, 'client.exception') }
//		clientSocket.writable = function(s) { s.Write('1234') }
		clientSocket.Connect( 'localhost', 9998 );
		dlist.push(clientSocket);

		for ( var i = 0; i < 10; i++ ) {

			Poll(dlist, 10);
			if ( !(++step % 4) ) {

				clientSocket.Write('1234');
			}
		}
		
	},


	NonBlockingUDPSocket: function() {
	
		var s2 = new Socket( Socket.UDP );
		s2.nonblocking = true;
		s2.Bind(9999);
		s2.readable = function(s) {

			var [data, ip, port] = s.RecvFrom();
			QA.ASSERT( data, '1234', 'received data' );
		}

		var s1 = new Socket( Socket.UDP );
		s1.reuseAddr = true;
		s1.nonblocking = true;
		s1.Connect('127.0.0.1', 9999);

		var dlist = [s1,s2]; //descriptor list

		var step = 0;
		var i = 0;
		while(++i < 50) {

			Poll(dlist, 5);
			if ( !(++step % 4) ) {

				s1.Write('1234');
			}
		}
	},


	Interval: function() {

		var t0 = IntervalNow();
		Sleep(250);
		var t = IntervalNow() - t0;
		QA.ASSERT( t >= 250 && t < 270, true, 'time accuracy' );
	},



	SharedMemory: function() {
		
		LoadModule('jsio');

		var mem = new SharedMemory( 'test.txt', 100 );
		mem.content = 'xxxxxx789';
		mem.Write('xxx456');
		mem.Write('123', 0);
		mem.Write('ABC', 9);
		var mem2 = new SharedMemory( 'test.txt', 100 );
		QA.ASSERT( mem2.Read(), '123456789ABC', 'content' );
		QA.ASSERT( mem2.content.length, 12, 'used memory length' );
		QA.ASSERT( mem2.content, '123456789ABC', 'content' );
		mem2.Write('Z',99);
		QA.ASSERT( mem.Read(1, 99), 'Z', 'writing at the end' );
	},
	
	
	FileIO: function() {
	
		var f = new File('qatest.txt')
		f.Open(File.CREATE_FILE | File.RDWR);
		f.Write('abcd');
		QA.ASSERT( f.exist, true, 'file exist' );
		f.Close();
		f.Open(File.CREATE_FILE | File.RDWR);
		f.Seek(1);
		QA.ASSERT( f.Read(), 'bcd', 'read file' );
		f.Close();
		f.Delete();	
		QA.ASSERT( f.exist, false, 'file delete' );
	}
	

})