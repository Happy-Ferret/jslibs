"use strict";
//RunJsircbot(false); throw 0;
//var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();
//Exec('../common/tools.js'); var QA = FakeQAApi;  RunLocalQAFile();
//LoadModule('jsstd'); Exec('../common/tools.js'); RunQATests('-exclude jstask');
//LoadModule('jsstd'); LoadModule('jsio'); currentDirectory += '/../../tests/jslinux'; Exec('start.js'); throw 0;

LoadModule('jsstd');
LoadModule('jsdebug');

SetPerfTestMode();




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


throw 0;









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

