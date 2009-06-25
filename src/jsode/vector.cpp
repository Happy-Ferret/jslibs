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
#include "vector.h"

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 2629 $
**/
BEGIN_CLASS( Vector )

DEFINE_FINALIZE() {
	
	VectorPrivate *pv = (VectorPrivate*)JL_GetPrivate(cx, obj);
	if ( !pv )
		return;
	JS_free(cx, pv);
}

DEFINE_PROPERTY( xSetter ) {
	
	VectorPrivate *pv = (VectorPrivate*)JL_GetPrivate(cx, obj);
	float f;
	JSBool ok = JsvalToFloat(cx, *vp, &f);
	pv->Set(pv->userData, 0, f);
	return ok;
}

DEFINE_PROPERTY( xGetter ) {

	VectorPrivate *pv = (VectorPrivate*)JL_GetPrivate(cx, obj);
	return FloatToJsval(cx, pv->Get(pv->userData, 0), vp);
}

DEFINE_PROPERTY( ySetter ) {
	
	VectorPrivate *pv = (VectorPrivate*)JL_GetPrivate(cx, obj);
	float f;
	JSBool ok = JsvalToFloat(cx, *vp, &f);
	pv->Set(pv->userData, 1, f);
	return ok;
}

DEFINE_PROPERTY( yGetter ) {

	VectorPrivate *pv = (VectorPrivate*)JL_GetPrivate(cx, obj);
	return FloatToJsval(cx, pv->Get(pv->userData, 1), vp);
}

DEFINE_PROPERTY( zSetter ) {
	
	VectorPrivate *pv = (VectorPrivate*)JL_GetPrivate(cx, obj);
	float f;
	JSBool ok = JsvalToFloat(cx, *vp, &f);
	pv->Set(pv->userData, 2, f);
	return ok;
}

DEFINE_PROPERTY( zGetter ) {

	VectorPrivate *pv = (VectorPrivate*)JL_GetPrivate(cx, obj);
	return FloatToJsval(cx, pv->Get(pv->userData, 2), vp);
}

DEFINE_HAS_INSTANCE() {

	*bp = !JSVAL_IS_PRIMITIVE(v) && JL_GetClass(JSVAL_TO_OBJECT(v)) == _class;
	return JS_TRUE;
}

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision: 2629 $"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)
	HAS_HAS_INSTANCE
	HAS_FINALIZE
	BEGIN_PROPERTY_SPEC
		PROPERTY( x )
		PROPERTY( y )
		PROPERTY( z )
	END_PROPERTY_SPEC
END_CLASS