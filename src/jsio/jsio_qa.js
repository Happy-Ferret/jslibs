// don't remove this first line !! ( see MemoryMapped test )
LoadModule('jsio');

/// File ancestor [ftr]

		var f = new File('');
		QA.ASSERT( f instanceof Descriptor, true, 'Descriptor inheritance' );


/// system info [ftr]

		QA.ASSERT( typeof systemInfo, 'object', 'system info' );
		var res = delete systemInfo.architecture;
		QA.ASSERT( res, false, 'delete a property' );
		var res = delete systemInfo.name;
		QA.ASSERT( res, false, 'delete a property' );
		var res = delete systemInfo.release;
		QA.ASSERT( res, false, 'delete a property' );
		QA.ASSERT_HAS_PROPERTIES( systemInfo, 'architecture,name,release' );


/// physical memory [ftr]

		QA.ASSERT( physicalMemorySize == physicalMemorySize && physicalMemorySize > 1000000, true, 'physical Memory Size' );


/// Noise generation 
		
		QA.ASSERT( GetRandomNoise(1).length, 1, 'random noise 1 byte' );
		QA.ASSERT( GetRandomNoise(3).length, 3, 'random noise 3 bytes' );


/// environment variables [ft]
		
		QA.ASSERT( GetEnv('PATH').length > 1, true, 'get an environment' );
		QA.ASSERT( GetEnv('sdfrwetwergfqwuyoruiqwye'), undefined, 'undefined environment variable' );


/// process priority [ftr]
	
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


/// host name [ft]

		switch (systemInfo.name) {
			case 'Windows_NT':
				
				QA.ASSERT( GetEnv('COMPUTERNAME').toLowerCase(), hostName.toLowerCase(), 'COMPUTERNAME and hostName' );
				break;
			default:
				QA.FAILED('(TBD) no test available for this system.');
		}


/// GetHostByName function [r]

		var res = Socket.GetHostsByName('localhost');
		QA.ASSERT( res.indexOf('127.0.0.1') != -1, true, 'localhost is 127.0.0.1' );

		var res = Socket.GetHostsByName(QA.RandomString(25));
		QA.ASSERT_TYPE( res, Array );
		QA.ASSERT( res.length, 0, 'find nonexistent hostName' );


/// GetHostByName function with hostName argument

		var res = Socket.GetHostsByName(hostName);
		QA.ASSERT( res.length >= 1, true, 'find hostName (may fail)' );


/// empty poll [tr]

		var t0 = IntervalNow();
		var count = Poll([], 100);
		var t = IntervalNow() - t0;
		QA.ASSERT( count, 0, 'descriptor event count' );
		QA.ASSERT( t >= 99 && t < 150, true, 'poll timeout (may fail if high CPU load) t='+t );


/// Non-blocking TCP Socket [tr]

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
				QA.ASSERT_STR( s.Read(), '1234', 'read data match' );
				count++;
			}
		}

		serverSocket.Bind( 9998, '127.0.0.1' );
		
		QA.ASSERT( GetObjectPrivate(serverSocket) != 0, true, 'descriptor' );
		
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
		
		while (dlist.length)
			dlist.pop().Close();

/// Creating a lot of sockets + GC [r]

	var s = [];
	for ( var j = 0; j < 1000; j++ ) {

		for ( var i=0 ; i < 100 ; i++ )
			s.push(new Socket( Socket.TCP ));
		s = [];
		QA.GC();
	}

/// Create/remove a lot of sockets [r]

	var s = [];
	for ( var i=0 ; i < 500 ; i++ )
		s.push(new Socket( Socket.TCP ));
	while (s.length)
		s.pop().Close();


