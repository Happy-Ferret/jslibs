LoadModule('jsstd');
LoadModule('jssqlite');

try {

	var code = "var db = new Database('');	db.testFun = function() { return 123 };	var res = db.Exec('SELECT testFun()'); Print(res)"
	var func = new Function(code);
	func();

} catch ( ex if ex instanceof SqliteError ) { // if ex instanceof SqliteError 
	
	Print( 'error ',ex.text,' ',ex.code,'\n' )
}



Halt(); // ============================================================================================

Print( 'version: ', Database.version, '\n' );

try {


	var db = new Database('test.db');

	db.Exec('PRAGMA page_size = 8192');
//	db.Exec('PRAGMA journal_mode = PERSIST');
//	db.Exec('PRAGMA locking_mode = EXCLUSIVE');
	db.Exec('PRAGMA synchronous = OFF');
//	db.Exec('PRAGMA fullfsync = 0');
	db.Exec('PRAGMA cache_size = 100000');
	
	var res = db.Exec('create table IF NOT EXISTS a (b integer primary key, c varchar, d integer)');
	

	db.Exec('BEGIN');
	for ( var i = 0; i < 40000; i++ ) {
	
		var stmt = db.Exec('insert into a values (NULL, "aaa", '+ i +')');
	}
	db.Exec('COMMIT');
	 
Print('count: ' + db.Exec('select count(*) from a') )


//	var res = db.Query("select @t, @x from a",{t:"b",x:"a"});


	Print( res )

} catch ( ex if ex instanceof SqliteError ) { // if ex instanceof SqliteError 
	
	Print( 'error ',ex.text,' ',ex.code,'\n' )

}

Print( 'Done.', '\n' );
