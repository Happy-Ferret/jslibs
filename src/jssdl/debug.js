try { 

LoadModule('jsio');
LoadModule('jsimage');
LoadModule('jsstd');
LoadModule('jssdl');
LoadModule('jsgraphics');
LoadModule('jsprotex');

unicodeKeyboardTranslation = true;
keyRepeatDelay = 300;
keyRepeatInterval = 50;

GlSetAttribute( GL_DOUBLEBUFFER, 1 );
GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
GlSetAttribute( GL_DEPTH_SIZE, 16 );
GlSetAttribute( GL_ACCELERATED_VISUAL, 1 );
SetVideoMode(800, 600, 0, OPENGL | RESIZABLE);

//maxFPS = 60;

//Ogl.Viewport(0,0,videoWidth, videoHeight);
Ogl.MatrixMode(Ogl.PROJECTION);
Ogl.Ortho(0, videoWidth, videoHeight, 0, 0, 1000);
Ogl.MatrixMode(Ogl.MODELVIEW);

var frame = 0;


var objects = [];

var evListeners = {};

function AddEventListener(type, fct) {

	var listeners = evListeners[type] || (evListeners[type] = []);
	listeners.push(fct);
}

function RemoveEventListener(type, fct) {

	evListeners[type].splice(evListeners[type].indexOf(fct), 1);
}

function FireListeners(evName /*, ...*/) {

	var listeners = evListeners[evName];
	if ( listeners ) {
	
		for each ( var fct in listeners )
			if ( fct.apply(this, arguments) === false )
				return;
	}
}

function SetForeground(obj) {
	
	var pos = objects.indexOf(obj);
	if ( pos == 0 )
		return;
	objects.splice(pos, 1);
	objects.unshift(obj);
}

////////////////////////

function Quad(color) {

	var _this = this;
	
	var px = 100;
	var py = 100;
	var w = 100;
	var h = 50;
	var border = 2;
	
	var clist;

	this.Draw = function() {

		Ogl.PushMatrix();
		Ogl.Translate(px, py);
		if ( clist ) {
			
			Ogl.CallList(clist);
		} else {

			clist = Ogl.NewList(Ogl.COMPILE);
			Ogl.Color(color);
			Ogl.Begin(Ogl.QUADS);
			Ogl.Vertex(0,0);  Ogl.Vertex(w,0);  Ogl.Vertex(w,h);  Ogl.Vertex(0,h);
			Ogl.End();

			Ogl.Color(1);
			Ogl.LineWidth(border);
			Ogl.Begin(Ogl.LINE_LOOP);
			Ogl.Vertex(0,0);  Ogl.Vertex(w,0);  Ogl.Vertex(w,h);  Ogl.Vertex(0,h);
			Ogl.End();
			Ogl.EndList();
		}
		Ogl.PopMatrix();
	}
	
	this.HasPoint = function(x,y) {
		
		return x >= px && y >= py && x <= px+w && y <= py+h;
	}
	
	function onmousedown(ev, b, x,y) {

		if ( !_this.HasPoint(x,y) )
			return true;
		
		SetForeground(_this);
		
		var dx = x-px;
		var dy = y-py;
		
		function onmousemove(ev, x1,y1) {
		
			px = x1-dx;
			py = y1-dy;
		}
		
		function onmouseup(ev, b, x,y) {
			
			RemoveEventListener('mousemove', onmousemove);
			RemoveEventListener('mouseup', onmouseup);
		}
		
		AddEventListener('mousemove', onmousemove);
		AddEventListener('mouseup', onmouseup);
		return false;
	}
	
//	AddEventListener('mousedown', onmousedown);
	this.onmousedown = onmousedown;
}


for ( var i = 0; i < 100; i++ ) {

	objects.push(new Quad([1,0,0]));
	objects.push(new Quad([0,1,0]));
	objects.push(new Quad([0,0,1]));
}

////////////////////////

function SurfaceReady() {
	
	frame++;
	
	Ogl.Clear(Ogl.COLOR_BUFFER_BIT|Ogl.DEPTH_BUFFER_BIT);
	Ogl.LoadIdentity();

	for ( var i = objects.length-1; i >= 0; --i )
		objects[i].Draw();

	Ogl.Flush();
	Ogl.Finish();

	GlSwapBuffers(true);
}


var listeners = {
	onMouseButtonDown:function(b, x, y) {
		
		var len = objects.length;
		for ( var i = 0; i < len; ++i ) {
			
			var o = objects[i];
			if ( o.onmousedown && o.onmousedown('onmousedown', b, x, y) === false )
				break;
		}
		
		FireListeners('mousedown', b, x, y);
	},
	onMouseButtonUp:function(b, x, y) {
		
		FireListeners('mouseup', b, x, y);
	},
	onMouseMotion:function(x, y, relx, rely, state, mod) {
	
//		[x,y] = Ogl.UnProject(x,y);
		FireListeners('mousemove', x, y);
	},
	onKeyDown:function(sym, mod, chr) {

		FireListeners('keydown',sym, mod, chr);
		
		if ( sym == K_SPACE ) {
			
			ProcessEvents( SurfaceReadyEvents() );
			SetVideoMode(desktopWidth, desktopHeight, undefined, OPENGL | RESIZABLE | (videoFlags & FULLSCREEN ? 0 : FULLSCREEN));
			Ogl.Viewport(0, 0, videoWidth, videoHeight);
			Ogl.MatrixMode(Ogl.PROJECTION);
			Ogl.LoadIdentity();
			Ogl.Ortho(0, videoWidth, videoHeight, 0, 0, 1000);
			Ogl.MatrixMode(Ogl.MODELVIEW);
		}
		
		if ( sym == K_ESCAPE )
			endSignal = true;
	},
	onQuit: function() {
	 
		endSignal = true;
	},
	onVideoResize: function(w, h) {
	
		Ogl.Viewport(0, 0, w, h);
		Ogl.MatrixMode(Ogl.PROJECTION);
		Ogl.LoadIdentity();
		Ogl.Ortho(0, w, h, 0, 0, 1000);
		Ogl.MatrixMode(Ogl.MODELVIEW);
	},
};


while ( !endSignal )
	var e = ProcessEvents( SDLEvents(listeners), EndSignalEvents(), SurfaceReadyEvents(SurfaceReady) );

} catch(ex) {

	throw( ex );
}
