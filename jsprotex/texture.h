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
#define PNORM(p) (((p)-PMIN) / (PMAX-PMIN))

// normalize the pixel value to range -1..1
#define PZNORM(p) (  (((p)-PMIN) / (PMAX-PMIN)) * 2 - 1  )

#define PTYPE float

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
	size_t width;
	size_t height;
	Pixel *buffer;
	Pixel *backBuffer;
};

inline void TextureSetupBackBuffer( Texture *tex ) {

	if ( tex->backBuffer == NULL )
		tex->backBuffer = (Pixel*)malloc( tex->width * tex->height * sizeof(Pixel) );
}


inline void TextureSwapBuffers( Texture *tex ) {
	
	Pixel *tmp = tex->buffer;
	tex->buffer = tex->backBuffer;
	tex->backBuffer = tmp;
}