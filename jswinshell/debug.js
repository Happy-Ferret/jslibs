LoadModule('jsstd');
LoadModule('jsnspr');
LoadModule('jsimage');
LoadModule('jswinshell');

var s = new Systray();

var exit = false;

//var image = new Png(new File('calendar.png').Open(File.RDONLY)).Load();
//Print( image.width+'x'+image.height+'x'+image.channels, '\n' );
  
s.icon = new Icon(new Png(new File('calendar.png').Open(File.RDONLY)).Load());
s.text = "test";
s.menu = { ico:"icon", del:"del", g1:{ grayed:1, text:'menu' }, sep1:{separator:true}, state:{text:"Start", checked:true}, 2:{text:"Stop"}, 4:"exit" }

s.onmousedown = function(button) { 
	
	s.PopupMenu();
}

//s.onmousemove = function(x,y) { Print('x:'+x+' y:'+y+' \n') }
s.oncommand = function(id) {
	
	if ( id == 4 ) exit = true;
	if ( id == 'state' ) s.menu[id].checked ^= 1;
	if ( id == 'del' ) s.Close();
//	if ( id == 'ico' ) { s.icon = parseInt(Math.random()*5)	}
}

while ( !endSignal && !exit ) {
	s.ProcessEvents();
	Sleep(100);//Print('.');
}

//File.stdout.Write("press enter");
//File.stdin.Read(1);