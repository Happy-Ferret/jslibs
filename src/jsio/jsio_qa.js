// don't remove this first line !! ( see MemoryMapped test )
loadModule('jsio');
loadModule('jsdebug');

/// listen: several times on the same address

	var s1 = new Socket();
	s1.bind(9999, '127.0.0.1');
	s1.listen();

	var s2 = new Socket();
	QA.ASSERTOP( function() s2.bind(9999, '127.0.0.1'), 'ex', IoError );
	QA.ASSERTOP( function() s2.listen(), 'ex', IoError );
	s1.close();
	s2.close();

/// bind: several times on the same address

	var s1 = new Socket();
	s1.bind(9999, '127.0.0.1');
	s1.listen();

	var s2 = new Socket();
	QA.ASSERTOP( function() s2.bind(9999, '127.0.0.1'), 'ex', IoError );
	s1.close();
	s2.close();

/// listen multiple times on the same fd

	var rdv = new Socket(); rdv.bind(9999, '127.0.0.1'); rdv.listen(); rdv.readable = true;
	var cl = new Socket(); cl.connect('127.0.0.1', 9999);
	var io = Descriptor.events([rdv,rdv,rdv,rdv]);
	processEvents( io );
	rdv.close();


/// recycle upe event

	var rdv = new Socket(); rdv.bind(9999, '127.0.0.1'); rdv.listen(); rdv.readable = true;
	var cl = new Socket(); cl.connect('127.0.0.1', 9999);
	var io = Descriptor.events([rdv]);
	processEvents( io );
	processEvents( io );
	rdv.close();


/// Socket shutdown(true) behavior [trm]

	var rdv = new Socket(); rdv.bind(9999, '127.0.0.1'); rdv.listen(); rdv.readable = true;
	var cl = new Socket(); cl.connect('127.0.0.1', 9999);
	processEvents( Descriptor.events([rdv]), timeoutEvents(2000) );
	var sv = rdv.accept(); rdv.close();

	var data = stringRepeat('x', 100);
	sv.write(data);
	sv.shutdown(true);

	QA.ASSERT_STR( cl.read(), data, 'data' );
	QA.ASSERT( cl.read(), undefined, 'No more data' );



/// Socket shutdown(false) behavior [trm]

	var rdv = new Socket(); rdv.bind(9999, '127.0.0.1'); rdv.listen(); rdv.readable = true;
	var cl = new Socket();
	cl.nonblocking = true;
	cl.connect('127.0.0.1', 9999);
	processEvents( Descriptor.events([rdv]), timeoutEvents(2000) );
	var sv = rdv.accept(); rdv.close();


	var data = stringRepeat('x', 100);
	sv.write(data);
	sv.shutdown(false);

	QA.ASSERT_STR( cl.read(), data, 'data' );
	QA.ASSERT( stringify(cl.read()), '', 'no data' );



/// basic Socket client/server [trm]

	var rdv = new Socket(); rdv.bind(9999, '127.0.0.1'); rdv.listen(); rdv.readable = true;
	var cl = new Socket(); cl.connect('127.0.0.1', 9999);
	processEvents( Descriptor.events([rdv]), timeoutEvents(2000) );
	var sv = rdv.accept(); rdv.close();

	sv.write(stringRepeat('x', 10000));
	sleep(10); // need a gracefull close
	sv.close();

	var data = [];
	var chunk;
	while ( (chunk = cl.read()) != undefined ) {
		
		data.push(chunk);
	}
	
	var result = join(data);

	QA.ASSERT( result.length, 10000, 'read amount' );
	QA.ASSERT( cl.read(), undefined, 'read done' );



/// Descriptor inheritance

	QA.ASSERT( new Socket().import, undefined, 'new Socket .import unavailable' );
	QA.ASSERT( Socket.import, undefined, 'Socket.import unavailable' );
	QA.ASSERT( Descriptor.import != undefined, true, 'Descriptor.import available' );


/// Binary data test

	var filename = QA.randomString(10);
	var file = new File(filename);
	file.content = "\xBC\x00\x30\x01"
	var data = stringify(file.content);
	file.content = undefined;
	QA.ASSERT_STR( data[0], '\xBC' );
	QA.ASSERT_STR( data[1], '\x00' );
	QA.ASSERT_STR( data[2], '\x30' );
	QA.ASSERT_STR( data[3], '\x01' );



