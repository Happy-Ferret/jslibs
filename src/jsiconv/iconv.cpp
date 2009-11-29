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

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	if ( !pv )
		return;
		
	int status = iconv_close(pv->cd); // if ( status == -1 ) error is in errno.
	if ( status == -1 )
		JL_REPORT_WARNING("iconv_close failure in Iconv finalize (%d).", errno);

	JS_free(cx, pv);
}

/**doc
$TOC_MEMBER $INAME
 $INAME( toCode, fromCode [ , toUseWide, fromUseWide ] )
  Constructs a new conversion object that transforms from _fromCode_ into _toCode_.
  $H arguments
   $ARG $STR toCode: destination encoding (see Iconv.list property)
   $ARG $STR fromCode: source encoding (see Iconv.list property)
   $ARG $BOOL toUseWide: destination use 16bit per char.
   $ARG $BOOL fromUseWide: source use 16bit per char.
  $H example
{{{
  var consEnc = new Iconv(consoleCodepage, 'UCS-2-INTERNAL', false, true); // source is wide (16bit), dest is not wide (8bit)
  Print( consEnc('été') );
}}}

**/
DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();
	JL_S_ASSERT_ARG_MIN(2);

	const char *tocode;
	const char *fromcode;

	JL_CHK( JsvalToString(cx, &JL_ARG(1), &tocode) );
	JL_CHK( JsvalToString(cx, &JL_ARG(2), &fromcode) );

	Private *pv;
	pv = (Private*)JS_malloc(cx, sizeof(Private));
	JL_CHK(pv);

	pv->remainderLen = 0;
	pv->cd = iconv_open(tocode, fromcode);

	if ( JL_ARG_ISDEF(3) )
		JL_CHK( JsvalToBool(cx, JL_ARG(3), &pv->wTo) );
	else
		pv->wTo = false;

	if ( JL_ARG_ISDEF(4) )
		JL_CHK( JsvalToBool(cx, JL_ARG(4), &pv->wFrom) );
	else
		pv->wFrom = false;

	if ( (size_t)pv->cd == (size_t)-1 ) {
		
		if ( errno == EINVAL )
			JL_REPORT_ERROR( "The conversion from %s to %s is not supported.", fromcode, tocode );
		else
			JL_REPORT_ERROR( "Unknown iconv error." );
	}
	JL_SetPrivate(cx, obj, pv);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( [ textData ] )
  Converts textData. If called without argument, this resets the conversion state to the initial state and returns $UNDEF.
**/
DEFINE_CALL() {
	
	char *outBuf = NULL; // keep on top

	JSObject *thisObj = JSVAL_TO_OBJECT(argv[-2]); // get 'this' object of the current object ...
	JL_S_ASSERT_CLASS(thisObj, classIconv);

	Private *pv;
	pv = (Private*)JL_GetPrivate(cx, thisObj);
	JL_S_ASSERT_RESOURCE( pv );

	size_t status;

	if ( argc = 0 ) {
		
		status = iconv(pv->cd, NULL, NULL, NULL, NULL); // sets cd's conversion state to the initial state.
		return JS_TRUE;
	}

	const char *inBuf;
	size_t inLen;

	if ( pv->wFrom ) { // source is wide.
		
		// (TBD) check the string size

		JSString *jsstr = JS_ValueToString(cx, JL_ARG(1));
		JL_ARG(1) = STRING_TO_JSVAL( jsstr );
		inLen = JL_GetStringLength(jsstr) * 2;
		inBuf = (char*)JS_GetStringChars(jsstr);
	} else {

		JL_CHK( JsvalToStringAndLength(cx, &JL_ARG(1), &inBuf, &inLen) );
	}

	const char *inPtr;
	inPtr = inBuf;
	size_t inLeft;
	inLeft = inLen;

	size_t outLen;


	outLen = inLen + MB_LEN_MAX + 512; // * 3/2; // * 1.5 (we use + MB_LEN_MAX to avoid remainderLen... section to failed with E2BIG)
	outBuf = (char*)JS_malloc(cx, outLen +1);
	JL_CHK( outBuf );

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
				
				// (TBD) manage to add a '?' char
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
						JL_CHK(outBuf);
						outPtr = outBuf + processedOut;
						outLeft = outLen - processedOut;
					}
					break;

				case EILSEQ: // An invalid multibyte sequence has been encountered in the input. *inPtr is left pointing to the beginning of the invalid multibyte sequence.

					if ( outLeft < MB_LEN_MAX + 1 ) {
						
						int processedOut = outPtr - outBuf;
						outLen = inLen * processedOut / (inPtr - inBuf) + 512; // try to guess a better outLen based on the current in/out ratio.
						outBuf = (char*)JS_realloc(cx, outBuf, outLen +1);
						JL_CHK(outBuf);
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

					JL_S_ASSERT(inLeft < MB_LEN_MAX, "Unable to manage incomplete multibyte sequence.");
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

	if ( JL_MaybeRealloc(outLen, length) ) {

		outBuf = (char*)JS_realloc(cx, outBuf, length +1);
		JL_CHK( outBuf );
	}
	outBuf[length] = '\0';

	if ( pv->wTo ) { // destination is wide.
		
		JSString *wstr = JS_NewUCString(cx, (jschar*)outBuf, (length+1) / 2);
		*JL_RVAL = STRING_TO_JSVAL(wstr);
	} else {

		JL_CHK( StringAndLengthToJsval(cx, JL_RVAL, outBuf, length) );
	}

	return JS_TRUE;
bad:
	if ( outBuf != NULL )
		JS_free(cx, outBuf);
	return JS_FALSE;
}



/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME $READONLY
  Lists locale independent encodings.
**/

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

#define JL_HAS_ICONVLIST
#ifdef JL_HAS_ICONVLIST
DEFINE_PROPERTY( list ) {

	JSObject *list = JS_NewArrayObject(cx, 0, NULL);
	JL_CHK( list );
	*vp = OBJECT_TO_JSVAL( list );
	IteratorPrivate ipv;
	ipv.cx = cx;
	ipv.list = list;
	ipv.listLen = 0;
	iconvlist(do_one, &ipv);
	return JL_StoreProperty(cx, obj, id, vp, true);
	JL_BAD;
}
#endif // JL_HAS_ICONVLIST


DEFINE_PROPERTY( version ) {

	char versionStr[16];
	strcpy( versionStr, IntegerToString( _LIBICONV_VERSION >> 8, 10 ) );
	strcat( versionStr, ".");
	strcat( versionStr, IntegerToString( _LIBICONV_VERSION & 0xFF, 10 ) );
	return StringToJsval(cx, versionStr, vp);
	return JL_StoreProperty(cx, obj, id, vp, true);
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	HAS_CALL

	BEGIN_STATIC_PROPERTY_SPEC
#ifdef JL_HAS_ICONVLIST
		PROPERTY_READ( list )
#endif // JL_HAS_ICONVLIST
		PROPERTY_READ( version )
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
