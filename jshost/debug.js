//LoadModule('jsstd');
LoadModule('jsio');



try {

    Poll(desc,1);
    
    

} catch ( ex if ex instanceof IoError ) { 
	Print( ex.text + ' ('+ex.code+')', '\n' );
} catch (ex) {
	throw(ex);
}


Halt();

var count = 0;

	var desc = [];

function DoGet() {

  var soc = new Socket();
	soc.nonblocking = true;
  soc.Connect( 'localhost', 8080 );

	soc.noDelay = true;
	soc.reuseAddr = true;

  soc.writable = function(s) {
  
    delete soc.writable;
    s.Write('GET\r\n');
    count++;
	count % 1000 || Print(count, '\n');

  }
  soc.readable = function(s) {
    
    var data = s.Read();
    if ( data.length != 0 ) {
    	//Print( data );
    } else {
    
    	delete soc.readable;
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
    Poll(desc,1);
    
    

} catch ( ex if ex instanceof IoError ) { 
	Print( ex.text + ' ('+ex.code+')', '\n' );
} catch (ex) {
	throw(ex);
}

/*


var hModule = LoadModule('jsstd');
Print( 'hModule: '+hModule, '\n' );

var hModule = LoadModule('jsstd');
Print( 'hModule: '+hModule, '\n' );


Print( 'Unsafe mode: '+_configuration.unsafeMode, '\n' );

Print('global:'+global, '\n');
global.test = 123;

Print ( scripthostpath, '\n' );
Print ( scripthostname, '\n' );


var i = 0;
function a() (i++ < 999) && a();
a();

*/