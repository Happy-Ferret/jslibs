LoadModule('jsstd');
LoadModule('jsnspr');


try {


	var ev = new Event();
	
	var evlist = [ev];
	
	Print( Poll( evlist, 100 ) );
	ev.Set();
	Print( Poll( evlist, 100 ) );
	


throw 0;

	var dlist = []; //descriptor list

	var step = 0;

	var server = new Socket( Socket.TCP );
	server.Bind(80, '127.0.0.1');
	server.Listen();
	server.readable = function(s) {
		
		var soc = s.Accept();
		dlist.push(soc);
		soc.readable = function(s) {

			var data = s.Recv();
			Print(data);
			if ( !data.length )
				delete s.readable;
		}
	}
	
	var client = new Socket( Socket.TCP );
	client.Connect('127.0.0.1', 80);
	
	dlist.push(client);
	dlist.push(server);

	while(!endSignal) {
		
		Print('.\n');
		Poll(dlist,100);
		step++;
		if ( !(step%5) ) {
		
			client.Send('test');
		}
	}
	

throw 0;





// UDP test
	var step = 0;

	var s1 = new Socket( Socket.UDP );
	s1.Connect('127.0.0.1', 1);
	var s2 = new Socket( Socket.UDP );
	s2.Bind(1);
	s2.readable = function(s) {
	
		Print( s.Recv() );
	}
	var dlist = [s1,s2]; //descriptor list
	while(!endSignal) {
		
		Print('.\n');
		Poll(dlist,100);
		step++;
		if ( !(step % 5) ) {
		
			s1.Send("test");
		}
	}

throw 0;

	Print( File.stdin.ReadAll().length );
	
throw 0;

//	new File('test.txt').content = undefined;

Socket.stdin.Accept();

throw 0;

/*
	var f = new File('test.txt');
	f.Open( File.RDWR | File.CREATE_FILE  );
	f.Write('test');
	f.Close();

	f.name = 'toto.txt'
	f.Delete();

	File.stdout.Write('stdout test !\n');

	Print( 'NSPR version = ' + NSPRVersion()  ,'\n' );
	
	
	Print( 'PATH = ' + GetEnv('PATH')  ,'\n' );
	Print( 'asdfasdfas = ' + GetEnv('asdfasdfas')  ,'\n' );
	Print( 'Host name = ' + HostName() ,'\n' );
	
	Print( 'has xxx directory: ' + new Directory('xxx').exist, '\n' );
	Print( 'has .svn directory: ' + new Directory('.svn').exist, '\n' );
*/


var end = false;
var soc = new Socket();
soc.Connect( 'www.google.com', 80 );
Print('Connecting...');

soc.writable = function(s) {
	
	Print('accepted`\n');
	delete soc.writable;
	s.Send('GET / HTTP/1.0\r\n\r\n');
}

soc.readable = function(s) {

	var data = s.Recv();
	
	if ( data.length == 0 ) {
		end = true;
		delete soc.readable;
	} else
		Print( data );
}


while(!endSignal && !end )
	Poll([soc],1000);


} catch ( ex if ex instanceof NSPRError ) {
	Print( 'NSPRError: ' + ex.text, '\n' );
} catch( ex ) {
	throw ex;
}
