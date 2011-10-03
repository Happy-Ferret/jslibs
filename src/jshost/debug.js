"use strict";
//RunJsircbot(false); throw 0;
//var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();
//Exec('../common/tools.js'); var QA = FakeQAApi;  RunLocalQAFile();
// LoadModule('jsstd'); Exec('../common/tools.js'); RunQATests('-exclude jstask Unserialization');
//LoadModule('jsstd'); LoadModule('jsio'); currentDirectory += '/../../tests/jslinux'; Exec('start.js'); throw 0;
//SetPerfTestMode();

LoadModule('jsstd');
LoadModule('jsio');
//LoadModule('jsdebug');

//_jsapiTests();

jslangTest()

	var test = new URIError();

	var s = new Serializer();
	s.Write(test);
	var s = new Unserializer(s.Done());
	Print( test.toSource(), '\n' );
	Print( s.Read().toSource(), '\n' );




throw 0;

//var tree = Reflect.parse('var i, j = {a:1, b:function(){}}, o = [/xx/gi, false, null,"", "x", undefined];;;;; o["abc"+1]; o[5]=i+3;', {loc:false});
var tree = Reflect.parse('if ( a == 5 ) {for (var i = 0; i < -100; i++) { Print.call(this, 123) } } else { 1}', {loc:true});

Print( 'tree:\n', uneval(tree), '\n' );

function processList(list, separator) {
	
	var len = list.length;
	var tmp = '';
	for ( var i = 0; i < len; ++i )
		tmp += separator + process(list[i]);
	return tmp.substr(1);
}

function process(node) {

	if ( node == null )
		return '';

	switch (node.type) {

		case 'Literal':
			if ( node.value === null )
				return 'null';
			switch (node.value.constructor.name) {
				case 'String':
					return '"' + node.value + '"';
				case 'Number':
					return node.value;
				case 'Boolean':
					return node.value;
				case 'RegExp':
					return node.value;
			}
			throw 'error';

		case 'Identifier':
			return node.name;

		case 'ThisExpression':
			return 'this';

		case 'Property':
			switch (node.kind) {
				case 'init':
					return process(node.key) + '=' + process(node.value);
			}
			throw 'error';

		case 'ReturnStatement':
			return 'return ' + process(node.argument);

		case 'YieldExpression':
			return 'yield' + (node.argument ? ' ' + process(node.argument) : '');

		case 'BlockStatement':
			return '{' + processList(node.body, ';') + '}';

		case 'FunctionExpression':
			return 'function' + (node.id ? ' '+process(node.id) : '') + '(' + processList(node.params, ',') + ')'+process(node.body);

		case 'CallExpression':
			return process(node.callee) + '(' + processList(node.arguments, ',') + ')';

		case 'AssignmentExpression':
			return process(node.left) + node.operator + process(node.right);

		case 'UnaryExpression':
			return node.prefix ? node.operator + process(node.argument) : process(node.argument) + node.operator;

		case 'BinaryExpression':
			return process(node.left) + node.operator + process(node.right);

		case 'MemberExpression':
			return process(node.object) + (node.computed ? '[' + process(node.property) + ']' : '.' + process(node.property));

		case 'ExpressionStatement':
			return process(node.expression);

		case 'UpdateExpression':
			return node.prefix ? node.operator + process(node.argument) : process(node.argument) + node.operator;

		case 'IfStatement':
			return 'if('+process(node.test)+')' + process(node.consequent) + ( node.alternate ? ' else ' + process(node.alternate) : '' );

		case 'ForStatement':
			return 'for(' + process(node.init) + ';' + process(node.test) + ';' + process(node.update) + ')' + process(node.body);

		case 'ObjectExpression':
			return '{' + processList(node.properties, ',') + '}';

		case 'ArrayExpression':
			return '[' + processList(node.elements, ',') + ']';

		case 'SequenceExpression':
			return  '(' + processList(node.expressions, ',') + ')';

		case 'VariableDeclarator':
			return node.id.name + (node.init ? '=' + process(node.init) : '');

		case 'VariableDeclaration':
			return node.kind + ' ' + processList(node.declarations, ',');
		
		case 'EmptyStatement':
			return ';';

		case 'Program':
			return processList(node.body, ';');
	}
	throw 'error miss '+node.type+' : '+Object.keys(node);
}

