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


#if defined(JL_ICONV_PROTO_ARG_NOT_CONST)
#define JL_ICONV_PROTO_ARG
#else
#define JL_ICONV_PROTO_ARG const
#endif

struct Private {
	iconv_t cd;
	bool wFrom;
	bool wTo;
	size_t remainderLen;
	char remainderBuf[MB_LEN_MAX];
	char invalidChar;
};


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Iconv ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

DEFINE_FINALIZE() { // called when the Garbage Collector is running if there are no remaing references to this object.

	if ( jl::HostRuntime::getJLRuntime( fop->runtime() ).skipCleanup() )
		return;

	Private *pv = (Private*)JL_GetPrivateFromFinalize(obj);
	if ( !pv )
		return;
	int status = iconv_close(pv->cd); // if ( status == -1 ) error is in errno.
	JS_freeop(fop, pv);
	
	// JL_ASSERT_WARN( status != -1, E_LIB, E_STR("iconv"), E_FIN, E_ERRNO(errno) ); // (TBD) send to log !

bad:
	return;
}

/**doc
$TOC_MEMBER $INAME
 $INAME( toCode, fromCode [ , toUsesWide, fromUsesWide ] )
  Constructs a new conversion object that transforms from _fromCode_ into _toCode_.
  $H arguments
   $ARG $STR toCode: destination encoding (see Iconv.list property)
   $ARG $STR fromCode: source encoding (see Iconv.list property)
   $ARG $BOOL toUsesWide: destination uses 16bit per char.
   $ARG $BOOL fromUsesWide: source uses 16bit per char.
  $H example
{{{
  loadModule('winshell'); // required for consoleCodepage
  var consEnc = new Iconv(consoleCodepage, 'UCS-2le', false, true); // source is wide (16bit), dest is not wide (8bit)
  print( consEnc('été') );
}}}

**/
DEFINE_CONSTRUCTOR() {

	JL_DEFINE_ARGS;

	Private *pv = NULL;

	JL_ASSERT_ARGC_RANGE(2, 4);
	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	{

		jl::StrData tocode(cx);
		jl::StrData fromcode(cx);
		JL_CHK( jl::getValue(cx, JL_ARG(1), &tocode) );
		JL_CHK( jl::getValue(cx, JL_ARG(2), &fromcode) );

		pv = (Private*)JS_malloc(cx, sizeof(Private));
		JL_CHK(pv);
		pv->cd = NULL;

		pv->remainderLen = 0;
		pv->cd = iconv_open(tocode, fromcode);

		if ( JL_ARG_ISDEF(3) )
			JL_CHK( jl::getValue(cx, JL_ARG(3), &pv->wTo) );
		else
			pv->wTo = false;

		if ( JL_ARG_ISDEF(4) )
			JL_CHK( jl::getValue(cx, JL_ARG(4), &pv->wFrom) );
		else
			pv->wFrom = false;

		if ( (size_t)pv->cd == (size_t)-1 ) {
		
			if ( errno == EINVAL )
				//JL_REPORT_ERROR_NUM( JLSMSG_INVALID_OPERATION ); //, "The conversion from %s to %s is not supported.", fromcode, tocode );
				JL_ERR( E_LIB, E_STR("iconv"), E_OPERATION, E_SEP, E_STR(fromcode), E_CONVERT, E_STR(tocode) );
			else
				JL_ERR( E_LIB, E_STR("iconv"), E_OPERATION, E_ERRNO(errno) );
		}

	}

	pv->invalidChar = '?';

	JL_SetPrivate(JL_OBJ, pv);
	

	return true;

bad:
	if ( pv ) {

		if ( pv->cd )
			iconv_close(pv->cd);
		JS_free(cx, pv);
	}
	return false;
}


