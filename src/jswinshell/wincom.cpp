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

#include <Objbase.h>

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 2555 $
**/
BEGIN_CLASS( WinCOM )

/**doc
$TOC_MEMBER $INAME
 $INAME()
**/
DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();
	JL_S_ASSERT_ARG( 1 );

//	const char *name;
//	JL_CHK( JsvalToString(cx, JL_ARG(1), &name) );

//	CLSID clsid;
//	GetClassFile(name, &clsid);

//	
	
	LPOLESTR name = (LPOLESTR)JS_GetStringChars(JS_ValueToString(cx, JL_ARG(1)));

	CLSID clsid;

	HRESULT res;
	if ( name[0] == L'{' )
		res = CLSIDFromString(name, &clsid);
	else
		res = CLSIDFromProgID(name, &clsid);
	if ( res != S_OK );

	IUnknown *unk = NULL;
	res = GetActiveObject(clsid, NULL, &unk);
	if ( res != S_OK )
		res = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER, IID_IUnknown, (void**)&unk);

	//GetActiveObject(

	//	CoCreateInstance(	

	return JS_TRUE;
	JL_BAD;
}

DEFINE_FINALIZE() {
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
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
	END_PROPERTY_SPEC

END_CLASS
