LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsimage');
LoadModule('jsode');
LoadModule('jsprotex');
LoadModule('jstrimesh');
LoadModule('jssdl');
LoadModule('jsgraphics');

function CreateOpenGLWindow() {

	GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
	GlSetAttribute( GL_DOUBLEBUFFER, 1 );
	GlSetAttribute( GL_DEPTH_SIZE, 16 );
	GlSetAttribute( GL_ACCELERATED_VISUAL, 1 );
	SetVideoMode( 640, 480, 32, OPENGL | RESIZABLE ); // | ASYNCBLIT // RESIZABLE FULLSCREEN
	Ogl.Viewport(0,0,640,480);
}

var textureId;
function DisplayTexture( texture ) {

	if ( !textureId )
		textureId = Ogl.GenTexture();
	Ogl.BindTexture(Ogl.TEXTURE_2D, textureId);

	Ogl.DefineTextureImage(Ogl.TEXTURE_2D, undefined, texture);
	Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MIN_FILTER, Ogl.LINEAR);
	Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MAG_FILTER, Ogl.LINEAR);

	Ogl.MatrixMode(Ogl.PROJECTION);
	Ogl.LoadIdentity();
//	Ogl.Ortho(0, width, 0, height, -1, 1);
	Ogl.MatrixMode(Ogl.MODELVIEW);
	Ogl.LoadIdentity();

	Ogl.Enable(Ogl.TEXTURE_2D);
	Ogl.Disable(Ogl.DEPTH_TEST);

	Ogl.Clear(Ogl.COLOR_BUFFER_BIT);
	
//	Ogl.Translate(0,0,-1);
	
	Ogl.Begin(Ogl.QUADS);
	Ogl.TexCoord(0, 0); Ogl.Vertex(-1,1);
	Ogl.TexCoord(1, 0); Ogl.Vertex(1,1);
	Ogl.TexCoord(1, 1); Ogl.Vertex(1,-1);
	Ogl.TexCoord(0, 1); Ogl.Vertex(-1,-1);
	Ogl.End();
	
	GlSwapBuffers();
}

function IsEnd() {
	
	var end;
	while (PollEvent({ onKeyDown: function(key, mod) end = key == 27 }));
	return end;
}

function GetKey() {
	
	var key;
	while (PollEvent({ onKeyDown: function(k) key = k }));
	return key;
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

function Dump() {
	
	for ( var i = 0; i < arguments.length; i++ )
		Print(uneval(arguments[i]), '  ');
	Print('\n');
}

function RunLocalQAFile() {
	
	LoadModule('jsio');
	global.QA = { __noSuchMethod__:function(id, args) {
		Print( id, ':', uneval(args), '\n' )
	} };
	Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');
	throw 0;
}


function RunQATests( argStr ) {

	LoadModule('jsio');
	currentDirectory += '/../..';
	global.arguments = Array.concat('qa.js', argStr.split(' '));
	Exec(global.arguments[0], false);
	throw 0;
}

function RunJsircbot( withDebuggerEnabled ) {

	LoadModule('jsio');
	LoadModule('jsdebug');
	withDebuggerEnabled && Exec('../jsdebug/debugger.js', false);
	currentDirectory += '/../../../jsircbot';
	global.arguments[1] = 'my_configuration.js'; // simulate: jshost main.js my_configuration.js
	Exec('main.js', false);
	throw 0;
}