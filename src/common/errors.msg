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

MSG_DEF(JSSMSG_NOT_AN_ERROR,             0, 0, JSEXN_NONE, "<Error #0 is reserved>")
MSG_DEF(JSSMSG_OUT_OF_RANGE,             1, 0, JSEXN_RANGEERR, "index out of range")

/*
MSG_DEF(JSSMSG_NOT_AN_ERROR,             0, 0, JSEXN_NONE, "<Error #0 is reserved>")
MSG_DEF(JSSMSG_CANT_OPEN,                1, 2, JSEXN_NONE, "can't open {0}: {1}") 
MSG_DEF(JSSMSG_TRAP_USAGE,               2, 0, JSEXN_NONE, "usage: trap [fun] [pc] expr") 
MSG_DEF(JSSMSG_LINE2PC_USAGE,            3, 0, JSEXN_NONE, "usage: line2pc [fun] line") 
MSG_DEF(JSSMSG_FILE_SCRIPTS_ONLY,        4, 0, JSEXN_NONE, "only works on JS scripts read from files") 
MSG_DEF(JSSMSG_UNEXPECTED_EOF,           5, 1, JSEXN_NONE, "unexpected EOF in {0}") 
MSG_DEF(JSSMSG_DOEXP_USAGE,              6, 0, JSEXN_NONE, "usage: doexp obj id") 
*/

/* errors to manage:

- Internal error: <what>.
- Missing <something> (an identified thing is missing to continue properly)
- Failed to <do something> ( the underlying native or spidermonkey API failed )
- Unable to <do something>, <why> ( logical error, misuse of jslibs API, Object cannot be constructed )
- Invalid operation: <what> ( API misuse )

- Invalid or uninitialized object (Method called on the wrong object.)
- Invalid argument count. ( misuse of jslibs API )
- Invalid argument value. ( misuse of jslibs API )
- Invalid data type <type> ( invalid type, Array expected )
- conversion failed / failed to convert to <type> ( Unable to convert to boolean )
- Value out of range: <info?>
- Limit reach: <what>

- Object cannot be constructed.
- Construction form is required.

- Not implemented yet.
- Available in Debug mode only.

*/
