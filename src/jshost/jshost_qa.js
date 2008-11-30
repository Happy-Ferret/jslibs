/// arguments [ftr]

		QA.ASSERT( global.arguments[2], 'arg2', 'script argument 2' );
		QA.ASSERT( global.arguments[3], 'arg3', 'script argument 3' );
		QA.ASSERT( global.arguments[4], 'arg4', 'script argument 4' );


/// global variables [ftr]

		QA.ASSERT( scripthostpath.length > 0, true, 'script host path' );
		QA.ASSERT( scripthostname.substr(0, 6), 'jshost', 'script host name' );
		QA.ASSERT_TYPE( _configuration, 'object', 'has configuration object' );
		QA.ASSERT_TYPE( global, 'object', 'has global object' );
		
		var mod = LoadModule('jsstd');
		var mod1 = LoadModule('jsstd');
		QA.ASSERT( mod, mod1, 'LoadModule' );
		QA.ASSERT( _configuration.unsafeMode, false, 'unsafe mode is inactive' );
		QA.ASSERT( global.arguments[0], 'qa.js', 'javascript program name' );


/// eval function [ftr]

		QA.ASSERT( typeof eval, 'function', 'eval function availability' );

