LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsgraphics');
LoadModule('jstrimesh');
LoadModule('jssdl');
LoadModule('jsode');

var world = new World;
world.gravity = [0,0,-9.809];
world.defaultSurfaceParameters.bounce = 0.8;
world.defaultSurfaceParameters.bounceVel = 0;

var floor = new GeomPlane(world.space);
floor.body = new Body(world);

var ball = new GeomSphere(world.space);
ball.body = new Body(world);
ball.body.position = [0,0,10];


GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
GlSetAttribute( GL_DOUBLEBUFFER, 1 );
GlSetAttribute( GL_DEPTH_SIZE, 16 );
SetVideoMode( 800, 600, 32, HWSURFACE | OPENGL | RESIZABLE ); // | ASYNCBLIT // RESIZABLE FULLSCREEN

with (Ogl) {

	Enable(DEPTH_TEST);
	Enable(BLEND); //Enable alpha blending
	BlendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA); //Set the blend function
	Enable(CULL_FACE);

	MatrixMode(PROJECTION);
	Perspective(60, 0.01, 1000);
	MatrixMode(MODELVIEW);

	LoadIdentity();
	PushMatrix();
}

grabInput = true;
showCursor = false;

var z = -10;


function Draw(t) {

	with (Ogl) {
	
		Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
		LoadIdentity();
		
		floor.position = [0,0,z];
		
		world.Step(t);
		
		Print( 'ball.body.position: ', ball.body.position, '\n' );
		

		
		Translate(0,0,z);
		Color(1,1,1);
		Begin(QUADS);
		Vertex(-1,-1);
		Vertex(1,-1);
		Vertex(1,1);
		Vertex(-1,1);
		End();
	}
}


var done = false;
var listeners = {
	onQuit: function() done = true,
	onKeyDown: function(key, mod) {
		
		switch (key) {
			case K_ESCAPE: 
				done = true;
				break;
		}
	},
	onVideoResize: function(w,h) {

		Ogl.Viewport(0, 0, w, h);
	},
	onMouseMotion: function(x,y,dx,dy,button) {
		
//		Print( x, ', ', y, ', ',dx, ', ',dy, ', ',button, '\n' );
		z += dy / 100;
	}
};


var t0, t1;

while ( !done ) {

	PollEvent(listeners);
	
	var t1 = TimeCounter();
	Draw(t1 - t0);
	t0 = t1;
	
	GlSwapBuffers();
	Sleep(10);
}
