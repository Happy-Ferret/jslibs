LoadModule('jstask');
LoadModule('jsio');

/// idle property [fr]

	function MyTask() {}

	var myTask = new Task(MyTask);

	myTask.Request(undefined);
	QA.ASSERT( myTask.idle, false, 'idle state');
	myTask.Response();
	QA.ASSERT( myTask.idle, true, 'idle state');
	myTask.Request(undefined);
	QA.ASSERT( myTask.idle, false, 'idle state');
	myTask.Response();
	QA.ASSERT( myTask.idle, true, 'idle state');

	myTask.Request(undefined);
	myTask.Request(undefined);
	QA.ASSERT( myTask.idle, false, 'idle state');
	myTask.Response();
	QA.ASSERT( myTask.idle, false, 'idle state');
	myTask.Response();
	QA.ASSERT( myTask.idle, true, 'idle state');


/// pending properties [r]

	function MyTask() {}

	var myTask = new Task(MyTask);

	QA.ASSERT( myTask.pendingRequestCount, 0, 'pendingRequestCount');
	QA.ASSERT( myTask.pendingResponseCount, 0, 'pendingResponseCount');

	myTask.Request(undefined);
	Sleep(10);

	QA.ASSERT( myTask.pendingRequestCount, 0, 'pendingRequestCount');
	QA.ASSERT( myTask.pendingResponseCount, 1, 'pendingResponseCount');

	myTask.Response();
	QA.ASSERT( myTask.pendingRequestCount, 0, 'pendingRequestCount');
	QA.ASSERT( myTask.pendingResponseCount, 0, 'pendingResponseCount');

	myTask.Request(undefined);
	myTask.Request(undefined);
	Sleep(10);
	QA.ASSERT( myTask.pendingRequestCount, 0, 'pendingRequestCount');
	myTask.Response();
	myTask.Response();
	QA.ASSERT( myTask.pendingRequestCount, 0, 'pendingRequestCount');
	QA.ASSERT( myTask.pendingResponseCount, 0, 'pendingResponseCount');

	
/// multiple tasks [rf]

	function MyTask(req) req;

	var myTask = new Task(MyTask);

	for ( var i = 0; i < 100; i++ )
		myTask.Request(i);

	var pending = myTask.pendingRequestCount + myTask.pendingResponseCount;
	QA.ASSERT( pending == 100 || pending == 99, true, 'pending count: '+pending);

	for ( var i = 0; i < 100; i++ )
		QA.ASSERT( myTask.Response(), i, 'task response');

	QA.ASSERT( myTask.pendingRequestCount, 0, 'pendingRequestCount');
	QA.ASSERT( myTask.pendingResponseCount, 0, 'pendingResponseCount');


/// task exception [rf]

	function MyTask(i) {
		throw 'my exception '+i;
	}

	var myTask = new Task(MyTask);
	myTask.Request(1);
	myTask.Request(2);
	myTask.Request(3);
	
	QA.ASSERT_EXCEPTION( function() myTask.Response(), 'my exception 1', 'the exception' );
	QA.ASSERT_EXCEPTION( function() myTask.Response(), 'my exception 2', 'the exception' );
	QA.ASSERT_EXCEPTION( function() myTask.Response(), 'my exception 3', 'the exception' );


/// task idle property [fr]

	function MyTask() {
		for ( var j = 0; j < 1000; j++ );
		return j;
	}

	var myTask = new Task(MyTask);

	for ( var i = 0; i < 100; i++ )
		myTask.Request(undefined);	

	var count = 0;
	while ( !myTask.idle ) {
	
		var res = myTask.Response();
		QA.ASSERT( res, 1000, 'task response');
		count++;
	}

	QA.ASSERT( myTask.pendingRequestCount, 0, 'pendingRequestCount');
	QA.ASSERT( myTask.pendingResponseCount, 0, 'pendingResponseCount');

	QA.ASSERT( count, 100, 'count');


/// task and LoadModule [r]

	function MyTask() {
	
		LoadModule('jsio');
		Sleep(10);
	}

	var myTask = new Task(MyTask);
	myTask.Request(undefined);
	myTask.Request(undefined);
	Sleep(5);
	QA.ASSERT( myTask.pendingRequestCount, 1, 'pendingRequestCount');
	QA.ASSERT( myTask.pendingResponseCount, 0, 'pendingResponseCount');
	Sleep(10);
	QA.ASSERT( myTask.pendingRequestCount, 0, 'pendingRequestCount');
	QA.ASSERT( myTask.pendingResponseCount, 1, 'pendingResponseCount');
	Sleep(10);
	QA.ASSERT( myTask.pendingRequestCount, 0, 'pendingRequestCount');
	QA.ASSERT( myTask.pendingResponseCount, 2, 'pendingResponseCount');
	
	myTask.Response();
	myTask.Response();

	QA.ASSERT( myTask.pendingResponseCount, 0, 'pendingResponseCount');
	


/// local context [fr]

	function MyTask() {
	
		if ( !('i' in this) )
			i = 0;
		i++;
		return i;
	}

	var myTask = new Task(MyTask);
	myTask.Request(undefined);
	myTask.Request(undefined);
	myTask.Request(undefined);

	QA.ASSERT( myTask.Response(), 1, 'response' );
	QA.ASSERT( myTask.Response(), 2, 'response' );
	QA.ASSERT( myTask.Response(), 3, 'response' );


/// request index [fr]

	function MyTask(req, index) index;
	
	var myTask = new Task(MyTask);
	myTask.Request(undefined);
	myTask.Request(undefined);
	myTask.Request(undefined);
	
	QA.ASSERT( myTask.Response(), 0, 'response' );
	QA.ASSERT( myTask.Response(), 1, 'response' );
	QA.ASSERT( myTask.Response(), 2, 'response' );


/// task returns a map object [fr]

	function MyTask(req, i) {
		
		if ( !i )
			LoadModule('jsstd');
		return Map({req:Blob(req), i:i});
	}
	
	var myTask = new Task(MyTask);
	myTask.Request('hello');
	myTask.Request('anita');

	var res = myTask.Response();
	QA.ASSERT( res.req, 'hello', 'response' );
	QA.ASSERT( res.i, 0, 'response' );
	
	var res = myTask.Response();
	QA.ASSERT( res.req, 'anita', 'response' );
	QA.ASSERT( res.i, 1, 'response' );



/// file access in a task [fr]
	
	function MyFileTask(filename, i) {
		
		if ( !i )
			LoadModule('jsio');
		var res = new File(filename).content;
		return res;
	}

	var myTask = new Task(MyFileTask);

	var filename = QA.RandomString(10)+'.tmp';
	new File(filename).content = 'XXX'+filename;
	myTask.Request(filename);
	Sleep(10);
	var res = myTask.Response();
	QA.ASSERT( res, 'XXX'+filename, 'response' );

