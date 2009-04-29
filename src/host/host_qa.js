/// standard error [ftr]

		var errBuffer = new Buffer();
		_configuration.stderr = function(chunk) errBuffer.Write(chunk);
		Warning('test')
		delete _configuration.stderr;
		QA.ASSERT( errBuffer.toString().indexOf('warning: test') != -1, true, 'stderr redirection ('+errBuffer+')' ); 

	/* try to understand this (see issue #95):
	- src/host/host_qa.js:7 stderr redirection, false != true
	*/

/// LoadModule function [ftr]
		
		var id = LoadModule('jsstd');
		QA.ASSERT( LoadModule('jsstd'), id, 'reloading the same module' ); 

