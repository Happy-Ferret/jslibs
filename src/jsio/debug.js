//loadModule('jsstd'); loadModule('jsio'); currentDirectory += '/../../tests/jslinux'; exec('start.js'); throw 0;
//loadModule('jsstd'); exec('../common/tools.js'); RunQATests('-exclude jstask -rep 1 jsio');

loadModule('jsstd'); loadModule('jsio');


/// Socket shutdown(false) behavior [trm]

try {

	var rdv = new Socket(); rdv.nonblocking = true; rdv.Bind(9999, '127.0.0.1'); rdv.Listen(); rdv.readable = true;
	var cl = new Socket();
//	cl.nonblocking = true;
	cl.Connect('127.0.0.1', 9999);
	ProcessEvents( Descriptor.Events([rdv]), TimeoutEvents(2000) );
	var sv = rdv.Accept(); rdv.Close();
	sv.nonblocking = true;
	
	sv.linger = 0;
	
	sv.Write(StringRepeat('x', 10000000));
	sv.Close();

	while ( cl.Read() != undefined );

	print( typeof cl.Read(), '\n' );




} catch( ex if ex instanceof IoError ) {

	print( ex.fileName+':'+ex.lineNumber+' '+ex.text+'\n' );
}



throw 0;




//loadModule('jsdebug'); gcZeal = 2;

//var f = new File('C:\\MSDOS.SYS').Open('r');
var f = new File("C:\\tmp\\vcredist.bmp").Open('r'); // MYDOCUMENTS

print(f.id.quote());

Halt();



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
f.Connect('apod.nasa.gov', 80);
f.Write('GET /apod/image/1105/cenAjets_many_1280.jpg HTTP/1.0\r\nAccept:*/*\r\n\r\n');
var image = Stringify(f);

print( image.length, 'bytes\n' );



throw 0;
	

if ( 0 ) {

	loadModule('jsstd');
	loadModule('jsio');
	loadModule('jswinshell');
	
	var f = new File('test.txt');
	f.Close();
	
	
	Halt();

/*	
	var process = new Process('jshost', ['-u', '-i', '_host.stdout(arguments)', '123', '-c']);
	var res = process.stdout.Read();
	print( res ==  "_host.stdout(arguments),123,-c");
*/

	var f = new File('c:/MSDOS.SYS').Open('r');
	
	print( f.id.quote() );

	throw 0;
}



loadModule('jsstd');  loadModule('jsio');
//RunJsircbot(false); throw 0;
// var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();
//exec('../../qaexp.js');  throw 0;
//var QA = FakeQAApi;  RunLocalQAFile();
//RunSavedQAFile('../../exitissue');






loadModule('jsstd'); exec('../common/tools.js');
RunQATests('-rep 4 udp');

loadModule('jsstd');
loadModule('jsio');
loadModule('jswinshell');

try {

	print( Socket.GetHostsByAddr('10.10.10.10')[0] );
} catch ( ex if ex instanceof IoError ) {

	print( ex.const, '\n' );
}


throw 0;





new File( DESKTOP+'\\test.txt' ).content = '1234';



Halt();



loadModule('jsio');
loadModule('jswinshell');

new File( DESKTOP+'\\test.txt' ).content = '1234';
Halt();

loadModule('jsdebug');
loadModule('jsstd');
loadModule('jstask');

//jsioTest(); Halt();

loadModule('jsstd');
loadModule('jsio');

const CRLF = '\r\n';
var descList = [];
var serv = new Socket(Socket.TCP);
serv.Bind(8081);
serv.Listen();
descList.push(serv);

function Respond() {

  this.httpHeader += this.Read();
  if ( this.httpHeader.indexOf(CRLF+CRLF) == -1 )
    return;

  print('Received: \n' + this.httpHeader + '\n');
  descList.splice(descList.indexOf(this), 1);

  var writeOp = this.Write(
    'HTTP/1.0 200 OK' + CRLF +
    'Content-Type: text/html; charset=utf-8' + CRLF +
    'Cache-Control: no-cache, must-revalidate' + CRLF +
    'Pragma: no-cache' + CRLF + CRLF +
    '<html><body>Hello from <a href="http://jslibs.googlecode.com/">jslibs</a> at ' + new Date() + '</body></html>' );
  this.Close();
};

serv.readable = function () {

  var desc = this.Accept();
  desc.httpHeader = '';
  desc.readable = Respond;
  descList.push(desc);
}

print('HTTP server minimal example. Point a web browser at http://localhost:8081. CTRL+C to exit\n');

