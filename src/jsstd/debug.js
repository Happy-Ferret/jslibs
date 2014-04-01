var loadModule = host.loadModule;
//loadModule('jsstd'); exec('../common/tools.js'); runQATests('jsstd'); throw 0;
//loadModule('jsstd'); exec('../common/tools.js'); global.QA = fakeQAApi;

loadModule('jsstd');

print( 'peakMemoryUsage: '+(peakMemoryUsage/(1024*1024)).toFixed(0) + 'MB\n' );
print( 'privateMemoryUsage: '+(privateMemoryUsage/(1024*1024)).toFixed(0) + 'MB\n' );
print( 'processTime: '+processTime.toFixed(0) + 'ms\n' );

throw 0;


loadModule('jsstd');
//jsstdTest();
throw 0;



loadModule('jsstd');
var res = sandboxEval('Math+" "+Math+" "+query()', function() 123);

//sandboxEval('', function() 123);

//sandboxEval('typeof query');
//sandboxEval('query()', function() 123);

print(res, '\n');






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
