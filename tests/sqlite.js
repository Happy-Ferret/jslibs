exec('deflib.js');
LoadModule('jssqlite');

try {

	print('database version: ' + Database.version ,'\n' );

	var db = new Database('test_database');
	var result = db.Query(arguments[1]);

	print( 'last insert row ID :'+db.lastInsertRowid ,'\n' );
	print( 'col names          :'+result.columnNames.toSource() ,'\n' );
	print( 'col indexes        :'+result.columnIndexes.toSource() ,'\n' );
	print( 'first row          :'+result.Row().toSource() ,'\n' );
	print( 'first col only     :'+result.Col(0).toSource() ,'\n' );

	result.Close();
	db.Close();

} catch ( ex if ex instanceof SqliteError ) {
	print( 'SqliteError: ' + ex.text + '('+ex.code+')' );


}