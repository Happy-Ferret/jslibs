LoadModule('jsstd');
LoadModule('jsnspr');

do {
	Print( eval( File.stdin.Read(4096) ), '\n' );
} while(line);