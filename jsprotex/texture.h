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

DECLARE_CLASS( Texture )

typedef struct {

	float r;
	float g;
	float b;
	float a;
} Pixel;

typedef struct {
	
	unsigned int width;
	unsigned int height;
	Pixel *buffer;
	Pixel *backBuffer;
} Texture;

static void TextureSwitchBuffers( Texture *tex ) {
	
	Pixel *tmp = tex->buffer;
	tex->buffer = tex->backBuffer;
	tex->backBuffer = tmp;
}