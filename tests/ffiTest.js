LoadModule('jsffi');

function VOID( value ) {

	return new NativeData().VOID;
}

function NULL() {

	var nat = new NativeData().PP;
	nat.Alloc()[0] = 0;
	return nat;
}

function INT( value ) {

	var nat = new NativeData().PI;
	nat.Alloc();
	if (value != undefined) nat[0] = value;
	return nat;
}

function U32( value ) {

	var nat = new NativeData().PU32;
	nat.Alloc();
	if (value != undefined) nat[0] = value;
	return nat;
}

function SZ( value ) {

	var nat = new NativeData;
	nat.String = value;
	return nat.PP;
}

function Sleep( time ) {

	new NativeModule('kernel32', true).Proc('Sleep')(VOID(), U32(time));
}

function Alert( text, caption ) {

	var ret = U32()
	new NativeModule('User32', true).Proc('MessageBoxA')(ret, NULL(), SZ(text), SZ(caption || 'Alert'), INT(4)); // using ordinal value of MessageBoxA
	return ret[0];
}



if ( Alert('Wait 2000ms ?') == 6 )
	Sleep(2000);
