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
// see Camera.h for documentation

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include "Camera.h"
#include "Selection.h"
#include "WorldStorage.h"
#include "Math.h"
#include "Graphics.h"
//#include "MetaverseClient.h"

namespace MetaverseClient
{
    extern int iMyReference;

    extern mvWorldStorage World;
    extern mvGraphicsClass mvGraphics;
    extern mvSelection Selector;
}
using namespace MetaverseClient;

void mvCameraClass::InitiateOrbitSlashAltZoom( int imousex, int imousey, mvCameraCurrentMoveType eMoveType )
{
    bool bHaveStartingCameraPos = false;

    int iClickedReference = Selector.GetClickedTopLevelObjectReference( imousex, imousey );
    Object *pCentreObject = World.GetObjectByReference( iClickedReference );
    DEBUG(  "iClickedReference=" << iClickedReference << " " << pCentreObject ); // DEBUG

    Vector3 OffsetFromCameraToCentre;

    if( pCentreObject != NULL )
    {
        if( bCameraEnabled )
        {
            OffsetFromCameraToCentre = pCentreObject->pos - CurrentCameraPos;
            bHaveStartingCameraPos = true;
        }
        else
        {
            Object *pAvatarObject = World.GetObjectByReference( MetaverseClient::iMyReference );
            if( pAvatarObject != NULL )
            {
                bHaveStartingCameraPos = true;
                OffsetFromCameraToCentre =  pCentreObject->pos - pAvatarObject->pos;
            }
        }
    }

    if( bHaveStartingCameraPos )
    {
        AltZoomCentrePos = pCentreObject->pos;
        DEBUG(  AltZoomCentrePos ); // DEBUG


        fAltZoomStartZoom = VectorMag( OffsetFromCameraToCentre );

        float DistanceInXYPlane = sqrt( OffsetFromCameraToCentre.x * OffsetFromCameraToCentre.x + OffsetFromCameraToCentre.y * OffsetFromCameraToCentre.y );

        fAltZoomStartRotateZAxis = asin( OffsetFromCameraToCentre.y / DistanceInXYPlane );
        if( OffsetFromCameraToCentre.x > 0 )
        {
            fAltZoomStartRotateZAxis = PI - fAltZoomStartRotateZAxis;
        }

        fAltZoomStartRotateYAxis = asin( 1 / sqrt( DistanceInXYPlane * DistanceInXYPlane / ( OffsetFromCameraToCentre.z * OffsetFromCameraToCentre.z ) + 1 ) );

        fAltZoomRotateZAxis = fAltZoomStartRotateZAxis;
        fAltZoomRotateYAxis = fAltZoomStartRotateYAxis;
        fAltZoomZoom = fAltZoomStartZoom;

        CurrentMove = eMoveType;
        iDragStartx = imousex;
        iDragStarty = imousey;

        GetCurrentCameraFromAltZoomCamera();

        bCameraEnabled = true;
    }
}

void mvCameraClass::GetCurrentCameraFromAltZoomCamera()
{
    Rot RotationAboutZAxis;
    AxisAngle2Rot( RotationAboutZAxis, ZAXIS, PI - fAltZoomRotateZAxis );

    Rot RotationAboutYAxis;
    AxisAngle2Rot( RotationAboutYAxis, YAXIS, fAltZoomRotateYAxis );

    Rot CombinedZYRotation;
    RotMultiply( CombinedZYRotation, RotationAboutZAxis, RotationAboutYAxis );

    float DeltaZ = fAltZoomZoom * sin( fAltZoomRotateYAxis );
    CurrentCameraPos.z = AltZoomCentrePos.z + DeltaZ;

    float DistanceInXYPlane = fAltZoomZoom * cos( fAltZoomRotateYAxis );
    CurrentCameraPos.x = AltZoomCentrePos.x + DistanceInXYPlane * cos( fAltZoomRotateZAxis );
    CurrentCameraPos.y = AltZoomCentrePos.y - DistanceInXYPlane * sin( fAltZoomRotateZAxis );

    CurrentCameraRot = CombinedZYRotation;
}

void mvCameraClass::UpdatePanCamera( int imousex, int imousey )
{}

void mvCameraClass::UpdateOrbitCamera( int imousex, int imousey )
{
    if( CurrentMove == ORBIT )
    {
        fAltZoomRotateZAxis = (float)( imousex - iDragStartx ) / 20.0f / 10.0f + fAltZoomStartRotateZAxis;
        fAltZoomRotateYAxis = (float)( imousey - iDragStarty ) / 20.0f / 10.0f + fAltZoomStartRotateYAxis;
    }
    else if( CurrentMove == ALTZOOM )
    {
        fAltZoomStartRotateZAxis = fAltZoomRotateZAxis;
        fAltZoomStartRotateYAxis = fAltZoomRotateYAxis;
        fAltZoomStartZoom = fAltZoomZoom;

        iDragStartx = imousex;
        iDragStarty = imousey;
        CurrentMove = ORBIT;
    }
    else if( CurrentMove == NONE )
    {}

    GetCurrentCameraFromAltZoomCamera();
}

void mvCameraClass::UpdateAltZoomCamera( int imousex, int imousey )
{
    if( CurrentMove == ALTZOOM )
    {
        // DEBUG(  " updatealtzoom " << imousex << " " << imousey ); // DEBUG
        fAltZoomRotateZAxis = (float)( imousex - iDragStartx ) / 20.0f / 10.0f + fAltZoomStartRotateZAxis;
        fAltZoomZoom = (float)( imousey - iDragStarty ) / 20.0f  + fAltZoomStartZoom;
    }
    else if( CurrentMove == ORBIT )
    {
        fAltZoomStartRotateZAxis = fAltZoomRotateZAxis;
        fAltZoomStartRotateYAxis = fAltZoomRotateYAxis;
        fAltZoomStartZoom = fAltZoomZoom;

        iDragStartx = imousex;
        iDragStarty = imousey;
        CurrentMove = ALTZOOM;
    }
    else if( CurrentMove == NONE )
    {}

    GetCurrentCameraFromAltZoomCamera();
}

void mvCameraClass::InitiatePanCamera( int imousex, int imousey )
{
    CurrentMove = PAN;
    iDragStartx = imousex;
    iDragStarty = imousey;
}

void mvCameraClass::InitiateOrbitCamera( int imousex, int imousey )
{
    InitiateOrbitSlashAltZoom( imousex, imousey, ORBIT );
}

void mvCameraClass::InitiateAltZoomCamera( int imousex, int imousey )
{
    InitiateOrbitSlashAltZoom( imousex, imousey, ALTZOOM );
}

void mvCameraClass::CancelCamera()
{
    bCameraEnabled = false;
}

void mvCameraClass::CameraMoveDone()
{
    CurrentMove = NONE;
}

void mvCameraClass::ApplyCameraToOpenGL()
{
    if( bCameraEnabled )
    {
        Rot InverseCameraRot;
        InverseRot( InverseCameraRot, CurrentCameraRot );
        mvGraphics.RotateToRot( InverseCameraRot );

        glTranslatef( - CurrentCameraPos.x, - CurrentCameraPos.y, - CurrentCameraPos.z  );
    }
}

