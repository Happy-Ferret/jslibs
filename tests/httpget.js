LoadModule('jsstd');
LoadModule('jsio');

try {

var count = 0;
var step = 0;

var dlist = []; //descriptor list

function StartServer() {

	var server = new Socket( Socket.TCP );
	server.nonblocking = true;
	server.noDelay = true;
	server.reuseAddr = true;

	server.Bind(80, '127.0.0.1');
	server.Listen();

	server.readable = function(s) {

		var soc = s.Accept();
		dlist.push(soc);
		soc.readable = function(s) {

			var data = s.Read();
			s.Close();
			let (p=dlist.indexOf(s)) p != -1 && dlist.splice(p,1);
			delete s.readable;
			
			++count%1000 || Print( count, '\n' );
		}
	}
	dlist.push(server);
}


function StartClient() {

	var client = new Socket( Socket.TCP );
	client.nonblocking = true;
	client.noDelay = true;
	client.reuseAddr = true;

	client.Connect('127.0.0.1', 80);
	client.writable = function(s) {

		if ( !s.connectContinue ) {

			Print('Error', '\n');
			delete s.writable;
			delete s.readable;
			s.Close();
			let (p=dlist.indexOf(s)) p != -1 && dlist.splice(p,1);
			return;
		}	
		delete s.writable;
		
		s.Write('GET\r\n');

	}
	client.readable = function(s) {

		if ( s.available == 0 ) {

			delete s.readable;
			s.Close();
			let (p=dlist.indexOf(s)) p != -1 && dlist.splice(p,1);
		}

	}
	dlist.push(client);
}


StartServer();

while(!endSignal) {

//	Print('.');
	Poll(dlist,1);
	step++;
	
	
	if ( true || !(step%5) ) {
		
		StartClient();
	}
}


} catch ( ex if ex instanceof IoError ) { 
	Print( ex.text + ' ('+ex.code+')', '\n' );
} catch (ex) {
	throw(ex);
}







/*

var count = 0;

	var desc = [];

function DoGet() {

  var soc = new Socket();
	soc.nonblocking = true;
	soc.noDelay = true;
	soc.reuseAddr = true;
  soc.Connect( 'localhost', 8080 );


  soc.writable = function(s) {
  
    delete soc.writable;
    s.Write('GET\r\n');
    count++;
//	count % 1000 || Sleep(1000);
	count % 1000 || Print(count, '\n');
  }
  
  
  soc.readable = function(s) {
    
//    Print(s.available, '\n')

    if ( s.available ) {
   	
   	var data = s.Read();
//   	Print( data );
//   	Halt();
    	//Print( data );
    } else {
    
    	delete soc.readable;
//    	s.Shutdown(); // both
    	s.Close();

    	var pos = desc.indexOf(s);
    	if ( pos != -1 )
    		desc.splice(pos,1);
    		
    	DoGet();
    }
  }
  
  desc.push(soc);
}


try {
  DoGet();
  while(!endSignal)
    Poll(desc,100);
    
    

} catch ( ex if ex instanceof IoError ) { 
	Print( ex.text + ' ('+ex.code+')', '\n' );
} catch (ex) {
	throw(ex);
}

*/