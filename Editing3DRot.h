// Copyright Hugh Perkins 2004
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURVector3E. See the GNU General Public License for
//  more details.
//
// You should have received a copy of the GNU General Public License along
// with this program in the file licence.txt; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-
// 1307 USA
// You can find the licence also on the web at:
// http://www.opensource.org/licenses/gpl-license.php
//

//! \file
//! \brief Manages 3d Rotation editing

#ifndef _EDITING3DROT_H
#define _EDITING3DROT_H

#include "Editing3D.h"
#include "BasicTypes.h"

//! \brief Manages 3d Rotation editing
//! 
//! What this module can do:
//! - Draw Rotation Edit Handles:
//!    - Call DrawRotEditHandles, passing in the object's scale
//!
//! - manage a constrained edit:
//!   - call InitiateHandleRotEdit, passing in the current mouse x and y, and the axis it is constrained to
//!   - when the mouse moves, call InteractiveHandleRotEdit, passing in the new mouse coordinates, and the constraint axis
//!   - the module will update the Rot of the selected object
//!
//! - manage a free edit:
//!   - call InitiateFreeRotEdit, passing in the current mouse x and y
//!   - when the mouse moves, call InteractiveHandleRotEdit, passing in the new mouse coordinates
//!   - the module will update the Rot of the selected object
class Editing3DRotClass
{
public:
   //! Render editing handles; pass in object's current scale (for handle sizing)
   void DrawEditHandles( const Vector3 &ScaleToUse );
   
   //! Initiate a free rotate edit; pass in mouse coordinates
   void InitiateFreeEdit( int mousex, int mousey );
   void InteractiveFreeEdit( int x, int y );
   
   void InitiateHandleEdit( int mousex, int mousey, mvEditing3DAxisType Axis );	
   void InteractiveHandleEdit( mvEditing3DAxisType Axis, int mousex, int mousey );
   
   void DrawSelectionHandle();
protected:
   float fStartRotationAngle;
   
   bool GetRotationAngleForObjectAndMouse( float &fRotationAngle, Vector3 ObjectVector3, Rot ObjectRot, mvEditing3DAxisType Axis, int mousex, int mousey );
};

#endif // _EDITING3DROT_H
