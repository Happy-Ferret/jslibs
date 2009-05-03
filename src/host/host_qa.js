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


/// String memory usage [tr]

		var length = 1024*1024;
		var times = 3;
		var data = [];

		QA.GC();
		var mem0 = privateMemoryUsage;
		disableGarbageCollection = true;

		for ( var i = 0; i < times; ++i ) {
		
			data.push( StringRepeat('a', length) );
			QA.GC();
		}
		var mem = (privateMemoryUsage - mem0) / length / times;
		disableGarbageCollection = false;

		QA.ASSERT( mem > 3 && mem < 3.02, true, 'string memory usage ('+mem+')' );

