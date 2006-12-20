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

#include "stdafx.h"
#include "world.h"
#include "joint.h"

BEGIN_CLASS( JointPlane )

DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING(_class);
	RT_ASSERT_ARGC(1);
	ode::dWorldID worldId;
	if ( ValToWorldID( cx, argv[0], &worldId) == JS_FALSE )
		return JS_FALSE;
	ode::dJointID jointId = ode::dJointCreatePlane2D(worldId, 0); // The joint group ID is 0 to allocate the joint normally.
	JS_SetPrivate(cx, obj, jointId);
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_CONSTRUCTOR

	HAS_PROTOTYPE( prototypeJoint )
	HAS_PRIVATE
END_CLASS
