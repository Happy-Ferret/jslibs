LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jstask');


var myTask = new Task(function(){});

for ( var i = 0; i < 10; i++ )
	myTask.Request(123);
	
	
//for ( var i = 0; i < 10; i++ )
//	myTask.Response();
	

