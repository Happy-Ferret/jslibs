loadModule('jsstd');
loadModule('jsio');

/// call all possible functions reachable in the scope [rmtf]

	var excludeList = ['done', 'Object.__proto__.__proto__', 'Iterator', '_host.stdin' ];

	loadModule('jswinshell'); excludeList.push('FileOpenDialog', 'Console.close');
	loadModule('jssdl'); excludeList.push('SetVideoMode', 'Iconify');
	loadModule('jsstd'); excludeList.push('Halt');
	loadModule('jsdebug'); excludeList.push('DebugBreak', 'DumpHeap');

	loadModule('jscrypt');
	loadModule('jsfont');
	loadModule('jsiconv');
	loadModule('jsimage');
	loadModule('jsio');
	loadModule('jsjabber');
	loadModule('jsode');
	loadModule('jsoglft');
	loadModule('jsprotex');
	loadModule('jssound');
	loadModule('jssqlite');
	loadModule('jssvg');
	loadModule('jstrimesh');
	loadModule('jsvideoinput');
	loadModule('jsz');
	
//	loadModule('jstask');
//	loadModule('jsffi');
//	loadModule('jsfastcgi');
//	loadModule('jsaudio');
// loadModule('jsgraphics');

	if ( _host.unsafeMode ) throw "Cannot run this test in unsafe mode (else crash)";

	var count = 0;
	var done = {__proto__:null};
	for each ( var item in excludeList )
		done[objectGCId(eval(item))] = true;
	
	function fct(obj, left) {

		if ( endSignal )
			halt();
		if ( isPrimitive(obj) )
			return;

		done[objectGCId(obj)] = true;
		var list = Object.getOwnPropertyNames(obj);
		for each ( var name in list ) {

			if ( name == 'arguments' )
				continue;

//print( left+'.'+name+'\n' );

			var nextObj;
			try {
				nextObj = obj[name];
			} catch(ex) {
				continue;
			}
			
			if ( done[objectGCId(nextObj)] )
				continue;

			try {
				if ( String.prototype.indexOf.call(nextObj, '[native code]') == -1 )
					continue;
			} catch(ex) {
				continue;
			}

			try {
				obj[name]();
			} catch(ex) {}

			try {
				nextObj();
			} catch(ex) {}

			fct(nextObj, left+'.'+name);
		}
	}

	fct(global, '');


/// host version info [rmtf]

	QA.ASSERT( _host.build > 43000, true, 'build version validity' );
	QA.ASSERT( _host.revision > 3400, true, 'revision version validity' );
	QA.ASSERT( _host.jsVersion >= 185, true, 'javascript version validity' );
	
	
	
/// bug bz#522024 [rmtf]

		var list = [];
		function add() {

			 list.push(arguments);
		}

		function run() {

			 for each ( var item in list )
				  item[0]();
		}

		for ( var i = 0; i < 10; i++ )
			 add(function(s) { });

		run();



/// JSOPTION_ANONFUNFIX option [frm]
	
	113 == function(x, y) {return x+y} (100, 13);

	
	
/// GC test [r]
		
		QA.GC();
		var s = stringRepeat('x', 100000);
		//		QA.ASSERT( gcMallocBytes > 100000 && gcMallocBytes < 301000, true, 'Before GC' ); // GC stat not available any more

		s = undefined;
		QA.GC();
		//		QA.ASSERT( gcMallocBytes < 100, true, 'After GC');
		


/// stdout standard output redirection [ftrm]

		var buffer = '';

		var prev = _host.stdout;
		_host.stdout = function(chunk) buffer += chunk;
		print('this_is_a_test');
		_host.stdout = prev;

		QA.ASSERT( buffer.indexOf('this_is_a_test') != -1, true, 'stdout redirection result' ); 



/// error in stderr [ftrm]

	var prev = _host.stderr;
	try { 
		_host.stderr = function() { fvoasudyfvoasuid() }
		wuiyoiryuoeyu();
	} catch(ex) {}
	_host.stderr = prev;



/// loadModule function [ftrm]
		
		var id = loadModule('jsstd');
		QA.ASSERT( loadModule('jsstd'), id, 'reloading the same module' );
		QA.ASSERT( loadModule('azyegyiazgiazygc'), false, 'loading inexisting module' );
		QA.ASSERT( loadModule(undefined), false, 'loading inexisting module' );
		QA.ASSERT( loadModule(0), false, 'loading inexisting module' );
		QA.ASSERT( loadModule(0.0), false, 'loading inexisting module' );
		QA.ASSERT( loadModule(''), false, 'loading inexisting module' );
		QA.ASSERT( loadModule(NaN), false, 'loading inexisting module' );
		//QA.ASSERT_EXCEPTION(function() loadModule(), RangeError, 'call loadModule() without arguments');



/// String memory usage (disabled GC) [tr d]

		var length = 1024*1024;
		var times = 3;
		var data = [];

		QA.GC();
		var mem0 = privateMemoryUsage;
		
		var prev = disableGarbageCollection;
		disableGarbageCollection = true;
		
		for ( var i = 0; i < times; ++i ) {
		
			data.push( stringRepeat('a', length) ); // disableGarbageCollection should be enough
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
	QA.ASSERT_STR( global.valueOf(), '[object global]', 'global class' );
	QA.ASSERT( uneval( global ).length > 0, true, 'uneval global' );
	QA.ASSERT( global.Math, Math, 'global std objects' );
	QA.ASSERT_HAS_PROPERTIES( global, '_host,arguments' );
	QA.ASSERT( global.arguments[0].indexOf('js') != -1, true , 'current script' );
	QA.ASSERT_TYPE( global.arguments, Array, 'arguments type' );



/// global _host object [f]

	QA.ASSERT( typeof global._host, 'object', '_host is object' );
	QA.ASSERT_HAS_PROPERTIES( global._host, 'unsafeMode,stdout,stderr' );

	
	
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



/// error messages []

	var buffer = '';
	var prev = _host.stderr;
	_host.stderr = function(chunk) buffer += chunk;
	
	var ex = undefined;
	try {
		loadModule();
	} catch (_ex) {
		ex = _ex
	}
	
	QA.ASSERT(!ex, _host.unsafeMode, "detect exception for empty loadModule call");

	_host.stderr = prev;
	
	QA.ASSERT_STR( buffer.length == 0, true, 'stderr redirection result' ); 
	
	if ( !_host.unsafeMode )
		QA.ASSERT_STR( ex.message.indexOf('number of arguments') != -1, true, 'loadModule() error' ); 



/// catched error messages [rmtf]

	var buffer = '';
	var prev = _host.stderr;
	_host.stderr = function(chunk) buffer += chunk;
	try {
		eval('azer()');
	} catch (ex) {}
	_host.stderr = prev;
	QA.ASSERT_STR( buffer.length == 0, true, 'stderr redirection result' );



/// mute error messages [rmtf]

	var prev = _host.stderr;
	delete _host.stderr;
	
	try {
		eval('azer()');
	} catch (ex) {}

	_host.stderr = prev;


/// NativeInterface hacking

	var b = new Blob('abc');

	QA.NO_CRASH( stringify(b), 'abc' );

	var c = {};
	c._NI_BufferGet = b._NI_BufferGet;
	QA.NO_CRASH( stringify(c) );

	QA.NO_CRASH( stringify({ _NI_BufferGet:function() {} }) );

	try {
//	QA.NO_CRASH( Stringify({ __proto__:b}) );
	} catch(ex) {}
