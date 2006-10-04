extern JSClass joint_class;
extern JSClass jointBall_class;
extern JSClass jointHinge_class;
extern JSClass jointSlider_class;
extern JSClass jointFixed_class;

JSObject *jointInitClass( JSContext *cx, JSObject *obj );

#define JOINT_SLOT_WORLD 0;