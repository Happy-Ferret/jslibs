LoadModule('jsstd');
LoadModule('jsnspr');
LoadModule('jsimage');
LoadModule('jswinshell');

var s = new Systray();

var exit = false;

//var image = new Png(new File('calendar.png').Open(File.RDONLY)).Load();
//Print( image.width+'x'+image.height+'x'+image.channels, '\n' );
  
s.icon = new Png(new File('calendar.png').Open(File.RDONLY)).Load();
s.text = "test";
s.menu = { a:"123", g1:{ grayed:1, text:'menu' }, sep1:{separator:true}, state:{text:"Start", checked:true}, 2:{text:"Stop"}, 4:"exit" }

s.onmouseup = function(button) { 
	
	button == 2 && s.PopupMenu();
}

//s.onmousemove = function(x,y) { Print('x:'+x+' y:'+y+' \n') }
s.oncommand = function(id) {
	
	if ( id == 4 ) exit = true;
	if ( id == 'state' ) s.menu[id].checked ^= 1;
}

while ( !endSignal && !exit ) {
	s.ProcessEvents();
	Sleep(100);//Print('.');
}

//File.stdout.Write("press enter");
//File.stdin.Read(1);