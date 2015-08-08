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
//! \brief Handles camera movement on client, such as pans, orbits etc

#ifndef _MVCAMERA_H
#define _MVCAMERA_H

#include "BasicTypes.h"

//! Possible move types, such as pan, orbit etc
enum mvCameraCurrentMoveType
{
	 NONE,
	 ALTZOOM,
	 ORBIT,
	 PAN
};

//! Handles camera movement on client, such as pans, orbits etc
class mvCameraClass
{
public:
    mvCameraCurrentMoveType CurrentMove; //!< current movetype, eg PAN or ORBIT
    
    bool bCameraEnabled;  //!< if camera is enabled (otherwise, just normal avatar view)
    Vector3 CurrentCameraPos;  //!< position of camera
    Rot CurrentCameraRot;  //!< rotation of camera
        
    void InitiatePanCamera( int imousex, int imousey ); //!< Call at start of camera pan
    void InitiateOrbitCamera( int imousex, int imousey ); //!< Call at start of camera orbit
    void InitiateAltZoomCamera( int imousex, int imousey ); //!< Call at start of camera zoom
    
    void UpdatePanCamera( int imousex, int imousey ); //!< Call during camera pan
    void UpdateOrbitCamera( int imousex, int imousey );  //!< CAll during camera orbit
    void UpdateAltZoomCamera( int imousex, int imousey );  //!< Call during camera zoom

   void CameraMoveDone();    //!< call at end of move
   void CancelCamera();  //!< Stop using camera (so goes back to avatar normal view)
   
   //< call this during OpenGL render, to translate/rotate appropriately for current camera pos/rotation
   void ApplyCameraToOpenGL();
   
   mvCameraClass()
   {
   	  CurrentMove = NONE;
   	  iDragStartx = 0;
   	  iDragStarty = 0;
   	  bCameraEnabled = false;
   }
   
protected:
    int iDragStartx;
    int iDragStarty;
    
    Vector3 AltZoomCentrePos;

    //Vector3 AltZoomStartOffset;
    //Rot AltZoomStartRot;
    
    float fAltZoomStartRotateZAxis;
    float fAltZoomStartRotateYAxis;
    float fAltZoomStartZoom;

    float fAltZoomRotateZAxis;
    float fAltZoomRotateYAxis;
    float fAltZoomZoom;

    void InitiateOrbitSlashAltZoom( int imousex, int imousey, mvCameraCurrentMoveType eMoveType );
    void GetCurrentCameraFromAltZoomCamera();
};

#endif // _MVCAMERA_H
