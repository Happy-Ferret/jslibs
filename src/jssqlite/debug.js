var loadModule = host.loadModule;
loadModule('jsstd');
//loadModule('jssqlite');

//loadModule('jsstd'); exec('../common/tools.js'); runQATests('jssqlite');


	loadModule('jstask');
	host.loadModule('jssqlite');
	var myTask = new Task(function() {
		new (host.loadModule('jssqlite').Database)().exec('123')
	});
	myTask.request();
	myTask.response();
	
	

throw 0;



	loadModule('jstask');
	var myTaskFct = function() {

		var loadModule = host.loadModule;
		loadModule('jssqlite');
		new Database().exec('123');
	}
	var myTask = new Task(myTaskFct);
	myTask.request();
	myTask.response();

throw 0;


/*
loadModule('jsstd');
loadModule('jssqlite');

for ( var i = 0; i < 30; i++ ) {

		var db = new Database(); // in-memory database
		db.exec('create table t1 (name,value);');
		db.exec('insert into t1 (name,value) values ("red","#F00")');
		db.exec('insert into t1 (name,value) values ("green","#0F0")');
		db.exec('insert into t1 (name,value) values ("blue","#00F")');

		var res = [ color.name+'='+color.value for each ( color in db.query('SELECT * from t1') ) ].join(',');
}

throw 0;
*/

//loadModule('jsstd'); exec('../common/tools.js');
//var QA = FakeQAApi;
//runLocalQAFile();
//runJsircbot(false); throw 0;
//runQATests('-rep 5 -exclude jstask jssqlite');



loadModule('jsstd');
loadModule('jssqlite');

var db = new Database(); // in-memory database
db.exec('create table t1 (name,value);');
db.exec('insert into t1 (name,value) values ("one","1")');
db.exec('insert into t1 (name,value) values ("two","2")');

var res = db.query('SELECT * from t1');

for ( var e of res ) {

	print( e.name, '\n' );
}

halt();


for (var i = 0; i < 11; i++ ) {

	print(i, '\n');

	var str = '';
	
	
//	str = [ it for each ( it in res ) ];
//	var arr = []; for each ( var it in res ) arr.push(it); 	str += arr; // path: stubs::IterMore

	print('\n');

}

throw 0;
