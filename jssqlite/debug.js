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


function testBinding() {

	var toto = 1234;
	var result1 = db.Query('SELECT @toto');
	// result1.toto = '12' + '\0' + '34';
	result1.toto = Blob('12' + '\0' + '34');

	
	Print( 'using binding: '+result1.Row()[0].length, '\n' );
	
	Print('changes: ' + db.changes, '\n');
	result1.Close();
}


try {

	Print('database version: ' + Database.version ,'\n' );
	var db = new Database('test_database');
	
	db.testFun = function(a) { return a*10 }
	db.jseval = function(s){ return eval(s) };
	
	Print( 'testFunc = ' + db.Exec('SELECT testFun(123), length(:toto), jseval("null") is null', {toto:Blob('qqwe\00\00fv1234'), aaa:null}) ,'\n' );	
	
	

//	var varTest = <toto><i>ti</i></toto>;
//	Print( 'Exec test = ' + db.Exec('SELECT :varTest') ,'\n' );
//	test(db);
	testBinding(db)
	
//	Print('Pragma:'+ st.Row() ,'\n');
//	st.Close();
//	Print( 'first col only     :'+result.Col(0).toSource() ,'\n' );
//	result.Close();

//CollectGarbage();

	db.Close();

} catch ( ex if ex instanceof SqliteError ) { // if ex instanceof SqliteError 
	Print( 'SqliteError: ' + ex.text + '('+ex.code+')', '\n' );
} catch( ex ) {
	throw ex;
}

Print( 'Done.', '\n' );