/// Non-blocking UDP Socket [tr]
	
		var s2 = new Socket( Socket.UDP );
		s2.nonblocking = true;
		s2.Bind(9999);
		s2.readable = function(s) {

			var [data, ip, port] = s.RecvFrom();
			QA.ASSERT_STR( data, '1234', 'received data' );
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


/// TCP get

		var res, host, hostList = ['proxy', 'www.google.com', 'localhost']; // try to find a web server on port 80
		do {

			host = hostList.shift();
			res = Socket.GetHostsByName(host);
		} while ( !res || res.length == 0 );
		
		QA.ASSERT( res && res.length > 0, true, 'unable to find a host' );

		
		try {
			
			var response = '';
		
			var soc = new Socket();
			soc.nonblocking = true;
			soc.Connect( host, 80 );
			soc.writable = function(s) {
			
				QA.ASSERT_TYPE( s, Socket,  'object is a Socket' );
				QA.ASSERT( s.closed , false,  'Socket descriptor is closed' );

				delete soc.writable;
				QA.ASSERT_HAS_PROPERTIES( s, 'Write' );
				s.Write('GET\r\n\r\n');
			}
			soc.readable = function(s) {
				
				var res = s.Read();
				if ( res )
					response += res;
			}
			
			var i = 0;
			while( ++i < 100 )
				Poll([soc], 20);
				
		} catch( ex if ex instanceof IoError ) {
			
			if (ex.code == -5981) {
				
				QA.FAILED( 'unable to connect to '+host );
				return;
			}
		}
		
		QA.ASSERT( response.length > 0, true, 'has response content from '+host );


/// closing Socket [ftr]

		var soc = new Socket();
		QA.ASSERT( soc.type, Descriptor.DESC_SOCKET_TCP, 'descriptor type' );
		QA.ASSERT( soc.closed, false, 'socket is not closed' );
		soc.Close();		
		QA.ASSERT( soc.closed, true, 'socket is closed' );


/// time interval [tr]

		var t0 = IntervalNow();
		Sleep(250);
		var t = IntervalNow() - t0;
		QA.ASSERT( t >= 249 && t < 275, true, 'time accuracy' );


/// shared memory simple test 1 [ftr]

		var fileName = 'qa'+QA.RandomString(10);
		var mem = new SharedMemory( fileName, 100 );
		mem.content = ' abcdef ';
		QA.ASSERT_STR( mem.content, ' abcdef ', 'content' );
		mem.Close();
		QA.ASSERT( new File(fileName).exist, false, 'SharedMemory file has been removed (linux only ?)' );
		

/// shared memory simple test 2 [ftr]

		var fileName = 'qa'+QA.RandomString(10);
		
		var mem = new SharedMemory( fileName, 100 );
		mem.content = 'xxxxxx789';
		mem.Write('xxx456');
		mem.Write('123', 0);
		mem.Write('ABC', 9);
		QA.ASSERT_STR( mem.Read(), '123456789ABC', 'content' );
		QA.ASSERT_STR( mem.content, '123456789ABC', 'content' );
		mem.Close();
		QA.ASSERT( new File(fileName).exist, false, 'SharedMemory file has been removed (linux only ?)' );


/// shared memory [ftr]

		var fileName = 'qa'+QA.RandomString(10);
		
		var mem = new SharedMemory( fileName, 100 );
		mem.content = 'xxxxxx789';
		mem.Write('xxx456');
		mem.Write('123', 0);
		mem.Write('ABC', 9);
		var mem2 = new SharedMemory( fileName, 100 );
		QA.ASSERT_STR( mem2.Read(), '123456789ABC', 'content' );
		QA.ASSERT( mem2.content.length, 12, 'used memory length' );
		QA.ASSERT_STR( mem2.content, '123456789ABC', 'content' );
		mem2.Write('Z',99);
		QA.ASSERT_STR( mem.Read(1, 99), 'Z', 'writing at the end' );
		mem.Close();
		mem2.Close();
		QA.ASSERT( new File(fileName).exist, false, 'SharedMemory file has been removed (linux only ?)' );


/// standard file descriptors [ftr]
		
		QA.ASSERT_HAS_PROPERTIES( File, 'stdin,stdout,stderr' );
		QA.ASSERT( File.stdin.type, File.FILE_FILE, 'stdin file type');
		QA.ASSERT( File.stdout.type, File.FILE_FILE, 'stdout file type');
		QA.ASSERT( File.stderr.type, File.FILE_FILE, 'stderr file type');


/// File I/O [ftr]

		var f = new File('qa_tmp_file.txt')
		f.Open(File.CREATE_FILE | File.RDWR);
		f.Write('abcd');
		QA.ASSERT( f.exist, true, 'file exist' );
		f.Close();
		f.Open(File.CREATE_FILE | File.RDWR);
		f.Seek(1);
		QA.ASSERT_STR( f.Read(), 'bcd', 'read file' );
		f.Close();
		f.Delete();	
		QA.ASSERT( f.exist, false, 'file delete' );


/// File info [ftr]
	
		var f = new File('qa_tmp_file_FileInfo.txt');
		
		QA.ASSERT( f.exist, false, 'file delete' );

		f.content = 'xx'
		QA.ASSERT( f.info.size, 2, 'file size' );

		QA.ASSERT( Math.abs( f.info.modifyTime - new Date().getTime() ) < 2000, true, 'file date' );

		f.content = 'xxx';
		QA.ASSERT( f.info.size, 3, 'file size' );

		QA.ASSERT( f.info.type, File.FILE_FILE, 'file type' );

		f.Delete();


/// File content [ftr]

		var data = String(new Date());
		var f = new File('qa_tmp_content.txt');
		QA.ASSERT( f.exist, false, 'file exist' );
		f.content = data;
		QA.ASSERT( f.exist, true, 'file exist' );
		QA.ASSERT_STR( f.content, data, 'file content' );
		f.content = undefined;
		QA.ASSERT( f.exist, false, 'file exist' );


/// File exception [ftr]

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


/// File constants [ftr]
	
		QA.ASSERT_HAS_PROPERTIES( File, 'RDONLY,WRONLY,RDWR,CREATE_FILE,APPEND,TRUNCATE,SYNC,EXCL' );
		QA.ASSERT_HAS_PROPERTIES( File, 'SEEK_SET,SEEK_CUR,SEEK_END' );
		QA.ASSERT_HAS_PROPERTIES( File, 'FILE_FILE,FILE_DIRECTORY,FILE_OTHER' );


/// Directory constants [ftr]
		
		delete Directory.SKIP_DOT_DOT;
		QA.ASSERT_HAS_PROPERTIES( Directory, 'SKIP_NONE,SKIP_DOT,SKIP_DOT_DOT,SKIP_BOTH,SKIP_HIDDEN' );


/// Directory [ftr]
		
		var f = new File('qa_tmp_dir.txt');
		f.content = 'test';
		var dir = Directory.List('./.', Directory.SKIP_BOTH | Directory.SKIP_DIRECTORY | Directory.SKIP_OTHER );
		QA.ASSERT( dir.indexOf('qa_tmp_dir.txt') != -1, true, 'directory listing' );
		f.content = undefined;


/// Directory exist [ftr]
		
		var d = new Directory('.');
		QA.ASSERT( d.exist, true, 'directory exist' );
		var d1 = new Directory('qa_directory_do_not_exist');
		QA.ASSERT( d1.exist, false, 'directory exist' );
		delete d1.name;
		QA.ASSERT( d1.name, 'qa_directory_do_not_exist', 'directory name' );


/// create process read pipe
	
		switch (systemInfo.name) {
			case 'Windows_NT':
/* previous implementation
				var cmdPath = GetEnv('ComSpec');
				QA.ASSERT( cmdPath.indexOf('cmd') != -1, true, 'cmd.exe path' );

				var res = CreateProcess(cmdPath, ['/c', 'dir']);
				QA.ASSERT_TYPE( res, Array, 'CreateProcess returns an array' );
				QA.ASSERT( res.length, 2, 'CreateProcess array length' );
				QA.ASSERT_TYPE( res[0], Descriptor, 'process stdin type' );
				QA.ASSERT_TYPE( res[1], Descriptor, 'process stdout type' );
				QA.ASSERT( res[1].Read(10).length, 10, 'reading Process stdout' );
*/
				var cmdPath = GetEnv('ComSpec');
				QA.ASSERT( cmdPath.indexOf('cmd') != -1, true, 'cmd.exe path' );
				var process = new Process(cmdPath, ['/c', 'dir']);
				QA.ASSERT_TYPE( process.stdin, Descriptor, 'process stdin type' );
				QA.ASSERT_TYPE( process.stdout, Descriptor, 'process stdout type' );
				QA.ASSERT_TYPE( process.stderr, Descriptor, 'process stderr type' );
				QA.ASSERT( process.stdout.Read(10).length, 10, 'reading Process stdout' );
				break;
			default:
				QA.FAILED('(TBD) no test available for this system.');
		}


/// create process wait for exitcode

		switch ( systemInfo.name ) {
			case 'Windows_NT':
				var cmd = GetEnv('ComSpec');
				var args1 = ['/c', 'cd fvasdfvasdfvasdfv'];
				var args2 = ['/c', 'dir'];
				break;
			case 'Linux':
				var cmd = GetEnv('SHELL');
				var args1 = ['-c', 'cd fvasdfvasdfvasdfv'];
				var args2 = ['-c', 'ls'];
				break;
			default:
				QA.FAILED('(TBD) no test available for this system.');
				return;
		}
				
		var process = new Process(cmd, args1);
		var exitCode = process.Wait();
		QA.ASSERT( exitCode, 1, 'process exit code 1' );

		var process = new Process(cmd, args2);
		var exitCode = process.Wait();
		QA.ASSERT( exitCode, 0, 'process exit code 0' );


/// create process error detection [ftr]

		try {

			var res = new Process('uryqoiwueyrqoweu');
		} catch( ex if ex instanceof IoError ) {

			QA.ASSERT( ex.code, -5994, 'CreateProcess error detection' );
			return;
		}
		QA.FAILED( "no exception (cf mozilla bug #113095)" );
	

/// create process detach
		
		switch ( systemInfo.name ) {
			case 'Windows_NT':
				var cmd = GetEnv('ComSpec');
				var args = ['/c', 'dir'];
				break;
			case 'Linux':
				var cmd = GetEnv('SHELL');
				var args = ['-c', 'ls'];
				break;
			default:
				QA.FAILED('(TBD) no test available for this system.');
				return;
		}
		
		var process = new Process(cmd, args);
		process.Detach();


/// current working directory [ftr]
	
		QA.ASSERT( 'currentDirectory' in global, true, 'has currentDirectory' );
		QA.ASSERT( typeof currentDirectory, 'string', 'current directory' );
		QA.ASSERT( currentDirectory.length >= 1, true, 'current directory length' );


/// MemoryMapped class [ftr]

		var thisFilename = ITEM.file;
		
		var m = new MemoryMapped(new File(thisFilename).Open('r'));
		
		QA.ASSERT( m.file instanceof File, true, 'instanceof .file memeber' );

		QA.ASSERT_STR( new Stream(m).Read(15), '// don\'t remove', 'convert to a Stream (1)' );
		QA.ASSERT_STR( new Stream(m).Read(15), '// don\'t remove', 'convert to a Stream (2)' );

		QA.ASSERT_STR( Stringify(m).substr(0,15), '// don\'t remove', 'stringify it' );
