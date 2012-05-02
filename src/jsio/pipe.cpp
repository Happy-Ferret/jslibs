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


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
 You cannot construct this class.$LF
 The aim of this class is to provide a way to access Descriptor properties and methods from an existing pipe.
 $H exemple
 {{{
 var p = new Process( 'c:\\windows\\System32\\cmd.exe', ['/c', 'dir', 'c:'] );
 p.wait();
 print( p.stdout.read() );
 }}}
**/
BEGIN_CLASS( Pipe )

DEFINE_FINALIZE() {

	FinalizeDescriptor(cx, obj); // defined in descriptor.cpp
}

CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS( 2 ) // SLOT_JSIO_DESCRIPTOR_IMPORTED, SLOT_JSIO_DESCRIPTOR_TIMEOUT
	IS_UNCONSTRUCTIBLE

	HAS_PROTOTYPE( Descriptor )

	HAS_FINALIZE

END_CLASS
