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

#define SURFACE_REAL_TYPE float
#define SURFACE_INDEX_TYPE int

struct Surface {

	unsigned int vertexCount;
	SURFACE_REAL_TYPE *vertex; // x,y,z,...
	SURFACE_REAL_TYPE *normal; // x,y,z,... 
	SURFACE_REAL_TYPE *textureCoordinate; // s,t,r,...
	SURFACE_REAL_TYPE *color; // r,g,b,a,...

	unsigned int indexCount;
	SURFACE_INDEX_TYPE *index;
};


ALWAYS_INLINE JSClass* JL_TrimeshJSClass( JSContext *cx ) {

	static JSClass *clasp = NULL; // it's safe to use static keyword because JSClass do not depend on the rt or cx.
	if (unlikely( clasp == NULL ))
		clasp = JL_GetCachedClassProto(JL_GetHostPrivate(cx), "Trimesh")->clasp;
	return clasp;
}


bool JL_JsvalIsTrimesh( JSContext *cx, jsval val ) {

	return JL_ValueIsClass(val, JL_TrimeshJSClass(cx));
}


Surface *GetTrimeshSurface( JSContext *, JSObject *obj ) {

	return (Surface*)JL_GetPrivate(obj);
}
