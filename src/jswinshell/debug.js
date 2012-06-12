var loadModule = host.loadModule;
// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  halt();

loadModule('jsio');
loadModule('jsstd');
loadModule('jswinshell');
loadModule('jssvg');



var loadModule = host.loadModule;

// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  halt();

loadModule('jswinshell');

loadModule('jsstd');
//loadModule('jssvg');

var s = new Systray();
s.icon = new Icon( 0 );

s.onmouseenter = function() {

	this.text =
		'peakMemoryUsage: '+(peakMemoryUsage/(1024*1024)).toFixed(0) + 'MB\n' +
		'privateMemoryUsage: '+(privateMemoryUsage/(1024*1024)).toFixed(0) + 'MB\n'+
		'processTime: '+processTime.toFixed(0) + 'ms\n';
}

s.onmouseup = function(button) {

	this.popupMenu([
		{label:'Quit', id:'quit'},
		null,
		{label:'run with window', id:'run', checked:registryGet('HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run', host.name) != undefined
	}]);
}

s.oncommand = function(item) {

	switch ( item.id ) {
		case 'run':
			registrySet('HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run', host.name, item.checked ? undefined : (host.path+'\\'+host.name));
			break;
		case 'quit':
			throw 0;
	}
}

var ev = s.events();
for (;;) {

	processEvents(ev);
}



throw 0;

//registrySet('HKEY_CURRENT_USER\\Software\\7-Zip\\xxx', '',  123);
//print( uneval(registryGet('HKEY_CURRENT_USER\\Software\\7-Zip\\xxx', '')), '\n' );

//registrySet('HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run', host.name, host.path+'\\'+host.name);

//registrySet('HKEY_CURRENT_USER\\Software\\7-Zip\\xxx', undefined, undefined);




//print( registrySet('HKEY_CURRENT_USER\\Software\\7-Zip\\xxx\\', undefined), '\n' );
//print( registryGet('HKEY_CURRENT_USER\\Software\\7-Zip\\xxx\\'), '\n' );


throw 0; //////////////////////////////////////////////////////////////////////


var s = new Systray();
s.icon = new Icon( 0 );

s.onmouseenter = function() {

	this.text =
		'peakMemoryUsage: '+(peakMemoryUsage/(1024*1024)).toFixed(0) + 'MB\n' +
		'privateMemoryUsage: '+(privateMemoryUsage/(1024*1024)).toFixed(0) + 'MB\n'+
		'processTime: '+processTime.toFixed(0) + 'ms\n';
}

s.onmouseup = function(button) {

	this.popupMenu(['Quit']);
}

s.oncommand = function(name) {

	if ( name == 'Quit' )
		throw 0;
}

var ev = s.events();
while ( !host.endSignal )
	processEvents( ev, host.endSignalEvents() )


throw 0; //////////////////////////////////////////////////////////////////////


var dch = directoryChangesInit('C:\\', 0x10, true); // 0x10: FILE_NOTIFY_CHANGE_LAST_WRITE

function changesNotificationFct() {

	print( directoryChangesLookup(dch).join('\n'), '\n');
}

var ev = directoryChangesEvents(dch, changesNotificationFct);

while ( !host.endSignal )
	processEvents( ev, host.endSignalEvents() );
	
	

throw 0; //////////////////////////////////////////////////////////////////////


try {

Console.onKeyDown = function() {

	print('.');
}

var ev = Console.events();

while ( !host.endSignal )
	processEvents( ev, host.endSignalEvents() );
	

} catch (ex) {
	print( uneval(ex) );
}


throw 0; //////////////////////////////////////////////////////////////////////



try {
	var fso = createComObject('Scripting.fileSystemObject');

	var tmp = fso.GetFolder('c:\\');
	for each ( var folder in tmp.SubFolders )
		print( folder.name.quote(), '\n' );


} catch(ex if ex instanceof WinError) {

	print( ex.text, ' ', ex.const, ' (', ex.code, ')', '\n' );
}


throw 0;



