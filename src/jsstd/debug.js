var loadModule = host.loadModule;
//loadModule('jsstd'); exec('../common/tools.js'); runQATests('jsstd'); throw 0;

loadModule('jsstd');


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
