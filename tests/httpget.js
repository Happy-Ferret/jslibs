LoadModule('jsstd');
LoadModule('jsio');

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
//	count % 1000 || Sleep(1000);
	count % 1000 || Print(count, '\n');

  }
  soc.readable = function(s) {
    
	var d = s.Read();
    if ( s.available ) {
   	
   	s.Read();
    	//Print( data );
    } else {
    
    	delete soc.readable;
    	s.Shutdown(); // both
    	s.Close();

    	var pos = desc.indexOf(s);

    	if ( pos != -1 )
    		desc.splice(pos,1);
    	Sleep(10)
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