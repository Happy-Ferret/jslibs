LoadModule('jsstd');
LoadModule('jssqlite');

function test(db) {

	var result = db.Query('SELECT 1 UNION SELECT 2 UNION ALL SELECT 2');
	Print( 'column count       :'+result.columnCount ,'\n' );
	Print( 'step result        :'+result.Step() ,'\n' );
	Print( 'last insert row ID :'+db.lastInsertRowid ,'\n' );
	Print( 'changes            :'+db.changes ,'\n' );
	Print( 'col names          :'+result.columnNames.toSource() ,'\n' );
	Print( 'col indexes        :'+result.columnIndexes.toSource() ,'\n' );

	result.Reset(); // required because the first Step()

	var r;
	while( (r = result.Row()) )
		Print( 'row          :'+r.toSource() ,'\n' );
}


try {

	Print('database version: ' + Database.version ,'\n' );
	var db = new Database('test_database');
	
	Print( 'value = ' + db.Exec('PRAGMA user_version') ,'\n' );
	
//	Print('Pragma:'+ st.Row() ,'\n');
//	st.Close();

	test(db);
	
//	Print( 'first col only     :'+result.Col(0).toSource() ,'\n' );


//	result.Close();

CollectGarbage();
	db.Close();


} catch ( ex if ex instanceof SqliteError ) { // if ex instanceof SqliteError 
	Print( 'SqliteError: ' + ex.text + '('+ex.code+')', '\n' );
} catch( ex ) {
	throw ex;
}

Print( 'Done.', '\n' );

