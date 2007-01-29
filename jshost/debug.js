LoadModule('jsstd');
Print( 'Unsafe mode: '+configuration.unsafeMode, '\n' );

global.test = 123;
Print( test )