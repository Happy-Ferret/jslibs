LoadModule('jsstd');
LoadModule('jsode');
LoadModule('jsimage');

Exec('../common/tools3d.js');

LoadModule('jsvideoinput');

var ui = new UI(320, 320);

var world = new World();
world.quickStepNumIterations = 5;
world.gravity = [0,0,-9.8 / 1];
world.linearDamping = 0.01;
world.angularDamping = 0.01;
//world.linearDampingThreshold = 0;
//world.angularDampingThreshold = 0;

world.defaultSurfaceParameters.mu = 100;//Infinity; // doc. 0 results in a frictionless contact, and dInfinity results in a contact that never slips.
world.defaultSurfaceParameters.softERP = 0.5; // Joint error and the error reduction parameter (ERP)
world.defaultSurfaceParameters.softCFM = 0.01; // Soft constraint and constraint force mixing (CFM)
world.defaultSurfaceParameters.bounce = 0;
//world.defaultSurfaceParameters.bounceVel = 2;


function Floor() {
	
	var self = this;

	this.castShadow = false;
	this.receiveShadow = true;

	this.geom = new GeomPlane(world.space);
	this.geom.params = [0,0,1,0];
	this.geom.name = 'floor';
	
	var shapeCL, objectCL;
	this.Compile = function() {

		var cx = 10, cy = 10;
		var sx = 5, sy = 5;

		objectCL = Ogl.NewList(true);

		Ogl.Material(Ogl.FRONT, Ogl.AMBIENT, 0, 0, 0, 1);
		Ogl.Material(Ogl.FRONT, Ogl.EMISSION, 0, 0, 0, 1);

		//Ogl.Scale(5);
		Ogl.Normal(0, 0, 1);
		Ogl.Begin(Ogl.QUADS);
		for ( var x = -cx; x < cx; x++ )
			for ( var y = -cy; y < cy; y++ ) {
				if ( (x + y) % 2 )
					Ogl.Material(Ogl.FRONT, Ogl.DIFFUSE, 0.3, 0.3, 0.3, 1);
				else
					Ogl.Material(Ogl.FRONT, Ogl.DIFFUSE, 0.8, 0.8, 0.8, 1);
				Ogl.TexCoord(0,0);
				Ogl.Vertex(x*sx,y*sy);
				Ogl.TexCoord(1,0);
				Ogl.Vertex((x+1)*sx,y*sy);
				Ogl.TexCoord(1,1);
				Ogl.Vertex((x+1)*sx,(y+1)*sy);
				Ogl.TexCoord(0,1);
				Ogl.Vertex(x*sx,(y+1)*sy);
			}
		Ogl.End();
		Ogl.EndList();
		return;
	}
	
	this.Render = function( shapeOnly ) {
		
		Ogl.PushMatrix();
		Ogl.CallList(objectCL);
		Ogl.PopMatrix();
	}	

	this.Compile();
}


function Ball(pos) {

	this.castShadow = true;
	this.receiveShadow = true;
	
	var geom = new GeomSphere(world.space);
	var body = new Body(world);
	geom.body = body;
	geom.radius = 4;
	body.mass.value = 200;
	body.position = pos;
	
	geom.contact = function(thisGeom, otherGeom) otherGeom.name != 'floor'; // don't hit the floor
	
	this.body = body;
	this.geom = geom;
	
	var shapeCL, objectCL;
	this.Compile = function() {

		shapeCL = Ogl.NewList(true);
		Ogl.DrawSphere(geom.radius, 16, 8, true);  // Ogl.DrawSphere(geom.radius, 4, 2); // diamond
		Ogl.EndList();
		
		objectCL = Ogl.NewList(true);
		Ogl.Material(Ogl.FRONT, Ogl.DIFFUSE, 1, 1, 1, 1);
		Ogl.Material(Ogl.FRONT, Ogl.EMISSION, 0, 0, 0, 1);
		Ogl.Material(Ogl.FRONT, Ogl.AMBIENT, 0, 0, 0, 1);
		Ogl.Material(Ogl.FRONT, Ogl.SPECULAR, 0.1, 0.1, 0.1, 1);
		Ogl.DrawSphere(geom.radius, 32, 16, true);
		Ogl.EndList();
	}
	
	this.Render = function( shapeOnly ) {
		
		Ogl.PushMatrix();
		Ogl.MultMatrix(geom);
		Ogl.CallList(shapeOnly ? shapeCL : objectCL);
		Ogl.PopMatrix();
	}
	this.Compile();	
}


