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


#include "misc.h"

#include "rsa.h"
#include "prng.h"
#include "hash.h"
#include "cipher.h"

DEFINE_UNSAFE_MODE;

extern "C" DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj) {

	SET_UNSAFE_MODE( GetConfigurationValue(cx, "unsafeMode" ) == JSVAL_TRUE );

	ltc_mp = ltm_desc; // register math

	const struct ltc_cipher_descriptor * cipherList[] = {
		&blowfish_desc, 
		&rc5_desc, 
		&rc6_desc, 
		&rc2_desc, 
		&saferp_desc,
		&safer_k64_desc, &safer_k128_desc, &safer_sk64_desc, &safer_sk128_desc,
		&rijndael_desc, &aes_desc,
		&rijndael_enc_desc, &aes_enc_desc,
		&xtea_desc,
		&twofish_desc,
		&des_desc, &des3_desc,
		&cast5_desc,
		&noekeon_desc,
		&skipjack_desc,
		&khazad_desc,
		&anubis_desc,
		&kseed_desc,
		&kasumi_desc,
//		&multi2_desc,
	};
	for ( int i=0; i<sizeof(cipherList)/sizeof(ltc_cipher_descriptor*); i++ )
		RT_ASSERT_1( register_cipher(cipherList[i]) != -1, "Unable to load cipher %s", cipherList[i]->name );

	const struct ltc_hash_descriptor * hashList[] = {
		&chc_desc,
		&whirlpool_desc,
		&sha512_desc,
		&sha384_desc,
		&sha256_desc,
		&sha224_desc,
		&sha1_desc,
		&md5_desc,
		&md4_desc,
		&md2_desc,
		&tiger_desc,
		&rmd128_desc,
		&rmd160_desc,
		&rmd256_desc,
		&rmd320_desc,
	};
	for ( int i=0; i<sizeof(hashList)/sizeof(ltc_hash_descriptor*); i++ )
		RT_ASSERT_1( register_hash(hashList[i]) != -1, "Unable to load hash %s", hashList[i]->name );

	const struct ltc_prng_descriptor * prngList[] = {
		&yarrow_desc,
		&fortuna_desc,
		&rc4_desc,
		&sprng_desc,
		&sober128_desc,
	};
	for ( int i=0; i<sizeof(prngList)/sizeof(ltc_prng_descriptor*); i++ )
		RT_ASSERT_1( register_prng(prngList[i]) != -1, "Unable to load prng %s", prngList[i]->name );

	SET_UNSAFE_MODE( GetConfigurationValue(cx, NAME_CONFIGURATION_UNSAFE_MODE ) == JSVAL_TRUE );

	INIT_STATIC();
	INIT_CLASS( CryptError );
	INIT_CLASS( Rsa );
	INIT_CLASS( Cipher );
	INIT_CLASS( Prng );
	INIT_CLASS( Hash );
	return JS_TRUE;
}