/**doc
$TOC_MEMBER $INAME
 $STR $INAME( [ textData ] )
  Converts textData. If called without argument, this resets the conversion state to the initial state and returns $UNDEF.
**/
DEFINE_FUNCTION( process ) {
	
	JL_DEFINE_ARGS;

	char *outBuf = NULL; // keep on top
	
	JL_ASSERT_INSTANCE(JL_OBJ, JL_CLASS(Iconv));

	Private *pv;
	pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	{

		jl::StrData data(cx);

		size_t status;

		if ( JL_ARGC == 0 ) {
		
			status = iconv(pv->cd, NULL, NULL, NULL, NULL); // sets cd's conversion state to the initial state.
			pv->remainderLen = 0;
			return true;
		}

		char *inBuf;
		size_t inLen;

		//if ( pv->wFrom ) { // source is wide.
		//	
		//	JSString *jsstr = JS::ToString(cx, JL_ARG(1));
		//	JL_ARG(1) = STRING_TO_JSVAL( jsstr );
		//	inLen = JL_GetStringLength(jsstr) * 2;
		//	inBuf = (char *)JS_GetStringChars(jsstr);
		//} else {

		//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), (const char**)&inBuf, &inLen) );
		//}

		JL_CHK( jl::getValue(cx, JL_ARG(1), &data) );

		JL_ICONV_PROTO_ARG char *inPtr;
		size_t inLeft;

		size_t outLen;
		char *outPtr;
		size_t outLeft;

		{

			JS::AutoCheckCannotGC nogc;

			inLen = data.length();
			if ( pv->wFrom ) {

				inLen *= 2;
				inBuf = (char*)data.toWStr(nogc);
			} else {

				inBuf = (char*)data.toStr(nogc);
			}


			inPtr = inBuf;
			inLeft = inLen;

			outLen = inLen + MB_LEN_MAX + 512; // (we use + MB_LEN_MAX to avoid "remainderLen section" to failed with E2BIG)
			outBuf = (char*)JS_malloc(cx, outLen +2);
			JL_CHK( outBuf );

			outPtr = outBuf;
			outLeft = outLen;

			if ( pv->remainderLen ) { // have to process a previous incomplete multibyte sequence ?

				JL_ICONV_PROTO_ARG char *tmpPtr;
				size_t tmpLeft;
				do {

					pv->remainderBuf[pv->remainderLen++] = *inPtr; // complete the sequence with incomming data
					inPtr++;
					inLeft--;

					tmpPtr = pv->remainderBuf;
					tmpLeft = pv->remainderLen;

					// iconv() proto in jslibs/libs/libiconv: extern size_t iconv (iconv_t cd, const char* * inbuf, size_t *inbytesleft, char* * outbuf, size_t *outbytesleft);
					status = iconv(pv->cd, &tmpPtr, &tmpLeft, &outPtr, &outLeft);

					if ( status != (size_t)(-1) )
						break;
			
					// E2BIG will never happens
					ASSERT( status != E2BIG );

					// (TBD) manage EILSEQ like this ??? :
					if ( errno == EILSEQ ) { // An invalid multibyte sequence has been encountered in the input.
				
						*outPtr = pv->invalidChar; // space should be available to acheive this.
						outPtr++;
						outLeft--;

						inPtr--; // rewind by 1
						inLeft++;
						break;
					}

				} while ( errno == EINVAL && pv->remainderLen < sizeof(pv->remainderBuf) );

				iconv(pv->cd, NULL, NULL, NULL, NULL); // reset
				pv->remainderLen = 0;
			}

			do {

				// iconv() proto in jslibs/libs/libiconv: extern size_t iconv (iconv_t cd, const char* * inbuf, size_t *inbytesleft, char* * outbuf, size_t *outbytesleft);
				status = iconv(pv->cd, &inPtr, &inLeft, &outPtr, &outLeft); // doc: http://www.manpagez.com/man/4/iconv/

				if ( status == (size_t)(-1) )
					switch ( errno ) {

						case E2BIG: { // There is not sufficient room at *outbuf.

								int processedOut = outPtr - outBuf;
								outLen = inLen * processedOut / (inPtr - inBuf) + 512; // try to guess a better outLen based on the current in/out ratio.
								outBuf = (char*)JS_realloc(cx, outBuf, outLen +2);
								JL_CHK(outBuf);
								outPtr = outBuf + processedOut;
								outLeft = outLen - processedOut;
							}
							break;

						case EILSEQ: // An invalid multibyte sequence has been encountered in the input. *inPtr is left pointing to the beginning of the invalid multibyte sequence.

							if ( outLeft < MB_LEN_MAX +1 ) {
						
								int processedOut = outPtr - outBuf;
								outLen = inLen * processedOut / (inPtr - inBuf) + 512; // try to guess a better outLen based on the current in/out ratio.
								outBuf = (char*)JS_realloc(cx, outBuf, outLen +2);
								JL_CHK(outBuf);
								outPtr = outBuf + processedOut;
								outLeft = outLen - processedOut;
							}

							status = iconv(pv->cd, NULL, NULL, &outPtr, &outLeft); // to set cd's conversion state to the initial state and store a corresponding shift sequence at *outbuf.
							*outPtr = pv->invalidChar;
							outPtr++;
							outLeft--;
							inPtr++;
							inLeft--;
							break;

						case EINVAL: { // An incomplete multibyte sequence has been encountered in the input.

							JL_CHKM( inLeft < sizeof(pv->remainderBuf), E_LIB, E_STR("iconv"), E_OPERATION, E_COMMENT("incomplete multibyte sequence"));
							jl::memcpy(pv->remainderBuf + pv->remainderLen, inPtr, inLeft); // save
							pv->remainderLen = inLeft;
							inPtr += inLeft;
							inLeft = 0;
							break;
						}
					}

			} while ( inLeft );


		} // scope


		size_t length;
		length = outPtr - outBuf;

		if ( length == 0 ) {
		
			JS_free(cx, outBuf);
			JL_RVAL.set(JL_GetEmptyStringValue(cx));
			return true;
		}

		if ( jl::maybeRealloc(outLen, length) ) {

			outBuf = (char*)JS_realloc(cx, outBuf, length +2);
			JL_CHK( outBuf );
		}

		outBuf[length] = '\0'; // (TBD) manage \0 for wide char

		JSString *jsEncStr;
		if ( pv->wTo ) { // destination is wide.
	
	//		if ( length % 2 != 0 )
	//			JL_REPORT_WARNING_NUM( JLSMSG_LOGIC_ERROR, "invalid string length"); // (TBD) or report an error ?
			JL_ASSERT_WARN( length % 2 == 0, E_STR("invalid string length") );

			((jschar*)outBuf)[length / 2] = 0;
			jsEncStr = JL_NewUCString(cx, (jschar*)outBuf, length / 2);
			JL_CHK( jsEncStr );
			JL_RVAL.setString(jsEncStr);
		} else {

			//jsEncStr = JL_NewString(cx, outBuf, length); // loose outBuf ownership	// JL_CHK( StringAndLengthToJsval(cx, JL_RVAL, outBuf, length) );
			//JL_CHK( jl::BufString(outBuf, length, true).toString(cx, JL_RVAL) );
			JL_RVAL.setString( JL_NewString(cx, outBuf, length) );
		}
		
	}

	return true;

bad:
	JS_free(cx, outBuf); // JS_free NULL pointer is legal.
	return false;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME
  Is the replacement char used when an invalid multibyte sequence has been encountered.
**/
DEFINE_PROPERTY_SETTER( invalidChar ) {

	JL_IGNORE(id, strict);

	Private *pv;
	pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	{

		jl::StrData chr(cx);
		JL_CHK( jl::getValue(cx, vp, &chr) );
		JL_ASSERT_WARN( chr.length() == 1, E_VALUE, E_LENGTH, E_NUM(1) );
		pv->invalidChar = chr.getCharAt(0);
	}

	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( invalidChar ) {

	JL_IGNORE(id);

	Private *pv;
	pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );	
//	JL_CHK( JL_StringAndLengthToJsval(cx, vp, &pv->invalidChar, 1) );
	JL_CHK( jl::setValue(cx, vp, jl::strSpec(&pv->invalidChar, 1)) );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( hasIncompleteSequence ) {

	JL_IGNORE(id);

	Private *pv;
	pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );	
	JL_CHK( jl::setValue(cx, vp, pv->remainderLen != 0) );
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME $READONLY
  Lists locale independent encodings.
**/

struct IteratorPrivate {
public:
	JSContext *cx;
	JS::PersistentRootedObject list;
	int listLen;
	IteratorPrivate(JSContext *cx)
	: list(cx) {
	}
};

int do_one( unsigned int namescount, const char * const * names, void* data ) {

	IteratorPrivate *ipv = (IteratorPrivate*)data;
	JS::RootedValue value(ipv->cx);
	while (namescount--) {

		// iconv_canonicalize
		JL_CHK( jl::setElement( ipv->cx, ipv->list, ipv->listLen, names[namescount] ) );
		ipv->listLen++;
	}
	return 0;
bad:
	return -1;
}

#ifndef JL_NOT_HAS_ICONVLIST
DEFINE_PROPERTY_GETTER( list ) {

	JS::RootedObject list(cx, JS_NewArrayObject(cx, 0));
	JL_CHK( list );
	vp.setObject(*list);
	{
		IteratorPrivate ipv(cx);
		ipv.cx = cx;
		ipv.list = list;
		ipv.listLen = 0;
		iconvlist(do_one, &ipv);
	}
	return jl::StoreProperty(cx, obj, id, vp, true); // create the list and store it once for all.
	JL_BAD;
}
#endif // JL_NOT_HAS_ICONVLIST


DEFINE_PROPERTY_GETTER( version ) {

#ifdef _LIBICONV_VERSION
	char versionStr[8];
	char tmp[32];
	strcpy( versionStr, jl::itoa10(_LIBICONV_VERSION >> 8, tmp) );
	strcat( versionStr, ".");
	strcat( versionStr, jl::itoa10(_LIBICONV_VERSION & 0xFF, tmp) );
#else
	char versionStr[7];
	strcpy( versionStr, "system");
#endif
	JL_CHK( jl::setValue(cx, vp, versionStr) );
	JL_CHK( jl::StoreProperty(cx, obj, id, vp, true) );  // create and store store the value once for all.
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( jsUC ) {

	switch ( JLHostEndian ) {
		case JLBigEndian:
			JL_CHK( jl::setValue(cx, vp, "UCS-2be") );
			break;
		case JLLittleEndian:
			JL_CHK( jl::setValue(cx, vp, "UCS-2le") );
			break;
		default:
			vp.setUndefined();
			break;
	}
	JL_CHK( jl::StoreProperty(cx, obj, id, vp, true) );
	return true;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))
	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC( process, 1 )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( invalidChar )
		PROPERTY_GETTER( hasIncompleteSequence )
	END_PROPERTY_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
#ifndef JL_NOT_HAS_ICONVLIST
		PROPERTY_GETTER( list )
#endif // JL_NOT_HAS_ICONVLIST
		PROPERTY_GETTER( version )
		PROPERTY_GETTER( jsUC )
	END_STATIC_PROPERTY_SPEC

END_CLASS


/**doc

=== Note ===
 To benefit JavaScript unicode strings, you have to use the UCS-2le encoding.$LF
 eg. `var conv = new Iconv('UCS-2le', 'UTF-8', true, false);` $LF
 See also the [http://en.wikipedia.org/wiki/List_of_Unicode_characters List of Unicode characters]

=== Example 1 ===
 Convert and convert back a string.
{{{
loadModule('jsstd');
loadModule('jsiconv');

var conv = new Iconv('UTF-8', 'ISO-8859-1');
var invConv = new Iconv('ISO-8859-1', 'UTF-8');
var converted = conv('été');
var result = invConv(converted);
print( result == 'été','\n' ); // should be true
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
print( result == 'été','\n' ); // should be true
}}}

=== Example 3 ===
 Convert a JavaScript for printing in a Windows console
{{{
loadModule('jsiconv');
loadModule('jswinshell'); // defines consoleCodepage
var conv = new Iconv(consoleCodepage, Iconv.jsUC, false, true);
print ( conv('été') );
}}}

**/