try {

function test() {
	var xmlhttp = createComObject("Microsoft.XMLHTTP");
	xmlhttp.open("GET", "http://www.google.com/", false);
	xmlhttp.onreadystatechange = function() { print(xmlhttp.readyState) }
//	xmlhttp.send();

//	Sleep(1000);
	print( xmlhttp.responseText );
}

test();
collectGarbage();

halt();



try {

	var fso = createComObject('Scripting.fileSystemObject');
	var file = fso.CreateTextFile("testfile.txt")

	for(var i = 1; i < 255; i++)
		file.write = 5;


} catch(ex if ex instanceof WinError) {

	print( ex.text, ' ', ex.const, ' (', ex.code, ')', '\n' );
}



halt();


var xml = '<doc><el attr="foo" attr1="bar">info</el></doc>';
var xmlReader = createComObject("Microsoft.XMLDOM");
xmlReader.loadXML(xml);
print('Output: ' + xmlReader.childNodes[0].childNodes[0].attributes[1].nodeValue); // 'foo'
print('Output: ' + xmlReader.childNodes[0].childNodes[0].childNodes[0].nodeValue); // 'Info'
halt();


//var fso = CreateComObject('MSWinsock.Winsock');

//	var fso = new ComObject('Scripting.FileSystemObject');
//	var file = fso.GetFile("foobar.txt");
//	file.Write("Hello World");

//	file.attributes();
//	file.attributes = 2;

//	file.Close();
//	print( o.CreateTextFile.dispid );
//	var a = o.test;


//var shell = new ComObject('WScript.Shell');
//shell.popup('Bonjour, tout le monde!');

//var typeLib = new ComObject('Scriptlet.TypeLib');
//print(typeLib.guid); // Should give you a new GUID for every typeLib instantiation

/*
var shell = new ComObject('WScript.shell');
var process = shell.exec('test.bat');
stdoutput = process.stdOut.readAll();
print('out: ',stdoutput); // Should output the stdout of the dir command
*/

//var shell = new ComObject('WScript.Shell');
//shell.exec('calc.exe');



	print('\nDone.\n');

} catch(ex if ex instanceof WinError) {
	

	print( ex.text, '\n' );
}


halt(); //////////////////////////////////////////////////////////////////////



//throw 0;


//loadModule('jsdebug'); gcZeal = 2;

//var f = new File('C:\\MSDOS.SYS').Open('r');
var f = new File("C:\\tmp");//.Open('r'); // MYDOCUMENTS

print(GUIDToString(f.id).quote());

halt();


var iconRed = new SVG();
iconRed.write(<svg><circle cx="8" cy="8" r="5" fill="orange"/></svg>);
iconRed = new Icon( iconRed.renderImage(16,16,3) );

var iconGreen = new SVG();
iconGreen.write(<svg><circle cx="8" cy="8" r="5" fill="lightgreen"/></svg>);
iconGreen = new Icon( iconGreen.renderImage(16,16,3) );

const SECOND = 1000;
const MINUTE = 60*SECOND;
const HOUR = 60*MINUTE;

function now() {
	
	return +new Date();
}

function today() {
	
	var now = new Date();
	var start = +new Date(now.getFullYear(), now.getMonth(), now.getDate());
	return [start, start + 86400000-1];
}

function toHMS(time, elts) {
	
	var res = '';
	var h = Math.floor(time / HOUR);
	if ( h > 0 )
		res += h+'h';
	time -= h * HOUR;
	var m = Math.floor(time / MINUTE);
	if ( m > 0 )
		res += ' '+m+'\'';
	time -= m * MINUTE;
	var s = Math.floor(time / SECOND);
	res += ' '+s+'"';
	return res;
}

var auto = true;
var events = [];

var systray = new Systray();
systray.icon = iconGreen; //systray.icon = new Icon( 0 );
systray.onmousedown = function( button ) {

	var menu = [];
	var pauseMenu = [];
	
	if ( button == 2 )
		systray.popupMenu(['Quit!']);

	var pauseList = {};	
	var today = today();
	var out = false;
	var time = now();
	
	var total = 0;
	
	for ( var i = 0; i < events.length; ++i ) {
		
		var state = events[i][0];
		var evtime = events[i][1];

		if ( evtime < today[0] || evtime > today[1] || state == 2 )
			continue;
			
		if ( state == 1 ) {
		
			for ( var j = i; j < events.length && events[j][0] != 0; ++j );
			var duration = (events[j] ? events[j][1] : now()) - evtime;
			total += duration;
			var d = new Date(evtime);
			pauseMenu.unshift({id:'p'+i, label:toHMS(duration)+' @ '+d.getHours()+':'+d.getMinutes()});
		}
			
		if ( (state == 1) == out ) {
			
			time += (state ? 1 : -1) * evtime;
			out = !out;
		}
	}
	
	if ( state == 1 ) {
		
		time -= now();
	}
	
	menu.push(toHMS(time));
	menu.push({label:'Idle ('+toHMS(total)+')', popup:pauseMenu});
	menu.push({label:'Force', icon:iconRed});
	menu.push({label:'Auto', icon:iconGreen});
	menu.push(null);
	menu.push('Quit');
	systray.popupMenu(menu);
}


