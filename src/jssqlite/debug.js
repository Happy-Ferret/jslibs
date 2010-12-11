LoadModule('jsstd');
LoadModule('jssqlite');

for ( var i = 0; i < 30; i++ ) {

		var db = new Database(); // in-memory database
		db.Exec('create table t1 (name,value);');
		db.Exec('insert into t1 (name,value) values ("red","#F00")');
		db.Exec('insert into t1 (name,value) values ("green","#0F0")');
		db.Exec('insert into t1 (name,value) values ("blue","#00F")');

		var res = [ color.name+'='+color.value for each ( color in db.Query('SELECT * from t1') ) ].join(',');
}

throw 0;

LoadModule('jsstd'); Exec('../common/tools.js');
//var QA = FakeQAApi;
//RunLocalQAFile();
//RunJsircbot(false); throw 0;
RunQATests('-rep 5 -exclude jstask jssqlite');



LoadModule('jsstd');
LoadModule('jssqlite');

var db = new Database(); // in-memory database
db.Exec('create table t1 (name,value);');
db.Exec('insert into t1 (name,value) values ("red","#F00")');


for (var i = 0; i < 11; i++ ) {

	Print(i, '\n');

	var str = '';
	
	var res = db.Query('SELECT * from t1');
	
	str = [ it for each ( it in res ) ];
//	var arr = []; for each ( var it in res ) arr.push(it); 	str += arr; // path: stubs::IterMore

	Print('\n');

}

throw 0;
