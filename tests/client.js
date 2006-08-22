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
s.Connect( '127.0.0.1', 80 );
s.sendBufferSize = 1000000;
for ( var buf='x'; buf.length<10000000; buf+=buf );
s.writable = function() {
  print('sending '+buf.length+' bytes...')
  s.Send(buf);
  print('done.\n');
};
Poll([s],1000);

