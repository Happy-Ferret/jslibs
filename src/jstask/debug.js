LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jstask');

LoadModule('jsstd');
LoadModule('jstask');

function MyTask( request ) {

	LoadModule('jsio');
	
	var f = new File('x').Open('w');
	f.Read();
	
	return 'r#' + request;
}

var myTask = new Task(MyTask);

for ( var i = 0; i < 10; i++ )
	myTask.Request(i);

try {
	
	while ( !myTask.idle )
		Print( myTask.Response(), ', ' );

} catch (ex if ex instanceof IoError ) {

Print( ex.code );
}


Halt(); ////////////////////////////////////////////////

function MyTask1( request, index ) {

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

var t1 = new Task(MyTask1, 1);

for ( var i = 0; i < 50; i++ )
	t1.Request('request '+i);


while ( !t1.idle ) {
//while ( t1.pendingRequestCount || t1.pendingResponseCount ) {

	try {
		Print(t1.Response(), '\n');
		
	} catch(ex) {
	
		Print(ex, '\n');
	}
}
