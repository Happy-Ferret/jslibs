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

#include <stddef.h>
#include <iconv.h>


struct Private {
	iconv_t cd;
	size_t tmpLen;
	char tmp[MB_LEN_MAX];
};


/**doc
$CLASS_HEADER
**/
BEGIN_CLASS( Iconv ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

DEFINE_FINALIZE() { // called when the Garbage Collector is running if there are no remaing references to this object.

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	if ( pv != NULL ) {
		
		iconv_close(pv->cd);
		JS_free(cx, pv);
	}
}


DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN(2);

	const char *tocode;
	const char *fromcode;

	J_CHK( JsvalToString(cx, &J_ARG(1), &tocode) );
	J_CHK( JsvalToString(cx, &J_ARG(2), &fromcode) );

	Private *pv = (Private*)JS_malloc(cx, sizeof(Private));
	J_S_ASSERT_ALLOC(pv);

	pv->cd = iconv_open(tocode, fromcode);

	J_S_ASSERT_2( pv->cd != NULL, "Unable to open %s to %s conversion.", fromcode, tocode );

	J_CHK( JS_SetPrivate(cx, obj, pv) );

	return JS_TRUE;
}


DEFINE_CALL() {

	J_S_ASSERT_ARG_MIN(1);
		
	JSObject *thisObj = JSVAL_TO_OBJECT(argv[-2]); // get 'this' object of the current object ...
	J_S_ASSERT_CLASS(thisObj, classIconv);

	Private *pv = (Private*)JS_GetPrivate(cx, thisObj);
	J_S_ASSERT_RESOURCE( pv );

	const char *inBuf;
	size_t inLen;
	J_CHK( JsvalToStringAndLength(cx, &J_ARG(1), &inBuf, &inLen) );

	char *outBuf;
	size_t outLen;

	outLen = inLen*2;
	outBuf = (char*)JS_malloc(cx, outLen);
	J_S_ASSERT_ALLOC( outBuf );

	char *tmpBuf;
	size_t tmpLen;


	if ( pv->tmpLen ) {
		
		size_t more = J_MIN(inBuf, MB_LEN_MAX - pv->tmpLen);

		memcpy(pv->tmp + pv->tmpLen, inBuf, more);

		pv->tmpLen += more;
	
		tmpBuf = pv->tmp;
		tmpLen = pv->tmpLen;

		size_t status = iconv(pv->cd, &tmpBuf, &tmpLen, &tmpBuf, &tmpLen);
	
	}

	tmpBuf = outBuf;
	tmpLen = outLen;

	
	do {
		
		size_t status = iconv(pv->cd, &inBuf, &inLen, &tmpBuf, &tmpLen);

		if ( status == (size_t)(-1) )
			switch ( errno ) { // doc: http://www.manpagez.com/man/3/iconv/
				case E2BIG: // There is not sufficient room at *outbuf.
					J_REPORT_ERROR("Internal error: buffer too small.");
					break;
				case EILSEQ: // An invalid multibyte sequence has been encountered in the input.
					if ( inLen-- )
						inBuf++;
					break;
				case EINVAL: { // An incomplete multibyte sequence has been encountered in the input.
					J_S_ASSERT(inLen < MB_LEN_MAX);
					pv->tmpLen = inLen;
					memcpy(pv->tmp, inBuf, inLen);
					break;
				}
			}
	} while(inLen);
	
	//MaybeRealloc

	return JS_TRUE;
}

CONFIGURE_CLASS // This section containt the declaration and the configuration of the class

	HAS_PRIVATE
	HAS_CALL

	HAS_CONSTRUCTOR
	HAS_FINALIZE

END_CLASS
