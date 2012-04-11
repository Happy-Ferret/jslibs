loadModule = host.loadModule;
loadModule('jsstd');
loadModule('jsio');
loadModule('jsdebug');

function pad(val, width, prefix) {

    var str = String(val);
    width -= str.length;
    if ( width > 0 )
        return new Array(width+1).join(prefix) + str;
    return str;
}

function formatVal(val) {
	
	if ( val === undefined )
		return 'undefined';
	val = uneval(val);
	if ( val.length > 40 )
		val = (val.substr(0, 20)+'...'+val.length+'...'+val.substr(-20)).quote();
	return val;
}

function isnan(val) {

	return (typeof(val) == 'number') && isNaN(val);
}

function isException(ex, ref) {

	if ( ex === ref )
		return true;
	if ( typeof(ref) != 'string' )
		return !( ex != ref && ex.constructor != ref && !(ex instanceof ref) );
	else
		return !( String(ex) != ('[object '+ref+']') && ( ex.constructor.name != ref ) )
}

function testOp( left, op, right ) {

	var res;
	switch ( op ) {
		case '==': res = (left == right) || (isnan(left) && isnan(right)); break;
		case '===': res = (left === right) || (isnan(left) && isnan(right)); break;
		case '!=': res = (left != right) && (!isnan(left) || !isnan(right)); break;
		case '!==': res = (left !== right) && (!isnan(left) || !isnan(right)); break;
		case '>': res = (left > right); break;
		case '<': res = (left < right); break;
		case '>=': res = (left >= right); break;
		case '<=': res = (left <= right); break;
		case 'has': res = (right in left); break;
		case '!has': res = (!(right in left)); break;
		case 'in': res = (left in right); break;
		case '!in': res = (!(left in right)); break;
		case 'instanceof': res = (left instanceof right); break;
		case '!instanceof': res = (!(left instanceof right)); break;
		case 'typeof': res = (typeof(left) == String(right)); break;
		case '!typeof': res = (typeof(left) != String(right)); break;
		case 'ex':
			try {
				void left();
				res = (right === undefined);
			} catch (ex) {
				res = (right !== undefined) && isException(ex, right);
			}
			break;
		default: res = undefined;
	}
	return res;
}

////////////////////////////////////////////////////////////////////////////////
// QA API

