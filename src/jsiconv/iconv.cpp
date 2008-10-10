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
	size_t remainderLen;
	char remainderBuf[MB_LEN_MAX];
};


/**doc
$CLASS_HEADER
**/
BEGIN_CLASS( Iconv ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

DEFINE_FINALIZE() { // called when the Garbage Collector is running if there are no remaing references to this object.

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	if ( pv != NULL ) {
		
		int status = iconv_close(pv->cd); // if ( status == -1 ) error is in errno.
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

	pv->remainderLen = 0;
	pv->cd = iconv_open(tocode, fromcode);

	if ( (size_t)pv->cd == (size_t)-1 ) {
		
		if ( errno == EINVAL )
			J_REPORT_ERROR_2( "The conversion from %s to %s is not supported.", fromcode, tocode );
		else
			J_REPORT_ERROR( "Unknown iconv error." );
	}
	J_CHK( JS_SetPrivate(cx, obj, pv) );
	return JS_TRUE;
}



DEFINE_CALL() {
		
	JSObject *thisObj = JSVAL_TO_OBJECT(argv[-2]); // get 'this' object of the current object ...
	J_S_ASSERT_CLASS(thisObj, classIconv);

	Private *pv = (Private*)JS_GetPrivate(cx, thisObj);
	J_S_ASSERT_RESOURCE( pv );

	size_t status;

	if ( !J_ARG_ISDEF(1) ) {
		
		status = iconv(pv->cd, NULL, NULL, NULL, NULL); // sets cd's conversion state to the initial state.
		return JS_TRUE;
	}

	const char *inBuf;
	size_t inLen;
	J_CHK( JsvalToStringAndLength(cx, &J_ARG(1), &inBuf, &inLen) );

	const char *inPtr = inBuf;
	size_t inLeft = inLen;

	char *outBuf;
	size_t outLen;

	outLen = inLen + MB_LEN_MAX + 512; // * 3/2; // * 1.5 (we use + MB_LEN_MAX to avoid remainderLen... section to failed with E2BIG)
	outBuf = (char*)JS_malloc(cx, outLen +1);
	J_S_ASSERT_ALLOC( outBuf );

	char *outPtr = outBuf;
	size_t outLeft = outLen;

	if ( pv->remainderLen ) { // have to process previous the incomplete multibyte sequence ?

		const char *tmpPtr;
		size_t tmpLeft;
		do {
			pv->remainderBuf[pv->remainderLen++] = *inPtr;
			inPtr++;
			inLeft--;

			tmpPtr = pv->remainderBuf;
			tmpLeft = pv->remainderLen;

			status = iconv(pv->cd, &tmpPtr, &tmpLeft, &outPtr, &outLeft);

			if ( status != (size_t)(-1) )
				break;

			// (TBD) manage EILSEQ like this ??? :
			if ( errno == EILSEQ ) { // An invalid multibyte sequence has been encountered in the input.

				inPtr--; // rewind by 1
				inLeft++;
				break;
			}

		} while ( errno == EINVAL && pv->remainderLen < sizeof(pv->remainderBuf) );
		pv->remainderLen = 0;
	}

	do {
		status = iconv(pv->cd, &inPtr, &inLeft, &outPtr, &outLeft); // doc: http://www.manpagez.com/man/3/iconv/

		if ( status == (size_t)(-1) )
			switch ( errno ) {
				case E2BIG: { // There is not sufficient room at *outbuf.

						int processedOut = outPtr - outBuf;
						outLen = inLen * processedOut / (inPtr - inBuf) + 512; // try to guess a better outLen based on the current in/out ratio.
						outBuf = (char*)JS_realloc(cx, outBuf, outLen +1);
						J_S_ASSERT_ALLOC(outBuf);
						outPtr = outBuf + processedOut;
						outLeft = outLen - processedOut;
					}
					break;

				case EILSEQ: // An invalid multibyte sequence has been encountered in the input. *inPtr is left pointing to the beginning of the invalid multibyte sequence.

					if ( outLeft < MB_LEN_MAX + 1 ) {
						
						int processedOut = outPtr - outBuf;
						outLen = inLen * processedOut / (inPtr - inBuf) + 512; // try to guess a better outLen based on the current in/out ratio.
						outBuf = (char*)JS_realloc(cx, outBuf, outLen +1);
						J_S_ASSERT_ALLOC(outBuf);
						outPtr = outBuf + processedOut;
						outLeft = outLen - processedOut;
					}

					status = iconv(pv->cd, NULL, NULL, &outPtr, &outLeft); // to set cd's conversion state to the initial state and store a corresponding shift sequence at *outbuf.
					*outPtr = '?';
					outPtr++;
					outLeft--;
					inPtr++;
					inLeft--;
					break;

				case EINVAL: { // An incomplete multibyte sequence has been encountered in the input.

					J_S_ASSERT(inLeft < MB_LEN_MAX, "Unable to manage incomplete multibyte sequence.");
					memcpy(pv->remainderBuf + pv->remainderLen, inPtr, inLeft); // save
					pv->remainderLen = inLeft;
					inPtr += inLeft;
					inLeft = 0;
					break;
				}
			}

	} while( inLeft );

	size_t length = outPtr - outBuf;

	if ( MaybeRealloc(outLen, length) ) {

		outBuf = (char*)JS_realloc(cx, outBuf, length +1);
		J_S_ASSERT_ALLOC(outBuf);
	}
	outBuf[length] = '\0';
	J_CHK( StringAndLengthToJsval(cx, J_RVAL, outBuf, length) );

//	JS_NewUCString(cx, (jschar*)outBuf,   // (TBD)

	return JS_TRUE;
}


struct IteratorPrivate {
	JSContext *cx;
	JSObject *list;
	size_t listLen;
};


int do_one( unsigned int namescount, const char * const * names, void* data ) {

	IteratorPrivate *ipv = (IteratorPrivate*)data;
	jsval value;
	while (namescount--) {

		StringToJsval(ipv->cx, names[namescount], &value); // iconv_canonicalize
		JS_SetElement(ipv->cx, ipv->list, ipv->listLen, &value);
		ipv->listLen++;
	}
	return 0;
}


/**doc
 * $OBJ $INAME $READONLY
**/
DEFINE_PROPERTY( list ) {

	if ( JSVAL_IS_VOID( *vp ) ) {

		JSObject *list = JS_NewArrayObject(cx, 0, NULL);
		J_S_ASSERT_ALLOC( list );
		*vp = OBJECT_TO_JSVAL( list );
		IteratorPrivate ipv = { cx, list, 0 };
		iconvlist(do_one, &ipv);
	}
	return JS_TRUE;
}


CONFIGURE_CLASS // This section containt the declaration and the configuration of the class

	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	HAS_CALL

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ_STORE( list )
	END_STATIC_PROPERTY_SPEC

END_CLASS
