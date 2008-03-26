({

	GlobalVariables: function(QA) {
		
		QA.ASSERT( scripthostpath.length > 0, true, 'script host path' );
		QA.ASSERT( scripthostname.substr(0, 6), 'jshost', 'script host name' );
		QA.ASSERT_TYPE( _configuration, 'object', 'has configuration object' );
		QA.ASSERT_TYPE( global, 'object', 'has global object' );
		
		var mod = LoadModule('jsstd');
		var mod1 = LoadModule('jsstd');
		QA.ASSERT( mod, mod1, 'LoadModule' );
		QA.ASSERT( _configuration.unsafeMode, true, 'unsafe mode is active' );
		QA.ASSERT( global.arguments[0], 'qarun.js', 'javascript program name' );
	}

})
