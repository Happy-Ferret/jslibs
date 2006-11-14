LoadModule('jsstd');
LoadModule('jsnspr');
LoadModule('jsimage');
LoadModule('jsode');
LoadModule('jsgraphics');
var glc = Exec('OpenGL.js');
var vk = Exec('WindowsKeys.js');

Exec('winTools.js');

var image=0;

/*
var world = new World;
//world.gravity = [0,0,-0.81];
var body1 = new Body(world);
var body2 = new Body(world);
var joint = new JointHinge(world);
joint.Attach(body1,body2);
joint.anchor = [4,4,0];
joint.axis = [0,0,1];
//joint.loStop = 0;
//joint.hiStop = -1;
//joint.bounce = 0.5;
//Print('bounce: '+ joint.bounce);
body1.linearVel = [0,10,0];
body2.linearVel = [0,-10,0];
//body1.angularVel = [0,0,5];
*/


var world = new World;
//world.Body = Body;
//var b = new world.Body();

world.gravity = [0,0,-9.81];

var body1 = new Body(world);
var geom1 = new GeomBox( world.space )
geom1.body = body1;

var body2 = new Body(world);
var geom2 = new GeomBox( world.space );
geom2.body = body2;



//body1.mass.SetBoxTotal(10,[1,1,100]);
//body.mass.mass = 1;
//body.mass.Translate([2,1,0]);
//body.mass.center = [2,0,0];
//body.mass.Adjust(10);

//var joint = new JointHinge(world);

//joint.Attach(body,body1);
//joint.anchor = [10,0,0];
//joint.axis = [1,1,1];

body1.position = [0,0,0]
body1.linearVel = [0,0,5];

body2.position = [0,0,5]
body2.linearVel = [0,0,-5];

new GeomPlane(world.space);




var win = new Window();
win.title = "Test";
var x = 600;
var y = 300;
var w = 500;
var h = 500;
win.rect = [x,y,x+w,y+h];
var gl = new Gl(win);

	gl.LoadIdentity();
	var list1 = gl.StartList();
	var tmp = new Transformation();
	for ( let x = -2000; x <= 2000; x += 100 )
		for ( let y = -2000; y <= 2000; y += 100 ) {
			
			gl.PushMatrix();
			gl.Translate(Math.random()*1000-500, Math.random()*1000-500, Math.random()*1000-500 );
			gl.Cube();
			gl.PopMatrix();
		}
	gl.EndList();


var mouse = new MouseMotion(win);
var camera = new Transformation();
camera.Product( new Transformation().Translation(-1,-1,10) );
//camera.Rotate( 90, 1,0,0);
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

var move = 0;

function Render() {

	var cameraPosition = new Transformation();
	cameraPosition.Translation( 0,0, -speed * run );
	camera.Product(cameraPosition);
	

	gl.Clear( glc.COLOR_BUFFER_BIT | glc.DEPTH_BUFFER_BIT );

	gl.LoadIdentity();
	gl.Color( 1,1,1 );
	for ( var x = -10; x<10; x++ )
		for ( var y = -10; y<10; y++ )
			gl.Quad(x,y,x+1,y+1);



	var m = new Transformation().Load(camera).Invert();

	gl.LoadMatrix(new Transformation().Load(m).Product(body1));
	gl.Cube();

	gl.LoadMatrix(new Transformation().Load(m).Product(body2));
	gl.Cube();



//	gl.LoadMatrix(new Transformation().Translation(joint.anchor[0], joint.anchor[1], joint.anchor[2]).InverseProduct(m));
//	gl.Axis();

	gl.LoadMatrix(m);
	gl.CallList(list1);

//	move += 0.1;
//			tmp.Load(m);
//			tmp.Translate( x, y, -50 );
//			tmp.Translate( x, y, Math.cos(move+(x*x+y*y)/5000)-10 );
//	}


	Sleep(1)
//	var t0 = IntervalNow();
//	Print( 'Rendering time: '+(IntervalNow()-t0)+'ms', '\n');
	CollectGarbage();
	gl.SwapBuffers();
	world.Step(0.005,true);
}





win.onidle = Render;

var _fullscreenState = false;
var _savedWindowSize;

win.onkeydown = function( key, l ) {

	switch (key) {
		case vk.ESC:
			win.Exit();
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

	gl.Viewport([0,0,w,h]);
	gl.Perspective( 60, 0.01, 10000 );	
	Render();
}

var x=0, y=0; // offset
var texture = new Jpeg(new File('R0010235.JPG').Open( File.RDONLY )).Load().Trim([0+x,0+y,256+x,256+y], true);

gl.Init();
gl.Perspective( 60, 0.01, 10000 );
//gl.Texture( texture );

//Window.absoluteClipCursor = [100,100, 200, 200 ]
win.ProcessEvents();
//Window.absoluteClipCursor = undefined;
Print('Done.', '\n');


