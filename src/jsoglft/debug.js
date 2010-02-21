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
SetVideoMode(300, 300, 32, HWACCEL | OPENGL | RESIZABLE, false);
maxFPS = 30;

Ogl.MatrixMode(Ogl.PROJECTION);
//Ogl.Ortho(0, 10, 0, 100, 0.01, 100);
Ogl.Perspective(60, 0.01, 100);
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

f3d.SetBackgroundColor([0,0,0]);
f3d.SetColor([1,0,0]);
var frame = 0;

function SurfaceReady() {

	frame++;

	var t = TimeCounter();

	Ogl.Clear(Ogl.COLOR_BUFFER_BIT | Ogl.DEPTH_BUFFER_BIT);
	Ogl.LoadIdentity();
	
//	Ogl.Light(Ogl.LIGHT0, Ogl.POSITION, [0, Math.sin(frame/10)*10, Math.cos(frame/10)*10, 0]);

	Ogl.Translate(-5,5,-15);

//	Ogl.Rotate(frame, 0,1,1);

	Ogl.Scale(0.1);
	Ogl.Scale(1,1,10);
//	Print( f3d.Measure('test', true)[3], '         \r' );

//	Ogl.RasterPos(0,0,0)
//	f3d.advance = false;
//	f3d.Draw('123', undefined, 0, 0);
//	f3d.Draw('456', undefined, 0, -f3d.height);
// f3d.Draw('789', undefined, 0, -f3d.height * 2);

	te.Draw();

	GlSwapBuffers(true);
//	Print((1000/(TimeCounter() - t)).toFixed(2), 'fps               \r');
}

function TextEdit(text) {

	var cursor = Ogl.NewList();
	Ogl.Color(1,0,0);
	Ogl.Begin(Ogl.QUADS);
	Ogl.Vertex(0,0);
	Ogl.Vertex(0,10);
	Ogl.Vertex(10,10);
	Ogl.Vertex(10,0);
	Ogl.End();
	Ogl.EndList();

	text = text || '';
	var pos = text.length;
	this.KeyEventHandler = function(sym, char) {
		
		if ( char >= ' ' )
			text = text.substr(0, pos) + char + text.substr(pos++);
		else if ( sym == K_RETURN || sym == K_KP_ENTER )
			text = text.substr(0, pos) + '\n' + text.substr(pos++);
		else if ( sym == K_LEFT && pos > 0 )
			pos--;
		else if ( sym == K_RIGHT && pos < text.length )
			pos++;
		else if ( sym == K_END ) {
			var next = text.indexOf('\n', pos);
			if ( next == -1 )
				pos = text.length;
			else
				pos = next;
		} else if ( sym == K_HOME ) {
			var prev = text.lastIndexOf('\n', pos-1);
			if ( prev == -1 )
				pos = 0;
			else
				pos = prev+1;
		} else if ( sym == K_BACKSPACE && pos > 0 )
			text = text.substr(0, --pos) + text.substr(pos+1);
		else if ( sym == K_DELETE && pos <= text.length )
			text = text.substr(0, pos) + text.substr(pos+1);
		else if ( sym == K_UP ) {
	
			var prev = text.lastIndexOf('\n', pos-1);
			if ( prev == -1 )
				return;
			var offset = pos - prev;
			var min = text.lastIndexOf('\n', prev-1);
			if ( offset > prev - min )
				pos = prev;
			else
				if ( min < 0 )
					pos = offset-1;
				else
					pos = min + offset;
					
		} else if ( sym == K_DOWN ) { // abc\ndefgh\nijk

			var next = text.indexOf('\n', pos) + 1;
			if ( next == 0 )
				return;
			var p = text.lastIndexOf('\n', pos-1) + 1;
			var offset = pos - p;
			var max = text.indexOf('\n', next);
			if ( max == -1 )
				max = text.length;
			if ( next + offset > max )
				pos = max;
			else
				pos = next + offset;
		}
	}
	
	this.Draw = function() {

		Ogl.LineWidth(2);
		
		var lines = text.split('\n');
		var y = 0, x = 0;
		
		var len = text.length;
		for ( var i = 0; i < len; i++ ) {
		
			Ogl.PushMatrix();
			var rnd = Math.random();
			Ogl.Translate(x, y);

			Ogl.Translate(Math.sin((frame/10+i)/20)*20, 0);
			Ogl.Rotate(Math.sin((frame+i)/10)*10,0,0,1);
			Ogl.Scale(1+Math.sin((frame+i)/20)/5);
			
		
			if ( i == pos ) {

				Ogl.Begin(Ogl.LINES);
				Ogl.Color(1,1,1);
				Ogl.Vertex(x,y);
				Ogl.Vertex(x,y+f3d.height);
				Ogl.End();
			}
			
			var chr = text[i];
			if ( chr == '\n' ) {

				y -= f3d.height;
				x = 0;
			} else if ( chr == ' ' ) {
			
				x += f3d.height / 3;
			} else {
			
				f3d.Draw(chr);
				x += f3d.Width(chr);
			}

//			Ogl.Rotate(rnd,0,0,-1);
			Ogl.PopMatrix();
		}
	}
}

var te = new TextEdit('abc\ndefgh\nijk');

var listeners = {
	onKeyDown:function(sym, mod, char) {

		te.KeyEventHandler(sym, char);
		
		if ( sym == K_ESCAPE )
			endSignal = true;
	},
	onQuit: function() {
	 
		endSignal = true;
	},
	onVideoResize: function(w, h) {

		ProcessEvents( SurfaceReadyEvents() );
		Ogl.Viewport(0, 0, w, h);
//		Print(w,'x',h, '\n');
	},
};

while ( !endSignal )
	var e = ProcessEvents( SDLEvents(listeners), EndSignalEvents(), SurfaceReadyEvents(SurfaceReady) );

} catch(ex) {

	Print( ex );
}