/// Basic File Read/Write [tr]

	var filename = QA.randomString(10);

	var f1 = new File(filename).open('w');
	f1.write('1234');
	f1.close();

	var f2 = new File(filename).open('r');
	f2.read(4);
	f2.close();

	f1.delete();



/// File Read [tr]

		var filename = QA.randomString(10);

		var f1 = new File(filename).open('w');
		var f2 = new File(filename).open('r');
		
		f1.write('1234');
		QA.ASSERT_STR( f2.read(5), '1234' );
		f1.write('56');
		QA.ASSERT_STR( f2.read(1), '5' );
		QA.ASSERT_STR( f2.read(1), '6' );
		QA.ASSERT( f2.read(1), undefined );
		QA.ASSERT( f2.read(), undefined );
		f1.write('78');
		QA.ASSERT_STR( f2.read(), '78' );
		QA.ASSERT( f2.read(), undefined );
		f1.write('9');
		QA.ASSERT_STR( f2.read(), '9' );
		QA.ASSERT( f2.read(), undefined );
		f1.write('');
		QA.ASSERT( f2.read(), undefined );
		var str = stringRepeat('x', 1023);
		f1.write(str);
		QA.ASSERT_STR( f2.read(), str );
		var str = stringRepeat('x', 1024);
		f1.write(str);
		QA.ASSERT_STR( f2.read(), str );
		var str = stringRepeat('x', 1025);
		f1.write(str);
		QA.ASSERT_STR( f2.read(), str );
		QA.ASSERT( f2.read(), undefined );
		var str = stringRepeat('x', 1000000);
		f1.write(str);
		QA.ASSERT_STR( f2.read(), str );
		QA.ASSERT( f2.read(), undefined );
		
		f1.close();
		f2.close();
		
		new File(filename).content = undefined;



/// File busy [trfm]	

		var filename = QA.randomString(10);

		var f2 = new File(filename).open('w');
		QA.ASSERTOP(function() { new File(filename).content = undefined }, 'ex', IoError, 'Checking busy file error' );
		f2.close();
		new File(filename).content = undefined;



/// File copy [tr]

		var filename = QA.randomString(10);

		function copy(fromFilename, toFilename) {

			var fromFile = new File(fromFilename).open(File.RDONLY);
			var toFile = new File(toFilename).open(File.WRONLY | File.CREATE_FILE | File.TRUNCATE);
			for ( var buf; (buf = fromFile.read(65536)) != undefined; )
			toFile.write(buf);
			toFile.close();
			fromFile.close();
		}
		
		var file = new File(filename).open('w');
		for ( var i = 0; i < 1000; i++ )
			file.write( stringRepeat('z', 1024) );
		file.close();

		copy( filename, 'copy_'+filename );
		
		var cf = new File( 'copy_'+filename );
		QA.ASSERT( cf.info.size, 1000*1024, 'copied file size');

		cf.open('r');
		QA.ASSERT_STR( cf.read(), stringRepeat('z', 1000*1024), 'copied data');
		cf.close();		

		cf.delete();
		file.delete();



/// File ancestor [ftrm]

		var f = new File('');
		QA.ASSERT( f instanceof File, true, 'File inheritance' );
		QA.ASSERT( f instanceof Descriptor, true, 'Descriptor inheritance' );



/// system info [ftmr]

		QA.ASSERTOP( global, 'has', 'architecture' );
		QA.ASSERTOP( global, 'has', 'systemName' );
		QA.ASSERTOP( global, 'has', 'systemRelease' );


/// physical memory [ftrm]

		QA.ASSERT( physicalMemorySize == physicalMemorySize && physicalMemorySize > 1000000, true, 'physical Memory size' );


/// Noise generation [m]
		
		QA.ASSERT( getRandomNoise(1).byteLength, 1, 'random noise 1 byte' );
		QA.ASSERT( getRandomNoise(3).byteLength, 3, 'random noise 3 bytes' );


/// environment variables [ftm]
		
		QA.ASSERT( getEnv('PATH').length > 1, true, 'get an environment' );
		QA.ASSERT( getEnv('sdfrwetwergfqwuyoruiqwye'), undefined, 'undefined environment variable' );


/// process priority [ftrm]
	
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


/// Process pipes type [ftrm]

	var cmdPath = getEnv('ComSpec');
	var process = new Process(cmdPath, ['/c', 'cd']);
	QA.ASSERTOP( process.stdout, 'instanceof', Descriptor, 'stdout instanceof Descriptor' );


