LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jstask');


function MyTask( request, index ) {

	
	if ( !index ) { // first request, it's time to initialise things
	
		LoadModule('jsstd');
		LoadModule('jsio');
		this.count = 0;
	}
	
	Sleep(10);

	if ( index == 5 ) // generate an error at the 5th request
		xxxx();
	
	if ( index == 9 ) // throw a custom exception at the 9th request
		throw 'myException';	
	
//	var f = new File('debug.js');
//	f.Open('r');
//	return 'arg-'+f.Read(30)+'...';

	return request + 'idx:'+index + (new Date());
}

var t1 = new Task(MyTask, -1);

for ( var i = 0; i < 50; i++ )
	t1.Request('request '+i);

var res;

while ( t1.pendingRequestCount ) {

	try {
		Print(t1.Response(), '\n');
	} catch(ex) {
		Print(ex, '\n');
	}
}
