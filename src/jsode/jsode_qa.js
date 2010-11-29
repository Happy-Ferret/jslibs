LoadModule('jsode');

/// crash 1 [rmtf]

  new Body(new World);


/// crash 2 [rmtf]

  var world = new World;
  var floor = new GeomPlane(world.space);
  floor.body = world.env;
  var a = floor.body;
