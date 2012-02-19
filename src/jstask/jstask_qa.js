loadModule('jstask');
loadModule('jsio');

/// crash about serializer cleanup [rmt]

	var t = new Task(function(){
		
		loadModule('jsstd');
		sleep(100);
		return "test";
	});

	t.request();
	sleep(10);


/// some new threads [f]

	var i = 0;
	while ( i++ < 17 ) {

		new Task(function(){});
	}

/// many new threads [r]

	var i = 0;
	while ( !endSignal && i++ < 100 ) {

		new Task(function(){});
		QA.gc();
	}


/// many Request with big data [r]

	var myTask = new Task(function(){});
	for ( var i = 0; i < 1000; i++ )
		myTask.request(stringRepeat('x', 1000));

	for ( var j = 0; j < 1000; j++ )
		myTask.response();


/// many Request/Response with low priority [r]

	var myTask = new Task(function(){}, -1);
	for ( var i = 0; i < 1000; i++ )
		myTask.request();
	for ( var j = 0; j < 1000; j++ )
		myTask.response();

		
/// many Request/Response with high priority [r]

	var myTask = new Task(function(){}, -1);
	sleep(2);
	for ( var i = 0; i < 1000; i++ )
		myTask.request();
	sleep(2);
	for ( var j = 0; j < 1000; j++ )
		myTask.response();


/// idle property [fr]

	function myTask() {}

	var myTask = new Task(myTask);

	myTask.request(undefined);
	QA.ASSERT( myTask.idle, false, 'idle state');
	myTask.response();
	QA.ASSERT( myTask.idle, true, 'idle state');
	myTask.request(undefined);
	QA.ASSERT( myTask.idle, false, 'idle state');
	myTask.response();
	QA.ASSERT( myTask.idle, true, 'idle state');

	myTask.request(undefined);
	myTask.request(undefined);
	QA.ASSERT( myTask.idle, false, 'idle state');
	myTask.response();
	QA.ASSERT( myTask.idle, false, 'idle state');
	myTask.response();
	QA.ASSERT( myTask.idle, true, 'idle state');


/// pending properties [r]

	function myTask() {}

	var myTask = new Task(myTask);

	QA.ASSERT( myTask.pendingRequestCount, 0, 'pendingRequestCount');
	QA.ASSERT( myTask.pendingResponseCount, 0, 'pendingResponseCount');

	myTask.request();
	sleep(20);

	QA.ASSERT( myTask.pendingRequestCount, 0, 'pendingRequestCount');
	QA.ASSERT( myTask.pendingResponseCount, 1, 'pendingResponseCount'); // ASSERT @src/jstask/jstask_qa.js:41 - pending properties [r] - pendingResponseCount - 0 !== 1

	myTask.response();
	QA.ASSERT( myTask.pendingRequestCount, 0, 'pendingRequestCount');
	QA.ASSERT( myTask.pendingResponseCount, 0, 'pendingResponseCount');

	myTask.request();
	myTask.request();
	sleep(10);
	QA.ASSERT( myTask.pendingRequestCount, 0, 'pendingRequestCount');
	myTask.response();
	myTask.response();
	QA.ASSERT( myTask.pendingRequestCount, 0, 'pendingRequestCount');
	QA.ASSERT( myTask.pendingResponseCount, 0, 'pendingResponseCount');

	
/// multiple requests [rf]

	var myTask = new Task(function(req) req);

	for ( var i = 0; i < 100; i++ )
		myTask.request(i);

	sleep(20);
	var pending = myTask.pendingRequestCount + myTask.pendingResponseCount;
	QA.ASSERT( pending == 100 || pending == 99, true, 'pending count: '+pending);    // (TBD) check this case: src/jstask/jstask_qa.js:XX pending count: 108, false != true - multiple tasks [rf]

	for ( var i = 0; i < 100; i++ )
		QA.ASSERT( myTask.response(), i, 'task response');

	QA.ASSERT( myTask.pendingRequestCount, 0, 'pendingRequestCount');
	QA.ASSERT( myTask.pendingResponseCount, 0, 'pendingResponseCount');


/// multiple tasks [rf]

	function taskFunction() {
		loadModule('jsstd');
		return expand('1$(a)5', { a:234 });
	}

	var tasks = [];
	for ( var i = 0; i < 30; i++ )
		tasks.push(new Task(taskFunction));

	for each ( var t in tasks )
		t.request();

	for each ( var t in tasks )
		QA.ASSERT( t.response(), '12345', 'task response' );


