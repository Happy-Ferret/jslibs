// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();


LoadModule('jsstd');
LoadModule('jssqlite');
			

var foo = 567;
function test() {

		var db = new Database();
		var res = db.Query('select $dfgyisdfugyidfugyid, "test", $foo, ?, ?aaa, $foo, ?, @0', [5,6,7]);
		Print(res.columnNames.join(',')+'='+res.Row(), '\n'); // "test",$foo,?,aaa,$foo,?,@0=test,567,5,6,567,7,5
}
test();


throw 0;

var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };

		var db = new Database();
		var res1 = db.Query('select 1');
		res1.columnNames[0];
		var res2 = db.Query('select 2');
		QA.ASSERT( res2.columnNames.join(','), '2', 'columns name' ); 


throw 0;
		

// run qa tests
	LoadModule('jsstd');
	LoadModule('jsio');
	currentDirectory += '/../..';
	arguments = Array.concat('qa.js', '-gcZeal 2 -rep 1 jssqlite'.split(' '));
	Exec(arguments[0], false);
	throw 0;
	
	
	

LoadModule('jsstd');
LoadModule('jssqlite');

var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };

try {



} catch(ex) {
	
	Print(ex.toSource());
}

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
