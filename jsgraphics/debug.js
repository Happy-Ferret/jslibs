LoadModule('jsstd');
LoadModule('jsnspr');
LoadModule('jsimage');
LoadModule('jsgraphics');

Exec('OpenGL.js');

var prevMouseX, prevMouseY;
var totalx=0, totaly=0,totalz=0, totalWheel=0;
var image=0;

var w = new Window();
w.title = "Test";
w.rect = [500,100,1000,500];
var gl = new Gl(w);

function Render() {

//	Print('Rendering image '+image++, '\n');
	gl.Test([totaly,totalx,0,-totalz/50-5]);
	gl.SwapBuffers();
}

w.onidle = Render;

var _savedMousePosition;

w.onmouseup = w.onmousedown = function(button, polarity) { // onmouseup AND onmousedown
	
	if (polarity)
		_savedMousePosition = w.cursorPosition;
	else {
		w.cursorPosition = _savedMousePosition;
		prevMouseX = prevMouseY = undefined;
	}
	Window.showCursor = !polarity;
	w.captureMouse = polarity;
}

w.onmousewheel = function(delta) {

	totalWheel += delta;
}

w.onmousemove = function( x,y, b1,b2,b3 ) { // mouse X, mouse Y, left, right, middle

	if ( b1 || b2 ) {

	//	Print( 'x:'+x+' y:'+y+' button1:'+b1+' button2:'+b2+' button3:'+b3, '\n' );
		var r = w.rect;
		var cx = Math.round((r[2]-r[0])/2); // cx & cy must be integer else comparaison will failed
		var cy = Math.round((r[3]-r[1])/2);
		if ( x != cx || y != cy ) {
			if ( prevMouseX != undefined && prevMouseY != undefined ) {
				
				if ( b2 ) { // Z-mode
				
					totalz += y - prevMouseY;
				} else {
				
					totalx += x - prevMouseX;
					totaly += y - prevMouseY;
				}
				w.cursorPosition = [cx,cy];
			}
		}
		prevMouseX = x;
		prevMouseY = y;
	}
}

var _fullscreenState = false;
var _savedWindowSize;

w.onchar = function( c, l ) {

//	Print( 'c:'+c+' l:'+l.toString(16), '\n' );
	switch (c) {
		case 'r':
			var r = w.rect;
			r[0]++;
			w.rect = r;
			break;
		case '\x1B':
			w.Exit();
			break;
		case '\x0D':
			_fullscreenState = !_fullscreenState;
			if ( _fullscreenState ) {
			
				_savedWindowSize = w.rect;           // save current window rectangle
				Window.Mode( [640, 480], 32, true ); // change mode 640x480, 32bits, temporarily fullscreen
				w.showFrame = false;                 // hide window frame ( remove all borders )
				w.rect = Window.desktopRect;         // extends the window to the screen size
				w.showCursor = false;                // hide the cursor
			} else {
			
				w.rect = _savedWindowSize;
				w.showCursor = true;
				w.showFrame = true;
				Window.Mode();              // revert to default
			}
			break;
		}
}

w.onsize = function( w, h ) {

	gl.Viewport(0,0,w,h);
	Render();
}

//var f = new File('R0010235.JPG');
var f = new File('R0010235.png');
f.Open( File.RDONLY );
//var img = new Jpeg(f);
var texture = new Png(f).Load();

var x=100, y=100; // offset
texture.Trim([0+x,0+y,64+x,64+y], true);

gl.Texture( texture );


w.ProcessEvents();
Print('Done.', '\n');