Box.prototype = {
	Compile:function() {

		this.shapeCL = Ogl.NewList(true);
		Ogl.Scale.apply(null, this.geom.lengths);
		DrawCube(true, false);
		Ogl.EndList();

		this.objectCL = Ogl.NewList(true);
		Ogl.Material(Ogl.FRONT, Ogl.DIFFUSE, 0, 1, 0, 1);
		Ogl.Material(Ogl.FRONT, Ogl.AMBIENT, 0, 0, 0, 1);
		Ogl.Material(Ogl.FRONT, Ogl.SPECULAR, 0, 0, 0, 1);
		Ogl.ShadeModel(Ogl.FLAT);
//		Ogl.CallList(this.shapeCL);
		Ogl.Scale.apply(null, this.geom.lengths);
		DrawCube();
		Ogl.ShadeModel(Ogl.SMOOTH);
		Ogl.EndList();
	},
	Render:function( shapeOnly ) {
	
		Ogl.PushMatrix();
		Ogl.MultMatrix(this.geom);
		if ( shapeOnly ) {
		
			Ogl.CallList(this.shapeCL);
		} else {
	
			Ogl.Material(Ogl.FRONT, Ogl.EMISSION, this.geom.contactVelocityAccu / 5, 0, 0, 1);
			this.geom.contactVelocityAccu /= 1.01
			Ogl.CallList(this.objectCL);
		}
		Ogl.PopMatrix();
	}, 
	GeomContact:function(thisGeom, otherGeom, contactVelocity) {

		this.contactVelocityAccu += contactVelocity;
	}
}

function Box( pos ) {

	this.castShadow = true;
	this.receiveShadow = true;
	
	this.geom = new GeomBox(world.space);
	this.body = new Body(world);
	this.geom.body = this.body;
	this.geom.lengths = [1,1,1];
	this.body.mass.value = 1;
	this.body.position = pos;
	this.body.autoDisable = true;
	this.geom.contactVelocityAccu = 0.0;
	this.geom.contact = this.GeomContact;
	this.Compile();
}



var scene = [];

scene.push( new Floor() );

var ball = new Ball([0, 0, -50]);
ball.body.linearVel = [0, 0, 100];
scene.push( ball );

/*
var ball = new Ball([0, 1, 40]);
ball.body.linearVel = [0, 0, -10];
scene.push( ball );
*/


var side = 5;
var gap = 1;
for ( var i = -side; i < side; ++i )
	for ( var j = -side; j < side; ++j ) {
		
		scene.push( new Box([i*(1+gap), j*(1+gap), 0.5]) );
		scene.push( new Box([i*(1+gap), j*(1+gap), 1.5]) );
		scene.push( new Box([i*(1+gap), j*(1+gap), 2.5]) );
		scene.push( new Box([i*(1+gap), j*(1+gap), 3.5]) );
		scene.push( new Box([i*(1+gap), j*(1+gap), 4.5]) );
		scene.push( new Box([i*(1+gap), j*(1+gap), 5.5]) );
		scene.push( new Box([i*(1+gap), j*(1+gap), 6.5]) );
		scene.push( new Box([i*(1+gap), j*(1+gap), 7.5]) );
	}


//scene.push( new Box([1,1, 0.5]) );

/*
var rx = 0;
scene.push({Render:function( shapeOnly ) {
	Ogl.PushMatrix();
	Ogl.Translate(0,0,10);
	Ogl.Rotate(rx++, 1,1,1);
	//Ogl.DrawBox(10, 10, 10);
	FullCube();
	Ogl.PopMatrix();	
}});
*/


var paused = true;//false;
ui.key.space = function(down) {
	
	paused = down;
};

ui.key.print = function(down) {
	
	var img = Ogl.ReadImage(true);
	new File('screenshot.png').content = EncodePngImage(img);
	img.Free();
};

var poly = false;
ui.key.p = function(down) {
	
	if ( !down )
		return;
	poly = !poly;
	Ogl.PolygonMode(Ogl.FRONT, poly ? Ogl.LINE : Ogl.FILL);
};


