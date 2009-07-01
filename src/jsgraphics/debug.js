LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsimage');
LoadModule('jsode');
LoadModule('jsprotex');
LoadModule('jstrimesh');
LoadModule('jssdl');
LoadModule('jsgraphics');

// OpenGl doc: http://www.opengl.org/sdk/docs/man/

function MinMax(val, min, max) val < min ? min : val > max ? max : val;

function Range(low, high) {

  this.low = low;
  this.high = high;
}

Range.prototype.__iterator__ = function() {

  for (var i = this.low; i <= this.high; i++)
    yield i;
};

function DumpMatrix(m) {
    
	for (var y = 0; y < 4; ++y) {
		Print('[ ' );
		for (var x = 0; x < 4; ++x)
			Print( m[x+y*4].toFixed(3) + '  ' );
		Print(']\n' );
	}
	Print('\n' );
}


var listList = { __proto__:null };
function CondNewList(name, invalidate) {
	
	if ( name in listList && !invalidate ) {
		
		Ogl.CallList(listList[name]);
		return false;
	} else {
		
		listList[name] = Ogl.NewList();
		return true;
	}
}



function Cloud( size, amp ) {

	var octaves = Math.log(size) / Math.LN2;
	var a = 1, s = 1;
	var cloud = new Texture(s, s, 1);
	cloud.ClearChannel();
	while ( octaves-- > 0 ) {
		
		cloud.AddNoise(a);
		a *= amp;
		s *= 2;
		cloud.Resize(s, s, false);
		cloud.BoxBlur(3, 3);
	}
	cloud.NormalizeLevels();
	return cloud;
}

function CreateCloudTextureLayer() {
	
	var texture = Cloud(32, 1);
	var gaussian = new Texture(texture.width, texture.height, 1);
	gaussian.Set(0);
	var curveGaussian = function(c) { return function(x) { return Math.exp( -(x*x)/(2*c*c) ) } }
	gaussian.AddGradiantRadial(curveGaussian(0.4), false);
	gaussian.Add(-0.1);
	texture.Mult(gaussian);

	with (Ogl) {
		var tid = GenTexture();
		BindTexture(TEXTURE_2D, tid);
		DefineTextureImage(TEXTURE_2D, undefined, texture);
		TexParameter(TEXTURE_2D, TEXTURE_MIN_FILTER, LINEAR);
		TexParameter(TEXTURE_2D, TEXTURE_MAG_FILTER, LINEAR);
		return tid;
	}
}

var perspective = new Transformation();
var mat = new Transformation();

var cx, cy, cz; // camera
var vx=0, vy=0, vz=2.5, t=0;
var lines = [];

GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
GlSetAttribute( GL_DOUBLEBUFFER, 1 );
GlSetAttribute( GL_DEPTH_SIZE, 16 );
SetVideoMode( 200, 200, 32, HWSURFACE | OPENGL | RESIZABLE ); // | ASYNCBLIT // RESIZABLE FULLSCREEN

Texture.RandSeed(1234);

var cloudLayerList = [ [ Texture.RandReal()-0.5, Texture.RandReal()-0.5, Texture.RandReal()-0.5, CreateCloudTextureLayer() ] for (i in new Range(0, 4)) ];

with (Ogl) {

	Hint(PERSPECTIVE_CORRECTION_HINT, NICEST);

//		PointParameter( POINT_SIZE_MIN, 0 );
//		PointParameter( POINT_SIZE_MAX, 1 );
		PointParameter( POINT_DISTANCE_ATTENUATION, [0, 0, 0.01/PixelWidthFactor(), 0] ); // 1/(a + b*d + c *d^2)

	Enable(POINT_SPRITE); // http://www.informit.com/articles/article.aspx?p=770639&seqNum=7
	TexEnv(POINT_SPRITE, COORD_REPLACE, TRUE);

	TexEnv(TEXTURE_ENV, TEXTURE_ENV_MODE, MODULATE);
//	TexEnv(TEXTURE_ENV, TEXTURE_ENV_COLOR, [1,1,1,0]);

	Enable(BLEND);

	// color = polygon * src + screen * dst
//	BlendFunc(ONE, ONE);
	BlendFunc(ONE_MINUS_DST_COLOR, ONE);


//	BlendFunc(DST_ALPHA, ONE_MINUS_DST_ALPHA);

// see. http://jerome.jouvie.free.fr/OpenGl/Tutorials/Tutorial9.php

	ClearColor(0.2,0.1,0.4,1);
}	
	

