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

#ifndef _TRIMESH_PUB_H_
#define _TRIMESH_PUB_H_

#define TRIMESH_CLASS_NAME "Trimesh"

#define SURFACE_REAL_TYPE float
#define SURFACE_INDEX_TYPE u_int32_t

struct Surface {

	size_t vertexCount;
	SURFACE_REAL_TYPE *vertex; // x,y,z,...
	SURFACE_REAL_TYPE *normal; // x,y,z,... 
	SURFACE_REAL_TYPE *textureCoordinate; // s,t,r,...
	SURFACE_REAL_TYPE *color; // r,g,b,...

	size_t indexCount;
	SURFACE_INDEX_TYPE *index;
};


bool IsTrimeshObject( JSContext *cx, JSObject *obj ) {

	return IsClassName(cx, obj, TRIMESH_CLASS_NAME );
}


Surface *GetTrimeshSurface( JSContext *cx, JSObject *obj ) {

	return (Surface*)JS_GetPrivate(cx, obj);
}


#endif // _TRIMESH_PUB_H_
