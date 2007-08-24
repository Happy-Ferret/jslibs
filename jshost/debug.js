#!test

var hModule = LoadModule('jsstd');
Print( 'hModule: '+hModule, '\n' );

var hModule = LoadModule('jsstd');
Print( 'hModule: '+hModule, '\n' );


Print( 'Unsafe mode: '+_configuration.unsafeMode, '\n' );

Print('global:'+global, '\n');
global.test = 123;

Print ( scripthostpath, '\n' );
Print ( scripthostname, '\n' );


var i = 0;
function a() (i++ < 999) && a();
a();

