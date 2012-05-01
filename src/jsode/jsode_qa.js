loadModule('jsode');


/// 2 memory leak

	var space = new Space();
	var floor = new GeomPlane(space);


/// 1 memory leak
	
	new Space();


/// JointGroup multiple destroy [p]

	var j = new JointGroup();
	j.destroy();
	QA.IS_UNSAFE || QA.ASSERTOP( function() j.destroy(), 'ex', Error );


/// Geom multiple destroy [p]

	var g = new GeomSphere();
	g.destroy();
	QA.IS_UNSAFE || QA.ASSERTOP( function() g.destroy(), 'ex', Error );


/// object type check [p]

	QA.IS_UNSAFE || QA.ASSERTOP( function() World.prototype.env.mass.value, 'ex', TypeError );


/// crash 1 [p]

  new Body(new World);


/// crash 2 [p]

  var world = new World;
  var floor = new GeomPlane(world.space);
  floor.body = world.env;
  var a = floor.body;


/// crash 3 [p]

	var w = new World();
	var geom = new GeomPlane(w.space);


/// crash 4 [p]

	var space = new Space();
	var geom = new GeomPlane(space);
	space.destroy();


/// memory leak test [p]

	var w = new World();
	var j1 = new JointFixed(w);
	j1.body1 = new Body(w);
	j1.body2 = w.env;
	var g = new GeomBox(w.space);
	g.body = j1.body1;

	new GeomBox(w.space);
	new GeomBox(w.space);
	new GeomBox(w.space);
