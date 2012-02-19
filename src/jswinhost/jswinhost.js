if ( !isfirstinstance ) throw('Already rnning...');
loadModule('jswinshell');
loadModule('jsstd');


/*
var cons = new Console()
_host.stderr = configuration.stdout = cons.write;
loadModule('jsstd');
print('toto');
*/
messageBox(isfirstinstance);