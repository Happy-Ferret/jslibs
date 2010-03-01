try { 

LoadModule('jsstd');
LoadModule('jssdl');
LoadModule('jsgraphics');
LoadModule('jsprotex');

GlSetAttribute( GL_DOUBLEBUFFER, 1 );
GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
GlSetAttribute( GL_ACCELERATED_VISUAL, 1 );
SetVideoMode(640,480, undefined, OPENGL);

maxFPS = 60;


const curveGaussian = function(c) { return function(x) { return Math.exp( -(x*x)/(2*c*c) ) } }
var tex = new Texture(16,16, 1).Set(0).AddGradiantRadial( curveGaussian( 0.5 ) );

with (Ogl) {

	Enable( TEXTURE_2D );
	var textureId = GenTexture();
	BindTexture(TEXTURE_2D, textureId);
	DefineTextureImage(TEXTURE_2D, ALPHA, tex);
	TexParameter(TEXTURE_2D, TEXTURE_MIN_FILTER, LINEAR);
	TexParameter(TEXTURE_2D, TEXTURE_MAG_FILTER, LINEAR);
	Enable(POINT_SPRITE);
	TexEnv(POINT_SPRITE, COORD_REPLACE, TRUE);
	Enable(BLEND);
	BlendFunc(SRC_ALPHA, ONE);
	PointSize(6);
	ClearDepth(0);
	ClearColor(0,0,0,0);
	MatrixMode(PROJECTION);
	Ortho(-10, 10, -10, 10, 0, 1);
	MatrixMode(MODELVIEW);
}


var frame = 0;
var sparkList = [];

function Dist(x0,y0, x1,y1) Math.sqrt((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0));
var Int = Math.floor;
function Rnd(min, max) Math.random() * (max-min) + min;
function Rndi(min, max) Math.floor(Math.random() * (max-min) + min);
function Rndv(val, range) val + (Math.random() * range*2) - range;



function Spark(x,y,angle,dist) {
	
	var lines = [x,y];
	var count = 1;
	
	var splitAt = Int(Rndv(10, 9));
	var dieAt = Int(Rndv(50, 10));
	
	var dustAt = 200;
	
	var len = Rnd(0.05, 0.2);
	var deltaVar = Rnd(0, 0.01);
	
	var m = Ogl.UnProject(mouseX, mouseY);
	var d1 = Dist( m[0], m[1], x + Math.cos(angle+deltaVar)*len, y + Math.sin(angle+deltaVar)*len );
	var d2 = Dist( m[0], m[1], x + Math.cos(angle-deltaVar)*len, y + Math.sin(angle-deltaVar)*len );
	var way = d1 > d2 ? -1 : 1;
	
//	var way = Math.random() > 0.5 ? 1 : -1;
	

	var r = Rnd(0,1);
	var g = Rnd(0,1);
	var b = Rnd(0,1);
	
	var callList;
	for (;;) {

//		Ogl.LineWidth(1+count/dustAt);

		var li = 1-count/dustAt;
		Ogl.Color( li* (deltaVar*10), li* (deltaVar*10), li > 0.5 ? li+1 : li*2 );

		if ( count == dustAt ) {
		
			if ( callList )
				Ogl.DeleteList(callList);
			return;
		}

		if ( callList ) {
			
			Ogl.CallList(callList);
		} else {

			if ( count == dieAt )
				callList = Ogl.NewList();

			if ( count <= dieAt ) {

				Ogl.Begin( Ogl.POINTS ); // LINE_STRIP
				for ( var i = 0; i < count; i++ )
					Ogl.Vertex(lines[i*2],lines[i*2+1], 0);
				Ogl.End();
			}

			if ( count == dieAt )
				Ogl.EndList();
		}


		if ( count < dieAt ) {

			x += Math.cos(angle)*len;
			y += Math.sin(angle)*len;

			angle += deltaVar * count * way;

			lines.push(x,y);
		}

		if ( count == splitAt ) {

			sparkList.push( new Spark(x,y,angle, dist + count) );
		}
	
		count++;
		yield;
	}
}


sparkList.push(new Spark(0,0,Math.PI/2, 0) );


function SurfaceReady() {
	
	frame++;

	Ogl.Clear(Ogl.COLOR_BUFFER_BIT|Ogl.DEPTH_BUFFER_BIT);

	for each ( spark in sparkList )
		try {
			spark.next();
		} catch( ex if ex instanceof StopIteration) {}


	Ogl.Flush();
	Ogl.Finish();

	GlSwapBuffers(true);
}


var listeners = {
	onMouseButtonDown:function(b, x, y) {
		
		var up = Ogl.UnProject(x,y);
		sparkList.push(new Spark(up[0], up[1], Math.PI/2, 0) );
	},
	onKeyDown:function(sym, mod, chr) {
		
		if ( sym == K_SPACE ) {
			
			ProcessEvents( SurfaceReadyEvents() );
			SetVideoMode(desktopWidth, desktopHeight, undefined, OPENGL | RESIZABLE | (videoFlags & FULLSCREEN ? 0 : FULLSCREEN));
			Ogl.Viewport(0, 0, videoWidth, videoHeight);
		}
		
		if ( sym == K_ESCAPE )
			endSignal = true;
	},
	onQuit: function() {
	 
		endSignal = true;
	},
	onVideoResize: function(w, h) {

		Ogl.Viewport(0, 0, w, h);
	},
};


while ( !endSignal )
	ProcessEvents( SDLEvents(listeners), EndSignalEvents(), SurfaceReadyEvents(SurfaceReady) );

} catch(ex) {

	throw( ex );
}
