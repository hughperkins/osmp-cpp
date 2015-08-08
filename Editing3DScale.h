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
//! \brief Manages 3d Scale editing
//! 
//! Manages 3d scale editing
//! 
//! What this module can do:
//! - Draw Scale Edit Handles:
//!    - Call DrawScaleEditHandles, passing in the object's scale
//!
//! - manage a constrained edit:
//!   - call InitiateHandleScaleEdit, passing in the current mouse x and y, and the axis it is constrained to
//!   - when the mouse moves, call InteractiveHandleScaleEdit, passing in the new mouse coordinates, and the constraint axis
//!   - the module will update the scale of the selected object
//!
//! - manage a free edit:
//!   - call InitiateFreeScaleEdit, passing in the current mouse x and y
//!   - when the mouse moves, call InteractiveHandleScaleEdit, passing in the new mouse coordinates
//!   - the module will update the scale of the selected object

#ifndef _EDITING3DSCALE_H
#define _EDITING3DSCALE_H

#include "Editing3D.h"
#include "BasicTypes.h"

//! \brief Manages 3d scale editing
//!
//! Manages 3d scale editing
//! 
//! What you can do with these functions:
//! - Draw Scale Edit Handles:
//!    - Call DrawScaleEditHandles, passing in the object's scale
//!
//! - manage a constrained edit:
//!   - call InitiateHandleScaleEdit, passing in the current mouse x and y, and the axis it is constrained to
//!   - when the mouse moves, call InteractiveHandleScaleEdit, passing in the new mouse coordinates, and the constraint axis
//!   - the module will update the scale of the selected object
//!
//! - manage a free edit:
//!   - call InitiateFreeScaleEdit, passing in the current mouse x and y
//!   - when the mouse moves, call InteractiveHandleScaleEdit, passing in the new mouse coordinates
//!   - the module will update the scale of the selected object
class Editing3DScaleClass
{
public:
	//! Render editing handles; pass in object's current scale (for handle sizing)
	void DrawEditHandles( const Vector3 &ScaleToUse );
	
	
	//! Initiate a contrained scale edit; pass in mouse coordinates, and constraint axis
	void InitiateHandleEdit( int mousex, int mousey, enum mvEditing3DAxisType Axis );
	//! Update the mouse coordinates for a constrained scale edit mouse drag; passing in new mouse coordinates; and constraint axis
  void InteractiveHandleEdit( mvEditing3DAxisType Axis, int mousex, int mousey );
		
		
	//! Initiate a free scale edit; pass in mouse coordinates
  void InitiateFreeEdit(int mousex, int mousey );
	//! Update the mouse coordinates for a free scale edit mouse drag; passing in new mouse coordinates
	void InteractiveFreeEdit( int x, int y );
protected:
};

#endif // _EDITING3DSCALE_H
