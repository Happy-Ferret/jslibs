LoadModule('jsstd');
LoadModule('jsnspr');

for (;;) {
	Print( eval( File.stdin.ReadAll() ), '\n' );
}