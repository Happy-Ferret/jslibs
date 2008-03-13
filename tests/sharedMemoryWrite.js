LoadModule('jsstd');
LoadModule('jsio');

var mem = new SharedMemory( 'test.txt', 4 );

var i = 0;
while (!endSignal) {

	mem.content = i++;
	Sleep(1000);
}