var listeners = {
	onQuit: function() { end = true },
	onKeyDown: function(key, mod) { end = key == K_ESCAPE },
	onVideoResize: function(w,h) { Ogl.Viewport(0, 0, w, h) },
	
	onMouseButtonDown: function(button, x, y) {
		if ( button == BUTTON_LEFT ) {
			
			x = 2*x/videoWidth - 1;
			y = -(2*y/videoHeight - 1);
			
			var p1 = mat.TransformVector([x,y,0,1]);
			p1[0] /= p1[3];
			p1[1] /= p1[3];
			p1[2] /= p1[3];

			var p2 = mat.TransformVector([x,y,1,1]);
			p2[0] /= p2[3];
			p2[1] /= p2[3];
			p2[2] /= p2[3];
			
			lines.push([p1,p2]);
		}
		if ( button == BUTTON_RIGHT ) {
			showCursor = false;
			grabInput = true;
		}
	},
	onMouseButtonUp: function(button) {
		
		if ( button == BUTTON_RIGHT ) {
			showCursor = true;
			grabInput = false;
		}
		if ( button == BUTTON_WHEELUP )
			vz -= modifierState & KMOD_LCTRL ? 0.1 : 1;
		if ( button == BUTTON_WHEELDOWN )
			vz += modifierState & KMOD_LCTRL ? 0.1 : 1;
		
	},
	onMouseMotion: function(px,py,dx,dy,button) {
		if ( grabInput ) {
			vx += dx;
			vy += dy;
		}
	}
}


Ogl.MatrixMode(Ogl.PROJECTION);
Ogl.Perspective(90, 0.1, 1000);
perspective.Load(Ogl);
Ogl.Enable(Ogl.DEPTH_TEST);

Ogl.MatrixMode(Ogl.MODELVIEW);

//Ogl.Enable(Ogl.LINE_SMOOTH);
Ogl.LineWidth(2);
Ogl.PointSize(2);


for (var end = false; !end ;) {
	
	t = TimeCounter();
	
	PollEvent(listeners);
	
	Ogl.LoadIdentity();

	var tmp = Math.cos(vy/400);
	cx = -Math.cos(vx/400)*Math.abs(tmp) * vz;
	cy = Math.sin(vx/400)*Math.abs(tmp) * vz;
	cz = Math.sin(vy/400) * vz;
	Ogl.LookAt(cx, cy, cz, 0,0,0, 0,0,1);

	mat.Load(Ogl);
	mat.Product(perspective)
	mat.Invert();
	
	with (Ogl) {

		Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);

		Enable(TEXTURE_2D);
		Enable(POINT_SPRITE);
		Disable(DEPTH_TEST);
		
		if (CondNewList('cloud', true)) {

			// var color = 0.3 + (Math.sin(t/400)/2+0.5) * 0.05;
			for each ( var [x,y,z, tid] in cloudLayerList ) {

				if ( false ) {
					
					Color(0.5, 0.4, 0.2);	
					BindTexture(TEXTURE_2D, tid);
					Begin(POINTS);
					Vertex(x, y, z);
					End();
				} else {
					
					var dis = Vector3Length(cx-x, cy-y, cz);
					
					PushMatrix();
					Translate(x, y);
					KeepTranslation(); // unlistable

					var fx = dis > 1 ? 1 : dis;
					Color(0.5 * fx, 0.4 * fx, 0.2 * fx);	
					Begin(QUADS);
					TexCoord( 0, 0 ); Vertex( -1, -1 );
					TexCoord( 1, 0 ); Vertex( +1, -1 );
					TexCoord( 1, 1 ); Vertex( +1, +1 );
					TexCoord( 0, 1 ); Vertex( -1, +1 );
					End();
					
					PopMatrix();
				}
			}
			EndList();
		}

//		PushMatrix();
//		KeepTranslation();
//		Color(0,1,0);
//		DrawDisk(1);
//		PopMatrix();

		Disable(TEXTURE_2D);
		Enable(DEPTH_TEST);
		Axis(1);

		for each ( var [p1, p2] in lines ) {

//			Disable( TEXTURE_2D );
//			Disable(POINT_SPRITE);
//			DrawPoint(2);

			Color(1,1,1);

			Begin(LINES);
			Vertex( p1[0], p1[1], p1[2] );
			Vertex( p2[0], p2[1], p2[2] );
			End(LINES);
			
/*
//			Ogl.PushMatrix();
//			LookAt(p1[0], p1[1], p1[2],  -Math.cos(vx/500)*1,Math.sin(vx/500)*1,vy/1000+1, 0,0,1);
			Begin(QUADS);
			Color(1,1,1);
			Vertex( p1[0], p1[1], p1[2] );
			Vertex( p1[0], p1[1], p1[2]+0.1 );
			Vertex( p2[0], p2[1], p2[2] );
			Vertex( p2[0], p2[1], p2[2]+0.1 );
			End(QUADS);
//			Ogl.PushMatrix();
*/
		}
	}

	GlSwapBuffers();
	Sleep(20);
}










