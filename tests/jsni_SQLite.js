//http://www.sqlite.org/capi3ref.html#sqlite3_finalize

// SQLITE_OK           0   /* Successful result */
// SQLITE_ERROR        1   /* SQL error or missing database */
// SQLITE_INTERNAL     2   /* An internal logic error in SQLite */
// SQLITE_PERM         3   /* Access permission denied */
// SQLITE_ABORT        4   /* Callback routine requested an abort */
// SQLITE_BUSY         5   /* The database file is locked */
// SQLITE_LOCKED       6   /* A table in the database is locked */
// SQLITE_NOMEM        7   /* A malloc() failed */
// SQLITE_READONLY     8   /* Attempt to write a readonly database */
// SQLITE_INTERRUPT    9   /* Operation terminated by sqlite_interrupt() */
// SQLITE_IOERR       10   /* Some kind of disk I/O error occurred */
// SQLITE_CORRUPT     11   /* The database disk image is malformed */
// SQLITE_NOTFOUND    12   /* (Internal Only) Table or record not found */
// SQLITE_FULL        13   /* Insertion failed because database is full */
// SQLITE_CANTOPEN    14   /* Unable to open the database file */
// SQLITE_PROTOCOL    15   /* Database lock protocol error */
// SQLITE_EMPTY       16   /* (Internal Only) Database table is empty */
// SQLITE_SCHEMA      17   /* The database schema changed */
// SQLITE_TOOBIG      18   /* Too much data for one row of a table */
// SQLITE_CONSTRAINT  19   /* Abort due to constraint violation */
// SQLITE_MISMATCH    20   /* Data type mismatch */
// SQLITE_MISUSE      21   /* Library used incorrectly */
// SQLITE_NOLFS       22   /* Uses OS features not supported on host */
// SQLITE_AUTH        23   /* Authorization denied */
// SQLITE_ROW         100  /* sqlite_step() has another row ready */
// SQLITE_DONE        101  /* sqlite_step() has finished executing */



_sqlite3Library = new NativeModule( 'P:\\dev\\sqlitedll-3_2_3\\sqlite3', true );


var str = new NativeData();
str.PP.Alloc();
_sqlite3Library.Proc('sqlite3_libversion')( str.PP );
if ( str.String < '3.2.3' )
	Error( 'bad SQLite version ( 3.2.3+ is needed )' );


function SQLite( dbname ) {

	var _db = new NativeData();
	_db.PP.Alloc();

	var status = new NativeData().PI.Alloc();
	_sqlite3Library.Proc('sqlite3_open')( status, SZ( dbname ), PTRTO( _db ) ); 


	this.Close = function() {

		var status = new NativeData().PI.Alloc();
		_sqlite3Library.Proc('sqlite3_close')( status, _db.PP );
		return status[0] == 0;
	}

	this.Query = function( sql ) {
	
		var ppstmt = new NativeData().PP.Alloc();
		ppstmt[0].PP.Alloc();

		var status = new NativeData().PI.Alloc();
		_sqlite3Library.Proc('sqlite3_prepare')( status, _db.PP, SZ( sql ), INT( sql.length ), ppstmt, PPTR() );
		return new SQLiteResult( _sqlite3Library, ppstmt[0].PP );
	}
}


function SQLiteResult( _sqlite3Library, _pstmt ) {

	var ret = new NativeData().PI.Alloc();
	_sqlite3Library.Proc('sqlite3_column_count')( ret, _pstmt );
	
	this.ColList = []; // cache columns name
	this.ColCount = ret[0];

	for ( var i=0; i<this.ColCount; i++ ) {

		var name = new NativeData();
		name.PP.Alloc();
		_sqlite3Library.Proc('sqlite3_column_name')( name.PP, _pstmt, INT( i ) );
		this.ColList[i] = name.String;
	}


	this.Finalize = function() {
	
		_sqlite3Library.Proc('sqlite3_finalize')( _pstmt );
	}

	this.Row = function( array ) {

		var status = new NativeData().PI.Alloc();
		_sqlite3Library.Proc('sqlite3_step')( status, _pstmt );
		var st = status[0];

		if ( st != 100 ) // SQLITE_ROW
			return null;
		
		var ret = new NativeData().PI.Alloc();
		_sqlite3Library.Proc('sqlite3_column_count')( ret, _pstmt );

		var row = array ? [] : {};

		for ( i in this.ColList ) {

			var text = new NativeData();
			text.PP.Alloc();
			_sqlite3Library.Proc('sqlite3_column_text')( text.PP, _pstmt, INT( i ) );
			row[ array ? i : this.ColList[i] ] = text.String;
		}

		return row;
	}

}

