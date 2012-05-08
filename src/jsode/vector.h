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


#pragma once

DECLARE_CLASS( Vector )

typedef void (*VectorSet_t)(void *userData, int index, float value);
typedef float (*VectorGet_t)(void *userData, int index);

struct VectorPrivate {
	void *userData;
	VectorSet_t Set;
	VectorGet_t Get;
};

ALWAYS_INLINE JSBool CreateVector( JSContext *cx, JSObject *parent, void *userData, VectorSet_t set, VectorGet_t get, jsval *val ) {
	
	JSObject *obj = JL_NewObjectWithGivenProto(cx, JL_CLASS(Vector), JL_CLASS_PROTOTYPE(cx, Vector), NULL);
	JL_CHK(obj);
	JL_CHK( JL_SetReservedSlot( obj, 0, OBJECT_TO_JSVAL(parent)) );
	VectorPrivate *pv = (VectorPrivate*)JS_malloc(cx, sizeof(VectorPrivate));
	JL_CHK(pv);
	pv->userData = userData;
	pv->Set = set;
	pv->Get = get;
	JL_SetPrivate( obj, pv);
	return JS_TRUE;
	JL_BAD;
}
