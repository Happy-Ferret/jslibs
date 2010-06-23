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

#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include "jlhelper.h"
#include "membuffer.h"

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

	unsigned int width, height;
	unsigned char channels;
};

inline JSClass* TextureJSClass( JSContext *cx ) {

	return JL_GetRegistredNativeClass(cx, "Texture");
}

#endif // _TEXTURE_H_
