LoadModule('jswinshell');
configuration.stderr = configuration.stdout = new Console().Write;
LoadModule('jsstd');

sdf();

Print('toto');
MessageBox('end');

