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

#include <stdlib.h>

#include <limits.h>
#include <float.h>

DECLARE_CLASS( Texture )

#define ABS(val) ( (val) < 0 ? -(val) : (val) )

#define MIN(val1, val2) ( (val1) < (val2) ? (val1) : (val2) )

#define MAX(val1, val2) ( (val1) > (val2) ? (val1) : (val2) )

#define MINMAX(val, min, max) ((val) > (max) ? (max) : (val) < (min) ? (min) : (val) )

// min 'invisible' value
#define PMINLIMIT FLT_MIN

// max 'invisible' value
#define PMAXLIMIT FLT_MAX

// min 'visible' value
#define PMIN (0.f)

// max 'visible' value
#define PMAX (1.f)

// full amplitude
#define PAMP (PMAX-PMIN)

// middle pixel value (gray)
#define PMID (PMIN+((PMAX-PMIN)/2))

// normalize the pixel value to range 0..1
#define PNORM(p) (( (p) -PMIN) / (PMAX-PMIN))

// un-normalize the pixel value from range 0..1
#define PUNNORM(p) ((p) * (PMAX-PMIN) + PMIN)

// normalize the pixel value to range -1..1
#define PZNORM(p) (PNORM(p) * 2 - 1)

// un-normalize the pixel value from range -1..1
#define PUNZNORM(p) ( (PUNNORM(p) + 1 ) / 2)


#define PTYPE float

#define PMAXCHANNELS 4

// channels :
// 1 : L
// 2 : LA
// 3 : RGB
// 4 : RGBA

struct Point {
	float x, y;
};

struct Pixel {
	union {
		struct { PTYPE r, g, b, a; };
		PTYPE composant[4];
	};
};

struct Texture {
	union {
		Pixel *buffer;
		PTYPE *cbuffer;
	};
	union {
		Pixel *backBuffer;
		PTYPE *cbackBuffer;
	};
	int width;
	int height;
	char channels;
};

inline void TextureSetupBackBuffer( Texture *tex ) {

	if ( tex->cbackBuffer == NULL )
		tex->cbackBuffer = (PTYPE*)malloc( tex->width * tex->height * tex->channels * sizeof(PTYPE) );
}

inline void TextureSwapBuffers( Texture *tex ) {
	
	PTYPE *tmp = tex->cbuffer;
	tex->cbuffer = tex->cbackBuffer;
	tex->cbackBuffer = tmp;
}
