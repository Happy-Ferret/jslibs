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
#include "jlhelper.cpp"
#include "jslibsModule.cpp"

#if defined(_MSC_VER)
#pragma comment(lib, "advapi32.lib") // rng_get_bytes.obj (function _rng_win32) needs __imp__CryptReleaseContext@8, __imp__CryptGenRandom@12, __imp__CryptAcquireContextA@20
#endif

#include "misc.h"
#include "asymmetricCipher.h"
#include "prng.h"
#include "hash.h"
#include "cipher.h"



/**doc t:header
$MODULE_HEADER
 jscrypt is a cryptographic toolkit that provides developers with a vast array of well known published block ciphers,
 one-way hash functions, chaining modes, pseudo-random number generators, public key cryptography and a plethora of other routines.
 The underlying native code of this module is based on [http://libtom.org/ LibTom library].
$FILE_TOC
**/

/**doc t:footer
$MODULE_FOOTER
**/

EXTERN_C DLLEXPORT JSBool ModuleInit(JSContext *cx, JSObject *obj, uint32_t id) {

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
		&multi2_desc,
	};

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

	const struct ltc_prng_descriptor * prngList[] = {
		&yarrow_desc,
		&fortuna_desc,
		&rc4_desc,
		&sprng_desc,
		&sober128_desc,
	};

	JL_CHK( InitJslibsModule(cx, id)  );

#ifdef GMP_DESC
	ltc_mp = gmp_desc; // register math
#elif LTM_DESC
	ltc_mp = ltm_desc; // register math
#endif

	int regStatus;

	for ( size_t i = 0; i < COUNTOF(cipherList); i++ ) {

		regStatus = register_cipher(cipherList[i]);
		JL_S_ASSERT( regStatus != -1, "Unable to load cipher %s", cipherList[i]->name );
	}

	for ( size_t i = 0; i < COUNTOF(hashList); i++ ) {

		regStatus = register_hash(hashList[i]);
		JL_S_ASSERT( regStatus != -1, "Unable to load hash %s", hashList[i]->name );
	}

	for ( size_t i = 0; i < COUNTOF(prngList); i++ ) {

		regStatus = register_prng(prngList[i]);
		JL_S_ASSERT( regStatus != -1, "Unable to load prng %s", prngList[i]->name );
	}

	INIT_STATIC();
	INIT_CLASS( CryptError );
	INIT_CLASS( AsymmetricCipher );
	INIT_CLASS( Cipher );
	INIT_CLASS( Prng );
	INIT_CLASS( Hash );

	return JS_TRUE;
	JL_BAD;
}
