LoadModule('jsstd');
LoadModule('jsnspr');

try {

  var soc = new Socket();
  soc.Connect( 'localhost', 8080 );
  soc.writable = function(s) {
  
    delete soc.writable;
    s.Send('GET\r\n');
  }
  soc.readable = function(s) {
    
    Print( s.Recv() );
  }
  while(!endSignal)
    Poll([soc],100);


} catch ( ex if ex instanceof NSPRError ) { 
	Print( ex.text + ' ('+ex.code+')', '\n' );
} catch (ex) {
	throw(ex);
}