function QAAPI(cx) {

	this.cx = cx; // expose the current context

	this.NO_CRASH = function() {
	}

	this.FAILED = function( message ) {

		cx.checkpoint('FAILED');
		cx.reportIssue('FAILED', message);
	}
	
	this.ASSERTOP = function( left, op, right, testName ) {

		cx.checkpoint('ASSERTOP', testName);

		var res = testOp(left, op, right);
		if ( res === undefined ) {

			cx.reportIssue('ASSERTOP: unknown operator "'+op+'"');
		} else if ( res !== true ) {

			cx.reportIssue(formatVal(left)+' !' + op +' '+formatVal(right), testName);
		}
	}

	this.ASSERT = function( value, expect, testName ) {
		
		cx.checkpoint('ASSERT', testName);

		if ( !testOp(value, '===', expect) )
			cx.reportIssue(formatVal(value)+' ! === '+formatVal(expect), testName);
	}

	this.ASSERT_STR = function( value, expect, testName ) {
	
		cx.checkpoint('ASSERT_STR', testName);

		value = stringify(value);
		expect = stringify(expect);

		if ( !testOp(value, '==', expect) )
			cx.reportIssue(formatVal(value)+' ! == '+formatVal(expect), testName);
	}

	// tools

	this.gc = function() {

		if ( !cx.cfg.nogcDuringTests )
			collectGarbage();
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


function addQaItemListFromSource(itemList, startDir, files) {

	var hidden = regexec(/\/\./);
	var srcFile = regexec(new RegExp(files));
	var qaExpr = /\/\*\*qa([^]*?)(?:\n([^]*?))?\*\*\//g;

	function linesBefore(str) {

        var count = 1, pos = 0;
        for ( ; (pos = str.indexOf('\n', pos)) != -1; ++count, ++pos );
        return count;
    }

	var newItemList = [];
	recursiveDir( startDir, function(file) {
	
		if ( !hidden(file.name) && srcFile(file.name) ) {

			print('.');

			var source = stringify(file.content);
			source = source.replace(/\r\n|\r/g, '\n'); // cleanup
			
			qaExpr.lastIndex = 0; // The index at which to start the next match.
			var res, item;
			while( (res = qaExpr.exec(source)) ) {
				
				var startPos = qaExpr.lastIndex - res[0].length;

				if ( item ) // adjust the previous followingTextEnd
					item.followingSourceTextEnd = startPos;

				var item = {};
				item.path = file.name.substr(0, file.name.lastIndexOf('/'));
				item.lastDir = item.path.substr(item.path.lastIndexOf('/')+1);
				item.fileName = file.name.substr(file.name.lastIndexOf('/')+1);
				item.source = source;
				item.followingSourceTextStart = qaExpr.lastIndex;
				item.followingSourceTextEnd = source.length;
				var iName = /DEFINE_(\w*)\( *(\w*) *\)/.exec(item.source.substring(item.followingSourceTextStart, item.followingSourceTextEnd));
				//item.name = res[1];
				item.name = item.lastDir + ':' + (iName ? (iName[2] || iName[1]) : '???');
				item.file = file.name;
				item.line = linesBefore(source.substr(0, startPos)) + 1;
				item.code = res[2]||'';
				item.flags = '';
				newItemList.push(item);
			}
		}
	});
	Array.prototype.push.apply(itemList, newItemList);
}



function addQaItemList(itemList, startDir, files) {

	var hidden = regexec(/\/\./);
	var qaFile = regexec(new RegExp(files));
	var newQaItem = regexec(/^\/\/\/\s*(.*?)\s*$/);
	var parseFlags = function(str) (/\[(.*?)\]/.exec(str)||[,''])[1];

	var index = 0;
	var newItemList = [];

	recursiveDir( startDir, function(file) {
	
		if ( !hidden(file.name) && qaFile(file.name) ) {

			print('.');

			var source = stringify(file.content);
			source = source.replace(regexec(/\r\n|\r/g), '\n'); // cleanup
			
			var lines = source.split('\n');
			
			var item = { // initialization item
				name: '[INIT]',
				path: file.name.substr(0, file.name.lastIndexOf('/')),
				file: file.name,
				line: 1,
				flags: '',
				code: [],
				init: true
			};
			
			for ( var l in lines ) {
				
				var res = newQaItem(lines[l]);
				if ( res ) {
					
					newItemList.push(item);
					item = {
						name: res[1],
						path: file.name.substr(0, file.name.lastIndexOf('/')),
						file: file.name,
						line: Number(l)+1,
						flags: parseFlags(res[1]),
						code: [],
					};
				}
				item.code.push(lines[l]);
			}
			newItemList.push(item);
		}
	});

	for each ( var item in newItemList )
		item.code = item.code.join('\n');

	Array.prototype.push.apply(itemList, newItemList);
}



function filterQaItemList(itemList, include, exclude, flags) {

	var newItemList = [ item for each ( item in itemList ) if (  item.init || (include?include(item.name)||include(item.file):true) && !(exclude?exclude(item.name)||exclude(item.file):false) && (!flags || flags(item.flags))  ) ];
	itemList.splice(0, itemList.length);
	Array.prototype.push.apply(itemList, newItemList);
}


function compileTests(itemList) {

	for each ( var item in itemList ) {
		
		try {

			item.relativeLineNumber = locate()[1]+1 - item.line;
			item.func = new Function('QA', item.code);
		} catch(ex) {
			
			item.func = function() {}
			var lineno = ex.lineNumber - item.relativeLineNumber;
			var message = 'COMPILATION: @'+ item.file +':'+ lineno +' - '+ item.name +' - '+ ex + ' : ' + item.code;
			throw message;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// tools

function parseCommandLine(cfg) {

	var args = host.arguments;
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

function reportInfo(cx, type, location, testName, checkName, details) {

	var message = type +' @'+ location +' "'+ (testName||'') +'", "'+ (checkName||'') +'" : '+ details;

	if ( type != 'CHECK' )
		cx.issueList.push(message);
	
	print( '\n  '+ message );
	
	if ( cx.cfg.logFilename ) {
	
		logFile = new File(cfg.logFilename);
		logFile.open('a+');
		logFile.write('\n' + message);
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
			
			reportInfo(this, 'ASSERT', this.item.file+':'+(locate(this.stackIndex+1)[1] - this.item.relativeLineNumber), this.item.name, checkName, message);
		},
		checkpoint:function(title, testName) {
			
			this.checkCount++;
			if ( cfg.verbose )
				reportInfo(this, 'CHECK', this.item.file+':'+(locate(this.stackIndex+1)[1] - this.item.relativeLineNumber), this.item.name, testName, title);
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

		if ( testCount >= cfg.stopAfterNTests )
			break;

		if ( cfg.loopForever )
			testIndex = Math.floor(Math.random() * itemList.length);

		cx.item = itemList[testIndex];

		if ( cx.item.init || cfg.runOnlyTestIndex == undefined || cfg.runOnlyTestIndex == testIndex ) {
			
			if ( !cfg.quiet )
				print( pad(testIndex, 4, ' ')+' - '+cx.item.file+':'+cx.item.line+' - '+cx.item.name+' ' );

			if ( cfg.gcZeal )
				gcZeal = cfg.gcZeal;

			cfg.nogcBetweenTests || collectGarbage();
//			disableGarbageCollection = cfg.nogcDuringTests;

			try {

				var t0 = timeCounter();
				for ( var i = cfg.repeatEachTest; i && !host.endSignal ; --i ) {

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
				cfg.quiet || print( ' ...'+(t1/cfg.repeatEachTest).toFixed(1) + 'ms' );
			} catch(ex) {

				reportInfo(cx, 'EXCEPTION', cx.item.file+':'+(ex.lineNumber - cx.item.relativeLineNumber), cx.item.name, '', ex);
			}

			if ( cfg.gcZeal )
				gcZeal = 0;

//			disableGarbageCollection = cfg.nogcBetweenTests;
			cfg.nogcBetweenTests || collectGarbage();
			
			cfg.quiet || print('\n');

		}

		if ( host.endSignal )
			break;

		if ( cx.issueList.length >= cfg.stopAfterNIssues )
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

	var stdout = host.stdout;
	var stderr = host.stderr;
	delete host.stdout;
	delete host.stderr;

	setPerfTestMode();
	collectGarbage();
//	disableGarbageCollection = true;
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

	host.stdout = stdout;
	host.stderr = stderr;

	var initSrc = [ '!'+item.func.toSource()+'();' for each ( item in itemList ) if ( item.init ) ];
	var initCount = initSrc.length;

	stdout( 'saving '+(itemList.length-initCount)+' items\n' );

	var testSrcList = [ item.func for each ( item in itemList ) if ( !item.init ) ];


	var perfTestFunction = function(qaapi, testList) {
	
		loadModule("jsstd");
		loadModule("jsdebug");
		var prev_stdout = host.stdout;
		var prev_stderr = host.stderr;
		delete host.stdout;
		delete host.stderr;
			
		setPerfTestMode();
		collectGarbage();
//		disableGarbageCollection = true;

		var t = timeCounter();
		var err = timeCounter() - t;

		var t = timeCounter();
		var len = testList.length;
		for ( var i = 0; i < len; ++i )
			testList[i](qaapi);
		t = timeCounter() - t - err;

		host.stdout = prev_stdout;
		host.stderr = prev_stderr;
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
		directory:'..', 
		files:'_qa\\.js$', 
		inlineOnly:false,
		priority:0, 
		flags:'', 
		export:'', 
		save:'', 
		load:'', 
		disableJIT:false, 
		listTestsOnly:false, 
		nogcBetweenTests:false, 
		nogcDuringTests:false, 
		stopAfterNIssues:Infinity, 
		stopAfterNTests:Infinity, 
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
	if ( cfg.load ) {
	
		testList = eval(new File(cfg.load).content);
	} else {

		print('Building');

		var testList = [];
		
		if ( !cfg.inlineOnly )
			addQaItemList(testList, cfg.directory, cfg.files);
			
		addQaItemListFromSource(testList, cfg.directory, '\\.cpp$');
		
		filterQaItemList(testList, itemInclude, itemExclude, matchFlags);
		testList = testList.sort( function(a,b) a.init ? -1 : 1 ); // put all init function at the top of the test list.
		compileTests(testList);
	}

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

	print('\n', 'Testing...', '\n');

	var savePrio = processPriority;
	processPriority = cfg.priority;
	var t0 = timeCounter();

	var [issueList, checkCount] = launchTests(testList, cfg);

	var t = timeCounter() - t0;
	processPriority = savePrio || 0; // savePrio may be undefined

	var separator = stringRepeat('-',79);

	print('\n', separator);
	print('\n', configurationText);
	print('\n', separator);
	print('\n', pad(issueList.length, 4, ' ') +' issues ('+cfg.repeatEachTest+'x '+ [t for each (t in testList) if (!t.init)].length +' tests = ' + checkCount + ' checks in ' + t.toFixed(2) + 'ms)');
	print('\n', separator);
	print('\n');
	issueList.sort();
	issueList.reduce( function(previousValue, currentValue, index, array) {

		 if ( previousValue != currentValue )
			print( ' ' + currentValue, '\n' );
		 return currentValue;
	}, undefined );
}

try {
	
	main();
} catch (ex) {
	
	print(uneval(ex));
}

print('\n', 'End.', '\n');


////////////////////////////////////////////////////////////////////////////////
// flags:
//
//	'd' for desactivated: the test is disabled.
//	'f' for fast: the test execution is fast. Time should be less that 10ms.
//	't' for time: the test execution time is always the same. The test do not use any variable-execution-time function (CollectGarbage, Poll, Socket, ...)
//	'r' for reliable: external parameters (like the platform, CPU load, TCP/IP connection, weather, ...) cannot make the test to fail.
//	'm' for low memory usage. The test uses the minimum amount of memory in the script part. no QA.randomString(300000000) or StringRepeat('x', 10000000000)
