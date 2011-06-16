//RunJsircbot(false); throw 0;
//var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();
//Exec('../common/tools.js'); var QA = FakeQAApi;  RunLocalQAFile();
//LoadModule('jsstd'); Exec('../common/tools.js'); RunQATests('-exclude jstask');
//LoadModule('jsstd'); LoadModule('jsio'); currentDirectory += '/../../tests/jslinux'; Exec('start.js'); throw 0;

LoadModule('jsstd');

var scheduler = new function() {

	var taskList = [];
	var currentTask;
	
	this.Step = function() {
		
		currentTask = taskList.shift();
		if ( !currentTask )
			return;
		
		try {
		
			currentTask.next();
			if ( currentTask )
				taskList.push(currentTask);
			
		} catch (ex if ex instanceof StopIteration) {
		}
	}
	
	this.Run = function(taskFct) {
		
		return taskList.push(taskFct());
	}
	
	this.Wait = function(task) {
	
		// stop the current task until 'task' is finished
	
	}
	
}



function Counter(name) {
	return function() {

		for (var i = 0; i < 10; ++i) {
		
			Print(name);
			
			var newTask = scheduler.Run( Counter('y') );
			
			scheduler.Wait(newTask);
			
			yield;
		}
	}
}


scheduler.Run( Counter('x') );




while ( !endSignal ) {

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

