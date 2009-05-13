LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jstask');

	var myTask = new Task(function() { throw '1234' } );
	myTask.Request();
	myTask.Response();
