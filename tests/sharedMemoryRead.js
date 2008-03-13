LoadModule('jsstd');
LoadModule('jsio');

var mem = new SharedMemory( 'test.txt', 4 );

while (!endSignal) {

	Print( mem.content, '\n' );
	Sleep(1000);
}
