LoadModule('jsstd');
LoadModule('jsio');

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
		//		QA.ASSERT( gcMallocBytes > 100000 && gcMallocBytes < 301000, true, 'Before GC' ); // GC stat not available any more

		s = undefined;
		QA.GC();
		//		QA.ASSERT( gcMallocBytes < 100, true, 'After GC');
		

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
		QA.ASSERT( LoadModule('azyegyiazgiazygc'), false, 'loading inexisting module' );
		QA.ASSERT( LoadModule(undefined), false, 'loading inexisting module' );
		QA.ASSERT( LoadModule(0), false, 'loading inexisting module' );
		QA.ASSERT( LoadModule(0.0), false, 'loading inexisting module' );
		QA.ASSERT( LoadModule(''), false, 'loading inexisting module' );
		QA.ASSERT( LoadModule(NaN), false, 'loading inexisting module' );
		QA.ASSERT_EXCEPTION(function() LoadModule(), RangeError, 'call LoadModule() without arguments');


/// String memory usage (disabled GC) [tr d]

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
	delete undefined;
	delete global.undefined;
	QA.ASSERT( undefined in global, true, 'undefined is in global object' );

	QA.ASSERT( undefined, (void 0), 'undefined is (void 0)' );
	undefined = 123;
	QA.ASSERT( undefined, (void 0), 'undefined is (void 0)' );


/// global object [f]

	QA.ASSERT( typeof global, 'object', 'global type' );
	delete global;
	QA.ASSERT( typeof global, 'object', 'global type after delete' );
	QA.ASSERT_STR( global.valueOf(), '[object Global]', 'global class' );
	QA.ASSERT( uneval( global ).length > 0, true, 'uneval global' );
	QA.ASSERT( global.Math, Math, 'global std objects' );
	QA.ASSERT_HAS_PROPERTIES( global, '_configuration,arguments' );
	QA.ASSERT( global.arguments[0].indexOf('js') != -1, true , 'current script' );
	QA.ASSERT_TYPE( global.arguments, Array, 'arguments type' );


/// global configuration object [f]

	QA.ASSERT( typeof global._configuration, 'object', '_configuration is object' );
	QA.ASSERT_HAS_PROPERTIES( global._configuration, 'unsafeMode,stdout,stderr' );

	
/// jit enabled [t]
	
	function noop() {}
	var t = +new Date();
	for ( var i = 0; i < 1000000; ++i );
		noop();
	t = +new Date() - t;
	QA.ASSERT( t < 30 , true, 'run time' );


/// blob revision number [f]
	
	QA.ASSERT( '_revision' in Blob, true, 'Blob revision');
	QA.ASSERT( Blob._revision > 3000, true, 'Blob revision number' );


/// error messages

	var buffer = '';
	var prev = _configuration.stderr;
	_configuration.stderr = function(chunk) buffer += chunk;
	
	var ex;
	try {
		LoadModule();
	} catch (_ex) {
		ex = _ex
	}

	_configuration.stderr = prev;
	
	QA.ASSERT_STR( buffer.length == 0, true, 'stderr redirection result' ); 
	QA.ASSERT_STR( ex.message.indexOf('not enough arguments') != -1, true, 'LoadModule() error' ); 


/// catched error messages

	var buffer = '';
	var prev = _configuration.stderr;
	_configuration.stderr = function(chunk) buffer += chunk;
	try {
		eval('azer()');
	} catch (ex) {}
	_configuration.stderr = prev;
	QA.ASSERT_STR( buffer.length == 0, true, 'stderr redirection result' ); 


