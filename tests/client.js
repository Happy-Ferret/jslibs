exec('deflib.js');
LoadModule('jsnspr');


/*
var list = [];

for ( var i=0; i<300; i++) {
	
	var s = new Socket();
	s.Connect( '127.0.0.1', 80 );
	s._count = 0;
	list.push(s);

	s.readable = function(soc) {
		
	  var buf = soc.Recv();
	  
	  soc._count += buf.length;
	  
		if( buf.length == 0 ) {
		  print( 'total recv ', soc._count, '\n' );
		
			delete soc.readable;
			list.splice( list.indexOf(soc), 1 );
		}
	}
}
while (list.length > 0) Poll(list,1000);
*/

var s = new Socket();
s.Connect( 'localhost', 80 );
s.writable = function() {

	print('writable event','\n');
	s.Send('GET / HTTP/1.x\r\n\r\n');
	
	print('connectStatus='+s.connectContinue ,'\n' );
	delete s.writable;
};

/*
s.readable = function() {
	
	var buf = s.Recv();
	print('readable event','\n');
	print('size:'+buf.length, '\n' );
	buf.length || delete s.readable;
}
*/

s.exception = function() {

	print('exception event','\n');
	delete s.exception;
}

try {
	while(!endSignal) {
		print('.');
		Poll([s],250);
	}
} catch ( ex if ex instanceof NSPRError ) { 
	print( ex.text + ' ('+ex.code+')', '\n' );
}