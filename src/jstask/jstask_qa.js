LoadModule('jstask');
LoadModule('jsio');


/// many Request with big data [r]

	var myTask = new Task(function(){});
	for ( var i = 0; i < 1000; i++ )
		myTask.Request(StringRepeat('x', 1000));

	for ( var j = 0; j < 1000; j++ )
		myTask.Response();


/// many Request/Response with low priority [r]

	var myTask = new Task(function(){}, -1);
	for ( var i = 0; i < 1000; i++ )
		myTask.Request();
	for ( var j = 0; j < 1000; j++ )
		myTask.Response();

		
/// many Request/Response with high priority [r]

	var myTask = new Task(function(){}, -1);
	Sleep(2);
	for ( var i = 0; i < 1000; i++ )
		myTask.Request();
	Sleep(2);
	for ( var j = 0; j < 1000; j++ )
		myTask.Response();


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

	myTask.Request();
	Sleep(20);

	QA.ASSERT( myTask.pendingRequestCount, 0, 'pendingRequestCount');
	QA.ASSERT( myTask.pendingResponseCount, 1, 'pendingResponseCount'); // ASSERT @src/jstask/jstask_qa.js:41 - pending properties [r] - pendingResponseCount - 0 !== 1

	myTask.Response();
	QA.ASSERT( myTask.pendingRequestCount, 0, 'pendingRequestCount');
	QA.ASSERT( myTask.pendingResponseCount, 0, 'pendingResponseCount');

	myTask.Request();
	myTask.Request();
	Sleep(10);
	QA.ASSERT( myTask.pendingRequestCount, 0, 'pendingRequestCount');
	myTask.Response();
	myTask.Response();
	QA.ASSERT( myTask.pendingRequestCount, 0, 'pendingRequestCount');
	QA.ASSERT( myTask.pendingResponseCount, 0, 'pendingResponseCount');

	
/// multiple tasks [rf]

	var myTask = new Task(function(req) req);

	for ( var i = 0; i < 100; i++ )
		myTask.Request(i);

	Sleep(20);
	var pending = myTask.pendingRequestCount + myTask.pendingResponseCount;
	QA.ASSERT( pending == 100 || pending == 99, true, 'pending count: '+pending);    // (TBD) check this case: src/jstask/jstask_qa.js:XX pending count: 108, false != true - multiple tasks [rf]

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

	function MyTask(req, i) {
	
		i || LoadModule('jsio');
		return currentDirectory;
	}

	var myTask = new Task(MyTask, -1);
	myTask.Request();
	QA.ASSERT( myTask.Response(), currentDirectory, 'currentDirectory');


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
	QA.ASSERT_STR( res.req, 'hello', 'response' );
	QA.ASSERT( res.i, 0, 'response' );
	
	var res = myTask.Response();
	QA.ASSERT_STR( res.req, 'anita', 'response' );
	QA.ASSERT( res.i, 1, 'response' );



/// file access in a task [fr]
	
	function MyFileTask(filename, i) {
		
		if ( !i )
			LoadModule('jsio');
		var file = new File(filename); 
		var res = file.content;
		file.content = undefined;
		return res;
	}

	var myTask = new Task(MyFileTask);

	var filename = QA.RandomString(10)+'.tmp';
	new File(filename).content = 'XXX'+filename;
	myTask.Request(filename);
	Sleep(10);
	var res = myTask.Response();
	QA.ASSERT_STR( res, 'XXX'+filename, 'response' );


/// task stderr custom test [frd]

	function MyFileTask() {

		_configuration.stderr('myerror');
	}

	var myTask = new Task(MyFileTask);
	myTask.Request(undefined);
	myTask.Response();


/// blocking TCP client

	var myTask = new Task(function() {
		
		LoadModule('jsio');
		var serverSocket = new Socket();
		serverSocket.Bind(8099, '127.0.0.1');
		serverSocket.Listen();

		serverSocket.readable = function(s) {

			s.Accept().Write('hello');
			s.Shutdown();
		}
		
		Poll([serverSocket], 1000);
		serverSocket.Close();
	});
	
	myTask.Request();

	var client = new Socket();
	client.Connect('127.0.0.1', 8099);
	var res = client.Read(5);
	QA.ASSERT_STR( res, 'hello', 'response' );

