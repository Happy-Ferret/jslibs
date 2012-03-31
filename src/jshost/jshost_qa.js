
/// access all properties of the global object [ft]

	for each ( item in global ) {

		var s = String(global[item]);
	}


/// jshost arguments [ftm]

	var process = new Process('jshost', ['-u', '-i', 'host.stdout(host.arguments.toString())', '123', '-test']);
	var res = stringify(process.stdout.read());
	QA.ASSERT_STR( res, "123,-test", "jshost arguments" );


/// jshost stderr [ftm]

	var process = new Process('jshost', ['-u', '-i', 'host.stderr("46t5be4qg6b5e46grb5we4g5rn4trnehirwerwer")']);
	var res = stringify(process.stderr.read());
	QA.ASSERT_STR( res, "46t5be4qg6b5e46grb5we4g5rn4trnehirwerwer", "jshost arguments" );


/// global variables [ftrm]

		QA.ASSERTOP( host.path.length, '>', 0, 'script host path' );
		QA.ASSERTOP( host.name.substr(0, 6), '==', 'jshost', 'script host name' );
		QA.ASSERTOP( host, 'typeof', 'object', 'has host object' );
		QA.ASSERTOP( global, 'typeof', 'object', 'has "global" property' );

		// Obsolete in Gecko 2 (Firefox 4) // QA.ASSERT( global, Object.__parent__, 'global points to the right global object' );
		
		var mod = loadModule('jsstd');
		var mod1 = loadModule('jsstd');
		QA.ASSERT( mod, mod1, 'loadModule' );
		QA.ASSERT( 'unsafeMode' in host, true, 'unsafe mode is present' );
		QA.ASSERT( host.arguments[0].substr(-5), 'qa.js', 'javascript program name' );


/// eval function [ftrm]

		QA.ASSERT( typeof eval, 'function', 'eval function availability' );


/// undefined mutability [ftrm]
		
		QA.ASSERT( undefined, (void 0), 'compare to void 0 (default)' );
		undefined = 123;
		QA.ASSERT( undefined, (void 0), 'compare to void 0 (changed)' );


/// embedded bootstrap script [ftrm]

		QA.ASSERT( 'bootstrapScript' in host, false, 'no embedded bootstrap script by default' );


/// host.endSignalEvents() handle object [ft]

	var h = host.endSignalEvents();
	QA.ASSERTOP(h, 'instanceof', Handle, 'handle object type');
	QA.ASSERTOP(h, '==', '[Handle  pev]', 'handle type string');
	QA.ASSERTOP(h.toString, 'instanceof', Function, 'handle toString is a function');
