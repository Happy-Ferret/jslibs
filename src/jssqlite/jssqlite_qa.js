loadModule('jssqlite');

/// crash [p]

	try {
		
		new Database().exec('123');
		
	} catch(ex) {

		var s = new Serializer();
		var u = new Unserializer( s.write(ex).done() );
		u.read();
	}


/// crash / deadlock [p]

	loadModule('jstask');
	var myTaskFct = function() {

		var loadModule = host.loadModule;
		loadModule('jssqlite');
		new Database().exec('123');
	}
	var myTask = new Task(myTaskFct);
	myTask.request();
	myTask.response();


/// sqlite version [p]

	var db = new Database();
	var r = db.exec('select sqlite_version()');
	QA.ASSERTOP( r.length, '>=', 5, 'sqlite version length' );


/// for each iteration over a Query result [p]

		var db = new Database(); // in-memory database
		db.exec('create table t1 (name,value);');
		db.exec('insert into t1 (name,value) values ("red","#F00")');
		db.exec('insert into t1 (name,value) values ("green","#0F0")');
		db.exec('insert into t1 (name,value) values ("blue","#00F")');

		var res = [ color.name+'='+color.value for each ( color in db.query('SELECT * from t1') ) ].join(',');
		QA.ASSERT_STR( res, 'red=#F00,green=#0F0,blue=#00F', 'result' );
		

/// InMemory Database [p]

		var db = new Database();
		var res = db.exec('SELECT 1');
		QA.ASSERT( res, 1, 'select 1' );
		db.close();


/// InMemory Database Create table [p]
		
		try {

			var db = new Database();
			var res = db.exec('create table a (id integer primary key, x varchar)');
			db.exec('insert into a values (NULL, "aaa")');
			var res = db.query('select * from a');
			var row = res.row(true);
			QA.ASSERT( row.x, 'aaa', 'read query result' );
			db.close();

		} catch ( ex if ex instanceof SqliteError ) { // if ex instanceof SqliteError 
			
			QA.FAILED( 'SqliteError: ' + ex.text + '('+ex.code+')' );
		}


/// Result format [p]

		var db = new Database();
		var res = db.exec('create table a (b integer primary key, c varchar, d integer)');
		db.exec('insert into a values (NULL, "aaa", 222)');
		db.exec('insert into a values (NULL, "b", 333)');
		QA.ASSERT( db.lastInsertRowid, 2, 'last insert rowid' );
		QA.ASSERT( db.changes, 1, 'changes count' );

		var res = db.query('select * from a');
		
		QA.ASSERT( res.columnCount, 3, 'column count' );
//		QA.ASSERT( res.expired, false, 'statement expired' ); // deprecated property has been removed
		
		QA.ASSERT( res.columnNames.join(','), 'b,c,d', 'columns name' ); 
		QA.ASSERT( res.columnIndexes.b, 0, 'column name index' ); 
		QA.ASSERT( res.columnIndexes.c, 1, 'column name index' ); 
		QA.ASSERT( res.columnIndexes.d, 2, 'column name index' ); 
		
		var row = res.row(true);
		QA.ASSERT( row.c, 'aaa', 'column by name' ); 

		res.reset();
		
		var row = res.row(false);
		QA.ASSERT( row[1], 'aaa', 'column by index' ); 

		var row = res.row(false);
		QA.ASSERT( row[1], 'b', 'column by index' ); 
		QA.ASSERT( row[2], 333, 'column by index' ); 

		QA.ASSERT( res.col(0), 2, 'column by index using col' ); 
		QA.ASSERT( res.col(1), 'b', 'column by index using col' ); 
		QA.ASSERT( res.col(2), 333, 'column by index using col' ); 

		db.close();


/// exceptions [p]

		var db = new Database();

		try {

			var res = db.query('select * from a');
			QA.FAILED( 'Failed to throw exception' );

		} catch ( ex if ex instanceof SqliteError ) { // if ex instanceof SqliteError 
			
			QA.ASSERT( ex.code, 1, 'exception code' );
			QA.ASSERT( ex.text, 'no such table: a', 'exception text' );
		}
		db.close();


/// Query no data [p]

		var db = new Database();

		var result = db.query('SELECT 1 UNION SELECT 2 UNION ALL SELECT 2');

		QA.ASSERT( result.columnCount, 1, 'column count' );
		QA.ASSERT( result.step(), true, 'has a next line' );
		QA.ASSERT( db.lastInsertRowid, 0, 'last insert row id' );
		QA.ASSERT( db.changes, 0, 'changes' );
		QA.ASSERT( result.columnNames[0], '1', 'first column name' );

		QA.ASSERT( result.columnIndexes[1], 0, 'col index' );

		result.reset(); // required because the first Step()

		QA.ASSERT( result.row()[0], 1, 'row 1' );
		QA.ASSERT( result.row()[0], 2, 'row 2' );
		QA.ASSERT( result.row()[0], 2, 'row 3' );


/// Database file [p]

		var db = new Database('test_sqlite_database');
		db.close();

		var file = new File('test_sqlite_database');
		QA.ASSERT( file.exist, true, 'database file exist' );
		file.delete();
		QA.ASSERT( file.exist, false, 'database file exist' );


/// Bindings [p]

		var db = new Database();

		var result1 = db.query('SELECT :toto');
		
		result1.toto = join(['12', '\0', '34'], true);

		QA.ASSERT( result1.row()[0].byteLength, 5, 'using binding' );
		QA.ASSERT( db.changes, 0, 'changes' );
		result1.close();
		db.close();


/// version [p]

		QA.ASSERT( Database.version[1], '.', 'version string' );
		QA.ASSERT( Database.version[3], '.', 'version string' );


/// named variables [p]

		var db = new Database();
		var res = db.exec('SELECT @varTest', { varTest:123} );
		QA.ASSERT( res, 123, 'row result' );
		db.close();


/// question mark [p]

		var db = new Database();
	
		var res = db.exec( 'select ?+?+?', [2, 3, 4] );
		QA.ASSERT( res, 9, 'addition using question mark' );

		var row = db.query('SELECT ?+?+?', {0:2,1:2,2:2,3:2,length:3}).row();
		QA.ASSERT( Number(row), 6, 'addition using question mark' );

		db.close();


/// Function binding [p]

		var db = new Database('');
	
		db.testFun = function(a) { return a*10 }
		db.jseval = function(s){ return eval(s) };
		
		var blob = join(['qqwe\0\0fv1234'], true);
		
		//var res = db.exec('SELECT testFun(123), length(:toto), jseval("null") is null', {toto:blob, aaa:null});
		var res = db.exec('SELECT testFun(123), length(:toto), jseval("null") is null', {toto:blob, aaa:null});

		QA.ASSERT( res, 1230, 'row result' );

		db.close();


/// columnNames property independency [p]

		var db = new Database();
		var res1 = db.query('select 1');
		res1.columnNames[0];
		var res2 = db.query('select 2');
		QA.ASSERT( res2.columnNames.join(','), '2', 'columns name' );


/// parameters [p]

		var db = new Database();
		var res = db.query('select "test", ?, ?aaa, ?, @0', [5,6,7]);
		QA.ASSERT_STR( res.columnNames.join(','), '"test",?,aaa,?,@0', 'columns' );
		QA.ASSERT_STR( res.row(), 'test,5,6,7,5', 'row' );
