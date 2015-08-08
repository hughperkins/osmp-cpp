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
//! \brief ThreeDeeEditing.h handles 3d editing within the renderer
//!
//£¡ThreeDeeEditing.h handles 3d editing within the renderer
//£¡
//! It supports: rot, translation and scale
//! uses Math.h for quaternion maths
//!
//! When someone starts a mouse drag, the current x and y values are stored
//! and used to calculate the apropriate rot/pos/scale as the passed in x,y coordinates change
//! when someone changes from rot to pos edit (for example), the current rot is written to the object, and the current
//! x, y is stored.
//!
//! Many of the functions from this module have now been outsourced to editing3dpos, editing3drot and editing3dscale
//! to keep the modules to a manageable size

#ifndef _THREEDEEEDITING_H
#define _THREEDEEEDITING_H

#include "Math.h"
#include "BasicTypes.h"

//! Current edit type, eg none, position, constrained position (poshandle), rot, scale
enum mvEditing3DCurrentEditType
{
	 EDITTYPE_NONE,
	 EDITTYPE_POS,
	 EDITTYPE_SCALE,
	 EDITTYPE_ROT,
	 EDITTYPE_POSHANDLE,
	 EDITTYPE_SCALEHANDLE,
	 EDITTYPE_ROTHANDLE
};

//! current edit bar type to show, ie none, pos, scale or rot
enum mvEditing3DCurrentEditBarType
{
	 EDITBAR_NONE,
	 EDITBAR_POS,
	 EDITBAR_SCALE,
	 EDITBAR_ROT
};

//! Did the user grab the x +ve axis, x -ve axis etc?
enum mvEditing3DAxisType
{
    AXIS_X_POS,
    AXIS_X_NEG,
    AXIS_Y_POS,
    AXIS_Y_NEG,
    AXIS_Z_POS,
    AXIS_Z_NEG
};		

#include "Editing3DScale.h"
#include "Editing3DPos.h"
#include "Editing3DRot.h"

//! 3d editing namespace; for anything to do with editing prims in 3d on the client
class Editing3DClass
{
public:
   Editing3DClass()
   {
      CurrentEditType = EDITTYPE_NONE;
      bAxisModifierOn = false;  //!< local or global axes?
      
      iDragStartX = 0;  //!< mouse x at start of edit drag
      iDragStartY = 0;  //!< mouse y at start of edit drag
      
      bShowEditBars = false;  //!< show edit bars?  This controls what happens when one calls DrawEditBarsToOpenGL()
   }
   
  void InitiateTranslateEdit( int mousex, int mousey );  //!< Called at the start of a translate/position edit mouse drag
  void InitiateScaleEdit( int mousex, int mousey );  //!< Called at the start of a scale edit mouse drag
  void InitiateRotateEdit( int mousex, int mousey );  //!< Called at the start of a rotate edit mouse drag

 //! Call during translate mouse drag, to update object attributes to new pos
  void UpdateTranslateEdit( bool bAltAxes, int mousex, int mousey );
  
 //! Call during scale mouse drag, to update object attributes to new scale
  void UpdateScaleEdit( bool bAltAxes, int mousex, int mousey );
  
 //! Call during rotate mouse drag, to update object attributes to new rotate
  void UpdateRotateEdit( bool bAltAxes, int mousex, int mousey );
  
//	void InteractiveRotEdit( int x, int y );
//	void InteractivePosEdit( int x, int y );
//	void InteractiveScaleEdit( int x, int y );
//	void InteractiveEdit( int x, int y );

	void EditDone(); //!< call at end of edit drag

	void DrawEditBarsToOpenGL();  //!< Call this during opengl drawworld call to render edit bars (if any)

	void ShowEditPosBars();  //!< Show position editing bars (so user can drag them)
	void ShowEditRotBars();  //!< Show rotate editing bars (so user can drag them)
	void ShowEditScaleBars();   //!< show scale editing bars (so user can drag them)
	void HideEditBars();   //!< Hide editing bars

  void EditingPreliminaries();  //!< Called at the start of each editing drag  (possibly during drag too?)
  
  //! Get position of object on screen, given passed in observer position/rotation in world, and target position in world
  Vector3 GetScreenPos( const Vector3 ObserverPos, const Rot ObserverRot, const Vector3 TargetPos3D );

   int iDragStartX;  //!< mouse x at start of edit drag
   int iDragStartY;  //!< mouse y at start of edit drag
   
   mvEditing3DCurrentEditBarType EditBarType; //!< current edit / editbar type
   mvEditing3DAxisType CurrentAxis; //!< current constraint axis, for constrained edits
	mvEditing3DCurrentEditType CurrentEditType;  //!< current edit type, ie scale, pos, or rotate

   Vector3 StartPos;  //!< position at start of edit drag
   Vector3 StartScale; //!< scale at start of edit drag
   Rot StartRot; //!< rotation at start of edit drag
   
protected:
   //	bool bEditing = false;

   bool bAxisModifierOn;  //!< local or global axes?
   
    //Rot rAvatar;
   //Rot rInverseAvatar;
      
   bool bShowEditBars;  //!< show edit bars?  This controls what happens when one calls DrawEditBarsToOpenGL()
   
   Editing3DScaleClass Editing3DScale;
   Editing3DPosClass Editing3DPos;
   Editing3DRotClass Editing3DRot;
};

#endif // _THREEDEEEDITING_H
