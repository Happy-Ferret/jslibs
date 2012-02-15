//LoadModule('jsstd'); Exec('../common/tools.js'); RunQATests('-exclude jstask sandbox');

LoadModule('jsstd');

jsstdTest(new SyntaxError());
