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

#include "com.h"


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 2555 $
**/
BEGIN_CLASS( ComEnum )

JSBool NewComEnum( JSContext *cx, IEnumVARIANT *ienumv, jsval *rval ) {

	JSObject *varObj = JS_NewObject(cx, _class, NULL, NULL);
	*rval = OBJECT_TO_JSVAL( varObj );
	JL_SetPrivate(cx, varObj, ienumv);
	ienumv->AddRef();
	return JS_TRUE;
}


DEFINE_FINALIZE() {

	if ( obj == *_prototype )
		return;
	IEnumVARIANT *ienumv = (IEnumVARIANT*)JL_GetPrivate(cx, obj);
	ienumv->Release();
}


DEFINE_FUNCTION_FAST( next ) {

	HRESULT hr;

	VARIANT *result = NULL;

	IEnumVARIANT *ienumv = (IEnumVARIANT*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE(ienumv);

	result = (VARIANT*)JS_malloc(cx, sizeof(VARIANT));
	VariantInit(result);
	
	hr = ienumv->Next(1, result, NULL);

	if ( hr != S_OK ) // The number of elements returned is less than 1.
		JL_CHK( JS_ThrowStopIteration(cx) );

	JL_CHK( VariantToJsval(cx, result, JL_FRVAL) ); // loose variant ownership
	return JS_TRUE;

bad:
	if ( result ) {

		VariantClear(result);
		JS_free(cx, result);
	}
	return JS_FALSE;
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision: 2555 $"))
	HAS_PRIVATE
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST( next )
	END_FUNCTION_SPEC

END_CLASS