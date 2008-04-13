LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsimage');
LoadModule('jsode');
LoadModule('jsgraphics');
LoadModule('jsprotex');

var vk = Exec('WindowsKeys.js');
Exec('winTools.js');


function Axis() {

	with (Ogl) {

		Begin(LINES);
		Color( 1,0,0 );
		Vertex( 0,0,0 );
		Vertex( 1,0,0 );
		Color( 0,1,0 );
		Vertex( 0,0,0 );
		Vertex( 0,1,0 );
		Color( 0,0,1 );
		Vertex( 0,0,0 );
		Vertex( 0,0,1 );
		End();
	}
}

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
	
/*
function ResizeWindow(w, h) {

	with (Ogl) {
		Viewport(0,0,w,h);
		MatrixMode(PROJECTION);
		LoadIdentity();
//		Ortho(0,0,10,10, -10, 10);
		Perspective( 90, -1000, 1000 );
		Print(123)
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

	ClearColor(0,0,0,1);	

	ClearDepth(1);
	Enable(DEPTH_TEST);
	DepthFunc(LEQUAL);

//	DepthRange(1000, -1000);
	
	Hint(PERSPECTIVE_CORRECTION_HINT, NICEST);
	Hint(LINE_SMOOTH_HINT, DONT_CARE);		

	Enable( TEXTURE_2D );
	BindTexture( TEXTURE_2D, GenTexture() );
	TexParameter(TEXTURE_2D, TEXTURE_MIN_FILTER, NEAREST); // GL_LINEAR
	TexParameter(TEXTURE_2D, TEXTURE_MAG_FILTER, NEAREST);
	//		var texture = new Jpeg(new File('R0010235.JPG').Open( File.RDONLY )).Load();
	var texture = new Texture(128, 128, 4);
	texture.Set([0,0,0,0]);
	const curveGaussian = function(c) { return function(x) { return Math.exp( -(x*x)/(2*c*c) ) } }
	texture.AddGradiantRadial( curveGaussian( 0.5 ), 0 );

//	BlendFunc(ONE, ONE);
	BlendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA);
	
	Enable(BLEND);

	DefineTextureImage( TEXTURE_2D, texture );

//		PointParameter( POINT_SIZE_MIN, 0 );
//		PointParameter( POINT_SIZE_MAX, 1 );
		PointParameter( POINT_DISTANCE_ATTENUATION, [0.1, 0, 0] ); // 1/(a + b*d + c *d^2)

	Enable(POINT_SPRITE); // http://www.informit.com/articles/article.aspx?p=770639&seqNum=7
	TexEnv(POINT_SPRITE, COORD_REPLACE, TRUE);

	MatrixMode(PROJECTION);
	Perspective( 60, 0.0001, 1000 );
}


with (Ogl) {

	var list = NewList();

		LineWidth(1);
		Begin(LINES);
		Vertex( 0, 1, 0 );
		Vertex( 0,-1, 0 );
		End();

		Color(1,1,1);
		Enable( TEXTURE_2D );
//		Enable( POINT_SMOOTH );
		PointSize(40);
		Begin(POINTS);
		Vertex( 0, 1, 0);
		Vertex( 0,-1, 0);
		End();

		Color(0.5,0.5,1);
		Disable( TEXTURE_2D );
		Enable(LINE_SMOOTH);
		
	EndList();
}

const degToRad = 360/Math.PI;

function Render(i) {
	with (Ogl) {

		Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
	
		MatrixMode(MODELVIEW);
		LoadIdentity();

		Translate(0, 0, -3);
		Rotate(i,1,1,1);

//		Translate(0, 0, -100);
		
		for (var n=0; n<100; n++) {

				Translate(0, 0, -0.1);
				Rotate(10, 0,0,1);
				CallList(list);
		}

	}
}


for (var i=0; !_quit; i++) {

	MaybeCollectGarbage();
	Ogl.Viewport(0,0,w,h);
	Render(i);
	win.SwapBuffers();
	win.ProcessEvents();
	Sleep(10);
}











/*

Halt(); /////////////////////////////////////////


var world = new World;
world.defaultSurfaceParameters.mu = 5;
world.defaultSurfaceParameters.softERP = 0.1;

world.gravity = [0,0,-9.81];
world.ERP = 1;

var box = [];

var b = new Body(world);
var g = new GeomBox( world.space );
g.body = b;	
b.position = [0,0,10]
box.push(b);


var floor = new GeomPlane(world.space);
floor.impact = function(n,thisgeom,othergeom,pos) { n||Print('impact '+othergeom.body.linearVel+'\n') }

var win = new Window();
win.title = "Test";
var x = 600;
var y = 300;
var w = 500;
var h = 500;
win.rect = [x,y,x+w,y+h];

win.CreateOpenGLContext();

Ogl.LoadIdentity();
var list1 = Ogl.NewList();
var tmp = new Transformation();
for ( let x = -2000; x <= 2000; x += 100 )
	for ( let y = -2000; y <= 2000; y += 100 ) {
		
		Ogl.PushMatrix();
		Ogl.Translate(Math.random()*1000-500, Math.random()*1000-500, Math.random()*1000-500 );
		Cube();
		Ogl.PopMatrix();
	}
Ogl.EndList();


var mouse = new MouseMotion(win);
var camera = new Transformation();
camera.Product( new Transformation().Translation(-1,-1,10) );
var cameraRotation = new Transformation();
cameraRotation.Rotate( 90, 1,0,0 );
//camera.LookAt(0,0,0);


Transformation.prototype.toString = function() {
	
	var mat = camera.Dump();
	var str = '';
	for ( var i = 0; i<4; i++) {
		for ( var j = 0; j<4; j++)
			str += (String(mat[4*j+i]).substr(0,5)) + '  ';
		str += '\n';
	}
	return str;
}

var tw=0, tx=0, ty=0, tz=0;
var speed = 1, run = 0;

mouse.button = function( b, polarity, b1, b2, b3 ) {

	mouse.infiniteMode = b1 || b2 || b3;
	if ( b == 2 )
	  run = polarity ? 1 : 0;
	if ( b == 3 )
		speed = 1;
}

mouse.delta = function( dx,dy,dw, b1,b2,b3 ) {

	if (dw != 0) {
	
		camera.Product(new Transformation().Translation( 0,0, -dw*10 ));
	}
	
	if ( b1 || b2 ) {

		tx += dx;
		ty += dy;
		
		camera.LoadRotation(new Transformation().RotationZ(tx/2).Product( new Transformation().RotationX(-ty/2) ));
	}
}




var time;
function Render() {


	var cameraPosition = new Transformation();
	cameraPosition.Translation( 0, 0, -speed * run );
	camera.Product(cameraPosition);
	
	Ogl.Clear( Ogl.COLOR_BUFFER_BIT | Ogl.DEPTH_BUFFER_BIT );

	var m = new Transformation().Load(camera).Invert();

//	Ogl.LoadMatrix(new Transformation().Load(m).Product(body1));
//	Cube();




//	var f = joint.body1Force;
//	Ogl.LoadMatrix(new Transformation().Load(body1).ClearRotation().InverseProduct(m) );
//	Ogl.Color(1,1,1);
//	Ogl.Line3D( 0,0,0, f[0],f[1],f[2] );
//	Ogl.Color(1,0,0);
//	Ogl.Line3D( 0,0,0, f[0],0,0 );
//	Ogl.Color(0,1,0);
//	Ogl.Line3D( 0,0,0, 0,f[1],0 );
//	Ogl.Color(0,0,1);
//	Ogl.Line3D( 0,0,0, 0,0,f[2] );


	for each( var b in box ) {
		Ogl.LoadMatrix(new Transformation().Load(m).Product(b) );
		Cube();
	}
	
//	Ogl.LoadMatrix(new Transformation().Translation(joint.anchor[0], joint.anchor[1], joint.anchor[2]).InverseProduct(m));
//	Ogl.Axis();

	Ogl.LoadMatrix(m);
	Ogl.Color(0.5,0.5,0.5);
	Ogl.Translate(0,0,0);
	for ( var x = -100; x<100; x+=2 )
		for ( var y = -100; y<100; y+=2 )
			Quad(x,y,x+1,y+1);


	Ogl.LoadMatrix(m);
	Ogl.CallList(list1);

//	move += 0.1;
//			tmp.Load(m);
//			tmp.Translate( x, y, -50 );
//			tmp.Translate( x, y, Math.cos(move+(x*x+y*y)/5000)-10 );
//	}


//	var t0 = IntervalNow();
//	Print( 'Rendering time: '+(IntervalNow()-t0)+'ms', '\n');
//	CollectGarbage();
//	Sleep(1);
	win.SwapBuffers();
	
	var tmp = IntervalNow();
	var delta = tmp-time;
	if ( delta > 50)
		delta = 50;
	time && world.Step(delta/1000, 10);
	time = tmp;
}





//win.onidle = Render;

var _fullscreenState = false;
var _savedWindowSize;
var _quit = false;

win.onkeydown = function( key, l ) {

	switch (key) {
		case vk.ESC:
			//win.Exit();
			_quit = true;
			break;
		case vk.ENTER:
			// toggle fullscreen
			_fullscreenState = !_fullscreenState;
			if ( _fullscreenState ) {
			
				_savedWindowSize = win.rect;           // save current window rectangle
				//Window.Mode( [640, 480], 32, true ); // change mode 640x480, 32bits, temporarily fullscreen
				win.showFrame = false;                 // hide window frame ( remove all borders )
				win.rect = Window.desktopRect;         // extends the window to the screen size
				win.showCursor = false;                // hide the cursor
			} else {
			
				win.rect = _savedWindowSize;
				win.showCursor = true;
				win.showFrame = true;
				//Window.Mode();              // revert to default
			}
			break;
		}
}

win.onsize = function( w, h ) {

	Ogl.Viewport(0,0,w,h);
	Ogl.Perspective( 60, 0.01, 10000 );	
	Render();
}

var x=0, y=0; // offset
//var texture = new Jpeg(new File('R0010235.JPG').Open( File.RDONLY )).Load().Trim([0+x,0+y,256+x,256+y], true);
//var texture = new Png(new File('calendar2a.png').Open( File.RDONLY )).Load();//.Trim([0+x,0+y,256+x,256+y], true);


// init
with (Ogl) {

	Enable(TEXTURE_2D);
	ShadeModel(FLAT);
	Enable(DEPTH_TEST);
	DepthFunc(LESS); // LEQUAL cause some z-conflict on far objects !
	// (TBD) understand why
	ClearDepth(1);
//	DepthRange( 0.01, 1000 );
	Enable(CULL_FACE);
	CullFace(BACK);
	FrontFace(CCW);
	Enable(LINE_SMOOTH);
//	Enable(LIGHTING);
//	Enable(LIGHT0);
//	Hint(PERSPECTIVE_CORRECTION_HINT, NICEST); // Really Nice Perspective Calculations
	ClearColor(0, 0, 0, 0);
//    Ogl.BlendFunc(SRC_ALPHA, ONE);
/// init
}



Ogl.Perspective( 60, 0.01, 10000 );
//Ogl.Texture( texture );

//Window.absoluteClipCursor = [100,100, 200, 200 ]

win.Open();

while (!_quit) {
	
	win.ProcessEvents();
	Render();
}

win.Close();
//Window.absoluteClipCursor = undefined;
Print('Done.', '\n');

*/
