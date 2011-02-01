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

/* exnType:
	JSEXN_NONE = -1,
	JSEXN_ERR,
	JSEXN_INTERNALERR,
	JSEXN_EVALERR,
	JSEXN_RANGEERR,
	JSEXN_REFERENCEERR,
	JSEXN_SYNTAXERR,
	JSEXN_TYPEERR,
	JSEXN_URIERR,
	JSEXN_LIMIT
*/

MSG_DEF(JLSMSG_NOT_AN_ERROR,           1000, 0, JSEXN_NONE, "<Error #0 is reserved>")
MSG_DEF(JLSMSG_OUT_OF_RANGE,           1001, 0, JSEXN_RANGEERR, "index out of range")
MSG_DEF(JLSMSG_TOO_FEW_ARGUMENTS,      1002, 1, JSEXN_RANGEERR, "not enough arguments (min is {0})")
MSG_DEF(JLSMSG_TOO_MANY_ARGUMENTS,     1003, 1, JSEXN_RANGEERR, "too many arguments (max is {0})")
MSG_DEF(JLSMSG_NEED_DEFINED,           1004, 0, JSEXN_TYPEERR, "value must be defined")
MSG_DEF(JLSMSG_EXPECT_TYPE,            1005, 1, JSEXN_TYPEERR, "{0} is expected")
MSG_DEF(JLSMSG_FAIL_TO_CONVERT_TO,     1006, 1, JSEXN_INTERNALERR, "fail to convert to {0}")
MSG_DEF(JLSMSG_INVALID_INSTANCE,       1007, 1, JSEXN_TYPEERR, "{0} instance is expected")
MSG_DEF(JLSMSG_NEED_CONSTRUCT,         1008, 0, JSEXN_TYPEERR, "object must be constructed")
MSG_DEF(JLSMSG_INVALID_RESOURCE,       1009, 0, JSEXN_INTERNALERR, "object state is invalid")
MSG_DEF(JLSMSG_INVALID_INHERITANCE,    1010, 1, JSEXN_TYPEERR, "object must be a {0}")
MSG_DEF(JLSMSG_INVALIDATED_OBJECT,     1011, 1, JSEXN_INTERNALERR, "{0} object invalidated")
MSG_DEF(JLSMSG_IMMUTABLE_OBJECT,       1012, 1, JSEXN_TYPEERR, "{0} is immutable")
MSG_DEF(JLSMSG_VALUE_OUTOFRANGE,       1013, 0, JSEXN_RANGEERR, "value out of range")
MSG_DEF(JLSMSG_PROGRAM_STOPPED,        1014, 0, JSEXN_NONE, "the program has been stopped")
MSG_DEF(JLSMSG_NOT_IMPLEMENTED,        1015, 0, JSEXN_ERR, "not implemented")
MSG_DEF(JLSMSG_OS_ERROR,               1016, 1, JSEXN_INTERNALERR, "OS: {0}")
MSG_DEF(JLSMSG_VALUE_LOSSOFDATA,       1017, 0, JSEXN_ERR, "loss of data or precision")
MSG_DEF(JLSMSG_FAIL_TO_CREATE,         1018, 1, JSEXN_INTERNALERR, "unable to create the {0}")
MSG_DEF(JLSMSG_INTERNAL_ERROR,         1019, 1, JSEXN_INTERNALERR, "internal error ({0})")
MSG_DEF(JLSMSG_INVALID_ARGUMENT_COUNT, 1020, 1, JSEXN_RANGEERR, "invalid argument count (need {0})")



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
