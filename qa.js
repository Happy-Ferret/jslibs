LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsdebug');

function RecursiveDir(path, callback) {
	
	(function(path) {

		var dir = new Directory(path);
		dir.Open();
		for ( var entry; ( entry = dir.Read(Directory.SKIP_BOTH) ); ) {

			var file = new File(dir.name+'/'+entry);
			switch ( file.info.type ) {
				case File.FILE_DIRECTORY:
					arguments.callee(file.name);
					break;
				case File.FILE_FILE:
					callback(file);
					break;
			}
		}
		dir.Close();
	})(path);
}


function CreateQaItemList(startDir, filter, flags) {

	var hidden = /\/\./;
	var qaFile = /\qa.js$/;
	var newQaItem = /^\/\/\/\s*(.*?)\s*$/;
	var parseFlags = function(str) (/\[(.*?)\]/(str)||[,''])[1];


	var itemList = [];
	var index = 0;

	RecursiveDir( startDir, function(file) {
	
		if ( !hidden(file.name) && qaFile(file.name) ) {

			var source = String(file.content);
			source = source.replace(/\r\n|\r/g, '\n'); // cleanup
			
			var lines = source.split('\n');
			
			var item = { name:'[INIT]', file:file.name, line:1, flags:'', code:[], init:true }; // for the initialization item
			
			for ( var l in lines ) {
				
				var res = newQaItem(lines[l]);
				if ( res ) {
					
					itemList.push(item);
					item = { file:file.name, line:Number(l)+1, name:res[1], flags:parseFlags(res[1]), code:[] };
				}
				item.code.push(lines[l]);
			}
			itemList.push(item);
		}
	})
	
	for each ( var item in itemList ) {

		try {
		
			item.relativeLineNumber = Locate()[1]+1;
			item.func = new Function('QA', item.code.join('\n'));
		} catch(ex) {
			
			item.func = function() {}
			var lineno = item.line + ex.lineNumber - item.relativeLineNumber;
			message = 'COMPILATION: @'+ item.file +':'+ lineno +' - '+ item.name +' - '+ ex;
			Print( '*** ' + message, '\n' );
		}
	}

	itemList = [ item for each ( item in itemList ) if (  item.init || (!filter || filter(item.name) || filter(item.file)) && (!flags || flags(item.flags))  ) ];
	itemList = itemList.sort( function(a,b) a.init ? -1 : 1 ); // put all init function at the top of the test list.
	return itemList;
}



function QAAPI(cx) {

	function FormatVariable(val) {

		if ( typeof(val) == 'string' ) {
		
			if ( val.length > 50 )
				return val.substr(0, 47).quote()+'...';
			return val.quote();
		}
		if ( val instanceof Array )
			return '['+val+']';
		return val;
	}
	
	this.__defineGetter__('cx', function() cx);

	this.FAILED = function( message ) {

		cx.ReportIssue( message );
	}

	this.ASSERT_TYPE = function( value, type, testName ) {

		cx.CheckPoint('ASSERT_TYPE', testName);
		if ( typeof(value) != type && !(value instanceof type) )
			cx.ReportIssue( 'Invalid type, '+type.name+' is expected.', testName );
	}

	this.ASSERT_EXCEPTION = function( fct, exType, testName ) {
		
		cx.CheckPoint('ASSERT_EXCEPTION', testName);
		try {
		
			fct();
			cx.ReportIssue( 'Exception not detected', testName );
		} catch(ex) {
	
			if ( (ex != exType) && !(ex instanceof exType) )
				cx.ReportIssue('Invalid exception ('+ex.constructor.name+' != '+exType.name+')', testName );
		}
	} 

	this.ASSERT = function( value, expect, testName ) {
	
		cx.CheckPoint('ASSERT', testName);
		if ( value !== expect && !(typeof(value) == 'number' && isNaN(value) && typeof(expect) == 'number' && isNaN(expect)) )
			cx.ReportIssue( FormatVariable(value)+' !== '+FormatVariable(expect), testName );
	}

	this.ASSERT_STR = function( value, expect, testName ) {
	
		cx.CheckPoint('ASSERT_STR', testName);
		if ( value != expect ) // value = String(value); expect = String(expect); // not needed because we use the != sign, not !== sign
			cx.ReportIssue( FormatVariable(value)+' != '+FormatVariable(expect), testName );
	}

   this.ASSERT_HAS_PROPERTIES = function( obj, names ) {
   	
		cx.CheckPoint('ASSERT_HAS_PROPERTIES', names);
   	for each ( var p in names.split(/\s*,\s*/) ) {
   	
   		if ( !(p in obj) )
	  			cx.ReportIssue( 'Property '+p.quote()+' not found.' );
	  	}
   }


	this.GC = function() {

		CollectGarbage();
	}

	var randomData = '';
	for ( var i = 0; i < 1024; ++i )
		randomData += String.fromCharCode(Math.random()*255);
   this.RandomData = function(length) {

        var data = '';
        while( data.length < length )
            data += randomData.substring( Math.random()*randomData.length, Math.random()*randomData.length );
        return data.substr(0, length);
   }

	var randomString = '';
	for ( var i = 0; i < 1024; ++i )
		randomString += Math.random().toString(36).substr(2);
   this.RandomString = function(length) { // [0-9A-Za-z]

        var data = '';
        while( data.length < length )
            data += randomString.substring( Math.random()*randomString.length, Math.random()*randomString.length );
        return data.substr(0, length);
   }
}




