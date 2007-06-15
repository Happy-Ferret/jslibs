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

#include <limits.h>
#include <float.h>

DECLARE_CLASS( Texture )

#define MINMAX(val, min, max) ((val) > (max) ? (max) : (val) < (min) ? (min) : (val) )

#define PMAXLIMIT FLT_MAX
#define PMINLIMIT FLT_MIN
#define PMIN (0.f)
#define PMAX (1.f)

#define PMID (PMIN+((PMAX-PMIN)/2))

// normalize the pixel value to range 0..1
#define PNORM(p) (((p)-PMIN) / (PMAX-PMIN))

#define PTYPE float

struct Pixel {
	union {
		struct { float r, g, b, a; };
		float composant[4];
	};
};

struct Texture {
	size_t width;
	size_t height;
	Pixel *buffer;
	Pixel *backBuffer;
};

inline void TextureSwapBuffers( Texture *tex ) {
	
	Pixel *tmp = tex->buffer;
	tex->buffer = tex->backBuffer;
	tex->backBuffer = tmp;
}