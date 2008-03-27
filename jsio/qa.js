({

	SystemInfo: function(QA) {

		QA.ASSERT( typeof systemInfo, 'object', 'system info' );
		var res = delete systemInfo.architecture;
		QA.ASSERT( res, false, 'delete a property' );
		var res = delete systemInfo.name;
		QA.ASSERT( res, false, 'delete a property' );
		var res = delete systemInfo.release;
		QA.ASSERT( res, false, 'delete a property' );
		QA.ASSERT_HAS_PROPERTIES( systemInfo, 'architecture,name,release' );
	},


	PhysicalMemory: function(QA) {

		QA.ASSERT( physicalMemorySize == physicalMemorySize && physicalMemorySize > 1000000, true, 'physical Memory Size' );
	},


	Noise: function(QA) {
	
		QA.ASSERT( GetRandomNoise(8).length, 8, 'random noise' );
	},


	Environment: function(QA) {
		
		QA.ASSERT( GetEnv('PATH').length > 1, true, 'get an environment' );
		QA.ASSERT( GetEnv('sdfrwetwergfqwuyoruiqwye'), undefined, 'undefined environment variable' );
	},


	ProcessPriority: function(QA) {
	
		var save = processPriority;
		
		processPriority = -1;
		QA.ASSERT( processPriority, -1, 'is thread priority -1' );
		processPriority = 0;
		QA.ASSERT( processPriority, 0, 'is thread priority 0' );
		processPriority = 1;
		QA.ASSERT( processPriority, 1, 'is thread priority 1' );
		processPriority = 2;
		QA.ASSERT( processPriority, 2, 'is thread priority 2' );
		
		processPriority = save;
	},


	GetHostByName: function(QA) {

		var res = Socket.GetHostsByName('localhost');
		QA.ASSERT( res.indexOf('127.0.0.1') != -1, true, 'localhost is 127.0.0.1' );

		var res = Socket.GetHostsByName(hostName);
		QA.ASSERT( res.length >= 1, true, 'find hostName' );
	},


	EmptyPoll: function(QA) {
		
		var t0 = IntervalNow();
		var count = Poll([], 100);
		var t = IntervalNow() - t0;
		QA.ASSERT( count, 0, 'descriptor event count' );
		QA.ASSERT( t >= 100 && t < 120, true, 'poll timeout' );
	},


	NonBlockingTCPSocket: function(QA) {

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

				QA.ASSERT( IsReadable(s), true, 'socket is readable' );
				QA.ASSERT( IsWritable(s), true, 'socket is writable' );
				QA.ASSERT( s.Read(), '1234', 'read data match' );
				count++;
			}
		}

		serverSocket.Bind( 9998, '127.0.0.1' );
		
		QA.ASSERT( DumpObjectPrivate(serverSocket) != 0, true, 'descriptor' );
		
		serverSocket.Listen();
		dlist.push(serverSocket);
		var clientSocket = new Socket();
		clientSocket.nonblocking = true;
		
		QA.ASSERT( clientSocket.nonblocking, true, 'non blocking state' );
		clientSocket.exception = function(s) { QA.ASSERT( true, false, 'client.exception') }

		clientSocket.Connect( 'localhost', 9998 );

		Sleep(50);

		QA.ASSERT( IsReadable(clientSocket), false, 'socket is readable' );
		QA.ASSERT( IsWritable(clientSocket), true, 'socket is writable' );

		dlist.push(clientSocket);

		for ( var i = 0; i < 10; i++ ) {

			Poll(dlist, 10);
			if ( !(++step % 4) ) {

				clientSocket.Write('1234');
			}
		}
	},

	NonBlockingUDPSocket: function(QA) {
	
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

	TCPGet: function(QA) {

		var host = 'proxy';
		try {
			
			var response;
		
			var soc = new Socket();
			soc.nonblocking = true;
			soc.Connect( host, 80 );
			soc.writable = function(s) {

				QA.ASSERT_TYPE( s, Socket,  'object is a Socket' );
				QA.ASSERT( s.closed , false,  'Socket descriptor is closed' );

				delete soc.writable;
				QA.ASSERT_HAS_PROPERTIES( s, 'Write' );
				s.Write('GET\r\n');
			}

			soc.readable = function(s) {
				
				response += s.Read();
			}

			var i = 0;
			while( ++i < 10 )
				Poll([soc], 20);
				
		} catch( ex if ex instanceof IoError ) {
			
			if (ex.code == -5981) {
				
				QA.FAILED( 'unable to connect to '+host );
				return;
			}
		}
		QA.ASSERT( !!response, true, 'host response length' );
	},
	
	ClosingSocket: function(QA) {

		var soc = new Socket();
		QA.ASSERT( soc.type, Descriptor.DESC_SOCKET_TCP, 'descriptor type' );
		QA.ASSERT( soc.closed, false, 'socket is not closed' );
		soc.Close();		
		QA.ASSERT( soc.closed, true, 'socket is closed' );
	},


	Interval: function(QA) {

		var t0 = IntervalNow();
		Sleep(250);
		var t = IntervalNow() - t0;
		QA.ASSERT( t >= 250 && t < 270, true, 'time accuracy' );
	},


	SharedMemory: function(QA) {
		
		LoadModule('jsio');
		var mem = new SharedMemory( 'qa_tmp_sm.txt', 100 );
		mem.content = 'xxxxxx789';
		mem.Write('xxx456');
		mem.Write('123', 0);
		mem.Write('ABC', 9);
		var mem2 = new SharedMemory( 'qa_tmp_sm.txt', 100 );
		QA.ASSERT( mem2.Read(), '123456789ABC', 'content' );
		QA.ASSERT( mem2.content.length, 12, 'used memory length' );
		QA.ASSERT( mem2.content, '123456789ABC', 'content' );
		mem2.Write('Z',99);
		QA.ASSERT( mem.Read(1, 99), 'Z', 'writing at the end' );
	},


	StdFile: function(QA) {
		
		QA.ASSERT_HAS_PROPERTIES( File, 'stdin,stdout,stderr' );
		QA.ASSERT( File.stdin.type, File.FILE_FILE, 'stdin file type');
		QA.ASSERT( File.stdout.type, File.FILE_FILE, 'stdout file type');
		QA.ASSERT( File.stderr.type, File.FILE_FILE, 'stderr file type');
	},


	FileIO: function(QA) {
	
		var f = new File('qa_tmp_file.txt')
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
	},


	FileContent: function(QA) {

		var data = String(new Date());
		var f = new File('qa_tmp_content.txt');
		QA.ASSERT( f.exist, false, 'file exist' );
		f.content = data;
		QA.ASSERT( f.exist, true, 'file exist' );
		QA.ASSERT( f.content, data, 'file content' );
		f.content = undefined;
		QA.ASSERT( f.exist, false, 'file exist' );
	},


	FileException: function(QA) {
	
		var f = new File('qa_tmp_notfound.txt')
		try {
			f.Open(File.RDONLY);
		} catch( ex if ex instanceof IoError ) {

			QA.ASSERT( ex.code, -5950, 'IoError code' );
			QA.ASSERT( ex.text, 'File not found', 'IoError text' );
			QA.ASSERT( ex.os != 0, true, 'OS code' );
		} catch(ex) {
			
			QA.ASSERT( false, true, 'IoError type' );
		}
	},


	FileConst: function(QA) {
	
		QA.ASSERT_HAS_PROPERTIES( File, 'RDONLY,WRONLY,RDWR,CREATE_FILE,APPEND,TRUNCATE,SYNC,EXCL' );
		QA.ASSERT_HAS_PROPERTIES( File, 'SEEK_SET,SEEK_CUR,SEEK_END' );
		QA.ASSERT_HAS_PROPERTIES( File, 'FILE_FILE,FILE_DIRECTORY,FILE_OTHER' );
	},


	DirectoryConst: function(QA) {
		
		delete Directory.SKIP_DOT_DOT;
		QA.ASSERT_HAS_PROPERTIES( Directory, 'SKIP_NONE,SKIP_DOT,SKIP_DOT_DOT,SKIP_BOTH,SKIP_HIDDEN' );
	},


	Directory: function(QA) {
		
		var f = new File('qa_tmp_dir.txt');
		f.content = 'test';
		var dir = Directory.List('./.', Directory.SKIP_BOTH | Directory.SKIP_DIRECTORY | Directory.SKIP_OTHER );
		QA.ASSERT( dir.indexOf('qa_tmp_dir.txt') != -1, true, 'directory listing' );
		f.content = undefined;
	},


	DirectoryExist: function(QA) {
		
		var d = new Directory('.');
		QA.ASSERT( d.exist, true, 'directory exist' );
		var d1 = new Directory('qa_directory_do_not_exist');
		QA.ASSERT( d1.exist, false, 'directory exist' );
		delete d1.name;
		QA.ASSERT( d1.name, 'qa_directory_do_not_exist', 'directory name' );
	},


	CreateProcessReadPipe: function(QA) {
		
		switch (systemInfo.name) {
			case 'Windows_NT':
				
				var res = CreateProcess('C:\\WINDOWS\\system32\\cmd.exe', ['/c', 'dir']);
				QA.ASSERT_TYPE( res, Array, 'CreateProcess returns an array' );
				QA.ASSERT( res.length, 2, 'CreateProcess array length' );
				QA.ASSERT_TYPE( res[0], Descriptor, 'process stdin type' );
				QA.ASSERT_TYPE( res[1], Descriptor, 'process stdout type' );
				QA.ASSERT( res[1].Read(10).length, 10, 'reading Process stdout' );
				break;
			default:
				QA.FAILED('(TBD) no test available for this system.');
		}
	},
	
	CreateProcessWaitExit: function(QA) {
		
		switch (systemInfo.name) {
			case 'Windows_NT':
				
				var res = CreateProcess('cmd.exe', ['/c', 'dir'], true);
				QA.ASSERT_TYPE( res, 'number', 'CreateProcess returns an array' );
				break;
			default:
				QA.FAILED('(TBD) no test available for this system.');
		}
	},
	
	CreateProcessErrorDetection: function(QA) {
		
		switch (systemInfo.name) {
			case 'Windows_NT':
				
				try {
				
					var res = CreateProcess('uryqoiwueyrqoweu', [], true);
				} catch( ex if ex instanceof IoError ) {
				
					QA.ASSERT( ex.code, -5994, 'CreateProcess error detection' );
				}
				break;
			default:
				QA.FAILED('(TBD) no test available for this system.');
		}
	},


	CWD: function(QA) {
	
		QA.ASSERT( 'currentWorkingDirectory' in global, true, 'has currentWorkingDirectory' );
		QA.ASSERT( typeof currentWorkingDirectory, 'string', 'current working directory' );
		QA.ASSERT( currentWorkingDirectory.length >= 1, true, 'cwd length' );
	}

})
