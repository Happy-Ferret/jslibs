var loadModule = host.loadModule;

//loadModule('jsstd'); loadModule('jsio'); currentDirectory += '/../../tests/jslinux'; exec('start.js'); throw 0;
//loadModule('jsstd'); exec('../common/tools.js'); runQATests('-exclude jstask -rep 1 jsio -stopAfterNIssues 1'); halt();
loadModule('jsstd'); exec('../common/tools.js'); runQATests('jsio'); throw 0;

loadModule('jstask');
loadModule('jsstd');
loadModule('jsio');
loadModule('jsdebug');


print( directorySeparator );
throw 0;

function createSocketPair() {

	var rdv = new Socket(); rdv.bind(9999, '127.0.0.1'); rdv.listen(); rdv.readable = true;
	var cl = new Socket(); cl.connect('127.0.0.1', 9999);
	processEvents( Descriptor.events([rdv]), timeoutEvents(1000) );
	var sv = rdv.accept(); rdv.close();
	return [cl, sv];
}

var [c, s] = createSocketPair();	

s.write('123');
s.close();
print( stringify(c) );
	
	
throw 0;

	
	var myTask = new Task(function() {
		
		var loadModule = host.loadModule;
		loadModule('jsio');
		var serverSocket = new Socket();
		serverSocket.reuseAddr = true;
		serverSocket.bind(8099, '127.0.0.1');
		serverSocket.listen();

		serverSocket.readable = function(s) {

			var s1 = s.accept();
			s1.linger = 1000;
			sleep(1000);
			s1.write('hello');
			s1.close();
		}

		poll([serverSocket], 1000);
		serverSocket.close();
	});

	myTask.request();

	var client = new Socket();
	client.connect('127.0.0.1', 8099);
	client.nonblocking = false;
	
	sleep(100);
	
	print( 'read...\n' )
	var res = client.read();
	print( stringify(res) + '\n' )
	


	myTask.response();

	print( 'end\n' )


throw 0;


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

	print( result.length, 10000, 'read amount' );
	print( cl.read(), undefined, 'read done' );



throw 0;



new Directory('asdfvsadfvsadf').exist;

var list = Directory.list('..', Directory.SKIP_DIRECTORY);

for ( var i in list ) {

	print( uneval(list[i]), '\n' );
}


halt();


//loadModule('jsstd'); loadModule('jsio');


/// Socket shutdown(false) behavior [trm]

try {

	var rdv = new Socket(); rdv.nonblocking = true; rdv.bind(9999, '127.0.0.1'); rdv.listen(); rdv.readable = true;
	var cl = new Socket();
//	cl.nonblocking = true;
	cl.connect('127.0.0.1', 9999);
	processEvents( Descriptor.events([rdv]), timeoutEvents(2000) );
	var sv = rdv.accept(); rdv.close();
	sv.nonblocking = true;
	
	sv.linger = 0;
	
	sv.write(stringRepeat('x', 10000000));
	sv.close();

	while ( cl.read() != undefined );

	print( typeof cl.read(), '\n' );




} catch( ex if ex instanceof IoError ) {

	print( ex.fileName+':'+ex.lineNumber+' '+ex.text+'\n' );
}



throw 0;




//loadModule('jsdebug'); gcZeal = 2;

//var f = new File('C:\\MSDOS.SYS').Open('r');
var f = new File("C:\\tmp\\vcredist.bmp").open('r'); // MYDOCUMENTS

print(f.id.quote());

halt();



throw 0;


var f1 = new File('');
var f2 = new File('');

f1.name = 'test';
print( f2.name );



throw 0;

var f1 = new File('');
var f2 = new File('');

f1.timeout = 123;
print(1, f2.timeout, '\n');
print(2, File.prototype.timeout, '\n');
print(3, f1.__proto__.__proto__.hasOwnProperty('timeout'), '\n');
print(4, File.prototype.hasOwnProperty('timeout'), '\n');
print(3, f1.__proto__.__proto__.timeout, '\n');


throw 0;


