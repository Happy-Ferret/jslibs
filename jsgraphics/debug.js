LoadModule('jsstd');
LoadModule('jsgraphics');

const GL_CURRENT_BIT        =0x00000001
const GL_POINT_BIT          =0x00000002
const GL_LINE_BIT           =0x00000004
const GL_POLYGON_BIT        =0x00000008
const GL_POLYGON_STIPPLE_BIT=0x00000010
const GL_PIXEL_MODE_BIT     =0x00000020
const GL_LIGHTING_BIT       =0x00000040
const GL_FOG_BIT            =0x00000080
const GL_DEPTH_BUFFER_BIT   =0x00000100
const GL_ACCUM_BUFFER_BIT   =0x00000200
const GL_STENCIL_BUFFER_BIT =0x00000400
const GL_VIEWPORT_BIT       =0x00000800
const GL_TRANSFORM_BIT      =0x00001000
const GL_ENABLE_BIT         =0x00002000
const GL_COLOR_BUFFER_BIT   =0x00004000
const GL_HINT_BIT           =0x00008000
const GL_EVAL_BIT           =0x00010000
const GL_LIST_BIT           =0x00020000
const GL_TEXTURE_BIT        =0x00040000
const GL_SCISSOR_BIT        =0x00080000
const GL_ALL_ATTRIB_BITS    =0x000fffff

///////////////////////////////////////

var prevMouseX, prevMouseY;
var totalx=0, totaly=0, totalWheel=0;

var w = new Window();
w.title = "Test";
w.rect = [500,100,1000,500];
var gl = new Gl(w);

w.onidle = function() {

	gl.Test([totaly,totalx,wheel*10]);
	gl.SwapBuffers();
}

var _savedMousePosition;

w.onmouseup = w.onmousedown = function(button, polarity) {
	
	if ( button == 1 ) {
		if (polarity)
			_savedMousePosition = w.cursorPosition;
		else {
			w.cursorPosition = _savedMousePosition;
			prevMouseX = prevMouseY = undefined;
		}
		Window.showCursor = !polarity;
		w.captureMouse = polarity;
	}
}

w.onmousewheel = function(delta) {

	totalWheel += delta;
}

w.onmousemove = function( x,y, b1,b2,b3 ) { // mouse X, mouse Y, left, right, middle

	if ( b1 ) {

	//	Print( 'x:'+x+' y:'+y+' button1:'+b1+' button2:'+b2+' button3:'+b3, '\n' );
		var r = w.rect;
		var cx = Math.round((r[2]-r[0])/2); // cx & cy must be integer else comparaison will failed
		var cy = Math.round((r[3]-r[1])/2);
		if ( x != cx || y != cy ) {
			if ( prevMouseX != undefined && prevMouseY != undefined ) {
				totalx += x - prevMouseX;
				totaly += y - prevMouseY;
				Print( totalx+' '+totaly, '\n' );
			}
			w.cursorPosition = [cx,cy];
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
			
				_savedWindowSize = w.rect; // save current window rectangle
				Window.Mode( [640, 480], 32, true ); // change mode 640x480, 32bits, temporarily fullscreen
				w.showFrame = false; // hide window frame ( remove all borders )
				w.rect = Window.desktopRect; // extends the window to the screen size
				w.showCursor = false; // hide the cursor
			} else {
			
				w.rect = _savedWindowSize;
				w.showCursor = true;
				w.showFrame = true;
				Window.Mode(); // revert to default
			}
			break;
		}
}

w.onsize = function( w, h ) {

	gl.Viewport(0,0,w,h);
}

w.ProcessEvents();
Print('Done.', '\n');


