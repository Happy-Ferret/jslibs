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

#ifndef _JSNATIVEINTERFACE_H_
#define _JSNATIVEINTERFACE_H_

#define NI_READ_RESOURCE "_NI_READ_RESOURCE"
typedef bool (*NIResourceRead)( void *pv, unsigned char *buf, unsigned int *amount );

#define NI_READ_MATRIX44 "_NI_READ_MATRIX"
// **pm
//   in: a valid float[16]
//  out: pointer provided as input OR another pointer to float
typedef int (*NIMatrix44Read)(void *pv, float **pm); // **pm allows NIMatrix44Read to return its own data pointer ( should be const )

#endif // _JSNATIVEINTERFACE_H_
