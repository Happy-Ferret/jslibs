// LoadModule('jsstd'); Exec('../common/tools.js');
// var QA = FakeQAApi;
// RunLocalQAFile();
// RunJsircbot(false);
// RunQATests('-rep 3 -exclude jstask');


LoadModule('jsstd');

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



//var v = { a:new Number(0.5), b:['aa', 1.3, [], [[[]]], undefined, null, 0 ], c:0.4, d:(new Array(3)), e:obj5 };

v = new Blob();
//v = new Test5();


var uv = uneval(v);
var vv = jslang_test(v);
var uvv = uneval(vv);
_configuration.stdout( uv == uvv, '\n' );
_configuration.stdout( uneval(v), '\n' );
_configuration.stdout( uneval(vv), '\n' );

