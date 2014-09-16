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

DEFINE_FINALIZE() {

//	if ( obj == jl::Host::getJLHost(fop->runtime()).getCachedProto(JL_THIS_CLASS_NAME) ) // we are finilizing the prototype ?
//		return;

	IEnumVARIANT *ienumv = (IEnumVARIANT*)JL_GetPrivateFromFinalize(obj);
	if ( !ienumv )
		return;
	ienumv->Release();
}


DEFINE_FUNCTION( next ) {

	JL_IGNORE(argc);

	VARIANT *result = NULL;

	JL_DEFINE_ARGS;
	JL_ASSERT_THIS_INSTANCE();

	IEnumVARIANT *ienumv = (IEnumVARIANT*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(ienumv);

	result = (VARIANT*)JS_malloc(cx, sizeof(VARIANT));
	VariantInit(result);
	
	HRESULT hr;
	hr = ienumv->Next(1, result, NULL);

	if ( hr != S_OK ) // The number of elements returned is less than 1.
		JL_CHK( JS_ThrowStopIteration(cx) );

	JL_CHK( VariantToJsval(cx, result, args.rval()) ); // loose variant ownership
	return true;

bad:
	if ( result ) {

		VariantClear(result);
		JS_free(cx, result);
	}
	return false;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 2555 $"))
	HAS_PRIVATE
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( next )
	END_FUNCTION_SPEC

END_CLASS


bool NewComEnum( JSContext *cx, IEnumVARIANT *ienumv, OUT JS::MutableHandleValue rval ) {

	JS::RootedObject varObj(cx, jl::newObjectWithGivenProto(cx, JL_CLASS(ComEnum), JL_CLASS_PROTOTYPE(cx, ComEnum)));
	JL_ASSERT_ALLOC( varObj );
	rval.setObject(*varObj);
	JL_SetPrivate(varObj, ienumv);
	ienumv->AddRef();
	return true;
	JL_BAD;
}