var f = new Socket();
f.connect('apod.nasa.gov', 80);
f.write('GET /apod/image/1105/cenAjets_many_1280.jpg HTTP/1.0\r\nAccept:*/*\r\n\r\n');
var image = stringify(f);

print( image.length, 'bytes\n' );



throw 0;
	

if ( 0 ) {

	loadModule('jsstd');
	loadModule('jsio');
	loadModule('jswinshell');
	
	var f = new File('test.txt');
	f.close();
	
	
	halt();

/*	
	var process = new Process('jshost', ['-u', '-i', 'host.stdout(host.arguments)', '123', '-c']);
	var res = process.stdout.read();
	print( res ==  "host.stdout(host.arguments),123,-c");
*/

	var f = new File('c:/MSDOS.SYS').open('r');
	
	print( f.id.quote() );

	throw 0;
}



loadModule('jsstd');  loadModule('jsio');
//runJsircbot(false); throw 0;
// var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  halt();
//exec('../../qaexp.js');  throw 0;
//var QA = FakeQAApi;  runLocalQAFile();
//runSavedQAFile('../../exitissue');






loadModule('jsstd'); exec('../common/tools.js');
runQATests('-rep 4 udp');

loadModule('jsstd');
loadModule('jsio');
loadModule('jswinshell');

try {

	print( Socket.getHostsByAddr('10.10.10.10')[0] );
} catch ( ex if ex instanceof IoError ) {

	print( ex.const, '\n' );
}


throw 0;





new File( DESKTOP+'\\test.txt' ).content = '1234';



halt();



loadModule('jsio');
loadModule('jswinshell');

new File( DESKTOP+'\\test.txt' ).content = '1234';
halt();

loadModule('jsdebug');
loadModule('jsstd');
loadModule('jstask');

//jsioTest(); halt();

loadModule('jsstd');
loadModule('jsio');

const CRLF = '\r\n';
var descList = [];
var serv = new Socket(Socket.TCP);
serv.bind(8081);
serv.listen();
descList.push(serv);

function respond() {

  this.httpHeader += this.read();
  if ( this.httpHeader.indexOf(CRLF+CRLF) == -1 )
    return;

  print('Received: \n' + this.httpHeader + '\n');
  descList.splice(descList.indexOf(this), 1);

  var writeOp = this.write(
    'HTTP/1.0 200 OK' + CRLF +
    'Content-Type: text/html; charset=utf-8' + CRLF +
    'Cache-control: no-cache, must-revalidate' + CRLF +
    'Pragma: no-cache' + CRLF + CRLF +
    '<html><body>hello from <a href="http://jslibs.googlecode.com/">jslibs</a> at ' + new Date() + '</body></html>' );
  this.close();
};

serv.readable = function () {

  var desc = this.accept();
  desc.httpHeader = '';
  desc.readable = respond;
  descList.push(desc);
}

print('HTTP server minimal example. point a web browser at http://localhost:8081. CTRL+C to exit\n');

while ( !host.endSignal )
 poll(descList, 50);

//while ( !host.endSignal )
//	processEvents( IOEvents(descList), host.endSignalEvents(), TimeoutEvents(100) );


//jsioTest();


halt();


var f = new File('com1:');
f.open(File.RDWR);
configureSerialPort(f, 9600);
f.write('A');
f.read(1);


f.writable = function() { print('writable') }
//f.readable = function() { print('readable') }

poll([f], 1000);

/*
f.write('A');
f.read(1);
*/

halt();


	var p = new Process(getEnv('ComSpec'), ['/c', 'svn', 'info', '--xml']); // '-r', 'HEAD', 
	var svnInfo = '';
	for ( let data; data = p.stdout.read(); )
		svnInfo += data;
		
	print( svnInfo );
		
		
		

halt();

exec('../common/tools.js');

var f = new File('my_test_file.txt').open();
new File('my_test_file.txt').content = '';
dump( f.read(1) );


new File('my_test_file.txt').content = 'a';

dump( f.read(1) );


halt();

	var socket = new Socket();
	socket.nonblocking = false;
	socket.connect('127.0.0.1', 3128);
	socket.shutdown();
	socket.available;


