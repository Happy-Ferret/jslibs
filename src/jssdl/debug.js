try { 

LoadModule('jsio');
LoadModule('jsimage');
LoadModule('jsstd');
LoadModule('jssdl');
LoadModule('jsgraphics');
LoadModule('jsprotex');

GlSetAttribute( GL_DOUBLEBUFFER, 1 );
GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
GlSetAttribute( GL_DEPTH_SIZE, 16 );
GlSetAttribute( GL_ACCELERATED_VISUAL, 1 );

unicodeKeyboardTranslation = true;
keyRepeatDelay = 300;
keyRepeatInterval = 50;

SetVideoMode(320, 200, undefined, HWACCEL | OPENGL | RESIZABLE);

maxFPS = 60;

Ogl.MatrixMode(Ogl.PROJECTION);
Ogl.Ortho(-10, 10, -10, 10, 0.01, 1000);
//Ogl.Perspective(60, 0.01, 1000);
Ogl.MatrixMode(Ogl.MODELVIEW);


const curveGaussian = function(c) { return function(x) { return Math.exp( -(x*x)/(2*c*c) ) } }
var tex = new Texture(16,16, 1).Set(0).AddGradiantRadial( curveGaussian( 0.5 ) );


with (Ogl) {
	Enable( TEXTURE_2D );
	var textureId = GenTexture();
	BindTexture(TEXTURE_2D, textureId);
	DefineTextureImage(TEXTURE_2D, ALPHA, tex);
	TexParameter(TEXTURE_2D, TEXTURE_MIN_FILTER, LINEAR);
	TexParameter(TEXTURE_2D, TEXTURE_MAG_FILTER, LINEAR);
	//		PointParameter(POINT_DISTANCE_ATTENUATION, [0, 0, 0.01]); // 1/(a + b*d + c *d^2)
	Enable(POINT_SPRITE);
	TexEnv(POINT_SPRITE, COORD_REPLACE, TRUE);
	//		TexEnv(TEXTURE_ENV, TEXTURE_ENV_MODE, MODULATE);
			Enable(BLEND);
	//		BlendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA);

// with LUMINANCE only		
	//BlendFunc(SRC_ALPHA, ONE); // radioactive cloud
	BlendFunc(SRC_ALPHA, ONE); // less radioactive cloud
	//BlendFunc(ONE_MINUS_DST_COLOR, ONE); // normal cloud
	//BlendFunc(ONE_MINUS_SRC_COLOR, ONE_MINUS_SRC_COLOR); // normal strange 1
	//BlendFunc(ONE_MINUS_DST_COLOR, ONE_MINUS_SRC_COLOR); // normal strange 2
	//BlendFunc(ZERO, ONE_MINUS_SRC_COLOR); // dark cloud
	//BlendFunc(SRC_COLOR, ONE_MINUS_SRC_COLOR); // less dark cloud

	PointSize(6);
}


var frame = 0;
var listenerList = [];
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
	
//	listenerList.push( function() sparkList.push( new Spark(x,y,angle, dist + count) ) );
	
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
					Ogl.Vertex(lines[i*2],lines[i*2+1], (dist+i)/100);
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

	Ogl.Clear(Ogl.COLOR_BUFFER_BIT);
	Ogl.LoadIdentity();
	
	Ogl.Translate(0,-0.5, -frame / 50 -10 );
//	Ogl.Translate(0,0,Rnd(-0.1,0.1) );
//	Ogl.LookAt(Math.cos(frame/100)*10, Math.sin(frame/100)*10, frame/100 + 5, 0,0,5, 0,0,1);
	
	for each ( spark in sparkList )
		try {
			spark.next();
		} catch( ex if ex instanceof StopIteration) {}


	Ogl.Flush();
	Ogl.Finish();

	// Ogl.ReadBuffer( Ogl.BACK );
	//	new File('movie'+frame+'.png').content = EncodePngImage(Ogl.RenderToImage());
	// Create a movie file from single image files (png, jpegs) - http://www.miscdebris.net/blog/2008/04/28/create-a-movie-file-from-single-image-files-png-jpegs/

	GlSwapBuffers(true);
}


var listeners = {
	onMouseButtonDown:function(b, x, y) {
		
		var up = Ogl.UnProject(x,y);
		sparkList.push(new Spark(up[0], up[1], 0, 0) );
//		Print(up,'\n');
	},
	onKeyDown:function(sym, mod, char) {
		
		if ( sym == K_SPACE ) {
			
			ProcessEvents( SurfaceReadyEvents() );
			SetVideoMode(desktopWidth, desktopHeight, 3, HWACCEL | OPENGL | RESIZABLE | (videoFlags & FULLSCREEN ? 0 : FULLSCREEN));
			Ogl.Viewport(0, 0, videoWidth, videoHeight);
		}
		
		for each ( listener in listenerList )
			listener(sym, mod, char);

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
	var e = ProcessEvents( SDLEvents(listeners), EndSignalEvents(), SurfaceReadyEvents(SurfaceReady) );

} catch(ex) {

	throw( ex );
}
