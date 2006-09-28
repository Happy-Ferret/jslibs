LoadModule('jsstd');
LoadModule('jssqlite');

//try {

	print('database version: ' + Database.version ,'\n' );

	var db = new Database('test_database');
	var result = db.Query(arguments[1]);
	print( 'column count       :'+result.columnCount ,'\n' );
	

//	print( 'step result        :'+result.Step() ,'\n' );
	print( 'last insert row ID :'+db.lastInsertRowid ,'\n' );
	print( 'changes            :'+db.changes ,'\n' );
	print( 'col names          :'+result.columnNames.toSource() ,'\n' );
	print( 'col indexes        :'+result.columnIndexes.toSource() ,'\n' );
	

	var r;
//	while( r = result.Row() )
//		print( 'row          :'+r.toSource() ,'\n' );
	
//	print( 'first col only     :'+result.Col(0).toSource() ,'\n' );



	result.Close();
	db.Close();


//} catch ( ex if ex instanceof SqliteError ) {
//	print( 'SqliteError: ' + ex.text + '('+ex.code+')' );
//}