/// Process arguments [ftrm]

	var process = new Process('jshost', ['-u', '-i', 'host.stdout(host.arguments.toString())', '1', '2', '3']);
	var res = stringify(process.stdout.read());
	QA.ASSERT_STR( res, "1,2,3", "Process arguments validity" );


/// Process default current directory [ftrm]

	var process = new Process('jshost', ['-u', '-i', 'loadModule = host.loadModule; loadModule("jsio"); host.stdout(currentDirectory)']);
	var res = stringify(process.stdout.read());
	QA.ASSERT_STR( res, currentDirectory );


/// directorySeparator test

	QA.ASSERT( directorySeparator.length,  1, 'directorySeparator length' );
	QA.ASSERT( directorySeparator == '/' || directorySeparator == '\\', true, 'directorySeparator value' );


/// pathSeparator test

	QA.ASSERT( pathSeparator.length, 1, 'pathSeparator length' );


/// Process current directory [ftrm]
	
	var process = new Process('jshost', ['-u', '-i', 'loadModule = host.loadModule; loadModule("jsio"); host.stdout(currentDirectory)'], '..');
	var res = stringify(process.stdout.read());
	QA.ASSERTOP( res, '==', currentDirectory.substr(0, currentDirectory.lastIndexOf(directorySeparator)) );


/// Process no stdio redirect [ftrm] (possible stdio perturbations)

	var cmdPath = getEnv('ComSpec');
	var process = new Process(cmdPath, ['/c', 'cd'], undefined, false);
	QA.ASSERTOP( process.stdout, '===', undefined, 'stdout is not defined' );
	QA.ASSERTOP( process.stderr, '===', undefined, 'stderr is not defined' );
	QA.ASSERTOP( process.stdin, '===', undefined, 'stdin is not defined' );


/// host name [ftm]

	switch (systemName) {
		case 'Windows_NT':
			
			QA.ASSERT( getEnv('COMPUTERNAME').toLowerCase(), hostName.toLowerCase(), 'COMPUTERNAME and hostName' );
			break;
		default:
			QA.FAILED('(TBD) no test available for this system.');
	}


/// GetHostByName function [rm]

		var res = Socket.getHostsByName('localhost');
		QA.ASSERT( res.indexOf('127.0.0.1') != -1, true, 'localhost is 127.0.0.1' );

		var res = Socket.getHostsByName(QA.randomString(25));
		QA.ASSERTOP( res, 'instanceof', Array );
		QA.ASSERT( res.length, 0, 'find nonexistent hostName' );


/// GetHostByName function with hostName argument [m]

		var res = Socket.getHostsByName(hostName);
		QA.ASSERT( res.length >= 1, true, 'find hostName (may fail)' );


/// empty poll [trm]

		var t0 = intervalNow();
		var count = poll([], 100);
		var t = intervalNow() - t0;
		QA.ASSERT( count, 0, 'descriptor event count' );
		QA.ASSERT( t >= 99 && t < 150, true, 'poll timeout (may fail if high CPU load) t='+t );


/// Non-blocking TCP Socket [trm]
		
		var count = 0;
		var dlist = [];
		var step = 0;

		var serverSocket = new Socket();
		serverSocket.reuseAddr = true;
		serverSocket.nonblocking = true;
		serverSocket.exception = function(s) { QA.ASSERT( true, false, 'serverSocket.exception') }
		serverSocket.readable = function(s) {

			var incomingClient = s.accept(); 
			dlist.push(incomingClient);
			incomingClient.readable = function(s) {

				QA.ASSERT( s.isReadable(), true, 'socket is readable' );
				QA.ASSERT( s.isWritable(), true, 'socket is writable' );
				QA.ASSERT_STR( s.read(), '1234', 'read data match' );
				count++;
			}
		}

		serverSocket.bind( 9998, '127.0.0.1' );
		
		QA.ASSERT( getObjectPrivate(serverSocket) != 0, true, 'descriptor' );
		
		serverSocket.listen();
		dlist.push(serverSocket);
		var clientSocket = new Socket();
		clientSocket.nonblocking = true;
		
		QA.ASSERT( clientSocket.nonblocking, true, 'non blocking state' );
		clientSocket.exception = function(s) { QA.ASSERT( true, false, 'client.exception') }

		clientSocket.connect( 'localhost', 9998 );

		sleep(50);

		QA.ASSERT( clientSocket.isReadable(), false, 'socket is readable' );
		QA.ASSERT( clientSocket.isWritable(), true, 'socket is writable' );

		dlist.push(clientSocket);

		for ( var i = 0; i < 10; i++ ) {

			poll(dlist, 10);
			if ( !(++step % 4) ) {

				clientSocket.write('1234');
			}
		}
		
		while (dlist.length)
		dlist.pop().close();


