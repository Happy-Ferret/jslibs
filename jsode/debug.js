LoadModule('jsstd');
LoadModule('jsode');



var world = new World;
world.gravity = [0,0,-0.81];

var body1 = new Body(world);
var body2 = new Body(world);
var joint = new JointHinge(world);

Print( joint.velocity);
Print( joint.maxForce);

joint.Attach(body1,body2);
joint.anchor = [4,4,0];
joint.axis = [1,0,0];
joint.loStop = 1;
joint.hiStop = 1.5;

body1.linearVel = [0,0,19];
body1.angularVel = [1,1,0];





/*

var world = new World;

//world.Body = Body;
//var b = new world.Body();

world.gravity = [0,0,-9.81];

var body = new Body(world);
var body1 = new Body(world);

body1.mass.SetBoxTotal(10,[1,1,100]);

//body.mass.mass = 1;
//body.mass.Translate([2,1,0]);
//body.mass.center = [2,0,0];
//body.mass.Adjust(10);

var joint = new JointHinge(world);

joint.Attach(body,body1);
joint.anchor = [10,0,0];
joint.axis = [1,1,1];
//joint.loStop = 0;
//joint.Destroy();

//body.linearVel = [0,0,15];
body.force = [0,0,100];

//body.linearVel = [10,10,10];

//body.angularVel = [10,0,0];

Print(body.torque);

for ( var i = 0; i<100; i++ ) {
//	Print( 'Step:', i,  '\n' );
	Print( ' Position:', body.position , '\n' );
//	Print( ' Force:', body.force , '\n' );
	world.Step(1,true);
}

Print( 'joint angle rate:'+joint.angleRate ,'\n');

*/