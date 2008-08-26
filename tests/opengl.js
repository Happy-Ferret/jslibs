LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsgraphics');




function Cube() {

	with (Ogl) {
		Begin(QUADS);		// Draw The Cube Using quads

		Color(1,0,0);
		Vertex( 0.5, 0.5,-0.5);	// Top Right Of The Quad (Right)
		Vertex( 0.5, 0.5, 0.5);	// Top Left Of The Quad (Right)
		Vertex( 0.5,-0.5, 0.5);	// Bottom Left Of The Quad (Right)
		Vertex( 0.5,-0.5,-0.5);	// Bottom Right Of The Quad (Right)

		Color(0.5,0,0);
		Vertex(-0.5, 0.5, 0.5);	// Top Right Of The Quad (Left)
		Vertex(-0.5, 0.5,-0.5);	// Top Left Of The Quad (Left)
		Vertex(-0.5,-0.5,-0.5);	// Bottom Left Of The Quad (Left)
		Vertex(-0.5,-0.5, 0.5);	// Bottom Right Of The Quad (Left)

		Color(0,1,0);
		Vertex( 0.5, 0.5,-0.5);	// Top Right Of The Quad (Top)
		Vertex(-0.5, 0.5,-0.5);	// Top Left Of The Quad (Top)
		Vertex(-0.5, 0.5, 0.5);	// Bottom Left Of The Quad (Top)
		Vertex( 0.5, 0.5, 0.5);	// Bottom Right Of The Quad (Top)

		Color(0,0.5,0);
		Vertex( 0.5,-0.5, 0.5);	// Top Right Of The Quad (Bottom)
		Vertex(-0.5,-0.5, 0.5);	// Top Left Of The Quad (Bottom)
		Vertex(-0.5,-0.5,-0.5);	// Bottom Left Of The Quad (Bottom)
		Vertex( 0.5,-0.5,-0.5);	// Bottom Right Of The Quad (Bottom)

		Color(0,0,1);
		Vertex( 0.5, 0.5, 0.5);	// Top Right Of The Quad (Front)
		Vertex(-0.5, 0.5, 0.5);	// Top Left Of The Quad (Front)
		Vertex(-0.5,-0.5, 0.5);	// Bottom Left Of The Quad (Front)
		Vertex( 0.5,-0.5, 0.5);	// Bottom Right Of The Quad (Front)

		Color(0,0,0.5);
		Vertex( 0.5,-0.5,-0.5);	// Top Right Of The Quad (Back)
		Vertex(-0.5,-0.5,-0.5);	// Top Left Of The Quad (Back)
		Vertex(-0.5, 0.5,-0.5);	// Bottom Left Of The Quad (Back)
		Vertex( 0.5, 0.5,-0.5);	// Bottom Right Of The Quad (Back)
		End();
	}
}



var win = new Window();
win.CreateOpenGLContext();
win.Open();

function ResizeWindow(w, h) {

	Ogl.Viewport(0,0,w,h);
	Ogl.MatrixMode(Ogl.PROJECTION);
	Ogl.LoadIdentity();
	Ogl.Ortho(0,0,10,10, 0, 100);
//	Ogl.Perspective( 90, -1000, 1000 );
	MatrixMode(MODELVIEW);

}

//var rect = win.rect;
//ResizeWindow( rect[2]-rect[0], rect[3]-rect[1] );


with (Ogl) {
	ShadeModel(FLAT);
	FrontFace(CCW);

	ClearColor(0.1,0.15,0.2,1);	

	ClearDepth(1);
//	Enable(DEPTH_TEST); // !!!!
	DepthFunc(LESS);

//	DepthRange(1000, -1000);
	
	Hint(PERSPECTIVE_CORRECTION_HINT, NICEST);
	Hint(LINE_SMOOTH_HINT, DONT_CARE);		
	Enable(LINE_SMOOTH);
	LineWidth(1);
}


var z = 0;

function Render() {

	with (Ogl) {
	
		Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
		LoadIdentity();
		Translate(0,0,z);
		Cube();
		
		Print( z, '\n' );
		z -= 0.1;
	}

	win.SwapBuffers();
	Sleep(100);
}


var end = false;

win.onkeydown = function( key, l ) {

	end = ( key == Window.VK_ESCAPE );
}

win.onsize = function(w,h) {

	ResizeWindow(w, h);
	Render();
}

while (!end && !endSignal) {

	win.ProcessEvents();
	Render();
}
