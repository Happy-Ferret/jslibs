LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jstask');
	
	var res = new File('debug.js').content;

	function MyFileTask(filename, i) {
		
		if ( !i )
			LoadModule('jsio');
		var res = new File(filename).content;
		return res;
	}

	var myTask = new Task(MyFileTask);

	var filename = 'qwrqwdrqwd'+'.tmp';
	new File(filename).content = 'XXX'+filename;
	myTask.Request(filename);
	Sleep(10);
	var res = myTask.Response();
	Print( res == 'XXX'+filename, '\n' );
