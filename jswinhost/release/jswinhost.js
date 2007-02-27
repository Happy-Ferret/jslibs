LoadModule('jswinshell');
configuration.stdout = new Console().Write;
configuration.stderr = MessageBox;
LoadModule('jsstd');


Print('toto');