Print( 'result:\n', process(tree), '\n' );


//_jsapiTests();


/*
var blob = Blob('abcdefghi');
var array = blob.ReloacateToArray();

Print( array.length, '\n' );

	var tmp;

function test() {

	tmp = array[8];
}

for ( var i = 0; i < 1000; ++i )
	test();

Print( tmp, '\n' );

*/



throw 0;


// doc. https://developer.mozilla.org/en/JavaScript/Reference/Global_Objects/Proxy

function Path() {
	
	function newProxy() {
		
		return Proxy.create(makeHandler({ __proto__:null }));
	}
	
	function makeHandler(obj) {

		return {
			getOwnPropertyDescriptor: function(name) {

				return Object.getOwnPropertyDescriptor(obj, name);
			},
			getPropertyDescriptor: function(name) {

				return Object.getOwnPropertyDescriptor(obj, name);
			},
			defineProperty: function(name, desc) {
			
				Object.defineProperty(obj, name, desc);
			},
			delete: function(name) {
				
				return delete obj[name];
			},
			get: function(receiver, name) {
				
				var proxy;
				if ( name in obj ) {
					
					proxy = obj[name];
				} else {
				
					proxy = newProxy();
					obj[name] = proxy;
				}
				return proxy;
			},
			enumerate: function() {
				
				return Object.keys(obj);
			}
		}
	}
	
	return newProxy();
}

Path.map = new WeakMap();

Path.set = function(path, data) {
	
	Path.map.set(path, data);
};

Path.has = function(path) {
	
	return Path.map.has(path);
};

Path.get = function(path) {
	
	return Path.map.get(path);
};

// test

p = new Path;

Path.set(p.aaa.x.bbb1, 111);
Path.set(p.aaa.x['bbb2'], 222);

Print( Path.get(p.aaa.x.bbb2), '\n' );

for each ( var x in p.aaa.x ) {

	Print( Path.get(x), '\n' );
}

Path.set(p.aaa.x.__proto__, 333 )
Print( Path.get(p.aaa.x.__proto__), '\n' );

p.z = p.aaa.x.__proto__;
delete p.aaa.x.__proto__;

Print( Path.get(p.aaa.x.__proto__), ' (moved)\n' );

Print( Path.get(p.z), '\n' );

Path.set(p.z.xxx, 444)

p.z.y.x.w.xxx = 123;







throw 0;


var taskList = [];

/*
function _ProcEnd(proc) {
	
	var atExit = proc.atExit;
	if ( atExit ) {
	
		var len = atExit.length;
		for ( var i = 0; i < len; ++i )
			taskList.push(atExit[i]);
	}
	proc.ended = true;
}


function StartProc(proc) {
	
	taskList.push(function inner(result) {
	
		try {
			proc.send(result)(inner);
		} catch (ex if ex === StopIteration) {
			_ProcEnd(proc);
		}
	});
	return proc;
}


function PWaitProc(proc) function(callback) {
		
	if ( !proc.atExit )
		proc.atExit = [];
	proc.atExit.push(function() taskList.push(callback));
}

function PIdle(callback) {

	 taskList.push(callback);
}

function PKill(proc) {
	
	if ( proc ) {
		
		proc.close();
		_ProcEnd(proc);
	} else {
		
		throw StopIteration;
	}
}
*/


// timer

var currentTime = Date.now();
var timeoutList = [];

function AddTimeout(time, fct) {

	timeoutList.push([currentTime + time, fct]);
	timeoutList.sort(function(a,b) a[0]-b[0]);
}

function ProcessTimeout() {

	var len = timeoutList.length;
	for ( var i = 0; i < len; ++i )
		if ( timeoutList[i][0] > currentTime )
			break;
	var exList = timeoutList.splice(0, i);
	var len = exList.length;
	for ( var i = 0; i < len; ++i )
		taskList.push(exList[i][1]);
}

function PSleep(time) function(callback) AddTimeout(time, callback);


