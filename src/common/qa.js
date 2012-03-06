loadModule('jsstd');
loadModule('jsio');
loadModule('jsdebug');


////////////////////////////////////////////////////////////////////////////////
// QA API


function QAAPI(cx) {

	function formatVariable(val) {

		if ( typeof(val) == 'string' ) {
		
			if ( val.length > 50 )
				return val.substr(0, 47).quote()+'...';
			return val.quote();
		}
		if ( val instanceof Array )
			return '['+val+']';
		return val;
	}
	

	function valueType(val) {
		
		var type = typeof(val);
		if (type == 'object' )
			return val.constructor.name;
		return type;
	}
	
	
	//	this.__defineGetter__('cx', function() cx);
	Object.defineProperty(this, 'cx', { get:function() cx });

	this.FAILED = function( message ) {

		cx.reportIssue( message, 'FAILURE' );
	}
	
	this.NO_CRASH = function() {}

	this.ASSERT_TYPE = function( value, type, testName ) {

		cx.checkpoint('ASSERT_TYPE', testName);
		if ( typeof(value) != type && !(value instanceof type) )
			cx.reportIssue( 'Invalid type, '+type.name+' is expected.', testName );
	}

	this.ASSERT_EXCEPTION = function( fct, exType, testName ) {
		
		cx.checkpoint('ASSERT_EXCEPTION', testName);
		try {
		
			fct();
			cx.reportIssue( 'Exception not detected (expect '+exType.toString()+')', testName );
		} catch(ex) {
			
			if ( typeof(exType) == 'string' ) {
			
				if ( ex.constructor.name != exType )
					cx.reportIssue('Invalid exception name ('+ex.constructor.name+' != '+exType+')', testName );
			} else {

				if ( ex != exType && ex.constructor != exType )
					cx.reportIssue('Invalid exception ('+ex.constructor.name+' != '+exType.name+')', testName );
			}
		}
	}

	this.ASSERT = function( value, expect, testName ) {

		cx.checkpoint('ASSERT', testName);
		if ( value !== expect && !(typeof(value) == 'number' && isNaN(value) && typeof(expect) == 'number' && isNaN(expect)) )
			cx.reportIssue( '=== ('+valueType(value)+')'+formatVariable(value)+', expect '+'('+valueType(expect)+')'+formatVariable(expect), testName );
	}

	this.ASSERT_STR = function( value, expect, testName ) {
	
		cx.checkpoint('ASSERT_STR', testName);
		if ( value != expect ) // value = String(value); expect = String(expect); // not needed because we use the != sign, not !== sign
			cx.reportIssue( '== '+formatVariable(value)+', expect '+formatVariable(expect), testName );
	}

   this.ASSERT_HAS_PROPERTIES = function( obj, names ) {
   	
		cx.checkpoint('ASSERT_HAS_PROPERTIES', names);
   	for each ( var p in names.split(/\s*,\s*/) ) {
   	
   		if ( !(p in obj) )
	  			cx.reportIssue( 'Property '+p.quote()+' not found.' );
	  	}
   }

	this.gc = function() {

		cx.cfg.nogcDuringTests || collectGarbage();
	}

	var randomData = '';
	for ( var i = 0; i < 1024; ++i )
		randomData += String.fromCharCode(Math.random()*255);
		
	this.randomData = function(length) {

		var data = '';
		while( data.length < length )
			data += randomData.substring( Math.random()*randomData.length, Math.random()*randomData.length );
		return data.substr(0, length);
	}

	var randomString = '';
	for ( var i = 0; i < 1024; ++i )
		randomString += Math.random().toString(36).substr(2);
   
	this.randomString = function(length) { // [0-9A-Za-z]
		
		if ( length == undefined )
			throw Error('RandomString() invalid argument');

		var data = '';
		while( data.length < length )
			data += randomString.substring( Math.random()*randomString.length, Math.random()*randomString.length );
		return data.substr(0, length);
   }
}



////////////////////////////////////////////////////////////////////////////////
// QA item creation


