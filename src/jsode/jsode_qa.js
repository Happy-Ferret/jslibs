loadModule('jsode');


/// 2 memory leak

	var space = new Space();
	var floor = new GeomPlane(space);


/// 1 memory leak
	
	new Space();


/// JointGroup multiple destroy [rmtf]

	var j = new JointGroup();
	j.destroy();
	QA.ASSERTOP( function() j.destroy(), 'ex', Error );


/// Geom multiple destroy [rmtf]

	var g = new GeomSphere();
	g.destroy();
	QA.ASSERTOP( function() g.destroy(), 'ex', Error );


/// object type check [rmtf]

	QA.ASSERTOP( function() World.prototype.env.mass.value, 'ex', TypeError );


/// crash 1 [rmtf]

  new Body(new World);


/// crash 2 [rmtf]

  var world = new World;
  var floor = new GeomPlane(world.space);
  floor.body = world.env;
  var a = floor.body;


/// crash 3 [rmtf]

	var w = new World();
	var geom = new GeomPlane(w.space);


/// crash 4 [rmtf]

	var space = new Space();
	var geom = new GeomPlane(space);
	space.destroy();


/// memory leak test [rmtf]

	var w = new World();
	var j1 = new JointFixed(w);
	j1.body1 = new Body(w);
	j1.body2 = w.env;
	var g = new GeomBox(w.space);
	g.body = j1.body1;

	new GeomBox(w.space);
	new GeomBox(w.space);
	new GeomBox(w.space);