while ( !endSignal )
 Poll(descList, 50);

//while ( !endSignal )
//	ProcessEvents( IOEvents(descList), EndSignalEvents(), TimeoutEvents(100) );


//jsioTest();


Halt();


var f = new File('com1:');
f.Open(File.RDWR);
ConfigureSerialPort(f, 9600);
f.Write('A');
f.Read(1);


f.writable = function() { print('writable') }
//f.readable = function() { print('readable') }

Poll([f], 1000);

/*
f.Write('A');
f.Read(1);
*/

Halt();


	var p = new Process(GetEnv('ComSpec'), ['/c', 'svn', 'info', '--xml']); // '-r', 'HEAD', 
	var svnInfo = '';
	for ( let data; data = p.stdout.Read(); )
		svnInfo += data;
		
	print( svnInfo );
		
		
		

Halt();

exec('../common/tools.js');

var f = new File('my_test_file.txt').Open();
new File('my_test_file.txt').content = '';
Dump( f.Read(1) );


new File('my_test_file.txt').content = 'a';

Dump( f.Read(1) );


Halt();

	var socket = new Socket();
	socket.nonblocking = false;
	socket.Connect('127.0.0.1', 3128);
	socket.Shutdown();
	socket.available;


/*
 var p = new Process( 'c:\\windows\\System32\\cmd.exe', ['/c', 'dir', 'c:'] );
 p.Wait();
 p.stdout.Read(0);
*/	



Halt(); //////////////////////////////////////////////////////////////////////



function ReverseLookup( ip ) {

	try {

		return Socket.GetHostsByAddr(ip)[0];
	} catch ( ex if ex instanceof IoError ) {

		return undefined; // not found
	}
}

print( ReverseLookup('10.0.0.99') );




Halt(); //////////////////////////////////////////////////////////////////////

	var myTask = new Task(function() {
		
		loadModule('jsio');
		var serverSocket = new Socket();
		serverSocket.reuseAddr = true;
		serverSocket.Bind(8099, '127.0.0.1');
		serverSocket.Listen();

		serverSocket.readable = function(s) {

			s.Accept().Write('hello');
			s.linger = 100;
			s.Close();
		}
		
		Poll([serverSocket], 500);
		serverSocket.Close();
	});
	
	myTask.Request();

	var client = new Socket();
	client.Connect('127.0.0.1', 8099);
	var res = client.Read();
	print( res, '\n' );




Halt(); //////////////////////////////////////////////////////////////////////

gcZeal = 2;


var s1 = new Socket( Socket.UDP );
s1.Connect('127.0.0.1', 9999);

var s2 = new Socket( Socket.UDP );
s2.Bind(9999);
s2.readable = function(s) {

	s.RecvFrom();
}


var dlist = [s1,s2];

Poll(dlist, 10);
s1.Write('x');

Poll(dlist, 10);
s1.Write('y');

Poll(dlist, 10);
s1.Write('z');



Halt(); //////////////////////////////////////////////////////////////////////

try {

	var res = new Process('uryqoiwueyrqoweu');
} catch( ex if ex instanceof IoError ) {

	print( ex.filename+':'+ex.lineno, '\n' );
}
	
	
		
Halt();

				

