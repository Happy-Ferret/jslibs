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

#include <jlhelper.h>
#include <jlclass.h>
//#include <jsvalserializer.h>

#include <jstypedarray.h>

#include <pool.h>


#define _USE_MATH_DEFINES
#include "math.h"

#include "matrix44.h"
#include "vector3.h"


#include "jstransformation.h"




#ifdef _MACOSX // MacosX platform specific
	#include <AGL/agl.h>
	#include <OpenGL/gl.h>
#endif

//#define GL_GLEXT_PROTOTYPES

#include <gl/gl.h>
#include "glext.h" // download at http://www.opengl.org/registry/api/glext.h (http://www.opengl.org/registry/#headers)

#include "oglError.h"

