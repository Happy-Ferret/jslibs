LoadModule('jsstd');
LoadModule('jsio');

function PrintWorld() {

	Print('World');
}


Print('Hello ');
PrintWorld();
Print('\n');

Print('Press Enter key to continue...');

File.stdin.Read(); // Wait until Enter is pressed in the console.
