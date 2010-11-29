LoadModule('jssqlite');

/// for each iteration over a Query result [ftrm]

		var db = new Database(); // in-memory database
		db.Exec('create table t1 (name,value);');
		db.Exec('insert into t1 (name,value) values ("red","#F00")');
		db.Exec('insert into t1 (name,value) values ("green","#0F0")');
		db.Exec('insert into t1 (name,value) values ("blue","#00F")');

		var res = [ color.name+'='+color.value for each ( color in db.Query('SELECT * from t1') ) ].join(',');
		QA.ASSERT_STR( res, 'red=#F00,green=#0F0,blue=#00F', 'result' );
		

/// InMemory Database [ftrm]

		var db = new Database();
		var res = db.Exec('SELECT 1');
		QA.ASSERT( res, 1, 'select 1' );
		db.Close();


/// InMemory Database Create table [ftrm]
		
		try {

			var db = new Database();
			var res = db.Exec('create table a (id integer primary key, x varchar)');
			db.Exec('insert into a values (NULL, "aaa")');
			var res = db.Query('select * from a');
			var row = res.Row(true);
			QA.ASSERT( row.x, 'aaa', 'read query result' );
			db.Close();

		} catch ( ex if ex instanceof SqliteError ) { // if ex instanceof SqliteError 
			
			QA.FAILED( 'SqliteError: ' + ex.text + '('+ex.code+')' );
		}


/// Result format [ftrm]

		var db = new Database();
		var res = db.Exec('create table a (b integer primary key, c varchar, d integer)');
		db.Exec('insert into a values (NULL, "aaa", 222)');
		db.Exec('insert into a values (NULL, "b", 333)');
		QA.ASSERT( db.lastInsertRowid, 2, 'last insert rowid' );
		QA.ASSERT( db.changes, 1, 'changes count' );

		var res = db.Query('select * from a');
		
		QA.ASSERT( res.columnCount, 3, 'column count' );
//		QA.ASSERT( res.expired, false, 'statement expired' ); // deprecated property has been removed
		
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


/// exceptions [ftrm]

		var db = new Database();

		try {

			var res = db.Query('select * from a');
			QA.FAILED( 'Failed to throw exception' );

		} catch ( ex if ex instanceof SqliteError ) { // if ex instanceof SqliteError 
			
			QA.ASSERT( ex.code, 1, 'exception code' );
			QA.ASSERT( ex.text, 'no such table: a', 'exception text' );
		}
		db.Close();


/// Query no data [ftrm]

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


/// Database file [ftrm]

		var db = new Database('test_sqlite_database');
		db.Close();

		var file = new File('test_sqlite_database');
		QA.ASSERT( file.exist, true, 'database file exist' );
		file.Delete();
		QA.ASSERT( file.exist, false, 'database file exist' );


/// Bindings [ftrm]

		var db = new Database();

		var result1 = db.Query('SELECT :toto');
		
		result1.toto = new Blob('12' + '\0' + '34');

		QA.ASSERT( result1.Row()[0].length, 5, 'using binding' );
		QA.ASSERT( db.changes, 0, 'changes' );
		result1.Close();
		db.Close();


/// version [ftrm]

		QA.ASSERT( Database.version[1], '.', 'version string' );
		QA.ASSERT( Database.version[3], '.', 'version string' );


/// named variables [ftrm]

		var db = new Database();
		var res = db.Exec('SELECT @varTest', { varTest:123} );
		QA.ASSERT( res, 123, 'row result' );
		db.Close();


/// question mark [ftrm]

		var db = new Database();
	
		var res = db.Exec( 'select ?+?+?', [2, 3, 4] );
		QA.ASSERT( res, 9, 'addition using question mark' );

		var row = db.Query('SELECT ?+?+?', {0:2,1:2,2:2,3:2,length:3}).Row();
		QA.ASSERT( Number(row), 6, 'addition using question mark' );

		db.Close();


/// Function binding [ftrm]

		var db = new Database('');
	
		db.testFun = function(a) { return a*10 }
		db.jseval = function(s){ return eval(s) };
		
		var blob = new Blob('qqwe\0\0fv1234');
		
		//var res = db.Exec('SELECT testFun(123), length(:toto), jseval("null") is null', {toto:blob, aaa:null});
		var res = db.Exec('SELECT testFun(123), length(:toto), jseval("null") is null', {toto:blob, aaa:null});

		QA.ASSERT( res, 1230, 'row result' );

		db.Close();


/// columnNames property independency [rmtf]

		var db = new Database();
		var res1 = db.Query('select 1');
		res1.columnNames[0];
		var res2 = db.Query('select 2');
		QA.ASSERT( res2.columnNames.join(','), '2', 'columns name' );


/// parameters [rmtf]

		var db = new Database();
		var res = db.Query('select "test", ?, ?aaa, ?, @0', [5,6,7]);
		QA.ASSERT_STR( res.columnNames.join(','), '"test",?,aaa,?,@0', 'columns' );
		QA.ASSERT_STR( res.Row(), 'test,5,6,7,5', 'row' );
