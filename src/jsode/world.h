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

DECLARE_CLASS( World )

#define DEFAULT_SURFACE_PARAMETERS_PROPERTY_NAME "defaultSurfaceParameters"
#define WORLD_SPACE_PROPERTY_NAME "space"
#define COLLIDE_FEEDBACK_FUNCTION_NAME "impact"


//#define WORLD_SLOT_CONTACTGROUP 0
#define WORLD_SLOT_SPACE 1

inline JSBool ValToWorldID( JSContext *cx, jsval val, ode::dWorldID *worldId ) {

	J_S_ASSERT_OBJECT(val);
	JSObject *worldObject = JSVAL_TO_OBJECT(val);
	J_S_ASSERT_CLASS(worldObject,&classWorld);
	*worldId = (ode::dWorldID)JS_GetPrivate(cx,worldObject);
	return JS_TRUE;
}

