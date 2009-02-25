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
#include <errno.h>

struct Private {
	iconv_t cd;
	bool wFrom;
	bool wTo;
	size_t remainderLen;
	char remainderBuf[MB_LEN_MAX];
};


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Iconv ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

DEFINE_FINALIZE() { // called when the Garbage Collector is running if there are no remaing references to this object.

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	if ( pv != NULL ) {
		
		int status = iconv_close(pv->cd); // if ( status == -1 ) error is in errno.
		JS_free(cx, pv);
	}
}

/**doc
 * $INAME( toCode, fromCode [ , toUseWide, fromUseWide ] )
  Constructs a new conversion object that transforms from _fromCode_ into _toCode_.
  $H arguments
   $ARG string toCode: destination encoding (see Iconv.list property)
   $ARG string fromCode: source encoding (see Iconv.list property)
   $ARG boolean toUseWide:
   $ARG boolean fromUseWide:
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN(2);

	const char *tocode;
	const char *fromcode;

	J_CHK( JsvalToString(cx, &J_ARG(1), &tocode) );
	J_CHK( JsvalToString(cx, &J_ARG(2), &fromcode) );

	Private *pv;
	pv = (Private*)JS_malloc(cx, sizeof(Private));
	J_S_ASSERT_ALLOC(pv);

	pv->remainderLen = 0;
	pv->cd = iconv_open(tocode, fromcode);

	if ( J_ARG_ISDEF(3) )
		J_CHK( JsvalToBool(cx, J_ARG(3), &pv->wTo) );
	else
		pv->wTo = false;

	if ( J_ARG_ISDEF(4) )
		J_CHK( JsvalToBool(cx, J_ARG(4), &pv->wFrom) );
	else
		pv->wFrom = false;

	if ( (size_t)pv->cd == (size_t)-1 ) {
		
		if ( errno == EINVAL )
			J_REPORT_ERROR_2( "The conversion from %s to %s is not supported.", fromcode, tocode );
		else
			J_REPORT_ERROR( "Unknown iconv error." );
	}
	J_CHK( JS_SetPrivate(cx, obj, pv) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
 * $STR $INAME( [ textData ] )
  Converts textData. If called without argument, this resets the conversion state to the initial state and returns $UNDEF.
**/
DEFINE_CALL() {
		
	JSObject *thisObj = JSVAL_TO_OBJECT(argv[-2]); // get 'this' object of the current object ...
	J_S_ASSERT_CLASS(thisObj, classIconv);

	Private *pv;
	pv = (Private*)JS_GetPrivate(cx, thisObj);
	J_S_ASSERT_RESOURCE( pv );

	size_t status;

	if ( !J_ARG_ISDEF(1) ) {
		
		status = iconv(pv->cd, NULL, NULL, NULL, NULL); // sets cd's conversion state to the initial state.
		return JS_TRUE;
	}

	const char *inBuf;
	size_t inLen;

	if ( pv->wFrom ) { // source is wide.

		JSString *jsstr = JS_ValueToString(cx, J_ARG(1));
		J_ARG(1) = STRING_TO_JSVAL( jsstr );
		inLen = JS_GetStringLength(jsstr) * 2;
		inBuf = (char*)JS_GetStringChars(jsstr);
	} else {

		J_CHK( JsvalToStringAndLength(cx, &J_ARG(1), &inBuf, &inLen) );
	}

	const char *inPtr;
	inPtr = inBuf;
	size_t inLeft;
	inLeft = inLen;

	char *outBuf;
	size_t outLen;

	outLen = inLen + MB_LEN_MAX + 512; // * 3/2; // * 1.5 (we use + MB_LEN_MAX to avoid remainderLen... section to failed with E2BIG)
	outBuf = (char*)JS_malloc(cx, outLen +1);
	J_S_ASSERT_ALLOC( outBuf );

	char *outPtr;
	outPtr = outBuf;
	size_t outLeft;
	outLeft = outLen;

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
		status = iconv(pv->cd, &inPtr, &inLeft, &outPtr, &outLeft); // doc: http://www.manpagez.com/man/4/iconv/

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

	size_t length;
	length = outPtr - outBuf;

	if ( MaybeRealloc(outLen, length) ) {

		outBuf = (char*)JS_realloc(cx, outBuf, length +1);
		J_S_ASSERT_ALLOC(outBuf);
	}
	outBuf[length] = '\0';

	if ( pv->wTo ) { // destination is wide.
		
		JSString *wstr = JS_NewUCString(cx, (jschar*)outBuf, length / 2);
		*J_RVAL = STRING_TO_JSVAL(wstr);
	} else {

		J_CHK( StringAndLengthToJsval(cx, J_RVAL, outBuf, length) );
	}

	return JS_TRUE;
	JL_BAD;
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
  Lists locale independent encodings.
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
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))
	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	HAS_CALL

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ_STORE( list )
	END_STATIC_PROPERTY_SPEC

END_CLASS


/**doc
=== Example 1 ===
 Convert and convert back a string.
{{{
LoadModule('jsstd');
LoadModule('jsiconv');

var conv = new Iconv('UTF-8', 'ISO-8859-1');
var invConv = new Iconv('ISO-8859-1', 'UTF-8');
var converted = conv('été');
var result = invConv(converted);
Print( result == 'été','\n' ); // should be true
}}}

=== Example 2 ===
 Convert and convert back a string char by char.
{{{
var conv = new Iconv('UTF-8', 'ISO-8859-1');
var invConv = new Iconv('ISO-8859-1', 'UTF-8');
var converted = conv('été');
var result = '';
for each ( var c in converted )
 result += invConv(c);
Print( result == 'été','\n' ); // should be true
}}}
**/
