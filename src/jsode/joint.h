/* ***** BEGIN LICENSE BLOCK *****
 * Version: GNU GPL 2.0
 *
 * The contents of this file are subject to the
 * GNU General Public License Version 2.0; you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * http://www.gnu.org/licenses/gpl.html
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * ***** END LICENSE BLOCK ***** */

extern bool _odeFinalization;

DECLARE_CLASS( JointGroup );
DECLARE_CLASS( Joint );
DECLARE_CLASS( JointBall );
DECLARE_CLASS( JointHinge );
DECLARE_CLASS( JointSlider );
DECLARE_CLASS( JointUniversal );
DECLARE_CLASS( JointPiston );
DECLARE_CLASS( JointFixed );
DECLARE_CLASS( JointAMotor );
DECLARE_CLASS( JointLMotor );
DECLARE_CLASS( JointPlane );

void FinalizeJoint(JSContext *cx, JSObject *obj);

JSBool ReconstructJoint( JSContext *cx, ode::dJointID jointId, JSObject **obj );

ALWAYS_INLINE JSBool JointToJSObject( JSContext *cx, ode::dJointID jointId, JSObject **obj ) { // (TBD) JSObject** = Conservative Stack Scanning issue ?

	*obj = (JSObject*)ode::dJointGetData(jointId);
	if ( *obj != NULL )
		return JS_TRUE;
	return ReconstructJoint(cx, jointId, obj);
}
