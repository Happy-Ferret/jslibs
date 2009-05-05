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
			
			var item = { name:'', file:file.name, line:1, flags:'', code:[], init:true }; // for the initialization item
			
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
			item.func = new Function('QA', 'ITEM', item.code.join('\n'));
		} catch(ex) {
			
			item.func = function() {}
			Print( '*** ' + ex + ' @' + item.file + ':' + (item.line + ex.lineNumber - item.relativeLineNumber) + ' ('+item.name+')', '\n' );
		}
	}

	itemList = [ item for each ( item in itemList ) if ( item.init || (!filter || filter(item.name) || filter(item.file)) && (!flags || flags(item.flags))  ) ];
	itemList = itemList.sort( function(a,b) a.init ? -1 : 1 );
	return itemList;
}


var currentItem;
var issues = 0;
var testCount = 0;
var errors = [];

var QAAPI = new function() {

	function CodeLocation() {
	
		var lineno = Locate(-2)[1] - currentItem.relativeLineNumber + currentItem.line;
		if ( isNaN(lineno) )
			lineno = '('+String(Locate(-2)[1]) +'-'+ (currentItem.relativeLineNumber + currentItem.line) + ')';
		return currentItem.file+':'+lineno;
	}
	
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

	this.REPORT = function( message ) {

		message += ' - '+currentItem.name;
		issues++;
		errors.push(message);
		Print( ' X >>> ', message, '\n' );
	}

	this.ASSERT_TYPE = function( value, type, testName ) {

		testCount++;
		if ( typeof(value) != type && !(value instanceof type) )
			this.REPORT( CodeLocation()+' '+(testName||'?')+', Invalid type, '+(type.name)+' is expected' );
	}

	this.FAILED = function( message ) {

		this.REPORT( CodeLocation()+' '+message );
	}
	
	this.ASSERT_EXCEPTION = function( fct, exType, testName ) {
		
		testCount++;
		try {
		
			fct();
			this.REPORT( CodeLocation()+' Exception not detected: '+testName );
		} catch(ex) {
	
			if ( (ex != exType) && !(ex instanceof exType) )
				this.REPORT( CodeLocation()+' Invalid exception ('+ex.constructor.name+' != '+exType.name+') for: '+testName );
		}
	} 

	this.ASSERT = function( value, expect, testName ) {

		testCount++;
		if ( value !== expect && !(isNaN(value) && isNaN(expect)) )
			this.REPORT( CodeLocation()+' '+(testName||'?') + ', '+FormatVariable(value)+' != '+FormatVariable(expect) );
	}

	this.ASSERT_STR = function( value, expect, testName ) {
	
		testCount++;
		if ( value != expect ) // value = String(value); expect = String(expect); // not needed because we use the != sign, not !== sign
			this.REPORT( CodeLocation()+' '+(testName||'?') + ', '+FormatVariable(value)+' != '+FormatVariable(expect) );
	}

   this.ASSERT_HAS_PROPERTIES = function( obj, names ) {
   	
   	for each ( var p in names.split(/\s*,\s*/) ) {
   	
			testCount++;
   		if ( !(p in obj) )
	  			this.REPORT( CodeLocation()+' Property '+p+' not found' );
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

	for each ( currentItem in itemList ) {

		try {
			
			Print( ' - '+currentItem.file+' - '+ (currentItem.name||'INIT?') );
			
			var globalPropertiesCount = global.__count__;
			gcZeal = conf.gcZeal;

			if( !conf.noGcBetweenTests )
				CollectGarbage();

			var m0 = privateMemoryUsage;
			var t0 = TimeCounter();

			if ( conf.nogcDuringTests )
				disableGarbageCollection = true;
				
			for ( var i = 0; i < conf.repeatEachTest; i++ )
				currentItem.func(QAAPI, currentItem);

			if ( conf.nogcDuringTests )
				disableGarbageCollection = false;

			var t = (TimeCounter() - t0) / conf.repeatEachTest;
			var m = (privateMemoryUsage - m0) / conf.repeatEachTest;
			Print( '  ('+t.toFixed(2) + 'ms / '+(m/1024)+'KB)' );

			if ( global.__count__ != globalPropertiesCount )
				QAAPI.REPORT( 'WARNING '+ currentItem.file +':'+ currentItem.line + ' ('+currentItem.name+') is using a global variable.' );
				
		} catch(ex) {

			var lineno = ex.lineNumber - currentItem.relativeLineNumber + currentItem.line;
			if ( isNaN(lineno) )
				lineno = '('+String(ex.lineNumber) +'-'+ (currentItem.relativeLineNumber + currentItem.line) + ')';
			QAAPI.REPORT( 'EXCEPTION '+ ex + ' at ' + currentItem.file + ':' + lineno + ' ('+currentItem.name+')' );
		}
		
		disableGarbageCollection = false; // in case of
		gcZeal = 0;

		Print( '\n' );

		if ( endSignal )
			break;
	}
}


function LaunchRandomTests(itemList, conf) {

	for each ( var item in itemList ) {

		if ( !item.init ) // list is sorted, init are first.
			break;
		item.func(QAAPI, currentItem);
	}

	while ( !endSignal ) {

		currentItem = itemList[Math.floor(Math.random() * itemList.length)];
		
		gcZeal = conf.gcZeal;

		try {
			
			for ( var i = 0; i < conf.repeatEachTest; i++ )
				currentItem.func(QAAPI, currentItem);

			if( !conf.noGcBetweenTests )
				CollectGarbage();
			
		} catch(ex) {

			var lineno = ex.lineNumber - currentItem.relativeLineNumber + currentItem.line;
			if ( isNaN(lineno) )
				lineno = '('+String(ex.lineNumber) +'-'+ (currentItem.relativeLineNumber + currentItem.line) + ')';
			QAAPI.REPORT( 'EXCEPTION '+ ex + ' at ' + currentItem.file + ':' + lineno + ' ('+currentItem.name+')' );
		}

		disableGarbageCollection = false; // in case of
		gcZeal = 0;
	}
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



var conf = { help:false, repeatEachTest:1, gcZeal:0, loopForever:false, directory:'src', priority:0, flags:'', save:'', load:'', disableJIT:false, listTestsOnly:false, nogcBetweenTests:false, nogcDuringTests:false };
ParseCommandLine(conf);
Print( 'configuraion: '+[k+'='+v for ([k,v] in Iterator(conf))].join(' / '), '\n\n' );

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
	
	Print([String.quote(t.file+' - '+t.name) for each ( t in testList )].join('\n'), '\n');
	Halt();
}

if ( conf.save )
	new File(conf.save).content = uneval(testList);

if ( conf.disableJIT )
	DisableJIT();

var savePrio = processPriority;
processPriority = conf.priority;
var t0 = TimeCounter();

( conf.loopForever ? LaunchRandomTests : LaunchTests )(testList, conf);

var t = TimeCounter() - t0;
processPriority = savePrio || 0; // savePrio may be undefined

Print( '\n\n', issues + ' issues / ' + testCount + ' tests in ' + t.toFixed(2) + 'ms ('+(conf.repeatEachTest)+' repeat).', '\n' );
errors.sort();
errors.reduce( function(previousValue, currentValue, index, array) {

    if ( previousValue != currentValue )
		Print( ' X ' + currentValue, '\n' );
    return currentValue;
}, undefined);


/* flags:

	'd' for desactivated: the test is disabled.
	'f' for fast: the test execution is fast. Time should be less that 10ms.
	't' for time: the test execution time is always the same. The test do not use any variable-execution-time function (CollectGarbage, Poll, Socket, ...)
	'r' for reliable: external parameters (like the platform, CPU load, TCP/IP connection, weather, ...) cannot make the test to fail.
	'm' for low memory usage. The test uses the minimum amount of memory in the script part. no QA.RandomString(300000000) or StringRepeat('x', 10000000000)
*/
