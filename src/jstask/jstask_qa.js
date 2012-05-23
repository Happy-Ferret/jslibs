loadModule('jstask');
loadModule('jsio');

/// open and close a task

	var t = new Task(function() {});
	t.close();


/// object un/serialization

	var t = new Task(function(test) {

		return {
			_serialize: function(serializer) {

				serializer.write(123);
			},
			_unserialize: function(unserializer) {
				
				return { a:unserializer.read() };
			}
		};
	});

	t.request('test');
	processEvents(t.events(), host.endSignalEvents());
	QA.ASSERTOP( uneval(t.response()), '==', '({a:123})', ReferenceError );


/// missing exception test

	var myTaskFct = function() {

		//new (host.loadModule('jssqlite').Database)().exec('123');
		xxx += 1;
	}
	var myTask = new Task(myTaskFct);
	myTask.request();
	QA.ASSERTOP( function() { myTask.response() }, 'ex', ReferenceError );


/// exception test []

	if ( QA.IS_UNSAFE ) // see "unsafe issue"
		return;

	loadModule('jstask');
	var myTaskFct = function() {

		host.loadModule();
	}
	var myTask = new Task(myTaskFct);
	myTask.request();
	QA.ASSERTOP( function() { myTask.response() }, 'ex', RangeError ); // unsafe issue


/// task result iteration using StopIteration

	var t = new Task(function(i,j) {

		if ( i === StopIteration )
			throw StopIteration;
		return j;
	});

	for ( var i = 0; i < 10; ++i )
		t.request();

	t.request(StopIteration);

	var res = '';
	for ( var r of t )
		res += r;

	QA.ASSERT( res, '0123456789' );



/// memory leak 4

	var myTaskFct = function() {
		
		new (host.loadModule('jssqlite').Database)().exec('123');
	}
	var myTask = new Task(myTaskFct);
	myTask.request();
	myTask.response();


/// memory leak 3

	var myTaskFct = function() {

		new (host.loadModule('jssqlite').Database);
	}
	
	var myTask = new Task(myTaskFct);
	myTask.request();
	myTask.response();


/// memory leak 2

	var myTaskFct = function() {

		host.loadModule('jsstd');
	}
	
	var myTask = new Task(myTaskFct);
	myTask.request();
	myTask.response();


/// memory leak 1

	var t = new Task(function(){});
	t.request();


/// crash when exception in the task function []

	var t = new Task(function(test) {

		throw 1;
	});

	t.request('test');

	processEvents(t.events(), host.endSignalEvents());


/// crash when error in the task function []

	var t = new Task(function(test){

		joazijozaeijv();
	});

	t.request('test');

	processEvents(t.events(), host.endSignalEvents());


/// crash about serializer cleanup []

	var t = new Task(function() {
		
		var loadModule = host.loadModule;
		loadModule('jsstd');
		sleep(100);
		return "test";
	});

	t.request();
	sleep(10);


/// some new threads []

	var i = 0;
	while ( i++ < 17 ) {

		new Task(function(){});
	}


/// many new threads []

	var i = 0;
	while ( !host.endSignal && i++ < 100 ) {

		new Task(function(){});
		QA.gc();
	}


/// many Request with big data []

	var myTask = new Task(function(){});
	for ( var i = 0; i < 1000; i++ )
		myTask.request(stringRepeat('x', 1000));

	for ( var j = 0; j < 1000; j++ )
		myTask.response();


/// many Request/Response with low priority []

	var myTask = new Task(function(){}, -1);
	for ( var i = 0; i < 1000; i++ )
		myTask.request();
	for ( var j = 0; j < 1000; j++ )
		myTask.response();

		
/// many Request/Response with high priority []

	var myTask = new Task(function(){}, -1);
	sleep(2);
	for ( var i = 0; i < 1000; i++ )
		myTask.request();
	sleep(2);
	for ( var j = 0; j < 1000; j++ )
		myTask.response();


/// idle property []

	var myTask = function() {}

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


/// pending properties []

	var myTask = function() {}

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

	
/// multiple requests []

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


/// multiple tasks []

	var taskFunction = function() {
		
		var loadModule = host.loadModule;
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


/// task exception []

	var myTask = function(i) {

		throw 'my exception '+i;
	}

	var myTask = new Task(myTask);
	myTask.request(1);
	myTask.request(2);
	myTask.request(3);
	
	QA.ASSERTOP( function() myTask.response(), 'ex', 'my exception 1', 'the exception' );
	QA.ASSERTOP( function() myTask.response(), 'ex', 'my exception 2', 'the exception' );
	QA.ASSERTOP( function() myTask.response(), 'ex', 'my exception 3', 'the exception' );


/// task idle property []

	var myTask = function() {
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


/// task and loadModule []

	var myTask = function(req, i) {
		
		var loadModule = host.loadModule;
		i || loadModule('jsio');
		return currentDirectory;
	}

	var myTask = new Task(myTask, -1);
	myTask.request();
	QA.ASSERT( myTask.response(), currentDirectory, 'currentDirectory');


/// local context []

	var myTask = function() {
	
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


/// request index []

	var myTask = function(req, index) index;
	
	var myTask = new Task(myTask);
	myTask.request(undefined);
	myTask.request(undefined);
	myTask.request(undefined);
	
	QA.ASSERT( myTask.response(), 0, 'response' );
	QA.ASSERT( myTask.response(), 1, 'response' );
	QA.ASSERT( myTask.response(), 2, 'response' );


/// task returns a map object []

	var myTask = function(req, i) {

		var loadModule = host.loadModule;
		if ( !i )
			loadModule('jsstd');
		return {req:req, i:i};
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



/// file access in a task []
	
	var myFileTask = function(filename, i) {

		var loadModule = host.loadModule;
		if ( !i )
			loadModule('jsio');
		var file = new File(filename); 
		var res = stringify(file.content);
		file.content = undefined;
		return res;
	}

	var myTask = new Task(myFileTask);

	var filename = QA.randomString(10)+'.tmp';

	new File(filename).content = 'XXX'+filename;
	myTask.request(filename);
	sleep(100);
	var res = myTask.response();
	QA.ASSERT_STR( res, 'XXX'+filename, 'response' );


/// task stderr custom test []

	var myFileTask = function() {

		host.stderr('myerror');
	}

	var myTask = new Task(myFileTask);
	myTask.request(undefined);
	myTask.response();


/// blocking TCP client []

	var myTask = new Task(function() {
		
		var loadModule = host.loadModule;
		loadModule('jsio');
		var serverSocket = new Socket();
		serverSocket.reuseAddr = true;
		serverSocket.bind(8099, '127.0.0.1');
		serverSocket.listen();

		serverSocket.readable = function(s) {

			var s1 = s.accept();
			sleep(100);
			s1.write('hello');
		}

		poll([serverSocket], 1000);
		serverSocket.linger = 1000;
		serverSocket.close();
	});

	myTask.request();

	var client = new Socket();
	client.connect('127.0.0.1', 8099);
	client.readable = true;
	poll([client], 1000);
	var res = client.read(5);
	QA.ASSERT_STR( res, 'hello', 'response' );
	myTask.response();
	client.close();
