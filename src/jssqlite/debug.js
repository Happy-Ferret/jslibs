// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

LoadModule('jsstd');
LoadModule('jssqlite');

   var db = new Database();
   db.Exec('create table t1 (name,id,c);');

   db.Exec('insert into t1 (name, id,c) values ("Fred",6,7)');
   db.Exec('insert into t1 (name, id,c) values ("Bart",3,4)');
   db.Exec('insert into t1 (name, id,c) values ("Sally","b","c")');

   var result1 = db.Query('Select ?, c From t1 Where id=?', ['name', 3] );
   
   
	Print('before Step: \n');
  Print( 'columnNames.toSource(): ' + result1.columnNames.toSource(), '\n' );   

   Print( 'Col(0): ' + result1.Col(0) + '\n');
   Print( 'Row().toSource(): ' + result1.Row().toSource(), '\n\n' );

   Print('after Step: \n');
   result1.Step();
   Print( 'columnNames.toSource(): ' + result1.columnNames.toSource(), '\n' );
   Print( 'Col(0): ' + result1.Col(0) + '\n');



Halt(); //////////////////////////////////////////////////////////////////



var db = new Database(); // in-memory database
db.Exec('create table t1 (name,value);');
db.Exec('insert into t1 (name,value) values ("red","#F00")');
db.Exec('insert into t1 (name,value) values ("green","#0F0")');
db.Exec('insert into t1 (name,value) values ("blue","#00F")');

Print( [ color.name for each ( color in db.Query('SELECT * from t1') ) ] );



Halt(); //////////////////////////////////////////////////////////////////

/*
try {

var db = new Database('');
db.testFun = function() { return 123 };
var res = db.Exec('SELECT testFun()');
Print(res);


} catch ( ex if ex instanceof SqliteError ) { // if ex instanceof SqliteError 
	
	Print( 'error ',ex.text,' ',ex.code,'\n' )
}


Halt();
*/

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