systray.oncommand = function( item, button ) {

	var id = item instanceof Object ? item.id || item.label : item;
	
	if ( button == 2 && id[0] == 'p' ) {
		
		events[Number(id.substr(1))][0] = 2;
	} else {
	
		switch (id) {
			case 'Force':
				idle(true, now());
				auto = false;
				break
			case 'Auto':
				idle(false, now());
				auto = true;
				break;
			case 'Quit':
				if ( messageBox( 'Quit ?', 'Question', MB_YESNO) == IDYES )
					throw 'end';
				break;
			case 'Quit!':
				throw 'end';
		}
	}
}

/*
var lastMoveUpdate;
systray.onmousemove = function(x,y) {
	
	var now = now();
	if ( lastMoveUpdate + 1000 > now ) // note: undefined > 1234 == false
		return;
	lastMoveUpdate = now;
	systray.text = toHMS(getTime());
}
*/ 

systray.onclose = function() {

	throw 'end';
}

function idle(polarity, date) {

	events.push([polarity ? 1 : 0, date]);
	systray.icon = polarity ? iconRed : iconGreen;
}

const idleTres = 1*SECOND; // 5*MINUTE; // 

var onTimeout;

function isIdle() {
	
	if ( auto && lastInputTime < idleTres ) {

		idle(false, now() - lastInputTime);
		onTimeout = notIdle;
	}
}

function notIdle() {
	
	if ( auto && lastInputTime > idleTres ) {
		
		idle(true, now() - lastInputTime);
		onTimeout = isIdle;
	}
}

isIdle();
  
try {

	for (;;)
		processEvents( systray.events(), timeoutEvents(0.1*SECOND, onTimeout) );
		
} catch (ex if ex == 'end') {}






halt(); //////////////////////////////////////////////////////////////////////





for ( var i = 0; i < 10; i++ ) {
	
	numlockState = !numlockState;
	capslockState = !capslockState;
	scrolllockState = !scrolllockState;
	sleep(50);
}	


halt(); //////////////////////////////////////////////////////////////////////





 loadModule('jsstd');
 loadModule('jswinshell');

 var s = new Systray();
 s.icon = new Icon( 0 );
 s.menu = { add:'Add', exit:'Exit', s1:{ separator:true } };
 s.onmousedown = function( button ) {

 	s.popupMenu();
 }

 s.oncommand = function( id, button ) {

 	switch ( id ) {
 		case 'exit':
 			host.endSignal = true;
 			break;
 		case 'add':
 			var fileName = fileOpenDialog( 'executable files|*.exe;*.com;*.cmd;*.bat|all files|*.*' );
 			if ( !fileName )
 				return;
 			var icon = extractIcon( fileName );
 			var text = fileName.substr( fileName.lastIndexOf( '\\' ) + 1 );
 			s.menu[fileName] = { icon:icon, text:text };
 			break;
 		default:
 			if ( button == 1 )
 				createProcess( id );
 			else
 				if ( messageBox( 'Remove item: ' + id + '? ', 'Question', MB_YESNO) == IDYES )
 					delete s.menu[id];
 		}
 }

while ( !host.endSignal )
	processEvents( s.events(), host.endSignalEvents() )


halt(); //////////////////////////////////////////////////////////////////////









try {
	
	var path = 'HKEY_LOCAL_MACHINE\\Software\\clients\\startMenuInternet';
	var defaultBrowser = registryGet(path+'\\'+registryGet(path, '')+'\\shell\\open\\command', '' );
	createProcess(undefined, defaultBrowser + ' http://jslibs.googlecode.com/');
	
//	print( uneval( RegistryGet('HKEY_CURRENT_USER\\Software\\7-Zip'), '\n' ) );

} catch(ex) {

	print(ex.text);
}

halt(); //////////////////////////////////////////////////////////////////////






// MessageBox() Flags
const MB = {
OK                  :0x000000,
OKCANCEL            :0x000001,
ABORTRETRYIGNORE    :0x000002,
YESNOCANCEL         :0x000003,
YESNO               :0x000004,
RETRYCANCEL         :0x000005,
CANCELTRYCONTINUE   :0x000006,

ICONHAND            :0x000010,
ICONQUESTION        :0x000020,
ICONEXCLAMATION     :0x000030,
ICONASTERISK        :0x000040,
USERICON            :0x000080,
ICONWARNING         :0x000030,
ICONERROR           :0x000010,
ICONINFORMATION     :0x000040,
ICONSTOP            :0x000010,

DEFBUTTON1          :0x000000,
DEFBUTTON2          :0x000100,
DEFBUTTON3          :0x000200,
DEFBUTTON4          :0x000300,

APPLMODAL           :0x000000,
SYSTEMMODAL         :0x001000,
TASKMODAL           :0x002000,
HELP                :0x004000,
NOFOCUS             :0x008000,

SETFOREGROUND       :0x010000,
DEFAULT_DESKTOP_ONLY:0x020000,
TOPMOST             :0x040000,
RIGHT               :0x080000,

RTLREADING          :0x100000
};


