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
	if ( value != undefined ) nat[0] = value;
	return nat;
}

function U32( value ) {

	var nat = new NativeData().PU32;
	nat.Alloc();
	if ( value != undefined  ) nat[0] = value;
	return nat;
}

function S32( value ) {

	var nat = new NativeData().PS32;
	nat.Alloc();
	if ( value != undefined ) nat[0] = value;
	return nat;
}


function DWORD( value ) {

	var nat = new NativeData().PU32;
	nat.Alloc();
	if ( value != undefined ) nat[0] = value;
	return nat;
}


function BYTE( value ) {

	var nat = new NativeData().PU8;
	nat.Alloc();
	if ( value != undefined ) nat[0] = value;
	return nat;
}


function SZ( value ) {

	var nat = new NativeData;
	nat.String = value;
	return nat.PP;
}


function MAKEPTR( ptr ) {

	var nat = new NativeData();
	nat.PP.Alloc()[0] = ptr;
	return nat.PP;
}


function PPTR() {

	var nat = new NativeData().PP.Alloc();
	nat[0].PP.Alloc();
	return nat;
}


function PTRTO( nativeData ) {

	var ptr = new NativeData().PP.Alloc();
	ptr[0] = nativeData;
  return ptr;
}


function PDATA( size, initByte ) {

	var nat = new NativeData().PP;
	nat.Alloc()[0].PS8.Alloc( size, initByte );
	return nat;
}
