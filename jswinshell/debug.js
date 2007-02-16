LoadModule('jsstd');
LoadModule('jsnspr');
LoadModule('jsimage');
LoadModule('jswinshell');

var s = new Systray();

var image = new Png(new File('calendar_16x16x3.png').Open(File.RDONLY)).Load();
Print( image.width+'x'+image.height+'x'+image.channels, '\n' );
  
s.icon = image;

File.stdout.Write("press enter");
File.stdin.Read(1);