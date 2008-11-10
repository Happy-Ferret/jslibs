LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jstask');


function MyTask(arg, round) {
/*
	LoadModule('jsstd');
	LoadModule('jsio');
	
	var query = WaitQuery();
	
	Reply(query+' reply');
		
	var f = new File('debug.js');
	f.Open('r');
	return 'arg-'+f.Read(30)+'...';
*/	
	throw 'myException';

	return arg+'-'+round;
}

var t1 = new Task(MyTask, -1);
//var t2 = new Task(MyTask);

t1.Run(1234);
Print( 't1 result: '+t1.resultWait, '\n' );

for ( var i=0; i<1000; i++ )
	t1.Run(i);


Print( 't1 has result: '+t1.hasResult+' / result: '+t1.resultWait, '\n' );

//t2.Run(MyTask);


//Print( t2.resultWait, '\n' );

