exec('deflib.js');
LoadModule('jssqlite');


print('database version: '+Database.version,'\n' );

var db = new Database('test_database');
var result = db.Query(arguments[1]);

print( 'last insert row ID :'+db.lastInsertRowid,'\n' );
print( 'col names :'+result.columnNames.toSource(),'\n' );

print( result.Row().toSource() );
result.Close();
db.Close();
