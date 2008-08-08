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
		
			item.relativeLineNumber = LocateLine(0); item.func = new Function('QA', 'ITEM', item.code.join('\n'));
		} catch(ex) {
			
			Print( ex + ' at ' + item.file + ':' + (item.line + ex.lineNumber - item.relativeLineNumber) + ' ('+item.name+')', '\n' );
		}
	}

	itemList = [ item for each ( item in itemList ) if ( !filter || item.init || ( filter(item.name) && !item.flags.d ) ) ];
	itemList = itemList.sort( function(a,b) a.init ? -1 : 1 );
	return itemList;
}


var currentItem;
var issues = 0;
var testCount = 0;
var errors = [];

var QAAPI = new function() {

	function CodeLocation() {
		
		return currentItem.file+':'+( LocateLine(-2) - currentItem.relativeLineNumber + currentItem.line);
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
			this.REPORT( CodeLocation()+' Failure not detected: '+testName );
		} catch(ex if ex instanceof exType) {
			// good
		} catch(ex) {
			
			this.REPORT( CodeLocation()+' Invalid exception ('+ex.constructor.name+' != '+exType.constructor.name+') for: '+testName );
		}
	} 

	this.ASSERT = function( value, expect, testName ) {

		testCount++;
		if ( value !== expect )
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

		 var str = '';
		 for ( ; str.length < length; str += Math.random().toString(36).substr(2) );
		 return str.substr(0, length);
   }
}


function LaunchTests(itemList, repeat) {

	if ( repeat == undefined )
		repeat = 1;

	for each ( currentItem in itemList ) {

		try {
			
			Print( currentItem.name, '\n' );
			
			for ( var i = 0; i < repeat; i++ )
				currentItem.func(QAAPI, currentItem);
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
			
			currentItem.func(QAAPI, currentItem);
		} catch(ex) {

			Print( ex + ' at ' + currentItem.file + ':' + (ex.lineNumber - currentItem.relativeLineNumber + currentItem.line) + ' ('+currentItem.name+')' );
			Print('\n');
		}
	}
}



_configuration.unsafeMode = true;

var savePrio = processPriority;
processPriority = 2;

var t0 = TimeCounter();

var repeat = 4;

if ( arguments[1] == '-r' ) {
	
	LaunchRandomTests(CreateQaItemList('src', undefined));

	repeat = 1;	// ...

} else {

	LaunchTests(CreateQaItemList('src', new RegExp(arguments[1] || '.*', 'i')), repeat);
}

var t = TimeCounter() - t0;

processPriority = savePrio || 0; // savePrio may be undefined


Print( '\n', issues + ' issues / ' + testCount + ' tests in ' + t.toFixed(2) + 'ms ('+repeat+' repeat).\n' );
errors.sort();
errors.reduce( function(previousValue, currentValue, index, array) {

    if ( previousValue != currentValue )
		Print( '- ' + currentValue, '\n' );
    return currentValue;
}, undefined);


/* flags:

	d for desactivated: the test is disabled.
	f for fast: the test execution is fast. Time should be less that 10ms.
	t for time: the test execution time is always the same. The test do not use any variable-execution-time function (CollectGarbage, Poll, Socket, ...)
	r for reliable: external parameters (like the platform, CPU load, TCP/IP connection, weather, ...) cannot make the test to fail.

*/
