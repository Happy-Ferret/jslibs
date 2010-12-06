if ( !isfirstinstance ) throw('Already rnning...');
LoadModule('jswinshell');
LoadModule('jsstd');


/*
var cons = new Console()
_host.stderr = configuration.stdout = cons.Write;
LoadModule('jsstd');
Print('toto');
*/
MessageBox(isfirstinstance);