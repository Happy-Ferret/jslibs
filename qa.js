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

		cx.ReportIssue( message, 'FAILURE' );
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

		cx.cfg.nogcDuringTests || CollectGarbage();
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


function CreateQaItemList(startDir, files, include, exclude, flags) {

	var hidden = /\/\./;
	var qaFile = new RegExp(files);
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

	itemList = [ item for each ( item in itemList ) if (  item.init || (include?include(item.name)||include(item.file):true) && !(exclude?exclude(item.name)||exclude(item.file):false) && (!flags || flags(item.flags))  ) ];
	itemList = itemList.sort( function(a,b) a.init ? -1 : 1 ); // put all init function at the top of the test list.
	return itemList;
}



function CommonReportIssue(cx, type, location, testName, checkName, details) {

	message = type +' @'+ location +' - '+ (testName||'') +' - '+ (checkName||'') +' - '+ details;
	cx.issueList.push(message);
	Print( '\n X '+ message, '\n' );
	
	if ( cx.cfg.logFilename ) {
	
		logFile = new File(cfg.logFilename);
		logFile.Open('a+');
		logFile.Write(message + '\n');
		logFile.Close();
	}
}


function LaunchTests(itemList, cfg) {

	var cx = { 
		checkCount:0, 
		issueList:[], 
		stackIndex:stackSize-1, 
		cfg:cfg, 
		ReportIssue:function(message, checkName) {
		
			CommonReportIssue(cx, 'ASSERT',  this.item.file+':'+(Locate(this.stackIndex+1)[1] - this.item.relativeLineNumber), this.item.name, checkName, message );
		},
		Checkpoint:function(title, testName) {
		
			this.checkCount++;
		}
	};

	var qaapi = new QAAPI(cx);
	

	if ( cfg.loopForever ) {

		for ( var i in itemList ) {

			if ( !itemList[i].init ) // list is sorted, init are first.
				break;
			itemList[i].func(qaapi);
		}
		itemList = itemList.slice(i);
	}	
	

	CollectGarbage();

	testIndex = 0;

	for (;;) {

		if ( cfg.loopForever )
			testIndex = Math.floor(Math.random() * itemList.length);

		cx.item = itemList[testIndex];
		
		if ( cx.item.init || cfg.runOnlyTestIndex == undefined || cfg.runOnlyTestIndex == testIndex ) {

			cfg.quiet || Print( ' - '+testIndex+' - '+cx.item.file+' - '+ cx.item.name );

			cfg.nogcBetweenTests || CollectGarbage();

			gcZeal = cfg.gcZeal;
			disableGarbageCollection = cfg.nogcDuringTests;

			try {

				var t0 = TimeCounter();
				for ( var i = cfg.repeatEachTest; i && !endSignal ; --i ) {

					void cx.item.func(qaapi);
					if ( cx.item.init )
						break;
				}
				var t1 = TimeCounter() - t0;
				cfg.quiet || Print( ' ...('+(t1/cfg.repeatEachTest).toFixed(1) + 'ms)' );
			} catch(ex) {

				CommonReportIssue(cx, 'EXCEPTION', cx.item.file+':'+(ex.lineNumber - cx.item.relativeLineNumber), cx.item.name, '', ex );
			}

			disableGarbageCollection = false;
			gcZeal = 0;
			
			cfg.nogcBetweenTests || CollectGarbage();
			cfg.quiet || Print('\n');

		}
		
		if ( cfg.stopAfterNIssues && issues > cfg.stopAfterNIssues )
			break;

		if ( endSignal )
			break;
			
		if ( !cfg.loopForever && ++testIndex >= itemList.length )
			break;

		if ( cfg.sleepBetweenTests )
			Sleep(cfg.sleepBetweenTests);
	}
	
	return [cx.issueList, cx.checkCount];
}



function ParseCommandLine(cfg) {

	var args = global.arguments;
	cfg.args = [];
	while ( args.length > 1 ) {

		if ( args[1][0] != '-' ) {
		
			cfg.args.push( args.splice(1,1) );
			continue;
		}
		var items = [ c for (c in cfg) if ( c.toLowerCase().indexOf(args[1].substr(1).toLowerCase()) == 0 ) ];
		if ( items.length > 1 )
			throw Error('Multiple argument match: '+items.join(', '));
		var item = items[0];
		if ( IsVoid(item) )
			throw Error('Invalid argument: '+args[1]);
		if ( IsBoolean(cfg[item]) ) {
		
			cfg[item] = !cfg[item];
			args.splice(1,1);
			continue;
		}
		cfg[item] = args[2];
		args.splice(1,2);
	}
}



var cfg = { help:false, repeatEachTest:1, gcZeal:0, loopForever:false, directory:'src', files:'_qa.js$', priority:0, flags:'', save:'', load:'', disableJIT:false, listTestsOnly:false, nogcBetweenTests:false, nogcDuringTests:false, stopAfterNIssues:0, logFilename:'', sleepBetweenTests:0, quiet:false, runOnlyTestIndex:undefined, exclude:undefined };
ParseCommandLine(cfg);
var configurationText = 'configuraion: '+['-'+k+' '+v for ([k,v] in Iterator(cfg))].join(' ');
Print( configurationText, '\n\n' );

if ( cfg.help )
	Halt();


function MatchFlags(flags) {
	
	if ( flags.indexOf('d') != -1 )
		return false;
	if ( !cfg.flags )
		return true;
	if ( !flags )
		return false;
	for each ( var c in cfg.flags )
		if ( flags.indexOf(c) == -1 )
			return false;
	 return true;
}


var itemInclude = new RegExp(cfg.args[0] || '.*', 'i');
var itemExclude = cfg.exclude ? new RegExp(cfg.exclude, 'i') : undefined;

var testList;
if ( cfg.load )
	testList = eval(new File(cfg.load).content);
else
	testList = CreateQaItemList(cfg.directory, cfg.files, itemInclude, itemExclude, MatchFlags);

if ( cfg.listTestsOnly ) {
	
	Print([String.quote(t.file+' - '+t.name) for each ( t in testList )].join('\n'), '\n', testList.length +' tests.', '\n');
	Halt();
}

if ( cfg.save )
	new File(cfg.save).content = uneval(testList);

if ( cfg.disableJIT )
	DisableJIT();

var savePrio = processPriority;
processPriority = cfg.priority;
var t0 = TimeCounter();

var [issueList, checkCount] = LaunchTests(testList, cfg);

var t = TimeCounter() - t0;
processPriority = savePrio || 0; // savePrio may be undefined

Print( '\n'+StringRepeat('-',97)+'\n', configurationText, '\n\n', issueList.length +' issues, '+cfg.repeatEachTest+'x '+ [t for each (t in testList) if (!t.init)].length +' tests, ' + checkCount + ' checks in ' + t.toFixed(2) + 'ms.', '\n' );
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
