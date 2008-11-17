LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jstask');

function MyTask( request, index ) {

	if ( !index ) { // first request, it's time to initialise things
	
		LoadModule('jsstd');
		LoadModule('jsio');
	}
	
	for ( var i = 0; i<100000; i++);
	
	return Map({1:'aaa', 2:Blob('123')});

/*	
	Sleep(5);

	if ( index == 5 ) // generate an error at the 5th request
		xxxx();
	
	if ( index == 9 ) // throw a custom exception at the 9th request
		throw 'myException';

	if ( index == 15 )
		return {}; // unserialisable jsval
	
//	var f = new File('debug.js');
//	f.Open('r');
//	return 'arg-'+f.Read(30)+'...';

	return request + 'idx:'+index + (new Date());
*/

}

var t1 = new Task(MyTask, -1);

for ( var i = 0; i < 50; i++ )
	t1.Request('request '+i);



while ( t1.pendingRequestCount || t1.pendingResponseCount ) {

	try {
		Print(t1.Response(), '\n');
		
	} catch(ex) {
	
		Print(ex, '\n');
	}
}
