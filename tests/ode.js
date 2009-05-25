LoadModule('jsstd');
LoadModule('jsode');

var world = new World;
world.gravity = [0,0,-9.81];
var floor = new GeomPlane(world.space);
//floor.body = world.env;

var body1 = new Body(world);
var geom = new GeomSphere(world.space);
geom.body = body1;
geom.impact = function(n, b1, b2, pos) { Print('impact '+n+' at [ '+pos[0].toFixed(2)+' , '+pos[1].toFixed(2)+' , '+pos[2].toFixed(2)+' ]\n') }

world.defaultSurfaceParameters.bounce = 0.7;
world.defaultSurfaceParameters.bounceVel = 0;

body1.position = [0,0,5];

while ( !endSignal ) {

	Sleep(10);
	world.Step(0.01);
}
