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

#include "stdafx.h"

#include <Objbase.h>
#include <ocidl.h>

#include "error.h"


DECLARE_CLASS( ComEnum )
DECLARE_CLASS( ComVariant )
DECLARE_CLASS( ComDispatch )
DECLARE_CLASS( ComObject )


bool JsvalToVariant( JSContext *cx, IN JS::HandleValue value, OUT VARIANT *variant );
bool VariantToJsval( JSContext *cx, IN VARIANT *variant, OUT JS::MutableHandleValue rval );

bool NewComVariant( JSContext *cx, VARIANT *variant, OUT JS::MutableHandleValue rval );
bool NewComVariantCopy( JSContext *cx, VARIANT *variant, OUT JS::MutableHandleValue rval );
bool NewComDispatch( JSContext *cx, IDispatch *pdisp, OUT JS::MutableHandleValue rval );
bool NewComEnum( JSContext *cx, IEnumVARIANT *enumv, OUT JS::MutableHandleValue rval );
