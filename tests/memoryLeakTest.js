/* ***** BEGIN LICENSE BLOCK *****
 * Version: GNU GPL 2.0
 *
 * The contents of this file are subject to the
 * GNU General Public License Version 2.0; you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * http://www.gnu.org/licenses/gpl.html
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * ***** END LICENSE BLOCK ***** */

LoadModule('jsstd'); 
LoadModule('jsio'); 

Exec('tools.js');
Exec('io.js');

var _time0 = Now();
function FormatedTime() StringPad(((Now()-_time0)/SECOND).toFixed(2), 7, ' ');
var thisSession = 'jsircbot_'+(Now())+'.log'; // used to create ONE log file by session
log.AddFilter( MakeLogFile(function() thisSession, false), LOG_ALL );
//log.AddFilter( MakeLogScreen(), LOG_FAILURE | LOG_ERROR | LOG_WARNING );


	var n = 0;


	StartAsyncProc( new function() {
		
		
		for (;;) {
			
			var [status, statusCode, reasonPhrase, headers, response] = yield function(cb) HttpRequest( 'http://localhost:8080/', undefined, 1, cb );

//			yield function(cb) TCPGet( 'localhost', 8080, 'abcd', 1, cb );
//			yield function(cb) UDPGet( 'localhost', 6789, 'abcd', 1, cb );
			



//			Print( status );
/*			
			if ( status == OK ) {

				b.Write(response);

				var tmp = b.Read( RandomRange(0, b.length/2) );
				b.Read( RandomRange(0, b.length) );
				b.Unread(tmp);
			}
*/
		}
	});


	var s1 = new Socket( Socket.UDP );
	s1.nonblocking = true;
	io.AddDescriptor(s1);
	s1.Bind( 6789, '127.0.0.1' );
	s1.readable = function(s) {
		
		var [data, ip, port] = s.RecvFrom();
//		var b = new Buffer();
//		b.Write(data);
	};

	var dataToSend = RandomString(8192);
	!function() {
	
		Socket.SendTo('localhost', 6789, dataToSend);
		io.AddTimeout(1, arguments.callee );
	}();

/*
	//UDPGet

	var s2 = new Socket( Socket.UDP );
	s2.nonblocking = true;
	io.AddDescriptor(s2);
	s2.Connect( 'localhost', 6789 );
	s2.writable = function(s) s.Write( dataToSend );
*/


/*

var proc1 = new AsyncProcHelper( function() {

	try {
	
		for (;;)
			yield AsyncSleep(1);

	} finally {

	}
} );


var proc2 = new AsyncProcHelper( function() {
	
	for (;;) {

		yield AsyncSleep(10);
		proc1.Start();
		yield AsyncSleep(10);
		proc1.Stop();
	}
} );

var obj = {};

proc2.Start();

*/


//UDPServer(6789, function(data) 'echo:'+data);
//UDPGet( '127.0.0.1' , 6789, 'abc', 100, function(state, data) { Print(data) } );



try {

	io.Process( function() { 
		
		if ( ++n%1000 == 0 ) {
			
			CollectGarbage();
			Print((currentMemoryUsage/1024).toFixed()+'  '+(peakMemoryUsage/1024).toFixed(), '\n');
		}
		return endSignal;
	} );
} catch(ex if ex instanceof IoError) {

	Print( ex.text );
}

/*
for (;!endSignal;) {

	for ( var i = 0; i < 10000; i++ ) {
		
		try {
			var [,b,c] = 'aaa bbb ccc'.split(/ /ig);
			
			ReportNotice( FormatedTime() );
			
		} finally {
		}
//		obj.toto = 123;
//		obj.toto = undefined;
	}
	
	//gc();
	CollectGarbage();
	Sleep(10);
}

*/