/// Creating a lot of sockets + GC [r]

	var s = [];
	for ( var j = 0; j < 15; j++ ) {

		for ( var i=0 ; i < 1000 ; i++ )
			s.push(new Socket( Socket.TCP ));
		s = [];
		QA.gc();
	}

/// Create/remove a lot of sockets [r]

	var s = [];
	for ( var i=0 ; i < 1000 ; i++ )
		s.push(new Socket( Socket.TCP ));
	while (s.length)
		s.pop().close();


/// Non-blocking UDP Socket [trm]
	
		var s2 = new Socket( Socket.UDP );
		s2.nonblocking = true;
		s2.bind(9999);
		s2.readable = function(s) {

			var [data, ip, port] = s.recvFrom();
			QA.ASSERT_STR( data, '1234', 'received data' );
		}

		var s1 = new Socket( Socket.UDP );
		s1.reuseAddr = true;
		s1.nonblocking = true;
		s1.connect('127.0.0.1', 9999);

		var dlist = [s1,s2]; //descriptor list

		var step = 0;
		var i = 0;
		while(++i < 50) {

			poll(dlist, 5);
			if ( !(++step % 4) ) {

				s1.write('1234');
			}
		}


/// TCP get [m]

	do {
		var res, host, hostList = ['proxy', 'www.google.com', 'localhost']; // try to find a web server on port 80
		do {

			host = hostList.shift();
			res = Socket.getHostsByName(host);
		} while ( !res || res.length == 0 );
		
		QA.ASSERT( res && res.length > 0, true, 'unable to find a host' );
		
		try {
			
			var response = '';
		
			var soc = new Socket();
			soc.nonblocking = true;
			soc.connect( host, 80 );
			soc.writable = function(s) {
			
				QA.ASSERTOP( s, 'instanceof', Socket, 'object is a Socket' );
				QA.ASSERT( s.closed , false,  'Socket descriptor is closed' );

				delete soc.writable;
				QA.ASSERTOP( s, 'has', 'write' );
				s.write('GET\r\n\r\n');
			}
			soc.readable = function(s) {
				
				var res = s.read();
				if ( res )
					response += res;
			}
			
			var i = 0;
			while( ++i < 400 )
				poll([soc], 5);
				
		} catch( ex if ex instanceof IoError ) {
			
			if (ex.code == -5981) {
				
				QA.FAILED( 'unable to connect to '+host );
				break;
			}
		}
		
		QA.ASSERT( response.length > 0, true, 'has response content from '+host );
		
	} while (0);


/// closing Socket [ftrm]

		var soc = new Socket();
		QA.ASSERT( soc.type, Descriptor.DESC_SOCKET_TCP, 'descriptor type' );
		QA.ASSERT( soc.closed, false, 'socket is not closed' );
		soc.close();		
		QA.ASSERT( soc.closed, true, 'socket is closed' );


/// time interval [trm]

		var t0 = intervalNow();
		sleep(250);
		var t = intervalNow() - t0;
		QA.ASSERT( t >= 249 && t < 275, true, 'time accuracy' );


/// shared memory simple test 1 [ftrm]

		var fileName = 'qa'+QA.randomString(10);
		var mem = new SharedMemory( fileName, 100 );
		mem.content = ' abcdef ';
		QA.ASSERT_STR( mem.content, ' abcdef ', 'content' );
		mem.close();
		QA.ASSERT( new File(fileName).exist, false, 'SharedMemory file has been removed (linux only ?)' );
		

/// shared memory simple test 2 [ftrm]

		var fileName = 'qa'+QA.randomString(10);
		
		var mem = new SharedMemory( fileName, 100 );
		mem.content = 'xxxxxx789';
		mem.write('xxx456');
		mem.write('123', 0);
		mem.write('ABC', 9);
		QA.ASSERT_STR( mem.read(), '123456789ABC', 'content' );
		QA.ASSERT_STR( mem.content, '123456789ABC', 'content' );
		mem.close();
		QA.ASSERT( new File(fileName).exist, false, 'SharedMemory file has been removed (linux only ?)' );


