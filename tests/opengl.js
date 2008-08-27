LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsgraphics');
LoadModule('jsprotex');
LoadModule('jsfont');


var win = new Window();
win.CreateOpenGLContext();
win.Open();

function ResizeWindow(w, h) {
	
	with (Ogl) {
	
		Viewport(0,0,w,h);
		MatrixMode(PROJECTION);
		LoadIdentity();
		Perspective( 90, 0.001, 1000 );
		MatrixMode(MODELVIEW);
	}
}


var rect = win.rect;
ResizeWindow( rect[2]-rect[0], rect[3]-rect[1] );


with (Ogl) { // Init OpenGL

	ShadeModel(FLAT);
	ClearColor(0.2, 0.3, 0.5, 0);
	Enable(TEXTURE_2D);

	Enable(ALPHA_TEST);
	
	FrontFace(FRONT_FACE);

	Enable(DEPTH_TEST);
//	DepthFunc(ALWAYS);

	Enable(CULL_FACE);
	CullFace(FRONT);

	Enable(BLEND); //Enable alpha blending
	BlendFunc(SRC_ALPHA, ONE_MINUS_SRC_COLOR);
	
//	AlphaFunc( LESS, 1 ); // http://www.opengl.org/documentation/specs/man_pages/hardcopy/GL/html/gl/alphafunc.html
	

	var tid = GenTexture();
	BindTexture(TEXTURE_2D, tid);
	TexParameter(TEXTURE_2D, TEXTURE_MIN_FILTER, LINEAR); // GL_LINEAR
	TexParameter(TEXTURE_2D, TEXTURE_MAG_FILTER, LINEAR);

	var f = new Font('c:\\windows\\fonts\\arial.ttf');
	f.size = 100;
	f.verticalPadding = -16;
	var img = f.DrawString('Hello world', true);
	var t = new Texture(img);
	
	var n = new Texture(t.width, t.height, 2);
	n.SetChannel(0, t, 0);
	n.SetChannel(1, t, 0);

	n.Resize(256,256, true);
	DefineTextureImage(TEXTURE_2D, undefined, n);
}


var angle = 0;

function Render() {

	with (Ogl) {
	
		Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
		LoadIdentity();
		Translate(0,0,-2);
		Rotate(angle, 0,1,0);
		Scale(1,-0.25,1);

		for ( var i = 0; i < 10; i++ ) {
			
			Translate(0, 0, 0.05);

			Color(1,1,1, 1);
			Begin(QUADS);
			
			TexCoord( 0, 0 );
			Vertex( -1, -1 );
			
			TexCoord( 1, 0 );
			Vertex( 1, -1 );
			
			TexCoord( 1, 1 );
			Vertex( 1, 1 );
			
			TexCoord( 0, 1 );
			Vertex( -1, 1 );
			
			End();


			Begin(QUADS);
			
			TexCoord( 0, 0 );
			Vertex( -1, -1 );
			
			TexCoord( 0, 1 );
			Vertex( -1, 1 );
			
			TexCoord( 1, 1 );
			Vertex( 1, 1 );
			
			TexCoord( 1, 0 );
			Vertex( 1, -1 );
			
			End();

		}

		angle += 1;
	}

	win.SwapBuffers();
}


win.onkeydown = function( key, l ) {

	endSignal = ( key == 27 );
}

win.onsize = function(w,h) {

	ResizeWindow(w, h);
	Render();
}

while (!endSignal) {

	win.ProcessEvents();
	Render();
}
