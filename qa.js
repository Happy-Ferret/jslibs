LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsdebug');


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

		cx.Checkpoint('ASSERT_TYPE', testName);
		if ( typeof(value) != type && !(value instanceof type) )
			cx.ReportIssue( 'Invalid type, '+type.name+' is expected.', testName );
	}

	this.ASSERT_EXCEPTION = function( fct, exType, testName ) {
		
		cx.Checkpoint('ASSERT_EXCEPTION', testName);
		try {
		
			fct();
			cx.ReportIssue( 'Exception not detected', testName );
		} catch(ex) {
	
			if ( (ex != exType) && !(ex instanceof exType) )
				cx.ReportIssue('Invalid exception ('+ex.constructor.name+' != '+exType.name+')', testName );
		}
	} 

	this.ASSERT = function( value, expect, testName ) {
	
		cx.Checkpoint('ASSERT', testName);
		if ( value !== expect && !(typeof(value) == 'number' && isNaN(value) && typeof(expect) == 'number' && isNaN(expect)) )
			cx.ReportIssue( FormatVariable(value)+' !== '+FormatVariable(expect), testName );
	}

	this.ASSERT_STR = function( value, expect, testName ) {
	
		cx.Checkpoint('ASSERT_STR', testName);
		if ( value != expect ) // value = String(value); expect = String(expect); // not needed because we use the != sign, not !== sign
			cx.ReportIssue( FormatVariable(value)+' != '+FormatVariable(expect), testName );
	}

   this.ASSERT_HAS_PROPERTIES = function( obj, names ) {
   	
		cx.Checkpoint('ASSERT_HAS_PROPERTIES', names);
   	for each ( var p in names.split(/\s*,\s*/) ) {
   	
   		if ( !(p in obj) )
	  			cx.ReportIssue( 'Property '+p.quote()+' not found.' );
	  	}
   }


	this.GC = function() {

		cx.conf.nogcDuringTests || CollectGarbage();
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
		
			item.relativeLineNumber = Locate()[1]+1 - item.line;
			item.func = new Function('QA', item.code.join('\n'));
		} catch(ex) {
			
			item.func = function() {}
			var lineno = ex.lineNumber - item.relativeLineNumber;
			message = 'COMPILATION: @'+ item.file +':'+ lineno +' - '+ item.name +' - '+ ex;
			Print( '*** ' + message, '\n' );
		}
	}

	itemList = [ item for each ( item in itemList ) if (  item.init || (!filter || filter(item.name) || filter(item.file)) && (!flags || flags(item.flags))  ) ];
	itemList = itemList.sort( function(a,b) a.init ? -1 : 1 ); // put all init function at the top of the test list.
	return itemList;
}



function CommonReportIssue(cx, type, location, testName, checkName, details) {

	message = type +' @'+ location +' - '+ (testName||'') +' - '+ (checkName||'') +' - '+ details;
	cx.issueList.push(message);
	Print( '\n X '+ message, '\n' );
}


function LaunchTests(itemList, conf) {

	var cx = { 
		checkCount:0, 
		issueList:[], 
		stackIndex:stackSize-1, 
		conf:conf, 
		ReportIssue:function(message, checkName) {
		
			CommonReportIssue(cx, 'ASSERT',  this.item.file+':'+(Locate(this.stackIndex+1)[1] - this.item.relativeLineNumber), this.item.name, checkName, message );
		},
		Checkpoint:function(title, testName) {
			this.checkCount++;
		}
	};

	var qaapi = new QAAPI(cx);

	for each ( var currentItem in itemList ) {

		cx.item = currentItem;
		Print( ' - '+currentItem.file+' - '+ currentItem.name );

		conf.noGcBetweenTests || CollectGarbage();
		gcZeal = conf.gcZeal;
		disableGarbageCollection = conf.nogcDuringTests;

		try {

			var m0 = privateMemoryUsage;
			var t0 = TimeCounter();
			for ( var i = conf.repeatEachTest; i && !endSignal ; --i )
				currentItem.func(qaapi);
			var t = (TimeCounter() - t0) / conf.repeatEachTest;
			var m = (privateMemoryUsage - m0) / conf.repeatEachTest;
			Print( '  ...('+t.toFixed(1) + 'ms, '+(m/1024).toFixed(1)+'KB)', '\n' );
		} catch(ex) {
			
			Print('\n');
			CommonReportIssue(cx, 'EXCEPTION', currentItem.file+':'+(ex.lineNumber - currentItem.relativeLineNumber), currentItem.name, '', ex );
		}
		
		disableGarbageCollection = false;
		gcZeal = 0;

		if ( conf.stopAfterNIssues && issues > conf.stopAfterNIssues )
			break;

		if ( endSignal )
			break;

		if ( conf.sleepBetweenTests )
			Sleep(conf.sleepBetweenTests);
	}
	
	return [cx.issueList, cx.checkCount];
}



