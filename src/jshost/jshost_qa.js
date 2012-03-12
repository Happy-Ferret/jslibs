
/// access all properties of the global object [ft]

	for each ( item in global ) {

		var s = String(global[item]);
	}


/// jshost arguments [ftm]

	var process = new Process('jshost.exe', ['-u', '-i', '_host.stdout(arguments.toString())', '123', '-test']);
	var res = toString(process.stdout.read());
	QA.ASSERT_STR( res, "123,-test", "jshost arguments" );


/// jshost stderr [ftm]

	var process = new Process('jshost.exe', ['-u', '-i', '_host.stderr("46t5be4qg6b5e46grb5we4g5rn4trnehirwerwer")']);
	var res = toString(process.stderr.read());
	QA.ASSERT_STR( res, "46t5be4qg6b5e46grb5we4g5rn4trnehirwerwer", "jshost arguments" );


/// global variables [ftrm]

		QA.ASSERT( scripthostpath.length > 0, true, 'script host path' );
		QA.ASSERT( scripthostname.substr(0, 6), 'jshost', 'script host name' );
		QA.ASSERT_TYPE( _host, 'object', 'has _host object' );
		QA.ASSERT_TYPE( global, 'object', 'has "global" property' );
		// Obsolete in Gecko 2 (Firefox 4) // QA.ASSERT( global, Object.__parent__, 'global points to the right global object' );
		
		var mod = loadModule('jsstd');
		var mod1 = loadModule('jsstd');
		QA.ASSERT( mod, mod1, 'loadModule' );
		QA.ASSERT( 'unsafeMode' in _host, true, 'unsafe mode is present' );
		QA.ASSERT( global.arguments[0].substr(-5), 'qa.js', 'javascript program name' );


/// eval function [ftrm]

		QA.ASSERT( typeof eval, 'function', 'eval function availability' );


/// undefined mutability [ftrm]
		
		QA.ASSERT( undefined, (void 0), 'compare to void 0 (default)' );
		undefined = 123;
		QA.ASSERT( undefined, (void 0), 'compare to void 0 (changed)' );


/// embedded bootstrap script [ftrm]

		QA.ASSERT( 'bootstrapScript' in _host, false, 'no embedded bootstrap script by default' );


/// EndSignalEvents handle object [ft]

	var h = EndSignalEvents();
	QA.ASSERT_TYPE(h, Handle, 'handle object type');
	QA.ASSERT_STR(h, '[Handle  pev]', 'handle type string');
	QA.ASSERT_TYPE(h.toString, Function, 'handle toString is a function');
	
