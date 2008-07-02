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
#include "data.h"

//#include "../common/jsNativeInterface.h"
#include "../common/queue.h"

BEGIN_CLASS( Data )

DEFINE_CONSTRUCTOR() {
		
	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();

	J_CHK( JS_SetPrototype(cx, obj, NULL) ); // this creates an empty object ( without __proto__, __parent__, toString, ... )
	return JS_TRUE;
}

CONFIGURE_CLASS
	HAS_CONSTRUCTOR
END_CLASS
