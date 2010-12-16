LoadModule('jsstd'); 
LoadModule('jsio'); 

//Print( Stringify(new Uint16Array([100,100,100])) );
//	var b = new Blob('abc');
//	Stringify({ __proto__:b});
//throw 0;

LoadModule('jsstd'); Exec('../common/tools.js');
//var QA = FakeQAApi;
//RunLocalQAFile();
//RunJsircbot(false); throw 0;
RunQATests('-rep 1 -gcZeal 0 -nogcB -nogcD -exclude jstask|blob'); //  Serialization|ProcessEvents
//  -loopForever -flags f -nogcB -nogcD 
//Exec('../../perfset.js'); throw 0; // -perf perfset.js 


function makeLogger(obj) {

	var proxy = Proxy.create({
		get: function(rcvf, name) {
			Print('get '+' '+name+' '+''+'\n');
			return obj[name];
		},
		set: function(rcvf, name, val) {
		
		rcvf.toto
			Print('set '+name+' '+val+' '+''+'\n');
			obj[name] = val;
			return true;
		},
		delete: function(name) {
			Print('del '+name+'\n');
			delete obj[name];
			return false;
		},
		invoke: function(receiver, name, args) {
			Print('call '+name+'\n');
			obj[name].apply(obj, args);
		},
		
	}, Object.getPrototypeOf(obj));
	return proxy;
}

var o = makeLogger({});


o.a = 5;
/*
o.a = 6;
o.b = 6;
Print( delete o.b );
Print( o.b, '\n' );
*/
o.x(0);

throw 0;






LoadModule('jsstd');
LoadModule('jssqlite');

var db = new Database(); // in-memory database
db.Exec('create table t1 (name,value);');
db.Exec('insert into t1 (name,value) values ("red","#F00")');

for (var i = 0; i < 10; i++ ) {

Print(i, '\n');


		var res = [ color.name+'='+color.value for each ( color in db.Query('SELECT * from t1') ) ].join(',');
		
		
//		QA.ASSERT_STR( res, 'red=#F00,green=#0F0,blue=#00F', 'result' );
}

throw 0;













var s = new Blob('ABC');
s['test'] = 's';
s[1] = 's';




LoadModule('jsstd');

var b1 = new Blob('abcdef');
var b2 = b1.concat('123');

var b1 = new Blob();
for ( c in b1 )
	Print(c+' ');


var s = new String('ABC');

for ( c in s )
	Print(c+' ');




var tmp = '';
var b = new Blob('ABC123');
for ( c in b )
	tmp += c;

Print(tmp);


b['2'] = 5;
b.Free();
b[0]



Print('END'); throw 0;





Print('END'); throw 0;


var b = new Blob('test');


b.match();


Print ( b == new String(''), '\n' );
Print ( b == false, '\n' );

Print ( b, '\n' );

jslang_test(b);



throw 0;


Test5.prototype = new function() {

	this._serialize = function() {
	
		return this.val;
	}
	
	this._unserialize = function(data) {
			
		var o = new Test5();
		o.val = data;
		return o;
	}
}

function Test5() {
}

var obj5 = new Test5;
obj5.val = 5566;


function Test0( a ) {
	
	a++;
	Print('a:'+a);
	return a; 
}


var v = {
	o:'\u1234\u5678\u9012\u3456\u7890',
	a:new Number(0.5), b:['aa', 1.3, [], [[[[[[{}]]]]]], undefined, null, 0 ], c:0.4, d:(new Array(3)), e:obj5,
	f:new Date('1/2/2006'), g:new Blob('123456789'), h:0, i:NaN, j:{},
	k:-1, l:[1,,3], m:'', n:1/3
};
 
v = new Blob('');
//v.test = 123;
//v = Test0;

var uv = uneval(v);
var vv = jslang_test(v);

var uvv = uneval(vv);
_configuration.stdout( '\n\n' );
_configuration.stdout( 'uneval(v) : ', uneval(v), '\n' );
_configuration.stdout( 'uneval(vv): ', uneval(vv), '\n' );
_configuration.stdout( 'result : ', uv == uvv, '\n' );

