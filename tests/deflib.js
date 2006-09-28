LoadModule('jsstd');
/*
Exec('jsni.js');



function LoadModule( fileName ) {

	var module = new NativeModule( fileName );
	var ret = new NativeData().PS32.Alloc();
	module.Proc('ModuleInit')( ret, MAKEPTR( JSContextToPtr() ), MAKEPTR( JSObjectToPtr( this ) ) );
	if ( ret[0] == 0 )
		throw( new Error('unable to load '+fileName) );
}
*/