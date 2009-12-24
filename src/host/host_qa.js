LoadModule('jsstd');

/// bug bz#522024

		var list = [];
		function Add() {

			 list.push(arguments);
		}

		function Run() {

			 for each ( var item in list )
				  item[0]();
		}

		for ( var i = 0; i < 10; i++ )
			 Add(function(s) { });

		Run();

/// JSOPTION_ANONFUNFIX option [frm]
	
	113 == function(x, y) {return x+y} (100, 13);

	
/// GC test [r]
		
		QA.GC();
		var s = StringRepeat('x', 100000);
		QA.ASSERT( gcMallocBytes > 100000 && gcMallocBytes < 301000, true, 'Before GC' );

		s = undefined;
		QA.GC();
		QA.ASSERT( gcMallocBytes < 100, true, 'After GC');
		

/// stdout standard output redirection [ftrm]

		var buffer = '';

		var prev = _configuration.stdout;
		_configuration.stdout = function(chunk) buffer += chunk;
		Print('this_is_a_test');
		_configuration.stdout = prev;

		QA.ASSERT( buffer.indexOf('this_is_a_test') != -1, true, 'stdout redirection result' ); 


/// LoadModule function [ftrm]
		
		var id = LoadModule('jsstd');
		QA.ASSERT( LoadModule('jsstd'), id, 'reloading the same module' );

/// LoadModule in a namespace [ftrm]

/// Load an inexisting module [ftrm]

		QA.ASSERT( LoadModule('azyegyiazgiazygc'), false, 'loading inexisting module' );

/// String memory usage (disabled GC) [tr]

		var length = 1024*1024;
		var times = 3;
		var data = [];

		QA.GC();
		var mem0 = privateMemoryUsage;
		
		var prev = disableGarbageCollection;
		disableGarbageCollection = true;
		
		for ( var i = 0; i < times; ++i ) {
		
			data.push( StringRepeat('a', length) ); // disableGarbageCollection should be enough
		}
		var mem = (privateMemoryUsage - mem0) / length / times;
		
		disableGarbageCollection = prev;

		QA.ASSERT( mem > 3 && mem < 3.02, true, 'string memory usage ('+mem+')' );


/// undefined is read-only [ftrm]

	QA.ASSERT( undefined in global, true, 'undefined is in global object' );
	delete undefined
	QA.ASSERT( undefined in global, true, 'undefined is in global object' );

	QA.ASSERT( undefined, (void 0), 'undefined is (void 0)' );
	undefined = 123;
	QA.ASSERT( undefined, (void 0), 'undefined is (void 0)' );
