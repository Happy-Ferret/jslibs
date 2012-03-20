loadModule('jsstd'); exec('../common/tools.js'); runQATests('-exclude jstask'); throw 0;

loadModule('jsstd');

var e;
try {

	sandboxEval('for (var i=0; i<10000000000; ++i);', undefined, 10);

	sdfGSDFG();

} catch(ex) {
	e = ex;
}

print(String(e), '\n');



//print(e instanceof OperationLimit, '\n');