// event

function PCreateEvent(manualReset, initialState) {

	return [[], initialState, manualReset];
}

function PResetEvent(event) {
	
	event[1] = false;
}

function PFireEvent(event) {

	var len = event[0].length;
	for ( var i = 0; i < len; ++i )
		taskList.push(event[0][i]);
	event[0].length = 0;
	event[1] = event[2];
}

function PWaitEvent(event) function(callback) {

	if ( event[1] )
		taskList.push(callback);
	else
		event[0].push(callback);
}


// Semaphore

function PCreateSemaphore(initialCount) {
	
	return [[], initialCount];
}

function PReleaseSemaphore(semaphore) {
	
	if ( semaphore[0].length )
		taskList.push(semaphore[0].shift());
	else
		++semaphore[1];
}

function PAcquireSemaphore(semaphore) function(callback) {

	if ( semaphore[1] > 0 ) {
	
		--semaphore[1];
		taskList.push(callback);
	} else {
	
		semaphore[0].push(callback);
	}
}


// Step

function Step(elapsed, maxWait) {

	currentTime += elapsed;
	ProcessTimeout();
	
	var wait;
	if ( timeoutList.length )
		wait = Math.min(timeoutList[0][0] - currentTime, maxWait);
	else
		wait = maxWait;

	var len = taskList.length;
	if ( len ) {
		
		var tmp = taskList;
		taskList = [];
		for ( var i = 0; i < len; ++i )
			tmp[i]();
	}
	
	Sleep(Math.floor(wait));
}

///

function Proc(fct, pProc) {
	
	var proc = this;
	if ( pProc ) {
	
		proc.parent = pProc;
		if ( pProc.childList )
			pProc.childList.push(proc);
		else
			pProc.childList = [proc];
	}
	proc.gen = fct(proc);
	taskList.push(function inner(result) {
	
		try {
		
			proc.gen.send(result)(inner);
		} catch (ex if ex === StopIteration) {
		
			proc.Kill();
		}
	});
}

Proc.prototype.Kill = function() {
	
	Print('x');
	
	this.gen.close();
	var atExit = this.atExit;
	if ( atExit != undefined ) {
	
		var len = atExit.length;
		for ( var i = 0; i < len; ++i )
			taskList.push(atExit[i]);
	}
	var childList = this.childList;
	if ( childList != undefined ) {
		
		var len = childList.length;
		for ( var i = 0; i < len; ++i ) {
		
			var tmp = childList[i];
			if ( tmp != undefined ) {
				
				tmp.Kill();
			}
		}
	}
	
	var parent = this.parent;
	if ( parent != undefined ) {
		
		var childList = parent.childList;
		if ( childList != undefined )
			childList[childList.indexOf(this)] = undefined;
	}
	
	this.ended = true;
}

Proc.prototype.YWait = function(proc) function(callback) {
	
	var tmp = function() taskList.push(callback);
	if ( proc.atExit )
		proc.atExit.push(tmp);
	else
		proc.atExit = [tmp];
}

Proc.prototype.YSleep = function(time) function(callback) {
	
	if ( time != undefined )
		AddTimeout(time, callback)
	else
		taskList.push(callback);
}


////////////////////////////////////
// test

function P2() function(proc) {

	for (var i = 0; i< 20; ++i) {
		
		Print('b');
		yield proc.YSleep();
	}
}


function P1(arg) function(proc) {
	
	for (var i = 0;; ++i) {
		
		if ( i == 10 ) {
		
			var newProc = new Proc(P2(), proc);
			//yield proc.YWait(newProc);
		}
			
		Print(arg);
		yield proc.YSleep();
	}
}


var p = new Proc(P1('a'));


for (var i = 0; !endSignal && i < 100; ++i ) {

	if ( i == 15 ) {
		
		p.Kill();
	}
	
	Step(5, 5);
}

throw 0;








