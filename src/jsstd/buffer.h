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

DECLARE_CLASS( Buffer )

#define SLOT_BUFFER_LENGTH 0

JSBool WriteRawData( JSContext *cx, JSObject *obj, size_t amount, char *str );

JSBool ReadRawAmount( JSContext *cx, JSObject *obj, size_t *amount, char *str );
JSBool ReadAmount( JSContext *cx, JSObject *obj, size_t amount, jsval *rval );

JSBool UnReadRawChunk( JSContext *cx, JSObject *obj, char *data, size_t length );