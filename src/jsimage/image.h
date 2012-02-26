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

#define SLOT_FUNCTION_ALLOC 0

ALWAYS_INLINE JSClass* JL_ImageJSClass( JSContext *cx ) {

	static JSClass *clasp = NULL; // it's safe to use static keyword because JSClass do not depend on the rt or cx.
	if (unlikely( clasp == NULL ))
		clasp = JL_GetCachedClassProto(JL_GetHostPrivate(cx), "Image")->clasp;
	return clasp;
}


inline JSObject* NewImage( JSContext *cx, int width, int height, int channels, void *data ) {

	const ClassProtoCache *cpc = JL_GetCachedClassProto(JL_GetHostPrivate(cx), "Image");
	JSObject *image = JL_NewObjectWithGivenProto(cx, cpc->clasp, cpc->proto, NULL);
	if ( image == NULL )
		return NULL;
	JS_DefineProperty(cx, image, "width", INT_TO_JSVAL(width), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperty(cx, image, "height", INT_TO_JSVAL(height), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperty(cx, image, "channels", INT_TO_JSVAL(channels), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	JL_SetPrivate(cx, image, data);
	return image;
}
