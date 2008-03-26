({

	InMemoryDatabase: function(QA) {

		LoadModule('jssqlite');
		var db = new Database();
		var res = db.Exec('SELECT 1');
		QA.ASSERT( res, 1, 'select 1' );
		db.Close();
	},


	InMemoryDatabaseCreateTable: function(QA) {
	
		LoadModule('jssqlite');
		try {

			var db = new Database();
			var res = db.Exec('create table a (id integer primary key, x varchar)');
			db.Exec('insert into a values (NULL, "aaa")');
			var res = db.Query('select * from a');
			var row = res.Row(true);
			QA.ASSERT( row.x, 'aaa', 'read query result' );
			db.Close();

		} catch ( ex if ex instanceof SqliteError ) { // if ex instanceof SqliteError 
			
			QA.REPORT( 'SqliteError: ' + ex.text + '('+ex.code+')' );
		}
	},


	ResultFormat: function(QA) {
	
		LoadModule('jssqlite');

		var db = new Database();
		var res = db.Exec('create table a (b integer primary key, c varchar, d integer)');
		db.Exec('insert into a values (NULL, "aaa", 222)');
		db.Exec('insert into a values (NULL, "b", 333)');
		QA.ASSERT( db.lastInsertRowid, 2, 'last insert rowid' );
		QA.ASSERT( db.changes, 1, 'changes count' );

		var res = db.Query('select * from a');
		
		QA.ASSERT( res.columnCount, 3, 'column count' );
		QA.ASSERT( res.expired, false, 'statement expired' );
		
		QA.ASSERT( res.columnNames.join(','), 'b,c,d', 'columns name' ); 
		QA.ASSERT( res.columnIndexes.b, 0, 'column name index' ); 
		QA.ASSERT( res.columnIndexes.c, 1, 'column name index' ); 
		QA.ASSERT( res.columnIndexes.d, 2, 'column name index' ); 
		
		var row = res.Row(true);
		QA.ASSERT( row.c, 'aaa', 'column by name' ); 

		res.Reset();
		
		var row = res.Row(false);
		QA.ASSERT( row[1], 'aaa', 'column by index' ); 

		var row = res.Row(false);
		QA.ASSERT( row[1], 'b', 'column by index' ); 
		QA.ASSERT( row[2], 333, 'column by index' ); 

		QA.ASSERT( res.Col(0), 2, 'column by index using Col' ); 
		QA.ASSERT( res.Col(1), 'b', 'column by index using Col' ); 
		QA.ASSERT( res.Col(2), 333, 'column by index using Col' ); 

		db.Close();

	},



	Exceptions: function(QA) {
	
		LoadModule('jssqlite');
		var db = new Database();

		try {

			var res = db.Query('select * from a');
			QA.REPORT( 'Failed to throw exception' );

		} catch ( ex if ex instanceof SqliteError ) { // if ex instanceof SqliteError 
			
			QA.ASSERT( ex.code, 1, 'exception code' );
			QA.ASSERT( ex.text, 'no such table: a', 'exception text' );
		}
		db.Close();
	},
	

	QueryNoData: function(QA) {
		
		LoadModule('jssqlite');

		var db = new Database();

		var result = db.Query('SELECT 1 UNION SELECT 2 UNION ALL SELECT 2');

		QA.ASSERT( result.columnCount, 1, 'column count' );
		QA.ASSERT( result.Step(), true, 'has a next line' );
		QA.ASSERT( db.lastInsertRowid, 0, 'last insert row id' );
		QA.ASSERT( db.changes, 0, 'changes' );
		QA.ASSERT( result.columnNames[0], '1', 'first column name' );

		QA.ASSERT( result.columnIndexes[1], 0, 'col index' );

		result.Reset(); // required because the first Step()

		QA.ASSERT( result.Row()[0], 1, 'row 1' );
		QA.ASSERT( result.Row()[0], 2, 'row 2' );
		QA.ASSERT( result.Row()[0], 2, 'row 3' );
	},


	DatabaseFile: function(QA) {
	
		LoadModule('jssqlite');
		var db = new Database('test_sqlite_database');
		db.Close();

		var file = new File('test_sqlite_database');
		QA.ASSERT( file.exist, true, 'database file exist' );
		file.Delete();
		QA.ASSERT( file.exist, false, 'database file exist' );
	},


	Binding: function(QA) {
	
		LoadModule('jssqlite');
		var db = new Database();

		var result1 = db.Query('SELECT :toto');
		result1.toto = Blob('12' + '\0' + '34');

		QA.ASSERT( result1.Row()[0].length, 5, 'using binding' );
		QA.ASSERT( db.changes, 0, 'changes' );
		result1.Close();
		db.Close();
	},


	Version: function(QA) {

		LoadModule('jssqlite');
		QA.ASSERT( Database.version[1], '.', 'version string' );
		QA.ASSERT( Database.version[3], '.', 'version string' );
	},


	NamedVariables: function(QA) {

		LoadModule('jssqlite');
		var db = new Database('test_sqlite_database');
		var res = db.Exec('SELECT @varTest', { varTest:123} );
		QA.ASSERT( res, 123, 'row result' );
		db.Close();
		new File('test_sqlite_database').Delete();
	},


	QuestionMark: function(QA) {

		LoadModule('jssqlite');
	
		var db = new Database('test_sqlite_database');
		var row = db.Query('SELECT ?+?+?', {0:2,1:2,2:2,3:2,length:3}).Row();
	
		QA.ASSERT( Number(row), 6, 'row result' );

		db.Close();
		new File('test_sqlite_database').Delete();
	},
	
	
	FunctionBinding: function(QA) {
	
		LoadModule('jssqlite');
	
		var db = new Database('');
	
		db.testFun = function(a) { return a*10 }
		db.jseval = function(s){ return eval(s) };
		
		var res = db.Exec('SELECT testFun(123), length(:toto), jseval("null") is null', {toto:Blob('qqwe\00\00fv1234'), aaa:null});

		QA.ASSERT( res, 1230, 'row result' );

		db.Close();
	},


	
	

})