var spotlightTexture = new OglTexture2D();
/*
spotlightTexture.LoadSVG(<svg version="1.1" baseProfile="full" xmlns:ev="http://www.w3.org/2001/xml-events" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns="http://www.w3.org/2000/svg" preserveAspectRatio="xMidYMid meet" zoomAndPan="magnify" id="Test File" viewBox="-21 -21 42 42">
	<defs>
	<radialGradient id="shine" cx=".2" cy=".2" r=".5" fx=".2" fy=".2">
	  <stop offset="0" stop-color="white" stop-opacity=".7"/>
	  <stop offset="1" stop-color="white" stop-opacity="0"/>
	</radialGradient>
	<radialGradient id="grad" cx=".5" cy=".5" r=".5" >
	  <stop offset="0" stop-color="yellow"/>
	  <stop offset=".75" stop-color="yellow"/>
	  <stop offset=".95" stop-color="#ee0"/>
	  <stop offset="1" stop-color="#e8e800"/>
	</radialGradient>
	</defs>
	<circle r="20" stroke="black" stroke-width=".15" fill="url(#grad)"/>
	<circle r="20" fill="url(#shine)"/>
	<g id="right">
	  <ellipse rx="2.5" ry="4" cx="-6" cy="-7" fill="black"/>
	  <path fill="none" stroke="black" stroke-width=".5" stroke-linecap="round" d="M 10.6,2.7 a 4,4,0 0,0 4,3"/>
	</g>
	<use xlink:href="#right" transform="scale(-1,1)"/>
	<path fill="none" stroke="black" stroke-width=".75" d="M -12,5 A 13.5,13.5,0 0,0 12,5 A 13,13,0 0,1 -12,5"/>
</svg>, 256, 256);
Ogl.BindTexture(Ogl.TEXTURE_2D, spotlightTexture);
*/
/*	
function curveGaussian(c) function(x) Math.exp( -(x*x)/(2*c*c) );
function sigmoid(c, d) function(x) 1/(Math.exp(c*(x-d))+1)

var texture = new Texture(256, 256, 1).Set(0);
//	texture.AddGradiantRadial( [1,0,1,0,1,0,1,0] );
texture.AddGradiantRadial( sigmoid( 50, 0.97 ) ).Add(-0.3);
spotlightTexture.LoadImage(texture);
texture.Free();
*/

/*
var vi = new VideoInput('QuickCam', 1, 1, 15); // try to get the smallest size and the lowest fps

vi.onImage = function(vi) {

	var image = vi.GetImage(false);
	var texture = new Texture(image).Resize(256,256, false);
	image.Free();
	function sigmoid(c, d) function(x) 1/(Math.exp(c*(x-d))+1)
//	texture.AddGradiantRadial( sigmoid(50, 0.97) ).Add(-1);
	spotlightTexture.LoadImage(texture);
	texture.Free();
}
*/


Ogl.Fog(Ogl.FOG_MODE, Ogl.LINEAR);
Ogl.Fog(Ogl.FOG_COLOR, [0.15, 0.2, 0.4, 1]);
Ogl.Fog(Ogl.FOG_DENSITY, 0);
Ogl.Hint(Ogl.FOG_HINT, Ogl.DONT_CARE);
Ogl.Fog(Ogl.FOG_START, 0.0);
Ogl.Fog(Ogl.FOG_END, 200.0);
//Ogl.Enable(Ogl.FOG);

ui.light.SetPosition(10,10,50);

var vmove = 0;

ui.Draw = function(frame) {

	Ogl.LookAt(Math.cos(vmove/100)*50, Math.sin(vmove/100)*50, Math.cos(vmove/100)*25+40, 0,0,0, 0,0,1);

	ui.light.Update();

	if ( !ui.keyState.s ) {

		Ogl.ActiveTexture(Ogl.TEXTURE1);
		Ogl.BindTexture(Ogl.TEXTURE_2D, spotlightTexture);
		Ogl.TexEnv(Ogl.TEXTURE_ENV, Ogl.TEXTURE_ENV_MODE, Ogl.MODULATE);
		Ogl.Enable(Ogl.TEXTURE_2D);
//		Ogl.Disable(Ogl.LIGHTING);

		ui.RenderWithShadows(function( flags ) {
			
			if ( flags & 4 )
				ui.light.EnableProjectorTextureCoordinates();

			for ( var i = scene.length - 1; i >= 0; --i ) {
			
				var object = scene[i];
				if ( object.castShadow != !(flags & 2) || object.receiveShadow != !(flags & 4) )
					object.Render(flags & 1); // != 0 : shapeOnly
			}
			
			if ( flags & 4 )
				ui.light.DisableProjectorTextureCoordinates();
			
		}, [0,0,1,0]);

		Ogl.Disable(Ogl.TEXTURE_2D);
	} else {

		for ( var i = scene.length - 1; i >= 0; --i )
			scene[i].Render(false);
	}

	if ( paused ) {
		
		DrawText('paused', function() {
			Ogl.Translate(0,0.3,-1);
			Ogl.Scale(0.01);
			Ogl.Color(1,0,0);
		});
	}
//	else {
		
		vmove++;
//	}
	
	Ogl.Finish();
	
//	var img = Ogl.ReadImage(true);
//	new File('frames/frame_'+frame+'.png').content = EncodePngImage(img);
//	img.Free();
//	CollectGarbage();
}

ui.Idle = function() {

	if ( !paused ) {
		
		world.Collide();
		world.Step(20);
	}
}

// ui.Loop(function(events) { events.push(vi.Events()) });
ui.Loop();