/// task exception [rf]

	function myTask(i) {
		throw 'my exception '+i;
	}

	var myTask = new Task(myTask);
	myTask.request(1);
	myTask.request(2);
	myTask.request(3);
	
	QA.ASSERT_EXCEPTION( function() myTask.response(), 'my exception 1', 'the exception' );
	QA.ASSERT_EXCEPTION( function() myTask.response(), 'my exception 2', 'the exception' );
	QA.ASSERT_EXCEPTION( function() myTask.response(), 'my exception 3', 'the exception' );


/// task idle property [fr]

	function myTask() {
		for ( var j = 0; j < 1000; j++ );
		return j;
	}

	var myTask = new Task(myTask);

	for ( var i = 0; i < 100; i++ )
		myTask.request(undefined);	

	var count = 0;
	while ( !myTask.idle ) {
	
		var res = myTask.response();
		QA.ASSERT( res, 1000, 'task response');
		count++;
	}

	QA.ASSERT( myTask.pendingRequestCount, 0, 'pendingRequestCount');
	QA.ASSERT( myTask.pendingResponseCount, 0, 'pendingResponseCount');

	QA.ASSERT( count, 100, 'count');


/// task and loadModule [r]

	function myTask(req, i) {
	
		i || loadModule('jsio');
		return currentDirectory;
	}

	var myTask = new Task(myTask, -1);
	myTask.request();
	QA.ASSERT( myTask.response(), currentDirectory, 'currentDirectory');


/// local context [fr]

	function myTask() {
	
		if ( !('i' in this) )
			i = 0;
		i++;
		return i;
	}

	var myTask = new Task(myTask);
	myTask.request(undefined);
	myTask.request(undefined);
	myTask.request(undefined);

	QA.ASSERT( myTask.response(), 1, 'response' );
	QA.ASSERT( myTask.response(), 2, 'response' );
	QA.ASSERT( myTask.response(), 3, 'response' );


/// request index [fr]

	function myTask(req, index) index;
	
	var myTask = new Task(myTask);
	myTask.request(undefined);
	myTask.request(undefined);
	myTask.request(undefined);
	
	QA.ASSERT( myTask.response(), 0, 'response' );
	QA.ASSERT( myTask.response(), 1, 'response' );
	QA.ASSERT( myTask.response(), 2, 'response' );


/// task returns a map object [fr]

	function myTask(req, i) {
		
		if ( !i )
			loadModule('jsstd');
		return Map({req:Blob(req), i:i});
	}
	
	var myTask = new Task(myTask);
	myTask.request('hello');
	myTask.request('anita');

	var res = myTask.response();
	QA.ASSERT_STR( res.req, 'hello', 'response' );
	QA.ASSERT( res.i, 0, 'response' );
	
	var res = myTask.response();
	QA.ASSERT_STR( res.req, 'anita', 'response' );
	QA.ASSERT( res.i, 1, 'response' );



/// file access in a task [fr]
	
	function myFileTask(filename, i) {
		
		if ( !i )
			loadModule('jsio');
		var file = new File(filename); 
		var res = file.content;
		file.content = undefined;
		return res;
	}

	var myTask = new Task(myFileTask);

	var filename = QA.randomString(10)+'.tmp';
	new File(filename).content = 'XXX'+filename;
	myTask.request(filename);
	sleep(10);
	var res = myTask.response();
	QA.ASSERT_STR( res, 'XXX'+filename, 'response' );


/// task stderr custom test [frd]

	function myFileTask() {

		_host.stderr('myerror');
	}

	var myTask = new Task(myFileTask);
	myTask.request(undefined);
	myTask.response();


/// blocking TCP client []

	var myTask = new Task(function() {

		loadModule('jsio');
		var serverSocket = new Socket();
		serverSocket.reuseAddr = true;
		serverSocket.bind(8099, '127.0.0.1');
		serverSocket.listen();

		serverSocket.readable = function(s) {

			s.accept().write('hello');
		}

		poll([serverSocket], 1000);
		serverSocket.linger = 1000;
		serverSocket.close();
	});

	myTask.request();

	var client = new Socket();
	client.connect('127.0.0.1', 8099);
	var res = client.read(5);
	QA.ASSERT_STR( res, 'hello', 'response' );
	myTask.response();
