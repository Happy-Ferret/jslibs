LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jstask');

function MyTask() {

	LoadModule('jsio');
	var f = new File('debug.js');
	f.Open('r');
	return f.Read();
}

var t = new Task();
t.Exec(MyTask);

Print( t.result, '\n' );
Sleep(100);
Print( t.result, '\n' );
