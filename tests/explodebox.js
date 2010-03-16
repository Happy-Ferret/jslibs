LoadModule('jsstd');
LoadModule('jsode');
LoadModule('jsimage');

Exec('../common/tools.js');

var ui = new UI(800, 600);

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

	this.geom = new GeomPlane(world.space);
	this.geom.params = [0,0,1,0];
	this.geom.name = 'floor';
	
	var clist;
	this.Compile = function() {

		var cx = 10, cy = 10;

		clist = Ogl.NewList(true);

		Ogl.Material(Ogl.FRONT, Ogl.AMBIENT, 0, 0, 0, 1);
		Ogl.Material(Ogl.FRONT, Ogl.EMISSION, 0, 0, 0, 1);

		Ogl.Scale(20, 20, 1);
		Ogl.Normal(0, 0, 1);
		Ogl.Begin(Ogl.QUADS);
		for ( var x = -cx; x < cx; x++ )
			for ( var y = -cy; y < cy; y++ ) {
				if ( (x + y) % 2 )
					Ogl.Material(Ogl.FRONT, Ogl.DIFFUSE, 0.3, 0.3, 0.3, 1);
				else
					Ogl.Material(Ogl.FRONT, Ogl.DIFFUSE, 0.8, 0.8, 0.8, 1);
				Ogl.Vertex(x,y);  Ogl.Vertex(x+1,y);  Ogl.Vertex(x+1,y+1);  Ogl.Vertex(x,y+1);
			}
		Ogl.End();
		Ogl.EndList();
		return;
	}
	
	this.Render = function(castShadow, receiveShadow ) {
		
		if ( castShadow ) return;
		
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
		Ogl.Material(Ogl.FRONT, Ogl.DIFFUSE, 1, 1, 1, 1);
		Ogl.Material(Ogl.FRONT, Ogl.EMISSION, 0, 0, 0, 1);
//		Ogl.Material(Ogl.FRONT, Ogl.AMBIENT, 0, 0, 0, 1);
//		Ogl.Material(Ogl.FRONT, Ogl.SPECULAR, 0.1, 0.1, 0.1, 1);
		
		Ogl.DrawSphere(geom.radius, 15, 15);
		Ogl.EndList();
	}
	
	this.Render = function( castShadow, receiveShadow ) {
		
		if ( receiveShadow ) return;
		
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
		Ogl.Material(Ogl.FRONT, Ogl.DIFFUSE, 0, 1, 0, 1);
//		Ogl.Material(Ogl.FRONT, Ogl.EMISSION, 0, 0, 0, 1);
//		Ogl.Material(Ogl.FRONT, Ogl.AMBIENT, 0, 0, 0, 1);
//		Ogl.Material(Ogl.FRONT, Ogl.SPECULAR, 0, 0, 0, 1);
		Ogl.DrawBox.apply(null, this.geom.lengths);
		Ogl.EndList();
	},
	Render:function( castShadow, receiveShadow ) {
		
		if ( receiveShadow ) return;
		
		Ogl.PushMatrix();
		Ogl.MultMatrix(this.geom);
		Ogl.Material(Ogl.FRONT, Ogl.EMISSION, this.geom.contactVelocityAccu / 5, 0, 0, 1);
		this.geom.contactVelocityAccu /= 1.01
		Ogl.CallList(this._clist);
		Ogl.PopMatrix();
	}, 
	GeomContact:function(thisGeom, otherGeom, contactVelocity) {

		this.contactVelocityAccu += contactVelocity;
	}
}

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

scene.push( new Floor() );


var side = 10;
var gap = 0.001;

for ( var i = -side; i < side; ++i )
	for ( var j = -side; j < side; ++j ) {
		
		scene.push( new Box([i*(1+gap), j*(1+gap), 0.5]) );
		scene.push( new Box([i*(1+gap), j*(1+gap), 1.5]) );
		scene.push( new Box([i*(1+gap), j*(1+gap), 2.5]) );
//		scene.push( new Box([i*(1+gap), j*(1+gap), 3.5]) );
	}


/* test
scene.push( { Render:function(cast, receive) {
	
	Ogl.Material(Ogl.FRONT, Ogl.DIFFUSE, 1, 1, 1, 1);
	Ogl.Material(Ogl.FRONT, Ogl.EMISSION, 1, 0, 0, 1);
	Ogl.Material(Ogl.FRONT, Ogl.AMBIENT, 0, 0, 0, 1);
//	Ogl.Material(Ogl.FRONT, Ogl.SPECULAR, 0, 0, 0, 1);
	Ogl.DrawBox(10,10,20)
}} );
*/



var paused = false;
ui.key.space = function(down) {
	
	paused = down;
};


var poly = false;
ui.key.p = function(down) {
	
	if ( !down )
		return;
	poly = !poly;
	Ogl.PolygonMode(Ogl.FRONT, poly ? Ogl.LINE : Ogl.FILL);
};


var vmove = 0;

ui.Draw = function(frame) {

	Ogl.LookAt(Math.cos(frame/100)*50, Math.sin(frame/100)*15, Math.cos(vmove/100)*25+27, 0,0,5, 0,0,1);

	ui.SetLight([15,15,30, 1]);

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
