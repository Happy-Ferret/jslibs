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


#pragma once

DECLARE_CLASS( World )

#define SLOT_WORLD_SPACE 0
#define SLOT_WORLD_DEFAULTSURFACEPARAMETERS 1

struct WorldPrivate {
	ode::dWorldID worldId;
	ode::dJointGroupID contactGroupId;
	ode::dSpaceID spaceId;
};


ALWAYS_INLINE bool JL_JsvalToWorldID( JSContext *cx, jsval val, ode::dWorldID *worldId ) {

	JL_ASSERT_IS_OBJECT(val, JL_CLASS_NAME(World));
	JSObject *worldObject = &val.toObject();
	JL_ASSERT_INSTANCE(worldObject, JL_CLASS(World));
	*worldId = ((WorldPrivate*)JL_GetPrivate(worldObject))->worldId;
	JL_ASSERT( *worldId, E_OBJ, E_STR(JL_CLASS_NAME(World)), E_INVALID ); // "Invalid world object."
	return true;
	JL_BAD;
}
