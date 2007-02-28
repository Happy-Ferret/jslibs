LoadModule('jsstd');
LoadModule('jsnspr');
LoadModule('jsimage');
LoadModule('jswinshell');

var clip = clipboard;
if ( clip != null ) {
	
	clipboard = 'tata';
	Print( clipboard );
	clipboard = clip;
}


MessageBox(FileOpenDialog('','executable files|*.exe;*.com;*.cmd;*.bat|all files|*.*'));

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
s.menu = { ico:"icon", del:"del", g1:{ grayed:1, text:'menu' }, sep1:{separator:true}, state:{text:"Start", checked:true}, 2:{text:"Stop", icon:calcIcon}, 4:"exit" }

s.onmousedown = function(button) { 
	
	if ( button == 1  )
		s.Focus()
	if ( button == 2 )
		s.PopupMenu();
}

s.onchar = function(c) { Print(c); }

s.onmousemove = function(x,y) {  }
s.onfocus = function(polarity) { s.icon = null }
s.onblur = function() { s.icon = trayIcon }

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

