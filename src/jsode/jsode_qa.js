loadModule('jsode');

/// crash 1 [rmtf]

  new Body(new World);


/// crash 2 [rmtf]

  var world = new World;
  var floor = new GeomPlane(world.space);
  floor.body = world.env;
  var a = floor.body;


/// crash 3 [rmtf]

	loadModule('jsode');
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
