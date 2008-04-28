LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsimage');
LoadModule('jsode');
LoadModule('jsgraphics');
LoadModule('jsprotex');

var vk = Exec('WindowsKeys.js');
Exec('winTools.js');


function DumpMatrix(m) {
    
    for (var x = 0; x < 4; x++) {
        Print('[  ' );
        for (var y = 0; y < 4; y++)
            Print( m[x+y*4].toFixed(1) + '  ' );
        Print(']\n' );
    }
}


function Axis(size) {

	with (Ogl) {

		Begin(LINES);
		Color( size,0,0 );
		Vertex( 0,0,0 );
		Vertex( size,0,0 );
		Color( 0,size,0 );
		Vertex( 0,0,0 );
		Vertex( 0,size,0 );
		Color( 0,0,size );
		Vertex( 0,0,0 );
		Vertex( 0,0,size );
		End();
	}
}

function Quad() {

	with (Ogl) {
		Disable( TEXTURE_2D );
		Begin(LINE_LOOP);
		Color(1,0.5,0.5);
		Vertex( -0.5, -0.5 );
		Color(0.5,1,0.5);
		Vertex( 0.5, -0.5 );
		Color(1,0.5,0.5);
		Vertex( 0.5, 0.5 );
		Color(0.5,1,0.5);
		Vertex( -0.5, 0.5 );
		End();
	}
}

/*
function Quad(x0, y0, x1, y1) {

	with (Ogl) {
		Begin(QUADS);
		TexCoord( 0, 0 );
		Vertex( x0, y0 );
		TexCoord( 1, 0 );
		Vertex( x1, y0 );
		TexCoord( 1, 1 );
		Vertex( x1, y1 );
		TexCoord( 0, 1 );
		Vertex( x0, y1 );
		End();
	}
}
*/

function Line(x0, y0, z0, x1, y1, z1) {

	with (Ogl) {
	
		Begin(LINES);
		Vertex(x0, y0, z0);
		Vertex(x1, y1, z1);
		End();
	}
}


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

var camera = new Transformation;
camera.Clear();

var objects = [];

	var win = new Window();
	win.title = "Test";
	var x = 200;
	var y = 200;
	var w = 500;
	var h = 500;
	win.rect = [x,y,x+w,y+h];

	var _quit = false;

	win.onkeydown = function( key, l ) {

		switch (key) {
			case vk.ESC:
				_quit = true;
				break;
		}
	}
	
	win.onmousedown = function(button) {
		
		var m = new Transformation();

		var p = new Transformation();
		Ogl.MatrixMode(Ogl.PROJECTION);
		p.Load(Ogl);
		
		m.Load(p);
//		m.Product(camera);

//		m.Invert();

		m.Clear();
		
		var tmp = new Transformation();
		tmp.Clear();
		tmp.Translation(1,1,1);
		
//		m.Product(tmp);
		vect = [1,1,1];
		m.TransformVector(vect);
		Print( 'transformed vector: \n'+vect.join('\n'), '\n' );
		
		
		
		
		var clientRect = win.clientRect;
		var width = clientRect[2] - clientRect[0];
		var height = clientRect[3] - clientRect[1];
		
		var cursorPosition = win.cursorPosition;
		var vect = [(-1 + 2 * ( cursorPosition[0] / width )) / p[0], (-1 + 2 * ( height - cursorPosition[1] ) / height) / p[5], -1];
		
		m.TransformVector(vect);
	
	
//		DumpMatrix(m);
		
//		Print( 'x: '+m[3], '\n' );
//		Print( 'y: '+m[7], '\n' );
//		Print( 'z: '+m[11], '\n' );
//		Print( 'transformed vector: \n'+vect.join('\n'), '\n' );
		
		var obj = {};
		obj.createTime = TimeCounter();
		obj.x = vect[0];
		obj.y = vect[1];
		obj.z = vect[2];
		objects.push(obj);
//		Print(win.cursorPosition, ' / ', clientRect[2]-clientRect[0], ',', clientRect[3]-clientRect[1], '\n');
	}

