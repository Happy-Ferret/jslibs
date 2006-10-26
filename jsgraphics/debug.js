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

var i = 0;

var mousex, mousey;
var totalx=0, totaly=0;

var w = new Window();
var gl = new Gl(w);
w.title = "Test";

w.onidle = function() {

	gl.Test([totalx, 0,Math.sin(totaly/100),1]);
	gl.SwapBuffers();	

	i++;
	if (i % 100 == 0) {
		Print( w.active, '\n');
	}
}

var savedMousePosition;

w.onmouseup = w.onmousedown = function(button, polarity) {
	
	if ( button == 1 ) {
		if (polarity)
			savedMousePosition = w.cursorPosition;
		else {
			w.cursorPosition = savedMousePosition;
			mousex = mousey = undefined;
		}
		Window.showCursor = !polarity;
		w.captureMouse = polarity;
	}
}


w.onmousemove = function( x,y, b1,b2,b3 ) {

	if ( !b1 )
		return;
//	Print( 'x:'+x+' y:'+y+' button1:'+b1+' button2:'+b2+' button3:'+b3, '\n' );
	var r = w.rect;
	var cx = Math.round((r[2]-r[0])/2); // cx & cy must be integer else comparaison will failed
	var cy = Math.round((r[3]-r[1])/2);
	if ( x != cx || y != cy ) {
		if ( mousex != undefined && mousey != undefined ) {
			totalx += x - mousex;
			totaly += y - mousey;
			Print( totalx+' '+totaly, '\n' );
		}
		w.cursorPosition = [cx,cy];
	}
	mousex = x;
	mousey = y;
}


var fullscreen = false;
var preferedrect;
w.onchar = function( c, l ) {

	Print( 'c:'+c+' l:'+l.toString(16), '\n' );
	
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
			fullscreen = !fullscreen;
			if ( fullscreen ) {
			
				preferedrect = w.rect;
				Window.Mode( [640, 480], 32, true );
				w.rect = Window.desktopRect;
				w.showFrame = false;
				w.showCursor = false;
			} else {
			
				w.rect = preferedrect;
				w.showCursor = true;
				w.showFrame = true;
				Window.Mode();
			}
			break;
		}
}

w.onsize = function( w, h ) {

	gl.Viewport(0,0,w,h);
	Print('resize ' +w +' '+ h, '\n')
//	gl.Clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
//	gl.SwapBuffers();
}

w.rect = [500,100,1000,500];
w.ProcessEvents();

Print('Done.'+i);


