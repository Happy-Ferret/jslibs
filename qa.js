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


function CreateQaItemList(startDir, filter) {

	var hidden = /\/\./;
	var qaFile = /\qa.js$/;

	var newQaItem = /^\/\/\/\s*(.*?)\s*$/;
	
	var GetFlags = function(str) {
		
		var flags = {};
		var res = /\[(.*?)\]/(str);
		if ( res )
			for each ( f in res[1] )
				flags[f] = true;
		return flags;
	}

	var itemList = [];
	var index = 0;

	RecursiveDir( startDir, function(file) {
	
		if ( !hidden(file.name) && qaFile(file.name) ) {

			var source = String(file.content);
			source = source.replace(/\r\n|\r/g, '\n'); // cleanup
			
			var lines = source.split('\n');
			
			var item = { file:file.name, line:1, flags:'', code:[], init:true }; // initialization item
			
			for ( var l in lines ) {
				
				var res = newQaItem(lines[l]);
				if ( res ) {
					
					itemList.push(item);
					item = { file:file.name, line:Number(l)+1, name:res[1], flags:GetFlags(res[1]), code:[] };
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
		
//		if ( !item.func )
//			Print( 'Unable to create the function', '\n' );

		
	}

	itemList = [ item for each ( item in itemList ) if ( !filter || item.init || ( (filter(item.file) || filter(item.name)) && !item.flags.d ) ) ];
	itemList = itemList.sort( function(a,b) a.init ? -1 : 1 );
	return itemList;
}


var currentItem;
var issues = 0;
var testCount = 0;
var errors = [];

var QAAPI = new function() {

	function CodeLocation() {
		
		return currentItem.file+':'+( Locate(-2)[1] - currentItem.relativeLineNumber + currentItem.line);
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
	
			if ( !(ex instanceof exType) )
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
		CollectGarbage();
	}

   this.RandomString = function(length) { // [0-9A-Za-z]
		
/*		
		var rndData = [], rndLen = 0;
		while( rndLen < length ) {

			var rnd = String(Math.random());
			rndData.push(rnd);
			rndLen += rnd.length;
		}
		return rndData.join('').substr(0, length);
*/
		return 'a6z5er46az54vraz6e54raz';
   }
}


function LaunchTests(itemList, conf) {

	for each ( currentItem in itemList ) {

		try {
			
			currentItem.name && Print( ' - '+currentItem.file+' - '+currentItem.name, '\n' );
			
			gcZeal = conf.gcZeal;
			for ( var i = 0; i < conf.repeat; i++ )
				currentItem.func(QAAPI, currentItem);
			gcZeal = 0;
			CollectGarbage();
		} catch(ex) {

			QAAPI.REPORT( ex + ' at ' + currentItem.file + ':' + (ex.lineNumber - currentItem.relativeLineNumber + currentItem.line) + ' ('+currentItem.name+')' );
		}

		if ( endSignal )
			break;
	}
}


function LaunchRandomTests(itemList) {

	for each ( var item in itemList ) {

		if ( item.init )
			item.func(QAAPI, currentItem);
		else
			break; // list is sorted, init are first.
	}

	while ( !endSignal ) {

		currentItem = itemList[Math.floor(Math.random() * itemList.length)];
		
		if ( currentItem.flags.d || !currentItem.flags.f )
			continue;

		try {
			
			gcZeal = conf.gcZeal;
			currentItem.func(QAAPI, currentItem);
			gcZeal = 0;
		} catch(ex) {

			Print( ex + ' at ' + currentItem.file + ':' + (ex.lineNumber - currentItem.relativeLineNumber + currentItem.line) + ' ('+currentItem.name+')' );
			Print('\n');
		}
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
		var dbldash = (args[1][1] == '-');
		var item = [ c for (c in conf) if ( dbldash && c == args[1].substr(2) || !dbldash && c[0] == args[1][1] ) ][0];
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



var conf = { help:false, repeat:1, gcZeal:0, loop:false, dir:'src', prio:0, exclude:'', flags:'' };
ParseCommandLine(conf);
Print( 'configuraion: '+[k+'='+v for ([k,v] in Iterator(conf))].join(' / '), '\n' );
if ( conf.help )
	Halt();

var savePrio = processPriority;
processPriority = conf.prio;
var t0 = TimeCounter();

if ( conf.loop )
	LaunchRandomTests(CreateQaItemList(conf.dir, undefined), conf);
else
	LaunchTests(CreateQaItemList(conf.dir, new RegExp(conf.args[0] || '.*', 'i')), conf);

var t = TimeCounter() - t0;
processPriority = savePrio || 0; // savePrio may be undefined


Print( '\n', issues + ' issues / ' + testCount + ' tests in ' + t.toFixed(2) + 'ms ('+(conf.repeat)+' repeat).', '\n' );
errors.sort();
errors.reduce( function(previousValue, currentValue, index, array) {

    if ( previousValue != currentValue )
		Print( ' X ' + currentValue, '\n' );
    return currentValue;
}, undefined);


/* flags:

	d for desactivated: the test is disabled.
	f for fast: the test execution is fast. Time should be less that 10ms.
	t for time: the test execution time is always the same. The test do not use any variable-execution-time function (CollectGarbage, Poll, Socket, ...)
	r for reliable: external parameters (like the platform, CPU load, TCP/IP connection, weather, ...) cannot make the test to fail.

*/
