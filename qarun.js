_configuration.unsafeMode = true;

LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsdebug');


function MakeTestList(directory) {

	var dirList = Directory.List(directory, Directory.SKIP_BOTH | Directory.SKIP_FILE | Directory.SKIP_OTHER );
	
	dirList.sort();

	var testList = {};
	for each ( var dirName in dirList ) {

		var f = new File(directory+'/'+dirName + '/qa.js');
		if ( f.exist ) {

			var qatests = Exec(f.name, false);
			for ( var testName in qatests )
				testList[ f.name + ':' + testName ] = qatests[testName];
		}
	}
	return testList;
}



function RecursiveDir(path) {
	
	var testList = [];
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
					testList.push(file.name);
					break;
			}
		}
		dir.Close();
	})(path);
	
	return testList;
}



function MakeTests( testList, filter, QAAPI, iterate ) {

	for ( var testName in testList ) {

		if ( !filter(testName) || testName[testName.indexOf(':')+1] == '_' )
			continue;
		
		Print( testName, '\n' );
		for ( var i = 0; i<iterate; i++ ) {

			testList[testName](QAAPI, testName);
			CollectGarbage();
			if ( endSignal )
				return;
		}
	}
}


var QAAPI = new function() {
	
	this.issues = 0;
	this.testCount = 0;
	this.errors = [];

	this.REPORT = function( message ) {

		this.issues++;
		this.errors.push(message);
//		Print( ' - ' + message, '\n' );
	}

	this.ASSERT_TYPE = function( value, type, testName ) {
		
		this.testCount++;
		if ( typeof(value) != type && !(value instanceof type) )
			this.REPORT( Locate(-1)+' '+(testName||'?')+', Invalid type, '+(type.name)+' is expected' );
	}

	this.FAILED = function( message ) {

		this.REPORT( message );
	}
	
	this.ASSERT_EXCEPTION = function( fct, exType, message ) {
		
		this.testCount++;
		try {
		
			fct();
			this.REPORT( Locate(-1)+' Failure not detected: '+message );
		} catch(ex if ex instanceof exType) {

			// good
		} catch(ex) {
			
			this.REPORT( Locate(-1)+' Invalid exception ('+ex.constructor.name+' != '+exType.constructor.name+') for: '+message );
		}
	} 


	this.ASSERT = function( value, expect, testName ) {

		this.testCount++;
		if ( value !== expect ) {
		
			value = '('+(''+typeof(value)).substr(0,3)+')'+ String(value).substr(0,50).quote()+'...';
			expect = '('+(''+typeof(expect)).substr(0,3)+')'+ String(expect).substr(0,50).quote()+'...';
			this.REPORT( Locate(-1)+' '+(testName||'?') + ', '+value+' != '+expect );
		}
	}

	this.ASSERT_STR = function( value, expect, testName ) {
	
		this.testCount++;
//		value = String(value); expect = String(expect); // not needed because we use the != sign, not !== sign

		if ( value != expect ) {
		
			value = '('+(''+typeof(value)).substr(0,3)+')'+ String(value).substr(0,50).quote()+'...';
			expect = '('+(''+typeof(expect)).substr(0,3)+')'+ String(expect).substr(0,50).quote()+'...';
			this.REPORT( Locate(-1)+' '+(testName||'?') + ', '+value+' != '+expect );
		}
	}

   this.ASSERT_HAS_PROPERTIES = function( obj, names ) {
   	
   	for each ( var p in names.split(/\s*,\s*/) ) {
   	
			this.testCount++;
   		if ( !(p in obj) )
	  			this.REPORT( Locate(-1)+' Property '+p+' not found' );
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

var savePrio = processPriority;
processPriority = 2;

var t0 = TimeCounter();

MakeTests(MakeTestList('src'), new RegExp(arguments[1]||'.*', 'i'), QAAPI, 3);

var t = TimeCounter() - t0;
processPriority = savePrio || 0; // savePrio may be undefined

Print( '\n', QAAPI.issues + ' issues / ' + QAAPI.testCount + ' tests in ' + t.toFixed(2) + 'ms.\n' );
QAAPI.errors.sort();
QAAPI.errors.reduce( function(previousValue, currentValue, index, array) {

    if ( previousValue != currentValue )
		Print( '- ' + currentValue, '\n' );
    return currentValue;
}, undefined);

