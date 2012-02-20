// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  halt();

loadModule('jsffi');

function VOID( value ) {

	return new NativeData().VOID;
}

function NULL() {

	var nat = new NativeData().PP;
	nat.alloc()[0] = 0;
	return nat;
}

function INT( value ) {

	var nat = new NativeData().PI;
	nat.alloc();
	if (value != undefined) nat[0] = value;
	return nat;
}

function U32( value ) {

	var nat = new NativeData().PU32;
	nat.alloc();
	if (value != undefined) nat[0] = value;
	return nat;
}

function SZ( value ) {

	var nat = new NativeData;
	nat.String = value;
	return nat.PP;
}

function sleep( time ) {

	new NativeModule('kernel32', true).proc('Sleep')(VOID(), U32(time));
}

function alert( text, caption ) {

	var ret = U32()
	new NativeModule('User32', true).proc('MessageBoxA')(ret, NULL(), SZ(text), SZ( caption || 'Alert' ), INT( 0 )); // using ordinal value of MessageBoxA
	return ret[0];
}

alert('Press ok and wait 2000ms', 'Alert box');



halt();


	function tests() {

		var check = 'check allocation';
		var arg1 = new NativeData();
		arg1.PS8.alloc(10000);

		for ( var i=0; i<10000; i++ )
			arg1.PS8[i] = 'd';

		for ( var i=0; i<10000; i++ )
			arg1.PS8[i];

		for ( var i=0; i<10000; i++ )
			if ( arg1.PS8[i] != 'd'.charCodeAt(0) )
				return check;
		arg1.PS8.free();



		var check = 'check unsigned long (32bits) max';
		var arg1 = new NativeData();
		arg1.PU32.alloc(1)[0] = 4294967295;
		if ( arg1.PU32[0] != 4294967295 )
			return check;

		var check = 'check signed long (32bits) max negative';
		var arg1 = new NativeData();
		arg1.PS32.alloc()[0] = -2147483648;
		if ( arg1.PS32[0] != -2147483648 )
			return check;

		var check = 'check signed long (32bits) max positive';
		var arg1 = new NativeData();
		arg1.PS32.alloc()[0] = 2147483647;
		if ( arg1.PS32[0] != 2147483647 )
			return check;



		check = 'check unsigned short type (16bits) max';
		var test = new NativeData();
		test.PU16.alloc()[0] = 65535;
		if ( test.PU16[0] != 65535 )
			return check;

		check = 'check signed short type (16bits) max negative';
		var test = new NativeData();
		test.PS16.alloc()[0] = -32768;
		if ( test.PS16[0] != -32768 )
			return check;

		check = 'check signed short type (16bits) max positive';
		var test = new NativeData();
		test.PS16.alloc()[0] = 32767;
		if ( test.PS16[0] != 32767 )
			return check;



		check = 'check unsigned char type (8bits) max';
		var test = new NativeData();
		test.PU8.alloc()[0] = 255;
		if ( test.PU8[0] != 255 )
			return check;

		check = 'check signed char type (8bits) max negative';
		var test = new NativeData();
		test.PS8.alloc()[0] = -128;
		if ( test.PS8[0] != -128 )
			return check;

		check = 'check signed char type (8bits) max positive';
		var test = new NativeData();
		test.PS8.alloc()[0] = 127;
		if ( test.PS8[0] != 127 )
			return check;


		check = 'check signed char type (8bits) with a double value';
		var test = new NativeData();
		test.PS8.alloc()[0] = 123.456;
		if ( test.PS8[0] != 123 )



		var check = 'check string access write with PS8';
		var arg1 = new NativeData();
		arg1.PP.alloc()[0].PS8.alloc(5);
		arg1.PP[0].PS8[0] = 'd';
		arg1.PP[0].PS8[1] = 'e';
		arg1.PP[0].PS8[2] = 'm';
		arg1.PP[0].PS8[3] = 'o';
		arg1.PP[0].PS8[4] = 0;
		if ( arg1.String != 'demo' )
			return check;


		var check = 'check string ( SZ )';
		var arg1 = new NativeData();
		arg1.String = 'wuyrouyquwfoiuasdhofiuasdyfroqwyrvoquweyrvbqwueyrvboqwueryvaobsdufyvqwleuyfvlasdufyvuqbwieufyvlabsdufyvbqwluefyvlsaduyf';
		if ( arg1.String != 'wuyrouyquwfoiuasdhofiuasdyfroqwyrvoquweyrvbqwueyrvboqwueryvaobsdufyvqwleuyfvlasdufyvuqbwieufyvlabsdufyvbqwluefyvlsaduyf' )
			return check;
		

		var check = 'check call sleep on kernel32.dll'
		var proc = new NativeModule('C:\\WINDOWS\\SYSTEM32\\kernel32').proc('Sleep');
		var arg1 = new NativeData();
		arg1.PU32.alloc(1)[0] = 300;
		proc( new NativeData().VOID, arg1.PU32 );

		var check = 'check call getSystemDirectoryA on kernel32'

		var ret = new NativeData();
		ret.PU32.alloc(); // unsigned long * ret = new unsigned long

		var arg1 = new NativeData();
		arg1.PP.alloc()[0].PS8.alloc( 200 ); // void** arg1 = new void*[]; arg1[0] = new char[200];

		var arg2 = new NativeData();
		arg2.PU32.alloc()[0] = 200;

		(new NativeModule('C:\\WINDOWS\\SYSTEM32\\kernel32')).proc('GetSystemDirectoryA')( ret.PU32, arg1.PP, arg2.PU32 );

		//print( arg1.String );
		return undefined;
	}

	print( 'runing tests...' + (tests() || 'nothing') + ' failed.' , '\n');
