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

#ifndef _VECTOR5_H_
#define _VECTOR5_H_

#include <stdlib.h>
#include <math.h>

typedef union {
    struct { float x, y, z, w, v; };
    float raw[5];
} Vector5;


inline void Vector5Set( Vector5 *v, const float _x, const float _y, const float _z, const float _w, const float _v ) {

	v->x = _x;
	v->y = _y;
	v->z = _z;
	v->w = _w;
	v->v = _v;
}


inline void Vector5Mult( Vector5 *rv, Vector5 *v, float s ) {

	rv->x = v->x * s;
	rv->y = v->y * s;
	rv->z = v->z * s;
	rv->w = v->w * s;
	rv->v = v->v * s;
}

inline void Vector5AddVector5( Vector5 *rv, Vector5 *v1, Vector5 *v2 ) {

	rv->x -= v1->x + v2->x;
	rv->y -= v1->y + v2->y;
	rv->z -= v1->z + v2->z;
	rv->w -= v1->w + v2->w;
	rv->v -= v1->v + v2->v;
}


#endif // _VECTOR5_H_