Halt();

	host = 'www.google.com'
	
	var response = '';

	var soc = new Socket();
	soc.nonblocking = true;
	soc.Connect( host, 80 );
	soc.writable = function(s) {
	
		delete soc.writable;
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
		
	print(response);

	

Halt();

/*
try {

	var res = new Process('uryqoiwueyrqoweu');
} catch( ex if ex instanceof IoError ) {

	print( ex.code, -5994, 'CreateProcess error detection' );
}
*/

var p = new Process( 'c:\\windows\\System32\\cmd.exe', ['/c', 'dir', 'c:']);
Sleep(1000);
print( p.stdout.Read(), '\n' );


Halt(); //////////////////////////////////////////////////////////

var mem = new SharedMemory( 'fileName', 100 );
var mem1 = new SharedMemory( 'fileName', 100 );

mem1.Close();
//mem.Close();



try {


	var fd = Descriptor.Import( 5000, Descriptor.DESC_FILE );

	print( fd)


} catch ( ex if ex instanceof IoError ) { 

	print( ex.toSource() );
	
}


Halt(); //////////////////////////////////////////////////////////

var f = new File('test.txt');
f.info.toto = 123;
f.content = 'xx'
print( f.info.size, '\n' );
f.content = 'xxx'
print( f.info.toto, '\n' );



Halt(); //////////////////////////////////////////////////////////

//var tmp = new File('test.txt');
//tmp.Open('w');
//tmp.Write('123');


var f = new File('test.txt');

try {

	f.Move('..');

} catch ( ex if ex instanceof IoError ) { 
	
	if ( ex.code != -5936 )
		throw(ex);
}


Halt(); //////////////////////////////////////////////////////////

var f = new File('directory.cpp');
f.Open("r");
var m = new MemoryMapped(f);
print(m);


Halt(); //////////////////////////////////////////////////////////


processPriority = 2;
while (!endSignal) {

	Sleep(100);
}

print( processPriority );

Sleep(500);
		
		
Halt(); //////////////////////////////////////////////////////////

var mem = new SharedMemory( 'test.txt', 4 );

var i = 0;
while (!endSignal) {

	mem.content = i++;
	Sleep(1000);
}


Halt(); //////////////////////////////////////////////////////////



var mem = new SharedMemory( 'test.txt', 100 );
mem.Write('test', 10);
//mem.content = "toto";
//print( Directory.List('.').join('\n'), '\n' );
var mem2 = new SharedMemory( 'test.txt', 4 );
print( mem2.content.length, '\n' );

Halt(); //////////////////////////////////////////////////////////

try {

    Poll(desc,1);
    
    

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
	s2.Bind(9999);
	s2.readable = function(s) {
	
//		print( 'port:'+s.sockPort+'\n' );
		var [data, ip, port] = s.RecvFrom();
		
		s.SendTo(ip, port, '5554');
		
		
//		new Socket( Socket.UDP ).SendTo( ip, port, 'receiving from '+ip+':'+port+'   '+ data.length );
	}
	
	var s1 = new Socket( Socket.UDP );
	s1.reuseAddr = true;
	s1.nonblocking = true;
	s1.Connect('127.0.0.1', 9999);
	
	s1.readable = function(s) { print( 'readable', '\n' ); }
//	s1.writable = function(s) { print( 'writable', '\n' ); }
	s1.exception = function(s) { print( 'exception', '\n' ); }
	s1.error = function(s) { print( 'error', '\n' ); }
	
	var dlist = [s1,s2]; //descriptor list


	var i = 0;
	while(++i < 20 && !endSignal) {
		
		print('.\n');
		Poll(dlist,100);
		step++;
		if ( !(step % 4) ) {
		
			s1.Write('1234');

		}
	}



Halt(); //////////////////////////////////////////////////////////


	var server = new Socket( Socket.TCP );
	server.Bind( 80, '127.0.0.1');
	server.Listen();
	
	Sleep(100);

	var server1 = new Socket( Socket.TCP );

	for ( var port = 80; !server1.Bind( port, '127.0.0.1' ) && port <= 82; port++ );
	
	print('port '+port);

	server1.Listen();

Halt(); //////////////////////////////////////////////////////////

//print( Socket.GetHostsByName('www.google.com') );


/*
var f = new File('toto.txt');
f.Open( "w+" );
File.stdin.Read();

f.Close();

Halt(); //////////////////////////////////////////////////////////	
	

print('\n * testing stdout \n');

	var f = File.stdout;
	f.Write(new Date());

print('\n * testing simple file write \n');

	var f = new File('test.txt');
	f.Open( File.RDWR | File.CREATE_FILE );
	f.Write(new Date());
	f.Close();
*/


print('\n * testing TCP socket \n');

	var dlist = []; //descriptor list

	var step = 0;

	var server = new Socket( Socket.TCP );
	server.Bind(80, '127.0.0.1');
	server.Listen();
	
	server.readable = function(s) {
		
		var soc = s.Accept();
		dlist.push(soc);
		soc.readable = function(s) {

			var data = s.Read();
			
			print('recv '+data+' from '+s.sockPort);
			if ( !data.length )
				delete s.readable;
		}
	}
	
	var client = new Socket( Socket.TCP );
	client.Connect('127.0.0.1', 80);
	
	print('seg: '+client.maxSegment);
	
	dlist.push(client);
	dlist.push(server);
	
	var i = 0;
	while(++i < 10) {
		
		print('.\n');
		Poll(dlist,100);
		step++;
		if ( !(step%5) ) {
		
			client.Write('test');
		}
	}
	



} catch ( ex if ex instanceof IoError ) {
	print( 'IoError: ' + ex.text + ' ('+ex.os+')', '\n' );
} catch( ex ) {
	throw ex;
}
