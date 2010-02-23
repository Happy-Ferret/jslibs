try { 

LoadModule('jsio');
LoadModule('jsimage');
LoadModule('jsstd');
LoadModule('jssdl');
LoadModule('jsgraphics');

GlSetAttribute( GL_DOUBLEBUFFER, 1 );
GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
GlSetAttribute( GL_DEPTH_SIZE, 16 );
GlSetAttribute( GL_ACCELERATED_VISUAL, 1 );

unicodeKeyboardTranslation = true;
keyRepeatDelay = 300;
keyRepeatInterval = 50;
//SetVideoMode(undefined, undefined, undefined, HWACCEL | OPENGL | RESIZABLE | FULLSCREEN);
SetVideoMode(512, 512, 30, OPENGL);
//maxFPS = 60;

Ogl.MatrixMode(Ogl.PROJECTION);
Ogl.Ortho(-10, 10, -10, 10, 0.01, 1000);
//Ogl.Perspective(60, 0.01, 1000);
Ogl.MatrixMode(Ogl.MODELVIEW);

Ogl.Enable( Ogl.TEXTURE_2D );


var frame = 0;

var listenerList = [];


var sparkList = [];

function Dist(x0,y0, x1,y1) Math.sqrt((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0));

function Rnd(min, max) Math.random() * (max-min) + min;
function Rndi(min, max) Math.floor(Math.random() * (max-min) + min);

function Rndv(val, range) val + (Math.random() * range*2) - range;


function Spark(x,y,angle,dist) {
	
	var lines = [x,y];
	var count = 1;
	
	var delta = 0;
	
	var splitAt = Rndv(3, 1);
	var dieAt = Math.floor(Rndv(100, 50));
	
	var dustAt = 100;
	
	var len = Rndv(0.1, 0.05);
	var deltaVar = Rndv(0.015, 0.015);
	
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
		Ogl.Color( deltaVar*10 * li, len * li, (li+0.5)*li );

		if ( count > dustAt ) {
		
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

				sparkList.push( new Spark(x,y,angle, dist + count) );
				splitAt = count + Rndv(1000, 5);
			}

			lines.push(x,y);
		}
	
		count++;
		yield;
	}
}


sparkList.push(new Spark(0,0,Math.PI/2, 0) );
//sparkList.push(new Spark(0,0,-Math.PI/2, 0) );


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
	
	//Ogl.ReadBuffer( Ogl.BACK );
	new File('movie'+frame+'.png').content = EncodePngImage(Ogl.RenderToImage());
	// Create a movie file from single image files (png, jpegs) - http://www.miscdebris.net/blog/2008/04/28/create-a-movie-file-from-single-image-files-png-jpegs/

	GlSwapBuffers(false);
}


var listeners = {
	onMouseButtonDown:function(b, x, y) {
		
		var up = Ogl.UnProject(x,y);
		sparkList.push(new Spark(up[0], up[1], 0, 0) );
//		Print(up,'\n');
	},
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
