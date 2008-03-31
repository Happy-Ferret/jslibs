_configuration.unsafeMode = true;

LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsdebug');


function MakeTestList(directory) {

	var dirList = Directory.List(directory, Directory.SKIP_BOTH | Directory.SKIP_FILE | Directory.SKIP_OTHER );
	dirList.sort();

	var testList = {};
	for each ( var dirName in dirList ) {

		var f = new File(dirName + '/qa.js');
		if ( f.exist ) {

			var qatests = Exec(f.name, false);
			for ( var testName in qatests )
				testList[ dirName + ':' + testName ] = qatests[testName];
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

		if ( !filter(testName) || testName[0] == '_' )
			continue;
		Print( testName, '\n' );
		for ( var i = 0; i<iterate; i++ ) {

			testList[testName](QAAPI);
			CollectGarbage();
			if ( endSignal )
				return;
		}
	}
}


var QAAPI = new function() {
	
	this.issues = 0;

	this.REPORT = function( message ) {

		this.issues++;
		Print( ' - ' + message, '\n' );
	}

	this.ASSERT_TYPE = function( value, type, testName ) {
		
		if ( typeof(value) != type && !(value instanceof type) ) {
			
			this.REPORT( (testName||'') + ' (@'+Locate(-1)+'), Invalid type, '+(type.name)+' is expected' );
		}
	}

	this.FAILED = function( message ) {

		this.REPORT( message );
	}
	
	this.ASSERT_EXCEPTION = function( fct, exType, message ) {
		
		try {
		
			fct();
			this.REPORT( 'Failure not detected: '+message );
		} catch(ex if ex instanceof exType) {

			// good
		} catch(ex) {
			
			this.REPORT( 'Invalid exception ('+ex.constructor.name+') for: '+message );
		}
	} 


	this.ASSERT = function( value, expect, testName ) {

		if ( value !== expect ) {
		
			value = '('+typeof(value)+')'+ String(value).substr(0,50).quote()+'...';
			expect = '('+typeof(expect)+')'+ String(expect).substr(0,50).quote()+'...';
			this.REPORT( (testName||'') + ', (@'+Locate(-1)+'), '+value+' != '+expect );
		}
	}

   this.ASSERT_HAS_PROPERTIES = function( obj, names ) {
   	
   	for each ( var p in names.split(/\s*,\s*/) )
   		if ( !(p in obj) )
	  			this.REPORT( 'property '+p+' not found' );
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

var perfTest = false;

if ( perfTest ) {

	var t0 = TimeCounter();
	var savePrio = processPriority;
	processPriority = 2;
} else {

	processPriority = -1;
}

MakeTests(MakeTestList('.'), new RegExp(arguments[1]||'.*', 'i'), QAAPI, 3);

if ( perfTest ) {

	var t = TimeCounter() - t0;
	
	Print( 'Time: '+t.toFixed(2) + 'ms.' );
	
}
processPriority = savePrio;

Print( QAAPI.issues + ' issues found' );