function LaunchTests(itemList, conf) {
	
	var issueList = [];
	var checkpointCount = 0;
	
	function ReportIssue(message, testName) {

		var lineno = Locate(this.stackIndex+1)[1] - this.item.relativeLineNumber + this.item.line;
		message = 'TEST: @'+ this.item.file+':'+lineno +' - '+ (testName||'') +' - '+ message;
		issueList.push(message);
		Print( '\n X '+ message, '\n' );
	}
	
	function CheckPoint(title, testName) {
	
		checkpointCount++;	
	}

	var cx = { stackIndex:stackSize-1, conf:conf, ReportIssue:ReportIssue, CheckPoint:CheckPoint };

	var qaapi = new QAAPI(cx);

	for each ( var currentItem in itemList ) {

		cx.item = currentItem;

		try {
			
			Print( ' - '+currentItem.file+' - '+ currentItem.name );
			
			gcZeal = conf.gcZeal;

			if( !conf.noGcBetweenTests )
				CollectGarbage();

			var m0 = privateMemoryUsage;
			var t0 = TimeCounter();

			if ( conf.nogcDuringTests )
				disableGarbageCollection = true;
			
			for ( var i = 0; i < conf.repeatEachTest; i++ )
				currentItem.func(qaapi);

			if ( conf.nogcDuringTests )
				disableGarbageCollection = false;

			var t = (TimeCounter() - t0) / conf.repeatEachTest;
			var m = (privateMemoryUsage - m0) / conf.repeatEachTest;
			Print( '  ...('+t.toFixed(1) + 'ms / '+(m/1024).toFixed(1)+'KB)' );

			Print('\n');
				
		} catch(ex) {
			
			Print('\n');
			
			var lineno = ex.lineNumber - currentItem.relativeLineNumber + currentItem.line;
			var message = 'EXCEPTION: @'+ currentItem.file+':'+lineno + ' - '+ currentItem.name +' - '+ ex;
			issueList.push(message);
			Print( ' X '+ message, '\n' );			
		}
		
		disableGarbageCollection = false; // in case of
		gcZeal = 0;

		if ( conf.stopAfterNIssues && issues > conf.stopAfterNIssues )
			break;

		if ( endSignal )
			break;
	}
	
	return [issueList, checkpointCount];
}



function LaunchRandomTests(itemList, conf) {

	var issueList = [];
	var checkpointCount = 0;
	
	function ReportIssue(message, testName) {

		var lineno = Locate(this.stackIndex+1)[1] - this.item.relativeLineNumber + this.item.line;
		message = 'TEST: @'+ this.item.file+':'+lineno +' - '+ (testName||'') +' - '+ message;
		issueList.push(message);
		Print( '\n X '+ message, '\n' );
	}
	
	function CheckPoint(title, testName) {
	
		checkpointCount++;	
	}

	var cx = { stackIndex:stackSize-1, conf:conf, ReportIssue:ReportIssue, CheckPoint:CheckPoint };

	var qaapi = new QAAPI(cx);


	for each ( var item in itemList ) {

		if ( !item.init ) // list is sorted, init are first.
			break;
		item.func(qaapi);
	}

	while ( !endSignal ) {

		currentItem = itemList[Math.floor(Math.random() * itemList.length)];
		cx.item = currentItem;
		
		gcZeal = conf.gcZeal;

		try {
			
			for ( var i = 0; i < conf.repeatEachTest; i++ )
				currentItem.func(qaapi);

			if( !conf.noGcBetweenTests )
				CollectGarbage();
			
		} catch(ex) {

			var lineno = ex.lineNumber - currentItem.relativeLineNumber + currentItem.line;
			var message = 'EXCEPTION: @'+ currentItem.file+':'+lineno + ' - '+ currentItem.name +' - '+ ex;
			issueList.push(message);
			Print( ' X '+ message, '\n' );
		}

		disableGarbageCollection = false; // in case of
		gcZeal = 0;
	}
	
	return [issueList, checkpointCount]; 
}


