LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jstask');

function MyTask() {

	LoadModule('jsio');
	var f = new File('debug.js');
	f.Open('r');
	return ''+f.Read(30)+'...';
}

var t = new Task();
t.Run(MyTask);
Print( t.result, '\n' );
//Sleep(100);
//Sleep(100);

