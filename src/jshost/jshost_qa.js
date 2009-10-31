/// configuration objet [ftrm]
	
	
	


/// access all properties of the global object [ft]

		for each ( item in global ) {

			var s = String(global[item]);
		}


/// jshost arguments [ftm]

		QA.ASSERT( global.arguments[2], 'arg2', 'script argument 2' );
		QA.ASSERT( global.arguments[3], 'arg3', 'script argument 3' );
		QA.ASSERT( global.arguments[4], 'arg4', 'script argument 4' );


/// global variables [ftrm]

		QA.ASSERT( scripthostpath.length > 0, true, 'script host path' );
		QA.ASSERT( scripthostname.substr(0, 6), 'jshost', 'script host name' );
		QA.ASSERT_TYPE( _configuration, 'object', 'has configuration object' );
		QA.ASSERT_TYPE( global, 'object', 'has "global" property' );
		QA.ASSERT( global, Object.__parent__, 'global points to the right global object' );
		
		var mod = LoadModule('jsstd');
		var mod1 = LoadModule('jsstd');
		QA.ASSERT( mod, mod1, 'LoadModule' );
		QA.ASSERT( 'unsafeMode' in _configuration, true, 'unsafe mode is present' );
		QA.ASSERT( global.arguments[0].substr(-5), 'qa.js', 'javascript program name' );


/// eval function [ftrm]

		QA.ASSERT( typeof eval, 'function', 'eval function availability' );


/// undefined mutability [ftrm]
		
		QA.ASSERT( undefined, (void 0), 'compare to void 0 (default)' );
		undefined = 123;
		QA.ASSERT( undefined, (void 0), 'compare to void 0 (changed)' );


/// embedded bootstrap script [ftrm]

		QA.ASSERT( 'bootstrapScript' in _configuration, false, 'no embedded bootstrap script by default' );