/// shared memory [ftrm]

		var fileName = 'qa'+QA.randomString(10);
		
		var mem = new SharedMemory( fileName, 100 );
		mem.content = 'xxxxxx789';
		mem.write('xxx456');
		mem.write('123', 0);
		mem.write('ABC', 9);
		var mem2 = new SharedMemory( fileName, 100 );
		QA.ASSERT_STR( mem2.read(), '123456789ABC', 'content' );
		QA.ASSERT( mem2.content.byteLength, 12, 'used memory length' );
		QA.ASSERT_STR( mem2.content, '123456789ABC', 'content' );
		mem2.write('Z',99);
		QA.ASSERT_STR( mem.read(1, 99), 'Z', 'writing at the end' );
		mem.close();
		mem2.close();
		QA.ASSERT( new File(fileName).exist, false, 'SharedMemory file has been removed (linux only ?)' );


/// standard file descriptors [ftrm]
		
		QA.ASSERTOP( File, 'has', 'stdin' );
		QA.ASSERTOP( File, 'has', 'stdout' );
		QA.ASSERTOP( File, 'has', 'stderr' );
		QA.ASSERTOP( File.stdin.type, '===', File.FILE_FILE, 'stdin file type');
		QA.ASSERTOP( File.stdout.type, '===', File.FILE_FILE, 'stdout file type');
		QA.ASSERTOP( File.stderr.type, '===', File.FILE_FILE, 'stderr file type');


/// File I/O [ftrm]

		var f = new File('qa_tmp_file.txt')
		f.open(File.CREATE_FILE | File.RDWR);
		f.write('abcd');
		QA.ASSERT( f.exist, true, 'file exist' );
		f.close();
		f.open(File.CREATE_FILE | File.RDWR);
		f.seek(1);
		QA.ASSERT_STR( f.read(), 'bcd', 'read file' );
		f.close();
		f.delete();	
		QA.ASSERT( f.exist, false, 'file delete' );


/// File info [ftrm]
	
		var f = new File('qa_tmp_file_FileInfo.txt');
		
		QA.ASSERT( f.exist, false, 'file delete' );

		f.content = 'xx'
		QA.ASSERT( f.info.size, 2, 'file size' );

		QA.ASSERT( Math.abs( f.info.modifyTime - new Date().getTime() ) < 2000, true, 'file date' );

		f.content = 'xxx';
		QA.ASSERT( f.info.size, 3, 'file size' );

		QA.ASSERT( f.info.type, File.FILE_FILE, 'file type' );

		f.delete();


/// File content [ftrm]

		var f = new File(QA.randomString(10));
		var data = String(new Date());
		QA.ASSERT( f.exist, false, 'file exist' );
		f.content = data;
		QA.ASSERT( f.exist, true, 'file exist' );
		QA.ASSERT_STR( f.content, data, 'file content' );
		f.content = undefined;
		QA.ASSERT( f.exist, false, 'file exist' );

		var f = new File(QA.randomString(10));
		QA.ASSERT( f.content, undefined, 'file2 does not exist' );


/// File exception [ftrm]

		var f = new File('qa_tmp_notfound.txt')
		try {
			f.open(File.RDONLY);
		} catch( ex if ex instanceof IoError ) {

			QA.ASSERT( ex.code, -5950, 'IoError code' );
			QA.ASSERT( ex.const, 'PR_FILE_NOT_FOUND_ERROR', 'IoError const' );
			QA.ASSERT( ex.text, 'File not found', 'IoError text' );
			QA.ASSERT( ex.os != 0, true, 'OS code' );
		} catch(ex) {
			
			QA.ASSERT( false, true, 'IoError type' );
		}


/// File constants [ftrm]

	QA.ASSERTOP( File, 'has', 'RDONLY' );
	QA.ASSERTOP( File, 'has', 'WRONLY' );
	QA.ASSERTOP( File, 'has', 'RDWR' );
	QA.ASSERTOP( File, 'has', 'CREATE_FILE' );
	QA.ASSERTOP( File, 'has', 'APPEND' );
	QA.ASSERTOP( File, 'has', 'TRUNCATE' );
	QA.ASSERTOP( File, 'has', 'SYNC' );
	QA.ASSERTOP( File, 'has', 'EXCL' );

	QA.ASSERTOP( File, 'has', 'SEEK_SET' );
	QA.ASSERTOP( File, 'has', 'SEEK_CUR' );
	QA.ASSERTOP( File, 'has', 'SEEK_END' );

	QA.ASSERTOP( File, 'has', 'FILE_FILE' );
	QA.ASSERTOP( File, 'has', 'FILE_DIRECTORY' );
	QA.ASSERTOP( File, 'has', 'FILE_OTHER' );


