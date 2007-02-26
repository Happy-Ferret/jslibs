LoadModule('jsstd');
Print( 'Unsafe mode: '+configuration.unsafeMode, '\n' );

Print('global:'+global, '\n');
global.test = 123;

