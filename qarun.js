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


function MakeTests( testList, filter, QAAPI, iterate ) {

	var savePrio = processPriority;
	processPriority = 2;

	var t0 = TimeCounter();
	for ( var testName in testList ) {

		if ( !filter(testName) )
			continue;

		if ( testName[0] == '_' )
			continue;

		Print( testName, '\n' );
		for ( var i = 0; i<iterate; i++ ) {

			testList[testName](QAAPI);
			CollectGarbage();
			if ( endSignal )
				return;
		}
	}

	var t = TimeCounter() - t0;
	Print( QAAPI.issues + ' issues found in '+ t.toFixed(2) + 'ms.' );
	processPriority = savePrio;
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

   this.RandomString = function(length) { // [0-9A-Za-z]

		 var str = '';
		 for ( ; str.length < length; str += Math.random().toString(36).substr(2) );
		 return str.substr(0, length);
   }
}

MakeTests(MakeTestList('.'), new RegExp(arguments[1]||'.*', 'i'), QAAPI, 5);

