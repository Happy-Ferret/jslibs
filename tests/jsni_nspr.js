/*
http://www.mozilla.org/projects/nspr/reference/html/index.html

PR_FAILURE = -1
PR_SUCCESS = 0

*/

var _nsprLibrary = new NativeModule( 'libnspr4', true );

var ret = INT();
_nsprLibrary.Proc('PR_VersionCheck')( ret, SZ( '4.6' ) );
if ( !ret[0] )
	Error( 'bad NSPR version ( 4.6+ is needed )' );


function nsprGetError() {

	var ret = S32();
	_nsprLibrary.Proc('PR_GetError')( ret );
	return ret[0];
}

function nsprGetErrorText() {

	var ret = S32();
	_nsprLibrary.Proc('PR_GetErrorTextLength')( ret );
	var length = ret[0];
	if ( length == 0 ) { // If a zero is returned, no error text is currently set
		return undefined;  // there is no text
	}
	var str = PDATA(length+1);
	_nsprLibrary.Proc('PR_GetErrorText')( ret, str );
	return pdata.String;
}

nsprError.prototype = new Error;
function nsprError() {
  this.number = nsprGetError();
  this.message = nsprGetErrorText();
}



function File_base() {
	
	this.Open = function( flags, mode ) {

		var ret = new NativeData();
		ret.PP.Alloc();
		_nsprLibrary.Proc('PR_Open')( ret.PP, SZ( this.fileName ), INT( flags || File.RDONLY ), INT( mode || 0 ) );
		if ( ret.PU32[0] == 0 )
			throw( new nsprError() );
		this._handler = ret.PP;
	}

	this.Close = function() {

		var ret = INT();
		_nsprLibrary.Proc('PR_Close')( ret, this._handler );
		//return ret[0];
	}

	this.Write = function( data ) {

		var ret = S32();
		_nsprLibrary.Proc('PR_Write')( ret, this._handler, SZ(data), S32( data.length ) );
		var returnValue = ret[0];
		if ( returnValue == -1 )
			throw( new nsprError() );
		return ret[0];
	}

	this.Read = function( length ) {

		var pdata = new NativeData();
		pdata.PP.Alloc()[0].PS8.Alloc(length);
		var ret = S32();
		_nsprLibrary.Proc('PR_Read')( ret, this._handler, pdata.PP, S32(length) );
		var returnValue = ret[0];
		if ( returnValue == -1 )
			throw( new nsprError() );
		return pdata.Data(returnValue);
	}

	this.Exists = function() {

		var ret = INT();
		_nsprLibrary.Proc('PR_Access')( ret, SZ( this.fileName ), INT( 1 ) );
		return ret[0] == 0; // PR_SUCCESS
	}

	this.Writable = function() {

		var ret = INT();
		_nsprLibrary.Proc('PR_Access')( ret, SZ( this.fileName ), INT( 2 ) ); // PR_ACCESS_WRITE_OK
		return ret[0] == 0; // PR_SUCCESS
	}

	this.Readable = function() {

		var ret = INT();
		_nsprLibrary.Proc('PR_Access')( ret, SZ( this.fileName ), INT( 3 ) ); // PR_ACCESS_READ_OK
		return ret[0] == 0; // PR_SUCCESS
	}

	this.Available = function() {

		var ret = U32();
		_nsprLibrary.Proc('PR_Available')( ret, this._handler );
		var returnValue = ret[0];
		if ( returnValue == -1 )
			throw( new nsprError() );
		return returnValue;
	}

}

File.prototype = new File_base;
function File( fileName ) {

	this.fileName = fileName;
}

File.RDONLY      = 0x01;
File.WRONLY      = 0x02;
File.RDWR        = 0x04;
File.CREATE_FILE = 0x08;
File.APPEND      = 0x10;
File.TRUNCATE    = 0x20;
File.SYNC        = 0x40;
File.EXCL        = 0x80;

////////////////////////////////////////////////////

function Dir_base() {

	this.Open = function() {

		var ret = new NativeData().PP.Alloc();
		_nsprLibrary.Proc('PR_OpenDir')( ret, SZ( this.dirName ) );
		this._handler = ret; // NULL if error
	}

	this.Read = function( flags ) { // NSPR_API(PRDirEntry*) PR_ReadDir(PRDir *dir, PRDirFlags flags);

		var ret = new NativeData();
		ret.PP.Alloc();
		_nsprLibrary.Proc('PR_ReadDir')( ret.PP, this._handler, INT( flags || 0 ) );
		if ( ret.PU32[0] == 0 )
			return null;
		return ret.PP[0].String;
	}

	this.Close = function() {

		var ret = INT();
		_nsprLibrary.Proc('PR_CloseDir')( ret, this._handler );
	}

}

Dir.prototype = new Dir_base;
function Dir( dirName ) {

	this.dirName = dirName;
}

Dir.SKIP_NONE    = 0x0;
Dir.SKIP_DOT     = 0x1;
Dir.SKIP_DOT_DOT = 0x2;
Dir.SKIP_BOTH    = 0x3;
Dir.SKIP_HIDDEN  = 0x4;

Dir.List = function( dirName ) {
	
	var list = [];
	var dir = new Dir( dirName );
	dir.Open();
	var entry;
	while ( entry = dir.Read() )
		list.push( entry );
	dir.Close();
	return list;
}