({

	TestDb: function(QA) {
		
		LoadModule('jssqlite');

		var db = new Database('test_sqlite_database');

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
		
		new File('test_sqlite_database').Delete();
	},
	
	
	Binding: function(QA) {
	
		LoadModule('jssqlite');
		var db = new Database('test_sqlite_database');

		var result1 = db.Query('SELECT :toto');
		result1.toto = Blob('12' + '\0' + '34');

		QA.ASSERT( result1.Row()[0].length, 5, 'using binding' );
		QA.ASSERT( db.changes, 0, 'changes' );
		result1.Close();
		db.Close();

		new File('test_sqlite_database').Delete();
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

	InMemoryDatabaseCreateTable: function(QA) {
	
		LoadModule('jssqlite');
		try {

			var db = new Database();

			var res = db.Exec('create table a (id integer primary key, x varchar)');
//			res.Close();

			
			db.Exec('insert into a values (NULL, "aaa")');
			db.Close();

		} catch ( ex if ex instanceof SqliteError ) { // if ex instanceof SqliteError 
			
			Print( 'SqliteError: ' + ex.text + '('+ex.code+')', '\n' );
		}
	
	},


	InMemoryDatabase: function(QA) {

		LoadModule('jssqlite');
		var db = new Database();
		var res = db.Exec('SELECT 1');
		QA.ASSERT( res, 1, 'select 1' );
		db.Close();
	},
	

	Exceptions: function(QA) {
	
		LoadModule('jssqlite');
		var db = new Database('test_sqlite_database');


		try {


		} catch ( ex if ex instanceof SqliteError ) { // if ex instanceof SqliteError 
			
//			QA.ASSERT( 'SqliteError: ' + ex.text + '('+ex.code+')', '\n' );
		}


//		QA.ASSERT( res, 1230, 'row result' );

	
		db.Close();
		new File('test_sqlite_database').Delete();
	}
	
	

})
