DECLARE_CLASS( Geom )
DECLARE_CLASS( GeomSphere )
DECLARE_CLASS( GeomBox )
DECLARE_CLASS( GeomPlane )
DECLARE_CLASS( GeomCapsule )
DECLARE_CLASS( GeomRay )

DECLARE_CLASS( SurfaceParameters )

JSBool SetupReadMatrix(JSContext *cx, JSObject *obj, ode::dGeomID geomId);