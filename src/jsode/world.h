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


ALWAYS_INLINE JSBool JL_JsvalToWorldID( JSContext *cx, jsval val, ode::dWorldID *worldId ) {

	JL_S_ASSERT_OBJECT(val);
	JSObject *worldObject = JSVAL_TO_OBJECT(val);
	JL_S_ASSERT_CLASS(worldObject, JL_CLASS(World));
	*worldId = ((WorldPrivate*)JL_GetPrivate(cx, worldObject))->worldId;
	JL_CHKM( *worldId, "Invalid world object." );
	return JS_TRUE;
	JL_BAD;
}
