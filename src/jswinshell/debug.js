LoadModule('jsstd');
LoadModule('jsimage');
LoadModule('jswinshell');

try {

	Print( uneval( RegistryGet('HKEY_CURRENT_USER\\Software\\7-Zip\\') ) );

} catch(ex) {

	Print(ex.text);
}

Halt(); //////////////////////////////////////////////////////////////////////

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
	
		MessageBeep();
		s.PopupMenu();
}

s.onmousemove = function( x,y ) {

	var pos = s.Position();
	Print( x-pos[0], ',', y-pos[1], '\n' );
}

s.oncommand = function( id, button ) {

	switch ( id ) {
		case 'exit':
			return true;
		case 'add':
			var fileName = FileOpenDialog( 'executable files|*.exe;*.com;*.cmd;*.bat|all files|*.*' );
			if ( !fileName )
				return;
			var icon = function(val,key) { 
				try {
					return ExtractIcon( key )
				} catch (ex) {}; // do not worry about ExtractIcon failures
			}
			var text = fileName.substr( fileName.lastIndexOf( '\\' ) + 1 );
			s.menu[fileName] = { icon:icon, text:text };
			break;
		default:
			if ( button == 1 ) {
				try {
					CreateProcess( id );
				} catch (ex) {}; // do not worry about CreateProcess failures
			} else
				if ( MessageBox( 'Remove item: ' + id + '? ', 'Question', 4) == 6 )
					delete s.menu[id];
		}
}

do { Sleep(100) } while ( !s.ProcessEvents() );








/*
var clip = clipboard;
if ( clip != null ) {
	
	clipboard = 'tata';
	Print( clipboard, '\n' );
	clipboard = clip;
}



//Print( FileOpenDialog('executable files|*.exe;*.com;*.cmd;*.bat|all files|*.*'), '\n' );
//Print( ExpandEnvironmentStrings('%SystemRoot%\\System32\\calc.exe'), '\n' );
//CreateProcess('C:\\WINDOWS\\system32\\calc.exe');

var s = new Systray();

var exit = false;

//var image = new Png(new File('calendar.png').Open(File.RDONLY)).Load();
//Print( image.width+'x'+image.height+'x'+image.channels, '\n' );

var trayIcon = new Icon(new Png(new File('calendar_16x16x3.png').Open(File.RDONLY)).Load());
var calcIcon = ExtractIcon( "C:\\WINDOWS\\system32\\calc.exe" );
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
		s.Focus()
	if ( button == 2 )
		s.PopupMenu();
}

s.onchar = function(c) { Print(c); }

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

//MessageBox( s.text );
//s.Flash();

while ( !endSignal && !exit ) {
	s.ProcessEvents();
	Sleep(100);//Print('.');
//	s.icon = blink ? trayIcon : null;
	blink = !blink;
//	s.visible = blink;
}

//File.stdout.Write("press enter");
//File.stdin.Read(1);

*/