Halt(); //////////////////////////////////////////////////////////////////////

var t = new Transformation(null);
t.LookAt(0,0,0, 1,0,0, 0,0,1);
t.Translate(0, 0, 1);

var v = [0,0,0];
Print( [c.toFixed(2) for each ( c in t.TransformVector(v) )].join('  '), '\n' );


Halt(); //////////////////////////////////////////////////////////////////////


var t = new Transformation(null);
t.Translate(0, 0, 0);
t.RotateZ(-90);
t.Translate(2, 0, 0);
t.RotateY(360);
t.RotateZ(45);
t.Rotate(45, 0,0,1);
t.Translate(1, 0, 0);
t.RotateY(90);
t.Translate(1, 0, 0);
t.RotateY(360);
t.Translate(1, 0, 0);
t.RotateX(360);
t.Translate(1, 0, 0);
t.RotateY(123);
t.Translate(5, 6, 7);
t.Translate(-5, -6, -7);

var v = [0,0,0];
Print( [c.toFixed(2) for each ( c in t.TransformVector(v) )].join('  '), '\n' );
t.Invert();
Print( [c.toFixed(1) for each ( c in t.TransformVector(v) )].join('  '), '\n' );
Print( '\n\n' );


Halt(); //////////////////////////////////////////////////////////////////////



for ( var i = 0; i < 16; i++ )
	Print( i%4 ? ' ' : '  ', t[i].toFixed(2) );
Print( '\n' );





Halt();

var t = new Transformation(undefined);
t.LookAt(-10,-10,10, 0,0,0, 0,0,1);
		
Transformation.Test();
Halt();



var t = new Transformation( undefined );
t.LookAt(1,1,10, 0,0,0);

DumpMatrix(t);

Halt();


function Axis(size) {

	with (Ogl) {
	
		LineWidth(1);
		Begin(LINES);
		Color( 1,0,0, 0.5 ); Vertex( 0,0,0 ); Vertex( size,0,0 );
		Color( 0,1,0, 0.5 ); Vertex( 0,0,0 ); Vertex( 0,size,0 );
		Color( 0,0,1, 0.5 ); Vertex( 0,0,0 ); Vertex( 0,0,size );
		End();
	}
}

function Quad() {

	with (Ogl) {
		LineWidth(1);
		Disable( TEXTURE_2D );
		Begin(LINE_LOOP);
		Color(1,0.5,0.5);
		Vertex( -1, -1 );
		Color(0.5,1,0.5);
		Vertex( 1, -1 );
		Color(1,0.5,0.5);
		Vertex( 1, 1 );
		Color(0.5,1,0.5);
		Vertex( -1, 1 );
		End();
	}
}

