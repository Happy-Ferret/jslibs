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

#include <jlhelper.h>
#include <jlclass.h>
//#include <jsvalserializer.h>

#define XMALLOC jl_malloc_fct
#define XCALLOC jl_calloc_fct
#define XREALLOC jl_realloc_fct
#define XFREE jl_free_fct

#include <tomcrypt.h>


#define RSA_SIGN_DEFAULT_SALT_LENGTH 16
#define ASYMMETRIC_CIPHER_PRNG_SLOT 0

struct PrngPrivate {

	ltc_prng_descriptor prng;
	prng_state state;
};


NEVER_INLINE JSBool FASTCALL
ThrowCryptError( JSContext *cx, int errorCode );

