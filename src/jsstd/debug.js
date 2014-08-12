var loadModule = host.loadModule;
//loadModule('jsstd'); exec('../common/tools.js'); runQATests('jsstd'); throw 0;
//loadModule('jsstd'); exec('../common/tools.js'); global.QA = fakeQAApi;

loadModule('jsstd');

	var prevStderr;
	[prevStderr, host.stderr] = [host.stderr, undefined];

	try {
		sandboxEval('for ( var i = 0; i < 10000000; ++i );', undefined, 10);
		print(false, '\n');
	} catch(ex if ex instanceof OperationLimit) {
		print(true, '\n');
	}

	print( sandboxEval('for ( var i = 0; i < 1000; ++i ); true', undefined, 0), '\n' );

	host.stderr = prevStderr;

	print( sandboxEval('123') == 123, '\n' );

	print( sandboxEval('') == undefined, '\n' );

	print( sandboxEval('this') != global, '\n' );

	sandboxEval('query', a=>a);

	print( sandboxEval('query(5)', a=>a) == 5, '\n' );

	print( sandboxEval('query([1,2,3])', a=>a) == '1,2,3', '\n' );

	sandboxEval('Math').cos.aaa = 123;
	print( !('aaa' in Math.cos) )



	//var res = sandboxEval('Math').cos == Math.cos
	//print(res);


throw 0;


	try {

		//var res = sandboxEval('throw ({x:123})', undefined, 1000);
		var res = sandboxEval('for ( var i = 0; i < 1000000; ++i );', undefined, 100);

	} catch(ex if ex instanceof OperationLimit) {

		print(ex);
	}

throw 0;


	//exec('dgvklsdfhgsvldukfhg');

	var o = {a:1, b:2};

	print( countProperties(o), '\n' );
	print( clearObject(o), '\n' );
	print( countProperties(o), '\n' );
	print( '\n' );

	(function test() { print( captureCurrentStack(), '\n' )})();

throw 0;




  var obj = { a:1, b:[2,3,4], c:{} };
  print( uneval(obj), '\n' ); // prints: ({a:1, b:[2, 3, 4], c:{}})

  clearObject(obj);
  print( uneval(obj) ); // prints: ({})



throw 0;


//var res = sandboxEval('Math+" "+Math+" "+query()', function() 123); print(res, '\n');
//sandboxEval('', function() 123);
//sandboxEval('typeof query');
//sandboxEval('query()', function() 123);

// var res = sandboxEval('for ( var i = 0; i < 1000000; ++i );', undefined, 1000);

//var res = sandboxEval('for ( var i = 0; i < 100000; ++i );');

throw 0;




stringRepeat('-', -1 )
throw 0;

print( 'peakMemoryUsage: '+(peakMemoryUsage/(1024*1024)).toFixed(0) + 'MB\n' );
print( 'privateMemoryUsage: '+(privateMemoryUsage/(1024*1024)).toFixed(0) + 'MB\n' );
print( 'processTime: '+processTime.toFixed(0) + 'ms\n' );
print( cpuId.quote() ); 

throw 0;


loadModule('jsstd');
//jsstdTest();
throw 0;



//jsstdTest();

host.stdout('press ctrl-c');
processEvents(timeoutEvents(1000), host.endSignalEvents());
host.stdout('done.');


throw 0;


loadModule('jsstd');

print( processTime, '\n' );
sleep(1000);
print( processTime, '\n' );
var d1 = Date.now(); while ( Date.now() - d1 < 1000 );
print( processTime, '\n' );
throw 0;


loadModule('jsstd');
print(privateMemoryUsage, '\n');
collectGarbage();
print(privateMemoryUsage, '\n');
//host.stdin();
throw 0;



sleep(1);
timeCounter();

timeCounter();
var t = timeCounter();
sleep(0, 1);
err = timeCounter() - t;

var t = timeCounter();
sleep(1000, 1);
print( timeCounter() - t - err, '\n' );

throw 0;



loadModule('jsstd');

var d1 = Date.now();
sleep(1000);
print( Date.now() - d1, '\n' );

var d2 = timeCounter();
sleep(1000);
print( timeCounter() - d2, '\n' );