/*
function Quad2D(x0, y0, x1, y1) {

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
	
		Begin(QUADS);
		TexCoord(0.0, 0.0); Vertex(-1.0, -1.0,  1.0);
		TexCoord(1.0, 0.0); Vertex( 1.0, -1.0,  1.0);
		TexCoord(1.0, 1.0); Vertex( 1.0,  1.0,  1.0);
		TexCoord(0.0, 1.0); Vertex(-1.0,  1.0,  1.0);

		TexCoord(1.0, 0.0); Vertex(-1.0, -1.0, -1.0);
		TexCoord(1.0, 1.0); Vertex(-1.0,  1.0, -1.0);
		TexCoord(0.0, 1.0); Vertex( 1.0,  1.0, -1.0);
		TexCoord(0.0, 0.0); Vertex( 1.0, -1.0, -1.0);

		TexCoord(0.0, 1.0); Vertex(-1.0,  1.0, -1.0);
		TexCoord(0.0, 0.0); Vertex(-1.0,  1.0,  1.0);
		TexCoord(1.0, 0.0); Vertex( 1.0,  1.0,  1.0);
		TexCoord(1.0, 1.0); Vertex( 1.0,  1.0, -1.0);

		TexCoord(1.0, 1.0); Vertex(-1.0, -1.0, -1.0);
		TexCoord(0.0, 1.0); Vertex( 1.0, -1.0, -1.0);
		TexCoord(0.0, 0.0); Vertex( 1.0, -1.0,  1.0);
		TexCoord(1.0, 0.0); Vertex(-1.0, -1.0,  1.0);

		TexCoord(1.0, 0.0); Vertex( 1.0, -1.0, -1.0);
		TexCoord(1.0, 1.0); Vertex( 1.0,  1.0, -1.0);
		TexCoord(0.0, 1.0); Vertex( 1.0,  1.0,  1.0);
		TexCoord(0.0, 0.0); Vertex( 1.0, -1.0,  1.0);

		TexCoord(0.0, 0.0); Vertex(-1.0, -1.0, -1.0);
		TexCoord(1.0, 0.0); Vertex(-1.0, -1.0,  1.0);
		TexCoord(1.0, 1.0); Vertex(-1.0,  1.0,  1.0);
		TexCoord(0.0, 1.0); Vertex(-1.0,  1.0, -1.0);
		End();
	}
}

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

SetVideoMode( 320, 200, 32, HWSURFACE | OPENGL | RESIZABLE ); // | ASYNCBLIT // RESIZABLE FULLSCREEN
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




Halt(); //////////////////////////////////////////////////////////////////////////////////////////////////////////

	var camera = new Transformation;
	camera.Clear();

	var objects = [];

	var win = new Window();
	win.title = "Test";
	var x = 800;
	var y = 600;
	var w = 200;
	var h = 200;
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
	
		/*
			1/ Object Coordinates are transformed by the ModelView matrix to produce Eye Coordinates.
			2/ Eye Coordinates are transformed by the Projection matrix to produce Clip Coordinates.
			3/ Clip Coordinate X, Y, and Z are divided by Clip Coordinate W to produce Normalized Device Coordinates.
			4/ Normalized Device Coordinates are scaled and translated by the viewport parameters to produce Window Coordinates.
		*/
		
		// http://www.gamedev.net/community/forums/topic.asp?topic_id=480322
		/*
			Expand to 3D homogenous coordinate. That is make a vector (x, y, z, 1). See below for Z-value.
			Map the X- and Y-value into normalized device coordinates, that is, from [0, viewport size] to the range [-1, 1].
			Multiply by the inverse of the projection matrix.
			Multiply by the inverse of the modelview matrix.
			Perform perspective division.
			The result after perspective division is world space coordinates. Skip the multiplication by the modelview matrix and you get the result in eye space instead.
		*/		
		
		var m = new Transformation();

		var p = new Transformation();
		Ogl.MatrixMode(Ogl.PROJECTION);
		p.Load(Ogl);
		p.Invert();
		
		var c = new Transformation();
		c.Load(camera);
		c.Invert();
	
		// 4/
		var clientRect = win.clientRect;
		var width = clientRect[2] - clientRect[0];
		var height = clientRect[3] - clientRect[1];
//		var viewport = Ogl.GetInteger(Ogl.VIEWPORT, 4);
//		var width = viewport[2] - viewport[0];
//		var height = viewport[3] - viewport[1];
		
		var cursorPosition = win.cursorPosition;
		var px = -1 + 2 * cursorPosition[0] / width;
		var py = -1 + 2 * ( height - cursorPosition[1] ) / height;
		
//		Print( ' px='+px,' py='+py, '\n' );
	
//		var vect = [px / p[0], py / p[5], -1];
		var vect = [px, py, -1, 1];
		
		// 2/
//		p.TransformVector(vect);

		// 1/
		c.TransformVector(vect);
		
		vect[0] /= p[0];
		vect[1] /= p[5];
		

		Print( 'world space coordinates vect '+[ v.toFixed(1) for each (v in vect) ].join(' '), '\n' );

		
		var obj = {};
		obj.createTime = TimeCounter();
		obj.x = vect[0];
		obj.y = vect[1];
		obj.z = vect[2]-1;
		objects.push(obj);
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
	var curveGaussian = function(c) { return function(x) { return Math.exp( -(x*x)/(2*c*c) ) } }
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
	Perspective( 50, 1, 100 );
}


const degToRad = 360/Math.PI;

camera.Translate(0, 0, -20);
//camera.Rotate(45, 1,0,0);

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
/*
			PointSize((-Math.cos(t/1000)+1)*10);
			Begin(POINTS);
			Vertex(obj.x, obj.y);
			End();	
*/

			PushMatrix();
			Translate( obj.x, obj.y, obj.z );
			Axis(1);
			PopMatrix();
			

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

