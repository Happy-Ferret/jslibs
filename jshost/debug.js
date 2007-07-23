

var hModule = LoadModule('jsstd');
Print( 'hModule: '+hModule, '\n' );
var hModule = LoadModule('jsstd');
Print( 'hModule: '+hModule, '\n' );

//Print( 'Unsafe mode: '+Configuration.unsafeMode, '\n' );

Print('global:'+global, '\n');
global.test = 123;

Print ( scripthostpath, '\n' );
Print ( scripthostname, '\n' );

UnloadModule(hModule);

