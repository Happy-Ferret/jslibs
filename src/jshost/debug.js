LoadModule('jsstd'); Exec('../common/tools.js');
var QA = FakeQAApi;
// RunLocalQAFile();
// RunJsircbot(false);
RunQATests('-rep 3 -exclude jstask'); throw 0;

	var s = 'Ab \0c';
	var b = Blob(s);
	
	Print( b.lastIndexOf('A', -1) );
	

Print('END')



throw 0;


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