/*
 var p = new Process( 'c:\\windows\\System32\\cmd.exe', ['/c', 'dir', 'c:'] );
 p.wait();
 p.stdout.read(0);
*/	



halt(); //////////////////////////////////////////////////////////////////////



function reverseLookup( ip ) {

	try {

		return Socket.getHostsByAddr(ip)[0];
	} catch ( ex if ex instanceof IoError ) {

		return undefined; // not found
	}
}

print( reverseLookup('10.0.0.99') );




halt(); //////////////////////////////////////////////////////////////////////

	var myTask = new Task(function() {
		
		loadModule('jsio');
		var serverSocket = new Socket();
		serverSocket.reuseAddr = true;
		serverSocket.bind(8099, '127.0.0.1');
		serverSocket.listen();

		serverSocket.readable = function(s) {

			s.accept().write('hello');
			s.linger = 100;
			s.close();
		}
		
		poll([serverSocket], 500);
		serverSocket.close();
	});
	
	myTask.request();

	var client = new Socket();
	client.connect('127.0.0.1', 8099);
	var res = client.read();
	print( res, '\n' );




halt(); //////////////////////////////////////////////////////////////////////

gcZeal = 2;


var s1 = new Socket( Socket.UDP );
s1.connect('127.0.0.1', 9999);

var s2 = new Socket( Socket.UDP );
s2.bind(9999);
s2.readable = function(s) {

	s.recvFrom();
}


var dlist = [s1,s2];

poll(dlist, 10);
s1.write('x');

poll(dlist, 10);
s1.write('y');

poll(dlist, 10);
s1.write('z');



halt(); //////////////////////////////////////////////////////////////////////

try {

	var res = new Process('uryqoiwueyrqoweu');
} catch( ex if ex instanceof IoError ) {

	print( ex.filename+':'+ex.lineno, '\n' );
}
	
	
		
halt();

				

halt();

	host = 'www.google.com'
	
	var response = '';

	var soc = new Socket();
	soc.nonblocking = true;
	soc.connect( host, 80 );
	soc.writable = function(s) {
	
		delete soc.writable;
		s.write('GET\r\n\r\n');
	}
	soc.readable = function(s) {
		
		var res = s.read();
		if ( res )
			response += res;
	}
	
	var i = 0;
	while( ++i < 100 )
		poll([soc], 20);
		
	print(response);

	

halt();

/*
try {

	var res = new Process('uryqoiwueyrqoweu');
} catch( ex if ex instanceof IoError ) {

	print( ex.code, -5994, 'CreateProcess error detection' );
}
*/

var p = new Process( 'c:\\windows\\System32\\cmd.exe', ['/c', 'dir', 'c:']);
sleep(1000);
print( p.stdout.read(), '\n' );


halt(); //////////////////////////////////////////////////////////

var mem = new SharedMemory( 'fileName', 100 );
var mem1 = new SharedMemory( 'fileName', 100 );

mem1.close();
//mem.Close();



try {


	var fd = Descriptor.import( 5000, Descriptor.DESC_FILE );

	print( fd)


} catch ( ex if ex instanceof IoError ) { 

	print( ex.toSource() );
	
}


halt(); //////////////////////////////////////////////////////////

var f = new File('test.txt');
f.info.toto = 123;
f.content = 'xx'
print( f.info.size, '\n' );
f.content = 'xxx'
print( f.info.toto, '\n' );



halt(); //////////////////////////////////////////////////////////

//var tmp = new File('test.txt');
//tmp.Open('w');
//tmp.Write('123');


var f = new File('test.txt');

try {

	f.move('..');

} catch ( ex if ex instanceof IoError ) { 
	
	if ( ex.code != -5936 )
		throw(ex);
}


halt(); //////////////////////////////////////////////////////////

var f = new File('directory.cpp');
f.open("r");
var m = new MemoryMapped(f);
print(m);


halt(); //////////////////////////////////////////////////////////


processPriority = 2;
while (!host.endSignal) {

	sleep(100);
}

print( processPriority );

