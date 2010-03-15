LoadModule('jsstd');
LoadModule('jsode');
LoadModule('jsimage');

Exec('../common/tools.js');

var ui = new UI(640, 480);

var world = new World();
world.quickStepNumIterations = 5;
world.gravity = [0,0,-9.8 / 5];
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
	var geom = new GeomPlane(world.space);
	geom.params = [0,0,1,0];
	
	this.geom = geom;
	
	this.geom.name = 'floor';
	
	var clist;
	this.Compile = function() {

		clist = Ogl.NewList(true);
			
		Ogl.Material(Ogl.FRONT, Ogl.AMBIENT, 0, 0, 0, 1);
		Ogl.Material(Ogl.FRONT, Ogl.DIFFUSE, 0, 0, 0, 1);
//		Ogl.Material(Ogl.FRONT, Ogl.EMISSION, 0, 0, 0, 1);
//		Ogl.Material(Ogl.FRONT, Ogl.SPECULAR, 0, 0, 0, 1);

		with (Ogl) {
			
			Disable(TEXTURE_2D);
			Normal(0,0,1);
			Scale(20,20,1);
			var cx=10, cy=10;
			Ogl.Material(Ogl.FRONT, Ogl.AMBIENT, 0, 0, 0, 1);
			Begin(QUADS);
			for ( var x = -cx; x < cx; x++ )
				for ( var y = -cy; y < cy; y++ ) {
					if ( (x + y) % 2 )
						Ogl.Material(Ogl.FRONT, Ogl.DIFFUSE, 0.3, 0.3, 0.3, 1);
					else
						Ogl.Material(Ogl.FRONT, Ogl.DIFFUSE, 0.8, 0.8, 0.8, 1);
					Vertex(x,y);  Vertex(x+1,y);  Vertex(x+1,y+1);  Vertex(x, y+1);
				}
			End();
		}
		Ogl.EndList();
		return;
	}
	
	this.Render = function(castShadow, receiveShadow ) {
		
		if ( castShadow )
			return;
		
		Ogl.PushMatrix();
		Ogl.CallList(clist);
		Ogl.PopMatrix();
	}	

	this.Compile();
}


function Ball(pos) {
	
	var geom = new GeomSphere(world.space);
	var body = new Body(world);
	geom.body = body;
	geom.radius = 3;
	body.mass.value = 100;
	body.position = pos;
	
	geom.contact = function(thisGeom, otherGeom) otherGeom.name != 'floor'; // don't hit the floor
	
	this.body = body;
	this.geom = geom;
	
	var clist;
	this.Compile = function() {

		clist = Ogl.NewList(true);
		Ogl.Material(Ogl.FRONT, Ogl.AMBIENT, 0, 0, 0, 1);
		Ogl.Material(Ogl.FRONT, Ogl.DIFFUSE, 1, 1, 1, 1);
//		Ogl.Material(Ogl.FRONT, Ogl.EMISSION, 0, 0, 0, 1);
//		Ogl.Material(Ogl.FRONT, Ogl.SPECULAR, 0.1, 0.1, 0.1, 1);
		
		Ogl.DrawSphere(geom.radius, 15, 15);
		Ogl.EndList();
	}
	
	this.Render = function( castShadow, receiveShadow ) {
		
		if ( receiveShadow )
			return
		Ogl.PushMatrix();
		Ogl.MultMatrix(geom);
		Ogl.CallList(clist);
		Ogl.PopMatrix();
	}
	this.Compile();	
}

Box.prototype = {
	Compile:function() {

		this._clist = Ogl.NewList(true);
//		Ogl.Material(Ogl.FRONT, Ogl.AMBIENT, 0, 0, 0, 1);
		Ogl.Material(Ogl.FRONT, Ogl.DIFFUSE, 0, 1, 0, 1);
//		Ogl.Material(Ogl.FRONT, Ogl.EMISSION, 0, 0, 0, 1);
//		Ogl.Material(Ogl.FRONT, Ogl.SPECULAR, 0, 0, 0, 1);
		Ogl.DrawBox.apply(null, this.geom.lengths);
		Ogl.EndList();
	},
	Render:function( castShadow, receiveShadow ) {
		if ( receiveShadow )
			return
		Ogl.PushMatrix();
		Ogl.MultMatrix(this.geom);
//		Ogl.Color(this.geom.contactVelocityAccu*2,1,0);
		Ogl.Material(Ogl.FRONT, Ogl.AMBIENT, this.geom.contactVelocityAccu, 0, 0, 1);
		this.geom.contactVelocityAccu /= 1.01

		Ogl.CallList(this._clist);
		Ogl.PopMatrix();
	}, 
	GeomContact:function(thisGeom, otherGeom, contactVelocity) {

		this.contactVelocityAccu += contactVelocity;
	}
}

var imp = 0;
function Box(pos) {
	
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

var ball = new Ball([0, 0, -50]);
ball.body.linearVel = [0, 0, 80];
scene.push( ball );

var side = 10;
var gap = 0.001;

for ( var i = -side; i < side; ++i )
	for ( var j = -side; j < side; ++j ) {
		
		scene.push( new Box([i*(1+gap), j*(1+gap), 0.5]) );
		scene.push( new Box([i*(1+gap), j*(1+gap), 1.5]) );
//		scene.push( new Box([i*(1+gap), j*(1+gap), 2.5]) );
//		scene.push( new Box([i*(1+gap), j*(1+gap), 3.5]) );
	}

/*
var side = 7;
var gap = 0.0001;
for ( var i = -side; i < side; ++i )
	for ( var j = -side; j < side; ++j ) {
		
		scene.push( new Box([i*(1+gap), j*(1+gap), 1.5]) );
	}
*/

		
scene.push( new Floor() );


var paused = false;
ui.key.space = function(down) {
	
	paused = down;
};

var vmove = 0;

/*
Ogl.Enable(Ogl.FOG);
Ogl.Fog(Ogl.FOG_MODE, Ogl.EXP2);
Ogl.Fog(Ogl.FOG_DENSITY, 0.01);
Ogl.Fog(Ogl.FOG_COLOR, [0.5,0.5,0.5]);
*/
ui.SetLight([10,10,30, 1]);

ui.Draw = function(frame) {

	Ogl.LookAt(Math.cos(frame/100)*50, Math.sin(frame/100)*15, Math.cos(vmove/100)*25+27, 0,0,5, 0,0,1);

	if ( !ui.keyState.s ) {

		ui.RenderWithShadows([0,0,1,0], function( castShadow, receiveShadow ) {

			for ( var i = scene.length - 1; i >= 0; --i )
				scene[i].Render(castShadow, receiveShadow);
		});
	} else {

		for ( var i = scene.length - 1; i >= 0; --i )
			scene[i].Render(false, false);
	}
	
	if ( paused ) {
		
		ui.DrawText('paused', function() {
			Ogl.Translate(0,0.3,-1);
			Ogl.Scale(0.01);
			Ogl.Color(1,0,0);
		});
	} else {
		
		vmove++;
	}
	
	Ogl.Finish();
	
	var img = Ogl.ReadImage();
	new File('frames/frame_'+frame+'.png').content = EncodePngImage(img);
	img.Free();
	CollectGarbage();
}

ui.Idle = function() {

	if ( !paused ) {
		
		world.Collide();
		world.Step(20);
	}
}

ui.Loop();