/*
Line3 Viewport::Ray( int winx, int winy ) {

  Vector4 in( -1 + 2 * ( (float)winx / (float)( _width - _offsetWidth ) ), -1 + 2 * (float)( _height - winy - _offsetHeight ) / (float)(_height - _offsetHeight ), -1, 1 );

  in.x /= ProjectionMatrix()[0];
  in.y /= ProjectionMatrix()[5];

  Matrix44 invView = Camera().Transformation();
  invView.Invert();

  Vector4 out = invView( in ); ...

												  Vector4 operator()( const Vector4 &vector ) const {
												    
													 return Vector4( _m[0]*vector.x + _m[4]*vector.y + _m[8]*vector.z  + _m[12]*vector.w,
																		  _m[1]*vector.x + _m[5]*vector.y + _m[9]*vector.z  + _m[13]*vector.w,
																		  _m[2]*vector.x + _m[6]*vector.y + _m[10]*vector.z + _m[14]*vector.w,
																		  _m[3]*vector.x + _m[7]*vector.y + _m[11]*vector.z + _m[15]*vector.w );
												  }
  

//  out.Rescale(); // should be done if needed

  Point3 position( invView.Translation() );
  Vector3 direction( out.x, out.y, out.z );
  direction -= position;
	return Line3( position, direction );
}
*/




/*
function ResizeWindow(w, h) {

	with (Ogl) {
		Viewport(0,0,w,h);
		MatrixMode(PROJECTION);
		LoadIdentity();
		Ortho(0,0,10,10, -10, 10);
//		Perspective( 90, -1000, 1000 );
	}
	Render();
}

win.onsize = ResizeWindow;		
*/

win.Open();
win.CreateOpenGLContext();


with (Ogl) {

//	ShadeModel(SMOOTH);
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

	Enable( TEXTURE_2D );
	BindTexture( TEXTURE_2D, GenTexture() );
	TexParameter(TEXTURE_2D, TEXTURE_MIN_FILTER, NEAREST); // GL_LINEAR
	TexParameter(TEXTURE_2D, TEXTURE_MAG_FILTER, NEAREST);
	TexEnv(TEXTURE_ENV, TEXTURE_ENV_MODE, MODULATE);
	TexEnv(TEXTURE_ENV, TEXTURE_ENV_COLOR, [0,0,0,0]);
	

//	var texture = new Jpeg(new File('R0010235.JPG').Open( File.RDONLY )).Load();
	var texture = new Texture(128, 128, 1);
	texture.Set([0]);
	const curveGaussian = function(c) { return function(x) { return Math.exp( -(x*x)/(2*c*c) ) } }
	texture.AddGradiantRadial( curveGaussian( 0.3 ), false );
	texture.AddNoise(0.1,0,0,0);

//	Enable(ALPHA_TEST);
//	AlphaFunc( GREATER, 0.5 );

	Enable(BLEND);
//	BlendFunc(ONE, ONE);
	BlendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA);
	
	DefineTextureImage( TEXTURE_2D, ALPHA, texture );

//		PointParameter( POINT_SIZE_MIN, 0 );
//		PointParameter( POINT_SIZE_MAX, 1 );
		PointParameter( POINT_DISTANCE_ATTENUATION, [0, 0, 0.01] ); // 1/(a + b*d + c *d^2)

	Enable(POINT_SPRITE); // http://www.informit.com/articles/article.aspx?p=770639&seqNum=7
	TexEnv(POINT_SPRITE, COORD_REPLACE, TRUE);

	MatrixMode(PROJECTION);
	LoadIdentity();
	Perspective( 60, 0.01, 1000 );
//	Ortho(0,0,10,10, -10, 10);
//	Ortho( -110,-100, 100, 100, 0, 100 );
}


with (Ogl) {

	var list = NewList();

		Color(0.5,0.5,1);
		Begin(LINES);
		Vertex( 0, 1, 0 );
		Vertex( 0,-1, 0 );
		End();

		Enable( TEXTURE_2D );
		PointSize(40);
		Color(1,1,1);
		Begin(POINTS);
		Vertex( 0, 1, 0);
		Vertex( 0,-1, 0);
		End();
		Disable( TEXTURE_2D );
		
	EndList();
}

const degToRad = 360/Math.PI;


camera.Translate(0, 0, -10);


function Render(imgIndex) {
	with (Ogl) {

		Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
	
		MatrixMode(MODELVIEW);
		LoadMatrix(camera);
		Quad();
	
		Enable( TEXTURE_2D );
		Color(1,1,1,1);
		for each ( var obj in objects ) {
			
			var t = TimeCounter() - obj.createTime;
			PointSize((-Math.cos(t/1000)+1)*10);
			Begin(POINTS);
			Vertex(obj.x, obj.y);
			End();			
		}
	}
}


for (var i=0; !_quit; i++) {

	MaybeCollectGarbage();
	Ogl.Viewport(0,0,w,h);
	
	var t0 = TimeCounter();
	Render(i);
//	Print( (1000/(TimeCounter() - t0)).toFixed(), ' fps\n' );
	win.SwapBuffers();
	win.ProcessEvents();
	Sleep(10);
}


