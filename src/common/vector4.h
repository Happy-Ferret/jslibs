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

#ifndef _VECTOR4_H_
#define _VECTOR4_H_

#include <stdlib.h>

#ifdef SSE

#include <xmmintrin.h>
#include <ivec.h>

typedef __declspec(align(4)) union {
    __m128 m128;
    struct { float x, y, z, w; };
    float raw[4];
} Vector4;

#else // SSE

#include <math.h>

typedef union {
    struct { float x, y, z, w; };
    float raw[4];
} Vector3;

#endif // SSE



#endif _VECTOR4_H_
