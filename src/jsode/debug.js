LoadModule('jsstd');
LoadModule('jsode');
LoadModule('jstrimesh');


var world = new World;


var t = new Trimesh();

t.DefineVertexBuffer([
	0,0,0,
	1,0,0,
	0,1,0,
	0,0,1,
]);

t.DefineIndexBuffer([
	0,3,1,
	0,1,2,
	0,2,3,
	1,3,2,
]);


var g = new GeomTrimesh( world.space, t );



Halt();

function main() {

	var g = new GeomBox();

//	return;

	/*
	var world = new World;
	world.gravity = [0,0,-0.81];

	var body1 = new Body(world);
	var body2 = new Body(world);
	var joint = new JointHinge(world);


	joint.body1 = body1;
	joint.body2 = body2;
	joint.anchor = [4,4,0];
	joint.axis = [1,0,0];
	joint.loStop = 1;
	joint.hiStop = 1.5;

	body1.linearVel = [0,0,19];
	body1.angularVel = [1,1,0];
	*/



	var world = new World;
	//world.Body = Body;
	//var b = new world.Body();

	//world.gravity = [0,0,-9.81];

	var body1 = new Body(world);
	var g = new GeomBox( world.space );
	g.body = body1;
	g.impact = function(n, against, pos) { Print('hit'+n+'@['+pos+']\n') }


	var body2 = new Body(world);
	new GeomBox( world.space ).body = body2;

	//body1.mass.SetBoxTotal(10,[1,1,100]);
	//body.mass.mass = 1;
	//body.mass.Translate([2,1,0]);
	//body.mass.center = [2,0,0];
	//body.mass.Adjust(10);

	var joint = new JointHinge(world);

	//joint.Attach(body,body1);
	joint.body1 = body1;
	joint.body2 = body2;
	joint.anchor = [10,0,0];
	joint.axis = [1,1,1];
	
	joint.CFM = Infinity;

	joint.useFeedback = true;

	body1.position = [0,0,0]
	new JointHinge(world)

	body2.position = [0,0,5]
	body2.linearVel = [0,0,-5];

	var floor = new GeomPlane(world.space);

	//body.linearVel = [10,10,10];
	//body.angularVel = [10,0,0];

	for ( var i = 0; i<20; i++ ) {
	//	Print( 'Step:', i,  '\n' );


	//	Print( ' Position:', body1.position , '\n' );

	//	Print( ' Force:', body.force , '\n' );
		world.Step(0.1,20);
		
		Print( ' ->', joint.body1Torque, '\n' );
		
	}

	//Print( 'joint angle rate:'+joint.angleRate ,'\n');
	
	delete g.impact;
}

main();

