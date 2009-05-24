LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsgraphics');
LoadModule('jstrimesh');
LoadModule('jssdl');


var speedX = 0;
var speedY = 0;

var tremeshId;
var trimeshTransformation = new Transformation(true);


function Init() {

	GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
	GlSetAttribute( GL_DOUBLEBUFFER, 1 );
	GlSetAttribute( GL_DEPTH_SIZE, 16 );

	SetVideoMode( 800, 600, 32, HWSURFACE | OPENGL | RESIZABLE ); // | ASYNCBLIT // RESIZABLE FULLSCREEN

	var t = new Trimesh();

	var vertexCount = 3000;
	var triangleCount = 2000;
	
	var vertex = [];
	for ( var i = 0; i < vertexCount*3; i++ )
		vertex.push(Math.random());

	var color = [];
	for ( var i = 0; i < vertexCount*4; i++ )
		color.push(Math.random());

	var index = [];
	for ( var i = 0; i < triangleCount*3; i++ )
		index.push(Math.floor( Math.random() * vertexCount ) );

	t.DefineVertexBuffer(vertex);
	t.DefineColorBuffer(color);
	t.DefineIndexBuffer(index);
	tremeshId = Ogl.LoadTrimesh(t);

	with (Ogl) {

		Enable(DEPTH_TEST);
		Enable(BLEND); //Enable alpha blending
		BlendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA); //Set the blend function
		Enable(CULL_FACE);
	  
		LoadIdentity();
		PushMatrix();
	}
}


function Draw() {

	trimeshTransformation.Product(new Transformation(true).RotateToVector(-speedX, speedY, 1));
	with (Ogl) {
	
		LoadMatrix(trimeshTransformation);
		Translate(-0.5, -0.5, -0.5);
		Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
		DrawTrimesh(tremeshId);
	}
}


showCursor = true;
var done = false;

var listeners = {
	onQuit: function() done = true,
	onKeyDown: function(key, mod) {
		
		switch (key) {
			case K_ESCAPE: 
				done = true;
				break;
			case K_g:
				grabInput = !grabInput;
				showCursor = !showCursor;
				break;
		}
	},
	onVideoResize: function(w,h) {

//		SetVideoMode(w, h);
		Ogl.Viewport(0, 0, w, h);
	},
	onMouseMotion: function(x,y,dx,dy,button) {
		
		if ( button & BUTTON_LMASK ) {

			speedX += dx;
			speedY += dy;
		}
	}
};


Init();

while ( !done ) {

	PollEvent(listeners);
	Draw();	
	GlSwapBuffers();
	Sleep(10);
}
