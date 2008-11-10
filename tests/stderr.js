LoadModule('jsio');
LoadModule('jsstd');

_configuration.stdout = function() {

	var logFile = new File('jslibs_log.txt');
	logFile.Open('a'); // append
	logFile.Write(Array.slice(arguments).join(''));
	logFile.Close();
};

_configuration.stderr = _configuration.stdout;

Print('hello world');
Foo('hi');
