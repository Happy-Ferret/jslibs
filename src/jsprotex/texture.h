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

#define PTYPE float

#define PMAXCHANNELS 4

// channels :
// 1 : L
// 2 : LA
// 3 : RGB
// 4 : RGBA


struct TextureStruct {

	PTYPE *cbuffer;
	size_t cbufferSize;

	PTYPE *cbackBuffer;
	size_t cbackBufferSize;

	unsigned int width, height, channels;
};


ALWAYS_INLINE JSClass* JL_TextureJSClass( JSContext *cx ) {

	static JSClass *clasp = NULL; // it's safe to use static keyword because JSClass do not depend on the rt or cx.
	if (unlikely( clasp == NULL ))
		clasp = JL_GetCachedClass(JL_GetHostPrivate(cx), "Texture");
	return clasp;
}
