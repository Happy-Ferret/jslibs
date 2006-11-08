LoadModule('jsstd');
LoadModule('jsnspr');
LoadModule('jsimage');
LoadModule('jsgraphics');
var glc = Exec('OpenGL.js');
var vk = Exec('WindowsKeys.js');

Exec('winTools.js');

var image=0;

var w = new Window();
w.title = "Test";
//w.rect = [500,500,1000,1000];
w.rect = [500,500,600,600];
var gl = new Gl(w);

var mouse = new MouseMotion(w);

var button = [0,0,0];
w.onmouseup = w.onmousedown = function(b, polarity) { // onmouseup AND onmousedown

	button[b] = polarity;
	mouse.infiniteMode = button[0] || button[1] || button[2];
}

var tw=0, tx=0, ty=0, tz=0; 

w.onmousewheel = function(delta) {

	tw += delta;
}

mouse.delta = function( dx,dy, b1,b2,b3 ) {
	
	if ( b1 ) {
	
		tx += dx;
		ty += dy;
	} else if ( b2 ) {

		tz += dy;
	}
}

function Render() {

	//Print('Rendering image '+image++, '\n');

	gl.Clear( glc.COLOR_BUFFER_BIT | glc.DEPTH_BUFFER_BIT );

	gl.Translate(0,0, -tz/50-5);
	gl.Rotate( ty/2, 1,0,0 );
//	gl.Rotate( tx/2, 0,1,0 );
//	gl.Rotate( tw*10, 0,0,1 );

	var t = new Transformation();
	
	t.Translate(0,0, -tz/50-5);
	t.Rotate( ty/2, 1,0,0 );
//	t.Rotate( tx/2, 0,1,0 );
//	t.Rotate( tw*10, 0,0,1 );
	gl.LoadMatrix(t);

	gl.Color(1,1,1);
	
	for ( var x = -100; x<10; x++ )
		for ( var y = -10; y<10; y++ )
			gl.Quad(x,y,x+1,y+1-0.001);
	
	gl.Axis();
	gl.Color( 0,5, 1, 1 );
//	
	var t0 = IntervalNow();
	CollectGarbage();
	gl.SwapBuffers();
	Print( 'SwapBuffers time: '+(IntervalNow()-t0)+'ms', '\n');
}

w.onidle = Render;

var _fullscreenState = false;
var _savedWindowSize;

w.onkeydown = function( key, l ) {

	switch (key) {
		case vk.ESC:
			w.Exit();
			break;
		case vk.ENTER:
			// toggle fullscreen
			_fullscreenState = !_fullscreenState;
			if ( _fullscreenState ) {
			
				_savedWindowSize = w.rect;           // save current window rectangle
				//Window.Mode( [640, 480], 32, true ); // change mode 640x480, 32bits, temporarily fullscreen
				w.showFrame = false;                 // hide window frame ( remove all borders )
				w.rect = Window.desktopRect;         // extends the window to the screen size
				w.showCursor = false;                // hide the cursor
			} else {
			
				w.rect = _savedWindowSize;
				w.showCursor = true;
				w.showFrame = true;
				//Window.Mode();              // revert to default
			}
			break;
		}
}

w.onsize = function( w, h ) {

	gl.Viewport([0,0,w,h]);
	gl.Perspective( 60, 0.01, 1000 );	
	Render();
}

var x=0, y=0; // offset
var texture = new Jpeg(new File('R0010235.JPG').Open( File.RDONLY )).Load().Trim([0+x,0+y,256+x,256+y], true);

gl.Init();
gl.Perspective( 60, 0.01, 1000 );
gl.Texture( texture );

//Window.absoluteClipCursor = [100,100, 200, 200 ]
w.ProcessEvents();
//Window.absoluteClipCursor = undefined;
Print('Done.', '\n');


