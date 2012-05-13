loadModule('jsstd');
loadModule('jsio');

/// call all possible functions reachable in the scope
	
	if ( QA.IS_UNSAFE ) // Cannot run this test in unsafe mode (else crash)
		return;

	var excludeList = ['done', 'Object.__proto__.__proto__', 'Iterator', 'host.stdin', 'setPerfTestMode' , 'jslangTest' ]; // 

	loadModule('jswinshell'); excludeList.push('fileOpenDialog', 'Console.close');
	loadModule('jssdl'); excludeList.push('setVideoMode', 'iconify');
	loadModule('jsstd'); excludeList.push('halt');
	loadModule('jsdebug'); excludeList.push('debugBreak');

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
	loadModule('jstask');
	
//	loadModule('jsffi');
//	loadModule('jsfastcgi');
//	loadModule('jsaudio');
//	loadModule('jsgraphics');


	var done = new Set();

	for ( var item of excludeList ) {
		try {
			done.add(eval(item));
		} catch(ex){}
	}
	
	function fct(obj, left) {

		if ( host.endSignal )
			halt();
			
		if ( isPrimitive(obj) )
			return;

		done.add(obj);
		var list = Object.getOwnPropertyNames(obj);
		for ( var name of list ) {

			if ( name == 'arguments' )
				continue;

//print( left+'.'+name+'\n' );

			var nextObj;
			try {
				nextObj = obj[name];
			} catch(ex) {
				continue;
			}
			
			if ( done.has(nextObj) )
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


/// host version info [p]

	QA.ASSERTOP( host._sourceId, '>', 3400, 'sourceId version validity' );
	QA.ASSERTOP( host.jsVersion, '>=', 185, 'javascript version validity' );
	
	
	
/// bug bz#522024 [p]

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



/// JSOPTION_ANONFUNFIX option []
	
	113 == function(x, y) {return x+y} (100, 13);

	
	
/// GC test []
		
		QA.gc();
		var s = stringRepeat('x', 100000);
		//		QA.ASSERT( gcMallocBytes > 100000 && gcMallocBytes < 301000, true, 'Before GC' ); // GC stat not available any more

		s = undefined;
		QA.gc();
		//		QA.ASSERT( gcMallocBytes < 100, true, 'After GC');
		


/// stdout standard output redirection [p]

		var buffer = '';

		var prev = host.stdout;
		host.stdout = function(chunk) buffer += chunk;
		print('this_is_a_test');
		host.stdout = prev;

		QA.ASSERTOP( buffer.indexOf('this_is_a_test'), '!=', -1, 'stdout redirection result' ); 



/// error in stderr [p]

	var prev = host.stderr;
	try { 
		host.stderr = function() { fvoasudyfvoasuid() }
		wuiyoiryuoeyu();
	} catch(ex) {}
	host.stderr = prev;



/// loadModule function [p]
		
		var id = loadModule('jsstd');
		QA.ASSERTOP( loadModule('jsstd'), '===', id, 'reloading the same module' );
		QA.ASSERTOP( loadModule('azyegyiazgiazygc'), '===', false, 'loading inexisting module' );
		QA.ASSERTOP( loadModule(undefined), '===', false, 'loading inexisting module' );
		QA.ASSERTOP( loadModule(0), '===', false, 'loading inexisting module' );
		QA.ASSERTOP( loadModule(0.0), '===', false, 'loading inexisting module' );
		QA.ASSERTOP( loadModule(''), '===', false, 'loading inexisting module' );
		QA.ASSERTOP( loadModule(NaN), '===', false, 'loading inexisting module' );
		//QA.ASSERT_EXCEPTION(function() loadModule(), RangeError, 'call loadModule() without arguments');



/// String memory usage (disabled GC) [d]

		var length = 1024*1024;
		var times = 3;
		var data = [];

		QA.gc();
		var mem0 = privateMemoryUsage;
		
		var prev = disableGarbageCollection;
		disableGarbageCollection = true;
		
		for ( var i = 0; i < times; ++i ) {
		
			data.push( stringRepeat('a', length) ); // disableGarbageCollection should be enough
		}
		var mem = (privateMemoryUsage - mem0) / length / times;
		
		disableGarbageCollection = prev;

		QA.ASSERTOP( mem, '>', 3, 'min string memory usage' );
		QA.ASSERTOP( mem, '<', 3.02, 'max string memory usage' );



/// undefined is read-only [p]

	QA.ASSERTOP( undefined, 'in', global, 'undefined is in global object' );
	delete undefined;
	delete global.undefined;
	QA.ASSERTOP( undefined, 'in', global, 'undefined is still in global object' );

	QA.ASSERTOP( undefined, '===', (void 0), 'undefined is (void 0)' );
	undefined = 123;
	QA.ASSERTOP( undefined, '===', (void 0), 'undefined is still (void 0)' );



/// global object []

	QA.ASSERTOP( global, 'typeof', 'object', 'global type' );
	delete global;
	delete global.global;
	QA.ASSERTOP( global, 'typeof', 'object', 'global type after delete' );
	QA.ASSERT_STR( global.valueOf(), '[object Global]', 'global class' );
	QA.ASSERTOP( uneval(global).length, '>', 0, 'uneval global' );
	QA.ASSERTOP( global.Math, '===', Math, 'global std objects' );
	QA.ASSERTOP( global, 'has', 'host' );
	QA.ASSERTOP( host, 'has', 'arguments' );
	QA.ASSERTOP( host.arguments[0].indexOf('js'), '!=', -1, 'current script' );
	QA.ASSERTOP( host.arguments, 'instanceof', Array, 'arguments type' );


/// global host object []

	QA.ASSERTOP( global.host, 'typeof', 'object', 'host is object' );
	QA.ASSERTOP( global.host, 'has', 'unsafeMode' );
	QA.ASSERTOP( global.host, 'has', 'stdout' );
	QA.ASSERTOP( global.host, 'has', 'stderr' );
	QA.ASSERTOP( global.host, 'has', 'stdin' );



/// error messages [p]

	var buffer = '';
	var prev = host.stderr;
	host.stderr = function(chunk) buffer += chunk;
	
	var ex = undefined;
	try {
		loadModule();
	} catch (_ex) {
		ex = _ex
	}
	
	QA.ASSERT(!ex, host.unsafeMode, "detect exception for empty loadModule call");

	host.stderr = prev;
	
	QA.ASSERT_STR( buffer.length == 0, true, 'stderr redirection result' ); 
	
	if ( !host.unsafeMode )
		QA.ASSERT_STR( ex.message.indexOf('number of arguments') != -1, true, 'loadModule() error' ); 



/// catched error messages [p]

	var buffer = '';
	var prev = host.stderr;
	host.stderr = function(chunk) buffer += chunk;
	try {
		eval('azer()');
	} catch (ex) {}
	host.stderr = prev;
	QA.ASSERT_STR( buffer.length == 0, true, 'stderr redirection result' );



/// mute error messages [p]

	var prev = host.stderr;
	delete host.stderr;
	
	try {
		eval('azer()');
	} catch (ex) {}

	host.stderr = prev;


/// NativeInterface hacking

	var b = stringify('abc', true);

	QA.NO_CRASH( stringify(b), 'abc' );

	var c = {};
	c._NI_BufferGet = b._NI_BufferGet;
	QA.NO_CRASH( stringify(c) );

	QA.NO_CRASH( stringify({ _NI_BufferGet:function() {} }) );

	try {
//	QA.NO_CRASH( stringify({ __proto__:b}) );
	} catch(ex) {}