/// Directory constants [ftrm]

	QA.ASSERTOP( Directory, 'has', 'SKIP_NONE' );
	QA.ASSERTOP( Directory, 'has', 'SKIP_DOT' );
	QA.ASSERTOP( Directory, 'has', 'SKIP_DOT_DOT' );
	QA.ASSERTOP( Directory, 'has', 'SKIP_BOTH' );
	QA.ASSERTOP( Directory, 'has', 'SKIP_HIDDEN' );

	delete Directory.SKIP_DOT_DOT;
	QA.ASSERTOP( Directory, 'has', 'SKIP_DOT_DOT', 'Directory const cannot be removed' );


/// Directory [ftrm]
		
		var f = new File('qa_tmp_dir.txt');
		f.content = 'test';
		var dir = Directory.list('./.', Directory.SKIP_BOTH | Directory.SKIP_DIRECTORY | Directory.SKIP_OTHER );
		QA.ASSERT( dir.indexOf('qa_tmp_dir.txt') != -1, true, 'directory listing' );
		f.content = undefined;


/// Directory exist [ftrm]
		
		var d = new Directory('.');
		QA.ASSERT( d.exist, true, 'directory exist' );
		var d1 = new Directory('qa_directory_do_not_exist');
		QA.ASSERT( d1.exist, false, 'directory exist' );
		delete d1.name;
		QA.ASSERT( d1.name, 'qa_directory_do_not_exist', 'directory name' );


/// create process read pipe [m]
	
		switch (systemName) {
			case 'Windows_NT':
				var cmdPath = getEnv('ComSpec');
				QA.ASSERT( cmdPath.indexOf('cmd') != -1, true, 'cmd.exe path' );
				var process = new Process(cmdPath, ['/c', 'date', '/T']);
				QA.ASSERTOP( process.stdin, 'instanceof', Descriptor, 'process stdin type' );
				QA.ASSERTOP( process.stdout, 'instanceof', Descriptor, 'process stdout type' );
				QA.ASSERTOP( process.stderr, 'instanceof', Descriptor, 'process stderr type' );
				var data = process.stdout.read(10);
				QA.ASSERT( data.byteLength, 10, 'reading Process stdout' );
				break;
			default:
				QA.FAILED('(TBD) no test available for this system.');
		}


/// create process wait for exitcode [m]

	do {
		switch ( systemName ) {
			case 'Windows_NT':
				var cmd = getEnv('ComSpec');
				var args1 = ['/c', 'cd fvasdfvasdfvasdfv'];
				var args2 = ['/c', 'cd']; // if 'dir' is used instead of 'cd', one should empty the stdout pipe or process.Wait() will block.
				break;
			case 'Linux':
				var cmd = getEnv('SHELL');
				var args1 = ['-c', 'cd fvasdfvasdfvasdfv'];
				var args2 = ['-c', 'ls'];
				break;
			default:
				QA.FAILED('(TBD) no test available for this system.');
				break;
		}
				
		var process = new Process(cmd, args1);
		var exitCode = process.wait();
		QA.ASSERT( exitCode, 1, 'process exit code 1' );

		var process = new Process(cmd, args2);
		var exitCode = process.wait();
		QA.ASSERT( exitCode, 0, 'process exit code 0' );
	
	} while (0);


/// create process error detection [ftrm]

	do {
		try {

			var res = new Process('uryqoiwueyrqoweu');
		} catch( ex if ex instanceof IoError ) {

			QA.ASSERT( ex.code, -5994, 'CreateProcess error detection' );
			break;
		}
		QA.FAILED( "no exception (cf mozilla bug #113095)" );
	} while (0);
	


/// create process detach [m]
	
	do {
		switch ( systemName ) {
			case 'Windows_NT':
				var cmd = getEnv('ComSpec');
				var args = ['/c', 'cd'];
				break;
			case 'Linux':
				var cmd = getEnv('SHELL');
				var args = ['-c', 'ls'];
				break;
			default:
				QA.FAILED('(TBD) no test available for this system.');
				break;
		}
		
		var process = new Process(cmd, args);
		process.detach();
	} while (0);