sleep(500);
		
		
halt(); //////////////////////////////////////////////////////////

var mem = new SharedMemory( 'test.txt', 4 );

var i = 0;
while (!host.endSignal) {

	mem.content = i++;
	sleep(1000);
}


halt(); //////////////////////////////////////////////////////////



var mem = new SharedMemory( 'test.txt', 100 );
mem.write('test', 10);
//mem.content = "toto";
//print( Directory.List('.').join('\n'), '\n' );
var mem2 = new SharedMemory( 'test.txt', 4 );
print( mem2.content.length, '\n' );

halt(); //////////////////////////////////////////////////////////

try {

    poll(desc,1);
    
    

} catch ( ex if ex instanceof IoError ) { 
	print( ex.text + ' ('+ex.code+')', '\n' );
} catch (ex) {
	throw(ex);
}

var desc;


try {


print('\n * testing UDP socket \n');

	var step = 0;
	
	var s2 = new Socket( Socket.UDP );
	s2.nonblocking = true;
	s2.bind(9999);
	s2.readable = function(s) {
	
//		print( 'port:'+s.sockPort+'\n' );
		var [data, ip, port] = s.recvFrom();
		
		s.sendTo(ip, port, '5554');
		
		
//		new Socket( Socket.UDP ).SendTo( ip, port, 'receiving from '+ip+':'+port+'   '+ data.length );
	}
	
	var s1 = new Socket( Socket.UDP );
	s1.reuseAddr = true;
	s1.nonblocking = true;
	s1.connect('127.0.0.1', 9999);
	
	s1.readable = function(s) { print( 'readable', '\n' ); }
//	s1.writable = function(s) { print( 'writable', '\n' ); }
	s1.exception = function(s) { print( 'exception', '\n' ); }
	s1.error = function(s) { print( 'error', '\n' ); }
	
	var dlist = [s1,s2]; //descriptor list


	var i = 0;
	while(++i < 20 && !host.endSignal) {
		
		print('.\n');
		poll(dlist,100);
		step++;
		if ( !(step % 4) ) {
		
			s1.write('1234');

		}
	}



halt(); //////////////////////////////////////////////////////////


	var server = new Socket( Socket.TCP );
	server.bind( 80, '127.0.0.1');
	server.listen();
	
	sleep(100);

	var server1 = new Socket( Socket.TCP );

	for ( var port = 80; !server1.bind( port, '127.0.0.1' ) && port <= 82; port++ );
	
	print('port '+port);

	server1.listen();

halt(); //////////////////////////////////////////////////////////

//print( Socket.GetHostsByName('www.google.com') );


/*
var f = new File('toto.txt');
f.open( "w+" );
File.stdin.read();

f.close();

halt(); //////////////////////////////////////////////////////////	
	

print('\n * testing stdout \n');

	var f = File.stdout;
	f.write(new Date());

print('\n * testing simple file write \n');

	var f = new File('test.txt');
	f.open( File.RDWR | File.CREATE_FILE );
	f.write(new Date());
	f.close();
*/


print('\n * testing TCP socket \n');

	var dlist = []; //descriptor list

	var step = 0;

	var server = new Socket( Socket.TCP );
	server.bind(80, '127.0.0.1');
	server.listen();
	
	server.readable = function(s) {
		
		var soc = s.accept();
		dlist.push(soc);
		soc.readable = function(s) {

			var data = s.read();
			
			print('recv '+data+' from '+s.sockPort);
			if ( !data.length )
				delete s.readable;
		}
	}
	
	var client = new Socket( Socket.TCP );
	client.connect('127.0.0.1', 80);
	
	print('seg: '+client.maxSegment);
	
	dlist.push(client);
	dlist.push(server);
	
	var i = 0;
	while(++i < 10) {
		
		print('.\n');
		poll(dlist,100);
		step++;
		if ( !(step%5) ) {
		
			client.write('test');
		}
	}
	



} catch ( ex if ex instanceof IoError ) {
	print( 'IoError: ' + ex.text + ' ('+ex.os+')', '\n' );
} catch( ex ) {
	throw ex;
}
