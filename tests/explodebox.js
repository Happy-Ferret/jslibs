LoadModule('jsstd');
LoadModule('jsode');
LoadModule('jsimage');

Exec('../common/tools.js');

var ui = new UI(640,480);

var world = new World();
world.quickStepNumIterations = 10;
world.gravity = [0,0,-9.8 / 10];
world.linearDamping = 0.01;
world.angularDamping = 0.01;
//world.linearDampingThreshold = 0;
//world.angularDampingThreshold = 0;


world.defaultSurfaceParameters.mu = Infinity; // doc. 0 results in a frictionless contact, and dInfinity results in a contact that never slips.
world.defaultSurfaceParameters.softERP = 0.1; // Joint error and the error reduction parameter (ERP)
world.defaultSurfaceParameters.softCFM = 0.0000001; // Soft constraint and constraint force mixing (CFM)
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
			
		with (Ogl) {
			
			Disable(TEXTURE_2D);
			Normal(0,0,1);
			Scale(20,20,1);
			var cx=10, cy=10;
			Begin(QUADS);
			for ( var x = -cx; x < cx; x++ )
				for ( var y = -cy; y < cy; y++ ) {
					if ( (x + y) % 2 )
						Color(0.2);
					else
						Color(0.8);
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
	geom.radius = 2;
	body.mass.value = 50;
	body.position = pos;
	
	geom.impact = function(thisGeom, otherGeom) otherGeom.name != 'floor'; // don't hit the floor
	
	this.body = body;
	this.geom = geom;
	
	var clist;
	this.Compile = function() {

		clist = Ogl.NewList(true);
		Ogl.Color(1,1,1);
		Ogl.DrawSphere(geom.radius, 15, 15);
		Ogl.EndList();
	}
	
	this.Render = function( compile, castShadow, receiveShadow ) {
		
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
//		Ogl.Color(1, 1, 0);
		Ogl.DrawBox.apply(null, this.geom.lengths);
		Ogl.EndList();
	},
	Render:function( compile, castShadow, receiveShadow ) {
		if ( receiveShadow )
			return
		Ogl.PushMatrix();
		Ogl.MultMatrix(this.geom);
//		this.geom.body.disabled && Ogl.Color(0, 0.5, 0);
		
		var vel = this.body.linearVel;
		var v = vel[0]+vel[1]+vel[2];
//		Ogl.Color(1+v/10,1,0);

		Print( this.body.force, '\n' );

		Ogl.Color(this.geom.impactVelocity*2, 1, 0);
		this.geom.impactVelocity /= 1.01;
		
		Ogl.CallList(this._clist);
		Ogl.PopMatrix();
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
	this.geom.impactVelocity = 0.0;
	this.geom.impact = function(thisGeom, otherGeom, impactVelocity) {

		this.impactVelocity += impactVelocity;
	}
	this.Compile();
}


var scene = [];

var ball = new Ball([0, 0, -50]);
ball.body.linearVel = [0, 0, 70];
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
		
scene.push( new Floor() );


var paused = false;
ui.key.space = function(down) {
	
	paused = down;
};

var vmove = 0;

ui.Draw = function(frame) {

	Ogl.LookAt(Math.cos(frame/100)*50, Math.sin(frame/100)*15, Math.cos(vmove/100)*20+22, 0,0,5, 0,0,1);

	ui.SetLight([10,10,10, 1]);

//	var clist = Ogl.NewList(true);
	for ( var i = scene.length - 1; i >= 0; --i )
		scene[i].Render(false, false, false);
//	Ogl.EndList();
//	Ogl.CallList(clist);
	
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
	
//	var img = Ogl.ReadImage();
//	new File('frame_'+frame+'.png').content = EncodePngImage(img);
//	img.Free();
//	CollectGarbage();
}

ui.Idle = function() {

	if ( !paused ) {
		
		world.Collide();
		world.Step(20);
	}
}

ui.Loop();
