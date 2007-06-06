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

#include "stdafx.h"

#include "texture.h"


BEGIN_STATIC


DEFINE_FUNCTION( Flat ) {

	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_OBJECT( argv[0] );
	RT_ASSERT_CLASS( JSVAL_TO_OBJECT(argv[0]), &classTexture );
	Texture *tex = (Texture *)JS_GetPrivate( cx, JSVAL_TO_OBJECT(argv[0]) );
	RT_ASSERT_RESOURCE(tex);

	unsigned int size = tex->width * tex->height;
	for ( unsigned int i = 0; i < size; i++ ) {

		tex->buffer->r = 0.f;
		tex->buffer->g = 0.f;
		tex->buffer->b = 0.f;
		tex->buffer->a = 1.f;
	}




	
	return JS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CONFIGURE_STATIC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( Flat )
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
	END_STATIC_PROPERTY_SPEC

END_STATIC
