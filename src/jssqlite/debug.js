var loadModule = host.loadModule;
//loadModule('jsstd'); exec('../common/tools.js'); runQATests('jssqlite'); throw 0;

host.interruptInterval = 1;
host.onInterrupt = () => { host.collectGarbage(true, 1) };

loadModule('jsstd');
loadModule('jssqlite');

	var db = new Database();
	db.myTest = function(val) {

		return val;
	}

	db.myErr = function() {

		testsa5df46sa5df4c6as5d4f();
	}

	try {
		print( db.exec('select myErr()'), '\n' );
	} catch(ex) {}

	print( db.exec('select myTest(1.5)'), '\n' );
	print( db.exec('select myTest("myString")'), '\n' );
	
	try {

		db.exec('select myTest()');
	} catch(ex) {
		
		print( 'EXCEPTION: ', ex, '\n' );
	}

	print( db.exec('select ?+?', [1,2]), '\n' );
	print( db.exec('select @a+@b', {a:1, b:2}), '\n' );
	print( db.exec('select @a+@b', []), '\n' );
	

	db.a = 1;
	db.b = 2;
	print( db.exec('select :a+:b'), '\n' );

	var res = db.query('select :a+:b');
	res.close();

	db.close();
	


throw 0;



try {


	var db = new Database('test.db', Database.READWRITE | Database.CREATE | Database.SHAREDCACHE);
	db.mcallback = function(val) {

		print(val.quote());
		return 'test';
	}
	db.exec('create table IF NOT EXISTS t1 (value)');
	db.exec('create temp trigger callbacktrigger after insert on t1 for each row begin select mcallback(new.value); end;');
	db.exec('insert into t1 (value) values ("\u0100")');


//	var db2 = new Database('test.db', Database.READWRITE | Database.CREATE | Database.SHAREDCACHE);

	var db2 = new Database();
	db2.exec('attach "test.db" as test');
	db2.exec('insert into test.t1 (value) values ("2")');


	} catch(e) {

		print(e);
	}
throw 0;


	var db = new Database();
	db.exec('create table IF NOT EXISTS t1 (value)');
	db.exec('insert into t1 (value) values ("\u0100")');
throw 0;


	var db = new Database();
	db.mcallback = function(val) {

		print(val, '\n');
	}
	db.exec('select mcallback(123)');
throw 0;



	var e;
	try {
		
		new Database().exec('123');
		
	} catch(e) {

		ex = e;
	}

	var s = new Serializer();
	var u = new Unserializer( s.write(ex).done() );
	u.read();

throw 0;

	var db = new Database(); // in-memory database
	db.exec('create table t1 (value);');
	db.exec('insert into t1 (value) values ("red")');
	db.exec('insert into t1 (value) values ("green")');
	db.exec('insert into t1 (value) values ("blue")');


	var res = db.query('SELECT * from t1');

	var tmp = 'tmp: ';

	for ( var item of res )
		tmp += item.value;

	print( tmp, '\n');




throw 0;


	//try {

	var db = new Database(); // in-memory database

	db.exec('create table t1 (data);');
	db.exec('insert into t1 (data) values (zeroblob(1000))');

	var stream = db.openBlobStream('t1', 'data', 1);

	print( stream.available, '\n' );

	stream.write('xxx');
	stream.position = 0;
	print( stream.read() );


	//var res = db.query('SELECT data from t1');

	//print(res.row());

	//} catch(ex) { print( ex.const, '\n' ); }

throw 0;


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