/// current working directory [ftrm]
	
		QA.ASSERT( 'currentDirectory' in global, true, 'has currentDirectory' );
		QA.ASSERT( typeof currentDirectory, 'string', 'current directory' );
		QA.ASSERT( currentDirectory.length >= 1, true, 'current directory length' );


/// MemoryMapped class [ftrm]

		var thisFilename = QA.cx.item.file;
		var m = new MemoryMapped(new File(thisFilename).open('r'));
		QA.ASSERT( m.file instanceof File, true, 'instanceof .file memeber' );
		QA.ASSERT_STR( new Stream(m).read(15), '// don\'t remove', 'convert to a Stream (1)' );
		QA.ASSERT_STR( new Stream(m).read(15), '// don\'t remove', 'convert to a Stream (2)' );
		QA.ASSERT_STR( stringify(m).substr(0,15), '// don\'t remove', 'toString it' );


/// MemoryMapped with offset [ftrm]

		var thisFilename = QA.cx.item.file;
		var m = new MemoryMapped(new File(thisFilename).open('r'));
		QA.ASSERT( m.file instanceof File, true, 'instanceof .file memeber' );
		
		m.offset = 3;
		QA.ASSERT( m.offset, 3, 'get offset value' );
		QA.ASSERT_STR( new Stream(m).read(5), 'don\'t', 'convert to a Stream (1)' );
		m.offset = 0;
		QA.ASSERT_STR( new Stream(m).read(15), '// don\'t remove', 'convert to a Stream (2)' );


/// File EOF test

	var filename = QA.randomString(10);

	var file = new File(filename).open('w');
	file.write('xyz');
	file.close();

	var fromFile = new File(filename).open(File.RDONLY);
	
	QA.ASSERT_STR( fromFile.read(1), 'x', 'before EOF' );
	QA.ASSERT_STR( fromFile.read(0), '', 'before EOF' );
	QA.ASSERT_STR( fromFile.read(3), 'yz', 'before EOF' );
	QA.ASSERT( stringify(fromFile.read(0)), '', 'after EOF' );
	QA.ASSERT( fromFile.read(1), undefined, 'after EOF' );
	QA.ASSERT( fromFile.read(), undefined, 'after EOF' );

	fromFile.close();

	file.delete();