// Dialog Box Command IDs
const ID = {
OK      :1,
CANCEL  :2,
ABORT   :3,
RETRY   :4,
IGNORE  :5,
YES     :6,
NO      :7,
CLOSE   :8,
HELP    :9,
TRYAGAIN:10,
CONTINUE:11,

TIMEOUT :32000
};

var s = new Systray();
s.icon = new Icon( 0 );
s.menu = { add:{ text:'Add', default:true}, exit:'Exit', s1:{ separator:true }, 'C:\\WINDOWS\\notepad.exe':'Notepad' };
s.onmousedown = function( button ) { 
	
		messageBeep();
		s.popupMenu();
}

s.onmousemove = function( x,y ) {

	var pos = s.position();
	print( x-pos[0], ',', y-pos[1], '\n' );
}

s.oncommand = function( id, button ) {

	switch ( id ) {
		case 'exit':
			return true;
		case 'add':
			var fileName = fileOpenDialog( 'executable files|*.exe;*.com;*.cmd;*.bat|all files|*.*' );
			if ( !fileName )
				return undefined;
			var icon = function(val,key) { 
				try {
					return extractIcon( key )
				} catch (ex) {}; // do not worry about ExtractIcon failures
			}
			var text = fileName.substr( fileName.lastIndexOf( '\\' ) + 1 );
			s.menu[fileName] = { icon:icon, text:text };
			break;
		default:
			if ( button == 1 ) {
				try {
					createProcess( id );
				} catch (ex) {}; // do not worry about CreateProcess failures
			} else
				if ( messageBox( 'Remove item: ' + id + '? ', 'Question', 4) == 6 )
					delete s.menu[id];
		}
}

do { sleep(100) } while ( !s.processEvents() );








/*
var clip = clipboard;
if ( clip != null ) {
	
	clipboard = 'tata';
	print( clipboard, '\n' );
	clipboard = clip;
}



//print( FileOpenDialog('executable files|*.exe;*.com;*.cmd;*.bat|all files|*.*'), '\n' );
//print( ExpandEnvironmentStrings('%SystemRoot%\\System32\\calc.exe'), '\n' );
//createProcess('C:\\WINDOWS\\system32\\calc.exe');

var s = new Systray();

var exit = false;

//var image = new Png(new File('calendar.png').Open(File.RDONLY)).Load();
//print( image.width+'x'+image.height+'x'+image.channels, '\n' );

var trayIcon = new Icon(new Png(new File('calendar_16x16x3.png').open(File.RDONLY)).load());
var calcIcon = extractIcon( "C:\\WINDOWS\\system32\\calc.exe" );
//var trayIcon = ExtractIcon( "C:\\Program Files\\Mozilla Firefox\\firefox.exe" );

s.icon = trayIcon;
s.text = "test";
s.menu = { 
	ico:"icon", 
	del:"del", 
	g1:{ 
		grayed:1,
		text:'menu'
	}, 
	sep1:{
		separator:true
	}, 
	state:{
		text:"Start", 
		checked:true
	}, 
	2:{
		text:"Stop", 
		icon:calcIcon
	}, 
	4:"exit" 
}

s.onmousedown = function(button) { 
	
	if ( button == 1  )
		s.focus()
	if ( button == 2 )
		s.popupMenu();
}

s.onchar = function(c) { print(c); }

s.onmousemove = function(x,y) {  }
//s.onfocus = function(polarity) { s.icon = null }
//s.onblur = function() { s.icon = trayIcon }

s.oncommand = function(id) {
	
	if ( id == 4 ) exit = true;
	if ( id == 'state' ) s.menu[id].checked ^= 1;
	if ( id == 'del' ) s.icon = null;
//	if ( id == 'ico' ) { s.icon = parseInt(Math.random()*5)	}
}

var blink = true;

//messageBox( s.text );
//s.Flash();

while ( !host.endSignal && !exit ) {
	s.processEvents();
	sleep(100);//print('.');
//	s.icon = blink ? trayIcon : null;
	blink = !blink;
//	s.visible = blink;
}

//File.stdout.Write("press enter");
//File.stdin.Read(1);

*/