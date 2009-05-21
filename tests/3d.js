LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsimage');
LoadModule('jsode');
LoadModule('jsgraphics');
LoadModule('jsprotex');
LoadModule('jstrimesh');
LoadModule('jssdl');


var time0 = new Date().valueOf();
var ax = 0, ay = 0, pos = 0;

var speedX = 0;
var speedY = 0;

var rotateX = 0;
var rotateY = 0;

var tremeshId;
var tremeshTransformation = new Transformation();
tremeshTransformation.Clear();


function Init() {

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

	with (Ogl) {
	
		var t = new Date().valueOf() - time0;

		LoadMatrix(tremeshTransformation);

		Translate(-0.5, -0.5, -0.5);
		Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
		DrawTrimesh(tremeshId);
	};

	var rotation = new Transformation();
	rotation.Clear();
	rotation.RotateToVector( -speedX, speedY, 1 );
	tremeshTransformation.Product(rotation);


}

GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
GlSetAttribute( GL_DOUBLEBUFFER, 1 );
GlSetAttribute( GL_DEPTH_SIZE, 16 );

SetVideoMode( 800, 600, 32, HWSURFACE | OPENGL | RESIZABLE ); // | ASYNCBLIT // RESIZABLE FULLSCREEN
Ogl.Perspective( 90, 0.01, 1000 );

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
	},
	onMouseMotion: function(x,y,dx,dy,button) {
		
		if ( button & BUTTON_LMASK ) {

			speedX += dx;
			speedY += dy;
		}
		
		ax += dx;
		ay += dy;
	}

};

Init();

while ( !done ) {

	PollEvent(listeners);
	Draw();	
	GlSwapBuffers();
	Sleep(10);
}

