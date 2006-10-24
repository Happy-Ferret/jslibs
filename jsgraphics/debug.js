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

var w = new Window("Test");
w.Create("test");
var gl = new Gl(w);

w.onidle = function() {

	gl.Test();
	gl.SwapBuffers();	

	i++;
	(i % 100) || Print( 'i='+i, '\n');
	
//	w.WaitForMessage();	
}

w.onmousemove = function( x,y,b1,b2 ) {

	Print( 'x:'+x+' y:'+y+' button1:'+b1+' button1:'+b2, '\n' );
}

w.onchar = function( c, l ) {

	Print( 'c:'+c+' l:'+l.toString(16), '\n' );
	if ( c.charCodeAt(0) == 27  )
		w.Exit();
		
	if ( c.charCodeAt(0) == 13 ) {
		if ( !w.fullScreen ) {
			Window.Mode( 800, 600, 32 );
			w.fullScreen = true;
			w.showCursor = false;
		} else {
			w.showCursor = true;
			w.fullScreen = false;
			Window.Mode();
		}
	}
		
		
}

w.onresize = function( w, h ) {

	gl.Viewport(0,0,w,h);
	Print('resize ' +w +' '+ h, '\n')
//	gl.Clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
//	gl.SwapBuffers();
}

w.ProcessEvents();

Print('Done.'+i);


