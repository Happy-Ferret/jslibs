//RunJsircbot(false); throw 0;
//var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();
//Exec('../common/tools.js'); var QA = FakeQAApi;  RunLocalQAFile();
//LoadModule('jsstd'); Exec('../common/tools.js'); RunQATests('-exclude jstask');
//LoadModule('jsstd'); LoadModule('jsio'); currentDirectory += '/../../tests/jslinux'; Exec('start.js'); throw 0;

LoadModule('jsstd');
LoadModule('jsdebug');

/*
var inside;
function gen() {
	
	Print('*** ', this.a);
	yield;
}

var g = gen.call( {a:1} );
g.next();


// In a generator, why the 'this' object is not the generator instance itself ?
// 

throw 0;
*/



SetPerfTestMode();


function ProcEnd(proc) {
	
	var atExit = proc.atExit;
	if ( atExit ) {
	
		var len = atExit.length;
		while ( len-- )
			atExit[len]();
	}
}


function StartAsyncProc( proc ) {
	
	function inner(result) {
		try {
			proc.send(result)(inner);
		} catch (ex if ex === StopIteration) { ProcEnd(proc) }
	}
	try {	
		proc.next()(inner);
	} catch (ex if ex === StopIteration) { ProcEnd(proc) }
}



var taskList = [];

function Step() {
	
	taskList.length && taskList.pop()();
}

function ScheduleProc(proc) {
	
	taskList.push(function() StartAsyncProc(proc));
	return proc;
}

function WaitNext(callback) {

	 taskList.push(callback);
}

function WaitProc(proc) function(callback) {
		
	if ( !proc.atExit )
		proc.atExit = [];
	proc.atExit.push(function() taskList.push(callback));
}

function KillProc(proc) {
	
	proc.close();
	ProcEnd(proc);
}




// user code:

function Proc1(name) {

	for ( var i = 0; i < 10; ++ i ) {

//		Print(name);
		yield WaitNext;
	}
//	Print(name+'-');
}


function Proc2(name) {

	for ( var i = 0; true; ++ i ) {

//		Print(name);
		
//		if ( i == 5 ) {
//			yield WaitProc(ScheduleProc(Proc1('y')));
//		}
//		ScheduleProc(Proc1());

		yield WaitProc(ScheduleProc(Proc1('y')));
		
//		yield WaitNext;
	}
	Print(name+'-');
}



var proc2 = ScheduleProc(Proc2('x'));



var t = Date.now();

for ( var i = 0; i < 10000 && !endSignal; ++i ) {

	Step(); Step(); Step(); Step(); Step(); Step(); Step(); Step(); Step(); Step(); Step(); Step(); Step(); Step(); Step(); Step(); 
	//Print('.');
}

var t = Date.now() - t;



Print( 'time:'+(1000 * (i*16) / t)+'fps', '\n' );

throw 0;




var scheduler = new function() {

	var taskList = [];
	var currentTask;
	
	this.Step = function() {
		
		if ( taskList.length == 0 )
			return;
	
		try {
			
			taskList[0].next();
			taskList.push(taskList.shift());
			
		} catch (ex if ex instanceof StopIteration) {
			
			var current = taskList.shift();
			current._atExit && current._atExit();
		}
	}
	
	this.Run = function(taskFct, sync) {
		
		var task = taskFct();
		if ( sync ) {

			var current = taskList.shift();
			task._atExit = function() taskList.push(current);
		}
		taskList.push(task);
	}
}


function Counter2(name) {
	return function() {

		for ( var i = 0; i < 10; ++ i ) {

			Print(name);
			yield;
		}
		Print(name+'-');
	}
}


function Counter(name) {
	return function() {

		for ( var i = 0; i < 10; ++ i ) {

			Print(name);
			i == 5 && scheduler.Run( Counter2('y'), 1 );
			yield;
		}
	}
}


scheduler.Run( Counter('x') );

var total = 2000;
while ( !endSignal && total-- ) {

	Print('.');
	scheduler.Step();
}

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

var b = Blob('123');

var obj = b; //[1, '2', { abcd:1 }, /qwe/, b ];

var obj2 = Deserialize(Serialize(obj));

Print(uneval(obj2));

