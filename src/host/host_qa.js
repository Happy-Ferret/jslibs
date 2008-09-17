/// standard error [ftr]

		var errBuffer = new Buffer();
		_configuration.stderr = function(chunk) errBuffer.Write(chunk);
		Warning('test')
		delete _configuration.stderr;
		QA.ASSERT( errBuffer.toString().indexOf('warning: test') != -1, true, 'stderr redirection' ); 

/// LoadModule function [ftr]
		
		var id = LoadModule('jsstd');
		QA.ASSERT( LoadModule('jsstd'), id, 'reloading the same module' ); 

