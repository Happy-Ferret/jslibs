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
#include <jsdate.h>

#include "com.h"


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 2555 $
**/
BEGIN_CLASS( ComObject )

/**doc
$TOC_MEMBER $INAME
 $INAME()
**/
DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();
	JL_S_ASSERT_ARG( 1 );

	HRESULT res;
	
	LPOLESTR name = (LPOLESTR)JS_GetStringChars(JS_ValueToString(cx, JL_ARG(1)));

	CLSID clsid;
	res = name[0] == L'{' ? CLSIDFromString(name, &clsid) : CLSIDFromProgID(name, &clsid);
	if ( FAILED(res) )
		JL_CHK( WinThrowError(cx, res) );

	IUnknown *punk = NULL;
	res = GetActiveObject(clsid, NULL, &punk);
	if ( FAILED(res) ) {
		
		res = CoCreateInstance(clsid, NULL, CLSCTX_SERVER | CLSCTX_INPROC_HANDLER, IID_IUnknown, (void FAR* FAR*)&punk);
		if ( FAILED(res) )
			JL_CHK( WinThrowError(cx, res) );
	}

	IDispatch FAR* pdisp = (IDispatch FAR*)NULL;
	res = punk->QueryInterface(IID_IDispatch, (void FAR* FAR*)&pdisp);
	if ( FAILED(res) )
		JL_CHK( WinThrowError(cx, res) );

	JL_CHK( NewComDispatch(cx, pdisp, rval) );

	return JS_TRUE;
	JL_BAD;
}

DEFINE_INIT() {

	HRESULT res;
	res = CoInitialize(NULL);
	return JS_TRUE;
}

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision: 2555 $"))
	HAS_INIT
//	HAS_PRIVATE
	HAS_CONSTRUCTOR

	BEGIN_FUNCTION_SPEC
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
	END_PROPERTY_SPEC

END_CLASS
