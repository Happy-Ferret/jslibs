try { 

LoadModule('jsdebug');
LoadModule('jsstd');
LoadModule('jssdl');
LoadModule('jsgraphics');
LoadModule('jsfont');
LoadModule('jsoglft');

GlSetAttribute( GL_DOUBLEBUFFER, 1 );
GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
GlSetAttribute( GL_DEPTH_SIZE, 16 );
GlSetAttribute( GL_ACCELERATED_VISUAL, 1 );

unicodeKeyboardTranslation = true;
keyRepeatDelay = 300;
keyRepeatInterval = 50;
SetVideoMode(1280, 1024, 32, HWACCEL | OPENGL | RESIZABLE | FULLSCREEN, false);
maxFPS = 60;

Ogl.MatrixMode(Ogl.PROJECTION);
//Ogl.Ortho(-1, 1, -1, 1, 0.01, 100);
Ogl.Perspective(60, 0.01, 1000);
Ogl.MatrixMode(Ogl.MODELVIEW);

Ogl.Enable(Ogl.DEPTH_TEST);
Ogl.DepthFunc(Ogl.LEQUAL);

//Ogl.Enable(Ogl.LIGHTING);
Ogl.Enable(Ogl.LIGHT0);

Ogl.ShadeModel(Ogl.SMOOTH);
Ogl.Material(Ogl.FRONT, Ogl.SPECULAR, [1.0, 1.0, 1.0, 1.0]);
Ogl.Material(Ogl.FRONT, Ogl.SHININESS, 50);

Ogl.ColorMaterial ( Ogl.FRONT_AND_BACK, Ogl.AMBIENT_AND_DIFFUSE ) ;
Ogl.Enable ( Ogl.COLOR_MATERIAL ) ;
Ogl.Color(1,1,1);

Ogl.Enable(Ogl.CULL_FACE);
Ogl.CullFace(Ogl.FRONT);

//Ogl.Enable(Ogl.BLEND);
//Ogl.BlendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);
//Ogl.Enable(Ogl.TEXTURE_2D);

var f = new Font('c:\\windows\\fonts\\arial.ttf');
var f3d = new Font3D(f, Font3D.FILLED, 12);
//f3d.tessellationSteps = 10;
Ogl.PixelStore( Ogl.UNPACK_ALIGNMENT, 1 );

Ogl.LineWidth(1);

var frame = 0;

var listenerList = [];


var sparkList = [];

function Rnd(min, max) Math.random() * (max-min) + min;
function Rndi(min, max) Math.floor(Math.random() * (max-min) + min);

function Rndv(val, range) val + (Math.random() * range*2) - range;


var total = 0;

function Spark(x,y,angle,dist) {
	
	var lines = [x,y];
	var count = 1;
	
	var delta = 0;
	
	var splitAt = Rndv(5, 1);
	var dieAt = Math.floor(Rndv(200, 5));
	
	var len = Rndv(0.2, 0.07);
	var deltaVar = Rndv(0.04, 0.04);
	var way = Math.random() > 0.5 ? 1 : -1;
	
	listenerList.push( function() sparkList.push( new Spark(x,y,angle, dist + count) ) );
	
	var callList;
	for (;;) {
/*		
		for ( var i = 1; i < count; i++ ) {
			
//			Ogl.Color(10 - dist/100);
			Ogl.LineWidth(5 - (dist+i)/5);
			Ogl.Begin( Ogl.LINES );
			Ogl.Vertex(lines[i*2-2],lines[i*2-1]);
			Ogl.Vertex(lines[i*2],lines[i*2+1]);
			Ogl.End();
		}
*/

//		Ogl.LineWidth(total10-dist/100);

//		Ogl.Color(dist/100);

		if ( callList ) {
			
			Ogl.CallList(callList);
		} else {

			if ( count == dieAt )
				callList = Ogl.NewList();

			if ( count <= dieAt ) {

				Ogl.Begin( Ogl.LINE_STRIP );
				for ( var i = 0; i < count; i++ )
					Ogl.Vertex(lines[i*2],lines[i*2+1], (dist+i)/100);
				Ogl.End();
			}

			if ( count == dieAt )
				Ogl.EndList();
		}


		if ( count < dieAt ) {

			delta += deltaVar;

			angle += delta * way;

			x += Math.cos(angle)*len;
			y += Math.sin(angle)*len;

			if ( count > splitAt ) {

//				sparkList.push( new Spark(x,y,angle, dist + count) );
				
//				splitAt = count + Rndv(40, 5);
//				splitAt = 999;
			}



			lines.push(x,y);
			count++;
			total++;
		}
		
		yield;
	
	}

}


sparkList.push(new Spark(0,0,Math.PI/2, 0) );



function SurfaceReady() {

	frame++;

	Ogl.Clear(Ogl.COLOR_BUFFER_BIT | Ogl.DEPTH_BUFFER_BIT);
	Ogl.LoadIdentity();
	
//	Ogl.Translate(0,-0.5, -frame / 50 -10 );
	
	Ogl.LookAt(Math.cos(frame/100)*10, Math.sin(frame/100)*10, frame/100 + 5, 0,0,5, 0,0,1);
	
	for each ( spark in sparkList )
		spark.next();
	

	GlSwapBuffers(true);
}


var listeners = {
	onKeyDown:function(sym, mod, char) {
		
		for each ( listener in listenerList )
			listener(sym, mod, char);

		if ( sym == K_ESCAPE )
			endSignal = true;
	},
	onQuit: function() {
	 
		endSignal = true;
	},
	onVideoResize: function(w, h) {

		ProcessEvents( SurfaceReadyEvents() );
		Ogl.Viewport(0, 0, w, h);
	},
};

while ( !endSignal )
	var e = ProcessEvents( SDLEvents(listeners), EndSignalEvents(), SurfaceReadyEvents(SurfaceReady) );

} catch(ex) {

	throw( ex );
}