/* ************

var taskList = [];

// timer

var currentTime = 0;
var timeoutList = [];

function AddTimeout(time, fct) {

	timeoutList.push([currentTime + time, fct]);
	timeoutList.sort(function(a,b) a[0]-b[0]);
}

function ProcessTimeout() {

	var len = timeoutList.length;
	for ( var i = 0; i < len; ++i )
		if ( timeoutList[i][0] > currentTime )
			break;
	var exList = timeoutList.splice(0, i);
	var len = exList.length;
	for ( var i = 0; i < len; ++i )
		taskList.push(exList[i][1]);
}




function Proc(fct) {
	this.fct = fct;
	this.step = 0;
	this.result = undefined;
}

Proc.prototype.Thaw = function(stateData) {
	
	this.step = stateData[0];
	this.result = stateData[1];
}

Proc.prototype.Freeze = function() {
	
	return [ this.step, this.result ];
}

Proc.prototype.Start = function() {
	
	var proc = this;
	taskList.push(function inner() {

		proc.result = proc.fct(proc)(proc, inner);
	});
}

Proc.prototype.Sleep = function(time) function(state, callback) {

	++state.step;
	AddTimeout(time, callback);
}

Proc.prototype.Idle = function() function(state, callback) {
	
	++state.step;
	taskList.push(callback);
}

Proc.prototype.Goto = function(step) function(state, callback) {
	
	state.step = step;
	taskList.push(callback);
}

Proc.prototype.End = function() function(state, callback) {
};



function Step(elapsed, maxWait) {

	currentTime += elapsed;
	ProcessTimeout();
	
	var wait;
	if ( timeoutList.length )
		wait = Math.min(timeoutList[0][0] - currentTime, maxWait);
	else
		wait = maxWait;

	var len = taskList.length;
	if ( len ) {
		
		var tmp = taskList;
		taskList = [];
		for ( var i = 0; i < len; ++i )
			tmp[i]();
	}
	
	Sleep(Math.floor(wait));
}


// user code

function test() {

	function testProc1( state ) {

		switch ( state.step ) {
			
			case 0:
			Print(state.step);
			return state.Idle();
			case 1:
			Print(state.step);
			return state.Sleep(500);
			case 2:
			Print(state.step);
			return state.Goto(0);
		}
		return state.End();
	}
	
	this.Init = function() {
	
		this.proc = new Proc(testProc1);
	}

	this.Start = function() {
		
		this.proc.Start();
	}

	this.Freeze = function() {
		
		return { proc:this.proc.Freeze() }
	}
	
	this.Thaw = function(stateData) {
		
		this.proc.Thaw(stateData.proc);
	}
	
}



var t = new test();
t.Init();
t.Start();

while ( !endSignal ) {
	
	var data = JSON.stringify(t.Freeze());
	t = new test();
	t.Init();
	t.Thaw(JSON.parse(data));
	t.Start();
	
	
	Step(10, 10);
}

Print( uneval(timeoutList) );


************ */



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/* ****************************************

var taskList = [];

function _ProcEnd(proc) {
	
	var atExit = proc.atExit;
	if ( atExit ) {
	
		var len = atExit.length;
		for ( var i = 0; i < len; ++i )
			taskList.push(atExit[i]);
	}
}

function StartProc(proc) {
	
	taskList.push(function inner(result) {
	
		try {
			proc.send(result)(inner);
		} catch (ex if ex === StopIteration) {
			_ProcEnd(proc);
		}
	});
	return proc;
}

function PWaitProc(proc) function(callback) {
		
	if ( !proc.atExit )
		proc.atExit = [];
	proc.atExit.push(function() taskList.push(callback));
}

function PIdle(callback) {

	 taskList.push(callback);
}

function PKill(proc) {
	
	if ( proc ) {
		
		proc.close();
		_ProcEnd(proc);
	} else {
		
		throw StopIteration;
	}
}


// timer

var currentTime = Date.now();
var timeoutList = [];

function AddTimeout(time, fct) {

	timeoutList.push([currentTime + time, fct]);
	timeoutList.sort(function(a,b) a[0]-b[0]);
}

function ProcessTimeout() {

	var len = timeoutList.length;
	for ( var i = 0; i < len; ++i )
		if ( timeoutList[i][0] > currentTime )
			break;
	var exList = timeoutList.splice(0, i);
	var len = exList.length;
	for ( var i = 0; i < len; ++i )
		taskList.push(exList[i][1]);
}

function PSleep(time) function(callback) AddTimeout(time, callback);


// event

function PCreateEvent(manualReset, initialState) {

	return [[], initialState, manualReset];
}

function PResetEvent(event) {
	
	event[1] = false;
}

function PFireEvent(event) {

	var len = event[0].length;
	for ( var i = 0; i < len; ++i )
		taskList.push(event[0][i]);
	event[0].length = 0;
	event[1] = event[2];
}

function PWaitEvent(event) function(callback) {

	if ( event[1] )
		taskList.push(callback);
	else
		event[0].push(callback);
}


// Semaphore

function PCreateSemaphore(initialCount) {
	
	return [[], initialCount];
}

function PReleaseSemaphore(semaphore) {
	
	if ( semaphore[0].length )
		taskList.push(semaphore[0].shift());
	else
		++semaphore[1];
}

function PAcquireSemaphore(semaphore) function(callback) {

	if ( semaphore[1] > 0 ) {
	
		--semaphore[1];
		taskList.push(callback);
	} else {
	
		semaphore[0].push(callback);
	}
}


// Step

function Step(maxWait) {

	currentTime = Date.now();
	ProcessTimeout();
	
	var wait;
	if ( timeoutList.length )
		wait = Math.min(timeoutList[0][0] - currentTime, maxWait);
	else
		wait = maxWait;

	var len = taskList.length;
	if ( len ) {
		
		var tmp = taskList;
		taskList = [];
		for ( var i = 0; i < len; ++i )
			tmp[i]();
	}
	
	Sleep(wait);
}


///////////////////////////////
// user code



function GetAResult(callback) {
	
	AddTimeout(10, function() callback('{res}'));
}



// yield PWaitProc(StartProc(Proc1('y')));

var ev = PCreateEvent(true);

var sem = PCreateSemaphore(3);


function Proc1(name) {

	try {
		
		var i = 0;

		for (;;) {
			
//			if ( i++ == 5 )
//				PKill();

			Print(name);
			
			yield PWaitEvent(ev);
			
//			yield PAcquireSemaphore(sem);
			
//			var res = yield GetAResult;
//			Print(res);
		}
	
	} finally {
		
		Print('[end of '+name+']');
	}
}




var p = Proc1('y');

StartProc(p);

//AddTimeout(2000, function() PKill(p));

//AddTimeout(1000, function() PFireEvent(ev));
//AddTimeout(2000, function() PFireEvent(ev));

AddTimeout(1000, function() PFireEvent(ev));

//AddTimeout(2000, function() PReleaseSemaphore(sem));
//AddTimeout(1000, function() PReleaseSemaphore(sem));
//AddTimeout(3000, function() PReleaseSemaphore(sem));

//Print(uneval(timeoutList)); throw 0;



//var t = Date.now();

//for ( var i = 0; i < 10000 && !endSignal; ++i ) {
while ( !endSignal ) {

	Step(5);
//	Print('.');
}

//var t = Date.now() - t;
//Print( 'time:'+(1000 * (i*1) / t)+'fps', '\n' );


************************* */



throw 0;





jslangTest('xxx'); throw 0;


LoadModule('jsz');

var i = 1000;
var o = { Read:function() { return i-- ? 'x' : '' } };
Print( Stringify(o).length );

throw 0;

var f = new ZipFile('test.zip');
f.Open(ZipFile.CREATE);
f.Select('toto/xxx.txt');
f.date = new Date(2008,6,4);
f.globalComment = 'a comment';
Print( 'global comment:', f.globalComment, '\n' );
Print( 'date:', f.date, '\n' );
f.extra = 'extra field';
f.Write('content data');
f.Close();

Print( '---\n' );

var g = new ZipFile('test.zip');
g.Open(ZipFile.READ);
g.Select('toto/xxx.txt');
//Print( 'global comment:', g.globalComment, '\n' );
//Print( g.filename, ' / ', g.date, ' / ', '\n' );
Print( Stringify(g).quote(), '\n' );
g.Close();



throw 0;
