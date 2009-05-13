LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsio');


Print( uneval(Socket.GetHostsByName('localhost')) );
		
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
		
	Print(response);

	

Halt();

/*
try {

	var res = new Process('uryqoiwueyrqoweu');
} catch( ex if ex instanceof IoError ) {

	Print( ex.code, -5994, 'CreateProcess error detection' );
}
*/

var p = new Process( 'c:\\windows\\System32\\cmd.exe', ['/c', 'dir', 'c:']);
Sleep(1000);
Print( p.stdout.Read(), '\n' );


Halt(); //////////////////////////////////////////////////////////

var mem = new SharedMemory( 'fileName', 100 );
var mem1 = new SharedMemory( 'fileName', 100 );

mem1.Close();
//mem.Close();



try {


	var fd = Descriptor.Import( 5000, Descriptor.DESC_FILE );

	Print( fd)


} catch ( ex if ex instanceof IoError ) { 

	Print( ex.toSource() );
	
}


Halt(); //////////////////////////////////////////////////////////

var f = new File('test.txt');
f.info.toto = 123;
f.content = 'xx'
Print( f.info.size, '\n' );
f.content = 'xxx'
Print( f.info.toto, '\n' );



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
Print(m);


Halt(); //////////////////////////////////////////////////////////


processPriority = 2;
while (!endSignal) {

	Sleep(100);
}

Print( processPriority );

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
//Print( Directory.List('.').join('\n'), '\n' );
var mem2 = new SharedMemory( 'test.txt', 4 );
Print( mem2.content.length, '\n' );

Halt(); //////////////////////////////////////////////////////////

try {

    Poll(desc,1);
    
    

} catch ( ex if ex instanceof IoError ) { 
	Print( ex.text + ' ('+ex.code+')', '\n' );
} catch (ex) {
	throw(ex);
}

var desc;


try {


Print('\n * testing UDP socket \n');

	var step = 0;
	
	var s2 = new Socket( Socket.UDP );
	s2.nonblocking = true;
	s2.Bind(9999);
	s2.readable = function(s) {
	
//		Print( 'port:'+s.sockPort+'\n' );
		var [data, ip, port] = s.RecvFrom();
		
		s.SendTo(ip, port, '5554');
		
		
//		new Socket( Socket.UDP ).SendTo( ip, port, 'receiving from '+ip+':'+port+'   '+ data.length );
	}
	
	var s1 = new Socket( Socket.UDP );
	s1.reuseAddr = true;
	s1.nonblocking = true;
	s1.Connect('127.0.0.1', 9999);
	
	s1.readable = function(s) { Print( 'readable', '\n' ); }
//	s1.writable = function(s) { Print( 'writable', '\n' ); }
	s1.exception = function(s) { Print( 'exception', '\n' ); }
	s1.error = function(s) { Print( 'error', '\n' ); }
	
	var dlist = [s1,s2]; //descriptor list


	var i = 0;
	while(++i < 20 && !endSignal) {
		
		Print('.\n');
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
	
	Print('port '+port);

	server1.Listen();

Halt(); //////////////////////////////////////////////////////////

//Print( Socket.GetHostsByName('www.google.com') );


/*
var f = new File('toto.txt');
f.Open( "w+" );
File.stdin.Read();

f.Close();

Halt(); //////////////////////////////////////////////////////////	
	

Print('\n * testing stdout \n');

	var f = File.stdout;
	f.Write(new Date());

Print('\n * testing simple file write \n');

	var f = new File('test.txt');
	f.Open( File.RDWR | File.CREATE_FILE );
	f.Write(new Date());
	f.Close();
*/


Print('\n * testing TCP socket \n');

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
			
			Print('recv '+data+' from '+s.sockPort);
			if ( !data.length )
				delete s.readable;
		}
	}
	
	var client = new Socket( Socket.TCP );
	client.Connect('127.0.0.1', 80);
	
	Print('seg: '+client.maxSegment);
	
	dlist.push(client);
	dlist.push(server);
	
	var i = 0;
	while(++i < 10) {
		
		Print('.\n');
		Poll(dlist,100);
		step++;
		if ( !(step%5) ) {
		
			client.Write('test');
		}
	}
	



} catch ( ex if ex instanceof IoError ) {
	Print( 'IoError: ' + ex.text + ' ('+ex.os+')', '\n' );
} catch( ex ) {
	throw ex;
}