function LaunchRandomTests(itemList, conf) {

	var cx = { 
		checkCount:0, 
		issueList:[], 
		stackIndex:stackSize-1, 
		conf:conf, 
		ReportIssue:function(message, checkName) {
		
			CommonReportIssue(cx, 'ASSERT',  this.item.file+':'+(Locate(this.stackIndex+1)[1] - this.item.relativeLineNumber), this.item.name, checkName, message );
		},
		Checkpoint:function(title, testName) {
			this.checkCount++;
		}
	};

	var qaapi = new QAAPI(cx);

	var initCount = 0;
	for each ( var item in itemList ) {

		if ( !item.init ) // list is sorted, init are first.
			break;
		item.func(qaapi);
		initCount++;
	}

	var logFile;
	if ( conf.logFilename ) {
	
		logFile = new File(conf.logFilename);
		logFile.Open('w');
		
	}
		
	if ( logFile ) {
		
		var configurationText = 'configuraion: '+[k+':'+v for ([k,v] in Iterator(conf))].join(' - ');
		logFile.Write( configurationText + '\n\n' );
	}

	while ( !endSignal ) {

		currentItem = itemList[ initCount + Math.floor(Math.random() * (itemList.length - initCount)) ];
		cx.item = currentItem;

		Print(cx.checkCount+' checks.', '\r');
//		Print(currentItem.name + '\n');
		
		if ( logFile ) {

			logFile.Write(currentItem.name + '\n');
			logFile.Sync();
		}		
		
		conf.noGcBetweenTests || CollectGarbage();
		gcZeal = conf.gcZeal;
		
		try {
			
			for ( var i = conf.repeatEachTest; i && !endSignal ; --i )
				currentItem.func(qaapi);
		} catch(ex) {

			CommonReportIssue(cx, 'EXCEPTION', currentItem.file+':'+(ex.lineNumber - currentItem.relativeLineNumber), currentItem.name, '', ex );
		}

		disableGarbageCollection = false;
		gcZeal = 0;
		
		if ( conf.sleepBetweenTests )
			Sleep(conf.sleepBetweenTests);
	}
	
	if ( logFile )
		logFile.Close();
	
	return [cx.issueList, cx.checkCount]; 
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



var conf = { help:false, repeatEachTest:1, gcZeal:0, loopForever:false, directory:'src', priority:0, flags:'', save:'', load:'', disableJIT:false, listTestsOnly:false, nogcBetweenTests:false, nogcDuringTests:false, stopAfterNIssues:0, logFilename:'', sleepBetweenTests:0 };
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

var [issueList, checkCount] = ( conf.loopForever ? LaunchRandomTests : LaunchTests )(testList, conf);

var t = TimeCounter() - t0;
processPriority = savePrio || 0; // savePrio may be undefined

Print( '\n----------\n', configurationText, '\n\n', issueList.length +' issues, '+conf.repeatEachTest+' * '+ [t for each (t in testList) if (!t.init)].length +' tests, ' + checkCount + ' checks in ' + t.toFixed(2) + 'ms.', '\n' );
issueList.sort();
issueList.reduce( function(previousValue, currentValue, index, array) {

    if ( previousValue != currentValue )
		Print( '- ' + currentValue, '\n' );
    return currentValue;
}, undefined );


/* flags:

	'd' for desactivated: the test is disabled.
	'f' for fast: the test execution is fast. Time should be less that 10ms.
	't' for time: the test execution time is always the same. The test do not use any variable-execution-time function (CollectGarbage, Poll, Socket, ...)
	'r' for reliable: external parameters (like the platform, CPU load, TCP/IP connection, weather, ...) cannot make the test to fail.
	'm' for low memory usage. The test uses the minimum amount of memory in the script part. no QA.RandomString(300000000) or StringRepeat('x', 10000000000)
*/
