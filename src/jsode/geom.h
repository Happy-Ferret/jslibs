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

#define SLOT_GEOM_SURFACEPARAMETER 0

DECLARE_CLASS( Geom )
DECLARE_CLASS( GeomSphere )
DECLARE_CLASS( GeomBox )
DECLARE_CLASS( GeomPlane )
DECLARE_CLASS( GeomCapsule )
DECLARE_CLASS( GeomRay )
DECLARE_CLASS( GeomTrimesh )

DECLARE_CLASS( SurfaceParameters )

JSBool SetupReadMatrix(JSContext *cx, JSObject *obj);