function ParseCommandLine(conf) {

	var args = global.arguments;
	conf.args = [];
	while ( args.length > 1 ) {

		if ( args[1][0] != '-' ) {
		
			conf.args.push( args.splice(1,1) );
			continue;
		}
		var items = [ c for (c in conf) if ( c.toLowerCase().indexOf(args[1].substr(1).toLowerCase()) == 0 ) ];
		if ( items.length > 1 )
			throw Error('Multiple argument match: '+items.join(', '));
		var item = items[0];
		if ( IsVoid(item) )
			throw Error('Invalid argument: '+args[1]);
		if ( IsBoolean(conf[item]) ) {
		
			conf[item] = !conf[item];
			args.splice(1,1);
			continue;
		}
		conf[item] = args[2];
		args.splice(1,2);
	}
}



var conf = { help:false, repeatEachTest:1, gcZeal:0, loopForever:false, directory:'src', priority:0, flags:'', save:'', load:'', disableJIT:false, listTestsOnly:false, nogcBetweenTests:false, nogcDuringTests:false, stopAfterNIssues:0 };
ParseCommandLine(conf);
var configurationText = 'configuraion: '+[k+':'+v for ([k,v] in Iterator(conf))].join(' - ');
Print( configurationText, '\n\n' );

if ( conf.help )
	Halt();


function MatchFlags(flags) {
	
	if ( flags.indexOf('d') != -1 )
		return false;
	if ( !conf.flags )
		return true;
	if ( !flags )
		return false;
	for each ( var c in conf.flags )
		if ( flags.indexOf(c) == -1 )
			return false;
	 return true;
}

var itemFilter = new RegExp(conf.args[0] || '.*', 'i');

var testList;
if ( conf.load )
	testList = eval(new File(conf.load).content);
else
	testList = CreateQaItemList(conf.directory, itemFilter, MatchFlags);

if ( conf.listTestsOnly ) {
	
	Print([String.quote(t.file+' - '+t.name) for each ( t in testList )].join('\n'), '\n', testList.length +' tests.', '\n');
	Halt();
}

if ( conf.save )
	new File(conf.save).content = uneval(testList);

if ( conf.disableJIT )
	DisableJIT();

var savePrio = processPriority;
processPriority = conf.priority;
var t0 = TimeCounter();

var [issueList, checkpointCount] = ( conf.loopForever ? LaunchRandomTests : LaunchTests )(testList, conf);

var t = TimeCounter() - t0;
processPriority = savePrio || 0; // savePrio may be undefined

Print( '\n\n', configurationText, '\n\n', issueList.length +' issues, '+ testList.length +' tests, ' + checkpointCount + ' checkpoints in ' + t.toFixed(2) + 'ms ('+(conf.repeatEachTest)+' repeat per test).', '\n' );
issueList.sort();
issueList.reduce( function(previousValue, currentValue, index, array) {

    if ( previousValue != currentValue )
		Print( ' X ' + currentValue, '\n' );
    return currentValue;
}, undefined );


/* flags:

	'd' for desactivated: the test is disabled.
	'f' for fast: the test execution is fast. Time should be less that 10ms.
	't' for time: the test execution time is always the same. The test do not use any variable-execution-time function (CollectGarbage, Poll, Socket, ...)
	'r' for reliable: external parameters (like the platform, CPU load, TCP/IP connection, weather, ...) cannot make the test to fail.
	'm' for low memory usage. The test uses the minimum amount of memory in the script part. no QA.RandomString(300000000) or StringRepeat('x', 10000000000)
*/