/// complete jsio tests

	/* for local tests
	var QA = { ASSERTOP: function(a, op, b, name) {
		
		if ( !eval('(a '+op+' b)') ) {

			print('error: '+a+' ! '+op+' '+b, '  '+name + ' / ' + locate(1).join(':')+'\n');
		}
	}};
	*/

	function createSocketPair() {

		var rdv = new Socket(); rdv.bind(9999, '127.0.0.1'); rdv.listen(); rdv.readable = true;
		var cl = new Socket(); cl.connect('127.0.0.1', 9999);
		processEvents( Descriptor.events([rdv]), timeoutEvents(1000) );
		var sv = rdv.accept(); rdv.close();
		return [cl, sv];
	}

	function runBasicTests(fdm, name) {

		var fd = fdm('abcde');
		QA.ASSERTOP( stringify(fd.read(0)), '==', '', name );
		QA.ASSERTOP( stringify(fd.read(0)), '==', '', name );
		QA.ASSERTOP( stringify(fd.read(1)), '==', 'a', name );
		QA.ASSERTOP( stringify(fd.read(100)), '==', 'bcde', name );
		QA.ASSERTOP( stringify(fd.read(0)), '==', '', name );
		var fd = fdm('fghij');
		QA.ASSERTOP( stringify(fd.read()), '==', 'fghij', name );
		QA.ASSERTOP( stringify(fd.read(0)), '==', '', name );
		var fd = fdm('');
		QA.ASSERTOP( fd.read(0).byteLength, '==', 0, name );
	}

	function testEof(fdm, name) {

		var fd = fdm('');
		QA.ASSERTOP( stringify(fd.read()), '==', undefined, name );
		QA.ASSERTOP( stringify(fd.read(0)), '==', '', name );
		QA.ASSERTOP( stringify(fd.read(1)), '==', undefined, name );
		QA.ASSERTOP( stringify(fd.read(10)), '==', undefined, name );
		QA.ASSERTOP( stringify(fd.read(100)), '==', undefined, name );
	}

	function testEmpty(fdm, name) {

		var fd = fdm('');
		QA.ASSERTOP( stringify(fd.read()), '==', '', name );
		QA.ASSERTOP( stringify(fd.read(0)), '==', '', name );
		QA.ASSERTOP( stringify(fd.read(1)), '==', '', name );
		QA.ASSERTOP( stringify(fd.read(10)), '==', '', name );
		QA.ASSERTOP( stringify(fd.read(100)), '==', '', name );
	}

	//

	[
	function linger0() {

		var [c, s] = createSocketPair();
		c.nonblocking = false;
		s.linger = 0; // triggers |res==0 && amount==1| 
		s.write('');
		s.close();
		return c;
	},
	function linger1() {

		var [c, s] = createSocketPair();
		c.nonblocking = false;
		s.linger = 1;
		s.write('');
		s.close();
		return c;
	},
	function linger100() {

		var [c, s] = createSocketPair();
		c.nonblocking = false;
		s.linger = 100; // triggers PR_CONNECT_RESET_ERROR that both returns |undefined|
		s.write('');
		s.close();
		return c;
	}
	].forEach(function(fdm) {

		var c = fdm('');
		QA.ASSERTOP( c.read(), '==', undefined, '' );
		var c = fdm('');
		sleep(50);
		QA.ASSERTOP( c.read(), '==', undefined, '' );
		var c = fdm('');
		sleep(150);
		QA.ASSERTOP( c.read(), '==', undefined, '' );
	});


	[
	function pipe(data) {

		switch ( systemName ) {
			case 'Windows_NT':
				return new Process(getEnv('ComSpec'), ['/c', 'echo/|set /p ='+data]).stdout;
			case 'Linux':
				return new Process(getEnv('SHELL'), ['-c', 'echo -n '+data]).stdout;
		}
		return null;
	},
	function file(data) {

		var f = new File('a_tmp_file.tmp');
		f.open('w');
		f.write(data);
		f.close();
		f.open('r');
		return f;
	},
	function s1(data) {

		var [c, s] = createSocketPair();
		c.nonblocking = true;
		s.write(data);
		s.close();
		return c;
	},
	function s2(data) {

		var [c, s] = createSocketPair();
		c.nonblocking = true;
		s.write(data);
		s.shutdown(true); // further sends will be disallowed.
		return c;
	},
	function s3(data) {

		var [c, s] = createSocketPair();
		c.nonblocking = false;
		s.write(data);
		s.shutdown(true);
		return c;
	},
	function s4(data) {

		var [c, s] = createSocketPair();
		c.nonblocking = true;
		s.write(data);
		s.close();
		return c;
	},
	function s5(data) {

		var [c, s] = createSocketPair();
		c.nonblocking = true;
		s.linger = 2000;
		s.write(data);
		s.close();
		return c;
	}
	].forEach( function(fdm) {

		var name = fdm.name;
		runBasicTests(fdm, name);
		testEof(fdm, name);
	} );


	[
	function s1(data) {

		var [c, s] = createSocketPair();
		c.nonblocking = true;
		s.write(data);
		return c;
	}
	].forEach( function(fdm) {

		var name = fdm.name;
		runBasicTests(fdm, name);
		testEmpty(fdm, name);
	} );


	[
	function s1(data) {

		var [c, s] = createSocketPair();
		c.nonblocking = false;
		s.write(data);
		return c;
	},
	function s2(data) {

		var [c, s] = createSocketPair();
		c.nonblocking = false;
		s.write(data);
		s.linger = 1000;
		s.close();
		sleep(100);
		return c;
	}
	].forEach( function(fdm) {

		var name = fdm.name;
		var fd = fdm('abcde');
		QA.ASSERTOP( stringify(fd.read(0)), '==', '', name );
		QA.ASSERTOP( stringify(fd.read(1)), '==', 'a', name );
		QA.ASSERTOP( stringify(fd.read(0)), '==', '', name );
		QA.ASSERTOP( stringify(fd.read(4)), '==', 'bcde', name );
	} );


	[
	function s1(data) {

		var [c, s] = createSocketPair();
		c.nonblocking = false;
		s.linger = 500; // http://msdn.microsoft.com/en-us/library/windows/desktop/ms738547(v=vs.85).aspx
		s.write(data);
		s.close();
		return c;
	}
	].forEach( function(fdm) {

		var name = fdm.name;
		var c = fdm('abcde');
		QA.ASSERTOP( stringify(c), '==', 'abcde', name );
	} );

	//

	var [c, s] = createSocketPair();

	s.write('123');
	s.close();
	QA.ASSERTOP( stringify(c), '==', '123', 'basic c-s test' );

