LoadModule('jsstd');
LoadModule('jsode');
LoadModule('jsimage');

Exec('../common/tools.js');

var ui = new UI(800, 600);

var world = new World();
world.quickStepNumIterations = 20;
world.gravity = [0,0,-9.8 / 10];
world.linearDamping = 0.001;
world.angularDamping = 0.001;
//world.linearDampingThreshold = 0;
//world.angularDampingThreshold = 0;


world.defaultSurfaceParameters.mu = Infinity; // doc. 0 results in a frictionless contact, and dInfinity results in a contact that never slips.
world.defaultSurfaceParameters.softERP = 0.7; // Joint error and the error reduction parameter (ERP)
world.defaultSurfaceParameters.softCFM = 0.0001; // Soft constraint and constraint force mixing (CFM)
world.defaultSurfaceParameters.bounce = 0;
//world.defaultSurfaceParameters.bounceVel = 2;

function Floor() {
	
	var self = this;
	var geom = new GeomPlane(world.space);
	geom.params = [0,0,1,0];
	
	this.geom = geom;
	
	var clist;
	this.Draw = function() {

		Ogl.PushMatrix();
		
		if ( clist ) {
		
			Ogl.CallList(clist);
		} else {
		
			clist = Ogl.NewList();
				
			with (Ogl) {
				
				Disable(TEXTURE_2D);
				Normal(0,0,1);
				Translate(-10,-10,0);
				Scale(5,5,1);
				var cx=10, cy=10;
				Begin(QUADS);
				for ( var x = -cx; x < cx; x++ )
					for ( var y = -cy; y < cy; y++ ) {
						if ( (x + y) % 2 )
							Color(0,0,0.5);
						else
							Color(0.9,0.8,0.7);
						Vertex(x,y);  Vertex(x+1,y);  Vertex(x+1,y+1);  Vertex(x, y+1);
					}
				End();
			}
			Ogl.EndList();
		}

		Ogl.PopMatrix();				
	}	
}


function Ball(pos) {
	
	var geom = new GeomSphere(world.space);
	var body = new Body(world);
	geom.body = body;
	geom.radius = 2;
	body.mass.value = 10;
	body.position = pos;
	
	this.body = body;
	this.geom = geom;
	
	var clist;
	this.Draw = function() {
		
		Ogl.PushMatrix();
		Ogl.MultMatrix(geom);
		if ( clist ) {
		
			Ogl.CallList(clist);
		} else {
		
			clist = Ogl.NewList();
			Ogl.Color(1,1,1);
			Ogl.DrawSphere(geom.radius, 15, 15);
			Ogl.EndList();
		}
		Ogl.PopMatrix();
	}
}

function Box(pos) {
	
	this.geom = new GeomBox(world.space);
	this.body = new Body(world);
	this.geom.body = this.body;
	this.geom.lengths = [1,1,1];
	this.body.mass.value = 0.1;
	this.body.position = pos;
	this.body.autoDisable = true;
}

Box.prototype = {
	Draw:function() {
		
		Ogl.PushMatrix();

		Ogl.MultMatrix(this.geom);
		Ogl.Color(0, this.geom.body.disabled ? 0 : 1, 0);

		if ( this._clist ) {
		
			Ogl.CallList(this._clist);
		} else {
		
			this._clist = Ogl.NewList();
			Ogl.DrawBox.apply(null, this.geom.lengths);
			Ogl.EndList();
		}
		
		Ogl.PopMatrix();
	}
}


var scene = [];

var ball = new Ball([0, 0, 20]);
ball.body.linearVel = [0,0,-100];
scene.push( ball );


var side = 10;
var gap = 0.001;

for ( var i = -side; i < side; ++i )
	for ( var j = -side; j < side; ++j ) {
		
		scene.push( new Box([i*(1+gap), j*(1+gap), 0.5]) );
		scene.push( new Box([i*(1+gap), j*(1+gap), 1.5]) );
	}
		
scene.push( new Floor() );


var stop = false;
ui.key.space = function(down) {
	
	stop = down;
};

ui.Draw = function(frame) {

	Ogl.LookAt(Math.cos(frame/100)*50, Math.sin(frame/100)*15, Math.cos(frame/100)*20+25, 0,0,10, 0,0,1);

	ui.SetLight([10,10,10]);

	for ( var i = scene.length - 1; i >= 0; --i )
		scene[i].Draw();
		
	if ( stop ) {
		
		Ogl.Color(1,0,0);
		ui.DrawText('paused', true);
	} else {
		
		world.Collide();
		world.Step(20);
	}
	
	Ogl.Finish();
/*	
	var img = Ogl.ReadImage();
	new File('frame_'+frame+'.png').content = EncodePngImage(img);
	img.Free();
*/	
}

ui.Loop();
