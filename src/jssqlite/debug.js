// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

LoadModule('jsstd');
LoadModule('jssqlite');
			
try {

		var db = new Database(); // in-memory database
		db.Exec('create table t1 (name,value);');
		db.Exec('insert into t1 (name,value) values ("red","#F00")');
		db.Exec('insert into t1 (name,value) values ("green","#0F0")');
		db.Exec('insert into t1 (name,value) values ("blue","#00F")');

		var res = [ color.name+'='+color.value for each ( color in db.Query('SELECT * from t1') ) ].join(',');
		QA.ASSERT_STR( res, 'red=#F00,green=#0F0,blue=#00F', 'result' );

} catch ( ex ) {
	
	Print( ex.const )
}		
