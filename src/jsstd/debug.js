loadModule('jsstd'); exec('../common/tools.js'); runQATests('jsstd -exclude jstask'); throw 0;

loadModule('jsstd');

print( sandboxEval('query()', function() 123), '\n' );