function recursiveDir(path, callback) {
	
	(function(path) {

		var dir = new Directory(path);
		dir.open();
		for ( var entry; ( entry = dir.read(Directory.SKIP_BOTH) ); ) {

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
		dir.close();
	})(path);
}

function regexec(regexp) regexp.exec.bind(regexp);

function createQaItemList(startDir, files, include, exclude, flags) {

	var hidden = regexec(/\/\./);
	var qaFile = regexec(new RegExp(files));
	var newQaItem = regexec(/^\/\/\/\s*(.*?)\s*$/);
	var parseFlags = function(str) (/\[(.*?)\]/.exec(str)||[,''])[1];

	var itemList = [];
	var index = 0;

	recursiveDir( startDir, function(file) {
	
		if ( !hidden(file.name) && qaFile(file.name) ) {

			var source = toString(file.content);
			source = source.replace(regexec(/\r\n|\r/g), '\n'); // cleanup
			
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
		
			var functionCode = item.code.join('\n');
		try {

			item.relativeLineNumber = locate()[1]+1 - item.line;
			item.func = new Function('QA', functionCode);
		} catch(ex) {
			
			item.func = function() {}
			var lineno = ex.lineNumber - item.relativeLineNumber;
			var message = 'COMPILATION: @'+ item.file +':'+ lineno +' - '+ item.name +' - '+ ex + ' : ' + functionCode;
			print( '*** ' + message, '\n' );
		}
	}

	itemList = [ item for each ( item in itemList ) if (  item.init || (include?include(item.name)||include(item.file):true) && !(exclude?exclude(item.name)||exclude(item.file):false) && (!flags || flags(item.flags))  ) ];
	itemList = itemList.sort( function(a,b) a.init ? -1 : 1 ); // put all init function at the top of the test list.
	return itemList;
}


////////////////////////////////////////////////////////////////////////////////
// tools

function parseCommandLine(cfg) {

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
		if ( item == undefined )
			throw Error('Invalid argument: '+args[1]);
		if ( isBoolean(cfg[item]) ) {
		
			cfg[item] = !cfg[item];
			args.splice(1,1);
			continue;
		}
		cfg[item] = args[2];
		args.splice(1,2);
	}
}


////////////////////////////////////////////////////////////////////////////////
// run tests


var globalIssueCount = 0;

function commonReportIssue(cx, type, location, testName, checkName, details) {

	globalIssueCount++;
	var message = type +' @'+ location +' - '+ (testName||'') +' - '+ (checkName||'') +' - '+ details;
	cx.issueList.push(message);
	print( '\n X '+ message, '\n' );
	
	if ( cx.cfg.logFilename ) {
	
		logFile = new File(cfg.logFilename);
		logFile.open('a+');
		logFile.write(message + '\n');
		logFile.close();
	}
}


function launchTests(itemList, cfg) {

	var exportFile;

	if ( cfg.export ) {
	
		exportFile = new File(cfg.export).open('w');
		exportFile.write("LoadModule('jsstd');var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\\n' ) } };\n");
	}

	var cx = { 
		checkCount:0, 
		issueList:[], 
		stackIndex:stackSize-1, 
		cfg:cfg, 
		reportIssue:function(message, checkName) {
		
			commonReportIssue(cx, 'ASSERT',  this.item.file+':'+(locate(this.stackIndex+1)[1] - this.item.relativeLineNumber), this.item.name, checkName, message );
		},
		checkpoint:function(title, testName) {
			
			if ( cfg.verbose )
				print( '\nCP - '+testName+': checkpoint: '+title+' ('+this.item.file+':'+(locate(this.stackIndex+1)[1] - this.item.relativeLineNumber)+')' );
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
	

	collectGarbage();

	var testIndex = 0;
	var testCount = 0;

	for (;;) {

		if ( cfg.loopForever )
			testIndex = Math.floor(Math.random() * itemList.length);

		cx.item = itemList[testIndex];
		
		if ( cx.item.init || cfg.runOnlyTestIndex == undefined || cfg.runOnlyTestIndex == testIndex ) {

			cfg.quiet || print( ' - '+testIndex+' - '+cx.item.file+':'+cx.item.line+' - '+ cx.item.name );

			if ( cfg.gcZeal )
				gcZeal = cfg.gcZeal;

			cfg.nogcBetweenTests || collectGarbage();
			disableGarbageCollection = cfg.nogcDuringTests;

			try {

				var t0 = timeCounter();
				for ( var i = cfg.repeatEachTest; i && !endSignal ; --i ) {

					++testCount;
					void cx.item.func(qaapi);
					if ( exportFile ) {

						exportFile.write('('+cx.item.func.toSource()+')(QA);\n');
						exportFile.sync();
					}
					if ( cx.item.init )
						break;
				}
				var t1 = timeCounter() - t0;
				cfg.quiet || print( ' ...('+(t1/cfg.repeatEachTest).toFixed(1) + 'ms)' );
			} catch(ex) {

				commonReportIssue(cx, 'EXCEPTION', cx.item.file+':'+(ex.lineNumber - cx.item.relativeLineNumber), cx.item.name, '', ex );
			}

			if ( cfg.gcZeal )
				gcZeal = 0;

			disableGarbageCollection = cfg.nogcBetweenTests;
			cfg.nogcBetweenTests || collectGarbage();
			
			cfg.quiet || print('\n');

		}

		if ( cfg.stopAfterNTests && (testCount >= cfg.stopAfterNTests) )
			break;

		if ( cfg.stopAfterNIssues && (globalIssueCount >= cfg.stopAfterNIssues) )
			break;

		if ( endSignal )
			break;
			
		if ( !cfg.loopForever && ++testIndex >= itemList.length )
			break;

		if ( cfg.sleepBetweenTests )
			sleep(cfg.sleepBetweenTests);
	}
	
	if ( exportFile ) {
		
		exportFile.close();
	}
	
	return [cx.issueList, cx.checkCount];
}



////////////////////////////////////////////////////////////////////////////////
// perf tests


function perfTest(itemList, cfg) {

	var i;
	var qaapi = { cx:{item:{file:cfg.perfTest}}, __noSuchMethod__:function() {}, randomData:function() 'qa_tmp_123456789', randomString:function() 'qa_tmp_abcdefghij' };

	var stdout = _host.stdout;
	var stderr = _host.stderr;
	delete _host.stdout;
	delete _host.stderr;

	setPerfTestMode();
	collectGarbage();
	disableGarbageCollection = true;
	sleep(100);

	timeCounter();
	var t = timeCounter();
	var timeError = timeCounter() - t;

	stdout('time error: '+timeError+'\n');

	function prefRefTest() {
		
		var tmp = 0.1;
		function dummy() { return i / 11.1 }
		for ( var i = 0; i < 1000; ++i )
			tmp += expand('$(A)$(B)', {A:dummy(i), B:stringRepeat('x',1)});
		return tmp;
	}
	
	prefRefTest();
	var t = timeCounter();
	prefRefTest();
	var perfRefTime = timeCounter() - t - timeError;

	stdout('ref time: '+perfRefTime+'\n');
	stdout('total items: '+itemList.length+'\n');


	var fastItems = [];
	for each ( var item in itemList ) {

		try {

			var bestTime = Infinity; 
			for ( i = 0; i < 3; ++i ) {
			
				sleep(2);
				t = timeCounter();
				void item.func(qaapi);
				t = timeCounter() - t - timeError;
				
				if ( t > perfRefTime * 1.0 ) {
					
					bestTime = Infinity;
					break;
				}
				
				if ( t < bestTime )
					bestTime = t;
			}

			if ( bestTime < perfRefTime || item.init )
				fastItems.push(item);
				
		} catch(ex) {}
	}

	stdout('fast items: '+fastItems.length+'\n');


	var timeConstantItems = [];
	for each ( var item in fastItems ) {

		try {

			var t0, diffTime = 0.0, totalTime = 0.0;
			for ( i = 0; i < 11; ++i ) {
			
				sleep(2);
				t = timeCounter();
				void item.func(qaapi);
				t = timeCounter() - t - timeError;
				
				totalTime += t;

				if ( t0 != undefined )
					diffTime += Math.abs(t0 - t);
				
				t0 = t;
			}
			
			var div = diffTime / totalTime;
			
			if ( div < 0.3 || item.init )
				timeConstantItems.push(item);
				
		} catch(ex) {}
			
	}
	itemList = timeConstantItems;


	
	stdout('time-constant items: '+itemList.length+'\n');

	_host.stdout = stdout;
	_host.stderr = stderr;

	var initSrc = [ '!'+item.func.toSource()+'();' for each ( item in itemList ) if ( item.init ) ];
	var initCount = initSrc.length;

	stdout( 'saving '+(itemList.length-initCount)+' items\n' );

	var testSrcList = [ item.func for each ( item in itemList ) if ( !item.init ) ];


	var perfTestFunction = function(qaapi, testList) {
	
		loadModule("jsstd");
		loadModule("jsdebug");
		var prev_stdout = _host.stdout;
		var prev_stderr = _host.stderr;
		delete _host.stdout;
		delete _host.stderr;
			
		setPerfTestMode();
		collectGarbage();
		disableGarbageCollection = true;

		var t = timeCounter();
		var err = timeCounter() - t;

		var t = timeCounter();
		var len = testList.length;
		for ( var i = 0; i < len; ++i )
			testList[i](qaapi);
		t = timeCounter() - t - err;

		_host.stdout = prev_stdout;
		_host.stderr = prev_stderr;
		print(len+' tests in '+ t.toFixed(4) +' ms\n');
	}

	var exportFile = new File(cfg.perfTest).open('w');
	exportFile.write( initSrc.join('\n')+'\n' );
	exportFile.write( '('+perfTestFunction.toSource()+')' );
	exportFile.write( '('+qaapi.toSource()+','+testSrcList.toSource()+')' );
	exportFile.close();
}



////////////////////////////////////////////////////////////////////////////////
// Main


function main() {

	var cfg = { // default configuration
		help:false,
		repeatEachTest:1,
		gcZeal:0, 
		loopForever:false, 
		directory:'src', 
		files:'_qa.js$', 
		priority:0, 
		flags:'', 
		export:'', 
		save:'', 
		load:'', 
		disableJIT:false, 
		listTestsOnly:false, 
		nogcBetweenTests:false, 
		nogcDuringTests:false, 
		stopAfterNIssues:0, 
		stopAfterNTests:0, 
		logFilename:'', 
		sleepBetweenTests:0,
		quiet:false, 
		verbose:false, 
		runOnlyTestIndex:undefined, 
		exclude:undefined,
		perfTest:''
	};


	parseCommandLine(cfg);

	var configurationText = 'configuraion: '+['-'+k+' '+v for ([k,v] in Iterator(cfg))].join('  ');
	print(configurationText, '\n\n');

	if ( cfg.help ) {
		
		print('(TBD)');
		return;
	}

	var itemInclude = regexec(new RegExp(cfg.args[0] || '.*', 'i'));
	var itemExclude = cfg.exclude ? regexec(new RegExp(cfg.exclude, 'i')) : undefined;


	function matchFlags(flags) {
		
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

	var testList;
	if ( cfg.load )
		testList = eval(new File(cfg.load).content);
	else
		testList = createQaItemList(cfg.directory, cfg.files, itemInclude, itemExclude, matchFlags);

	if ( cfg.listTestsOnly ) {
		
		print([String.quote(t.file+' - '+t.name) for each ( t in testList )].join('\n'), '\n', testList.length +' tests.', '\n');
		return;
	}

	if ( cfg.save )
		new File(cfg.save).content = uneval(testList);

	if ( cfg.disableJIT )
		disableJIT();
		
	if ( cfg.perfTest ) {
	
		perfTest(testList, cfg);
		return;
	}

	var savePrio = processPriority;
	processPriority = cfg.priority;
	var t0 = timeCounter();

	var [issueList, checkCount] = launchTests(testList, cfg);

	var t = timeCounter() - t0;
	processPriority = savePrio || 0; // savePrio may be undefined

	print( '\n'+stringRepeat('-',97)+'\n', configurationText, '\n\n', issueList.length +' issues, '+cfg.repeatEachTest+'x '+ [t for each (t in testList) if (!t.init)].length +' tests, ' + checkCount + ' checks in ' + t.toFixed(2) + 'ms.', '\n' );
	issueList.sort();
	issueList.reduce( function(previousValue, currentValue, index, array) {

		 if ( previousValue != currentValue )
			print( '- ' + currentValue, '\n' );
		 return currentValue;
	}, undefined );
}


main();
print('Done\n');


////////////////////////////////////////////////////////////////////////////////
// flags:
//
//	'd' for desactivated: the test is disabled.
//	'f' for fast: the test execution is fast. Time should be less that 10ms.
//	't' for time: the test execution time is always the same. The test do not use any variable-execution-time function (CollectGarbage, Poll, Socket, ...)
//	'r' for reliable: external parameters (like the platform, CPU load, TCP/IP connection, weather, ...) cannot make the test to fail.
//	'm' for low memory usage. The test uses the minimum amount of memory in the script part. no QA.randomString(300000000) or StringRepeat('x', 10000000000)
