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

#include "Editing3D.h"
#include "Math.h"
#include "WorldStorage.h"
//#include "keyandmouse.h"
#include "Selection.h"
#include "Constants.h"
//#include "Animation.h"
#include "PlayerMovement.h"
#include "RendererImpl.h"
#include "Graphics.h"
#include "Camera.h"
#include "Editing3DPos.h"
#include "MetaverseClient.h"

using namespace MetaverseClient;

namespace MetaverseClient
{
    extern mvWorldStorage World;
    extern int iMyReference;
    extern mvSelection Selector;
    //extern Animation animator;
    extern PlayerMovementClass PlayerMovement;
    extern mvGraphicsClass mvGraphics;
    extern mvCameraClass Camera;
    extern Editing3DClass Editing3D;
}

//! Manages 3d Position editing
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



void Editing3DRotClass::InteractiveFreeEdit( int x, int y )
{
    Vector3 OurPos;
    Rot OurRot;

    if( Camera.bCameraEnabled )
    {
        OurPos = Camera.CurrentCameraPos;
        OurRot = Camera.CurrentCameraRot;
    }
    else
    {
        Object *p_Object = World.GetObjectByReference( MetaverseClient::iMyReference );
        if( p_Object != NULL )
        {
            OurPos = p_Object->pos;
            OurRot = p_Object->rot;
        }
        else
        {
            return;
        }
    }

    Rot rInverseOurRot;
    InverseRot( rInverseOurRot, OurRot );

    float HalfWinWidth = RendererImpl::GetWindowWidth() / 2;
    float HalfWinHeight = RendererImpl::GetWindowHeight() / 2;

    float zRoll =  (x - Editing3D.iDragStartX) / HalfWinWidth;
    float yRoll = - (y - Editing3D.iDragStartY) / HalfWinHeight;
    float amount = sqrt( (float)((Editing3D.iDragStartX-x)*(Editing3D.iDragStartX-x) + (Editing3D.iDragStartY-y)*(Editing3D.iDragStartY-y) )) * mvConstants::PI2 / HalfWinHeight;


    Rot ArcBallRot;

    Vector3 ArcBallAxis;
    ArcBallAxis.x = 0;
    ArcBallAxis.y = yRoll;
    ArcBallAxis.z = zRoll;
    AxisAngle2Rot( ArcBallRot, ArcBallAxis, amount );

    Rot rTransposedToAv, rRotated, rNewRot;

    RotMultiply( rTransposedToAv, rInverseOurRot, Editing3D.StartRot );
    RotMultiply( rRotated, ArcBallRot, rTransposedToAv );
    RotMultiply( rNewRot, OurRot, rRotated );

    SelectionIteratorTypedef iterator;
    iterator = Selector.SelectedObjects.begin();
    int iSelectedReference = iterator->second.iReference;
    int iSelectedArrayNum = World.GetArrayNumForObjectReference( iSelectedReference );
    World.GetObject( iSelectedArrayNum )->rot = rNewRot;
}

bool Editing3DRotClass::GetRotationAngleForObjectAndMouse( float &fRotationAngle, Vector3 ObjectVector3, Rot ObjectRot, mvEditing3DAxisType Axis, int mousex, int mousey )
{
    bool bRotAngleGot = false;

    Vector3 OurPos;
    Rot OurRot;
    Rot rInverseOurRot;

    if( Camera.bCameraEnabled )
    {
        OurPos = Camera.CurrentCameraPos;
        OurRot = Camera.CurrentCameraRot;
    }
    else
    {
        Object *p_Object = World.GetObjectByReference( MetaverseClient::iMyReference );
        if( p_Object != NULL )
        {
            OurPos = p_Object->pos;
            OurRot = p_Object->rot;
        }
        else
        {
            return false;
        }
    }
    InverseRot( rInverseOurRot, OurRot );

    float fDistanceFromUsToObject = VectorMag( ObjectVector3 - OurPos );

    float fScalingFromPosToScreen = mvGraphics.GetScalingFrom3DToScreen( fDistanceFromUsToObject );

    Vector3 ScreenObjectPos = Editing3D.GetScreenPos( OurPos, OurRot, ObjectVector3 );
    Vector3 ScreenMousePos( 0, RendererImpl::GetWindowWidth() - mousex, RendererImpl::GetWindowHeight() - mousey );

    // mousepoint is a point on the mouseray into the screen, with x = p_Object->pos.x
    Vector3 ScreenVectorObjectToMousePoint = ScreenMousePos - ScreenObjectPos;
    Vector3 VectorObjectToMousePointObserverAxes = ScreenVectorObjectToMousePoint * ( 1.0f / fScalingFromPosToScreen );

    //DEBUG(  " screenobjectpos: " << ScreenObjectPos << " screenmousepos: " << ScreenMousePos << " objecttomouse: " << ScreenVectorObjectToMouse ); // DEBUG

    Vector3 RotationAxisObjectAxes;
    switch( Axis )
    {
        case AXIS_X_POS:
        RotationAxisObjectAxes = XAXIS;
        break;

        case AXIS_Y_POS:
        RotationAxisObjectAxes = YAXIS;
        break;

        case AXIS_Z_POS:
        RotationAxisObjectAxes = ZAXIS;
        break;
    }

    Rot rInverseObjectRot;
    InverseRot( rInverseObjectRot, ObjectRot );

    Vector3 RotationAxisWorldAxes;
    RotationAxisWorldAxes = RotationAxisObjectAxes * rInverseObjectRot;
    // MultiplyVectorByRot( RotationAxisWorldAxes, rInverseObjectRot, RotationAxisObjectAxes );
    //    DEBUG(  " RotationAxisWorldAxes " << RotationAxisWorldAxes ); // DEBUG

    Vector3 RotationAxisObserverAxes;
    RotationAxisObserverAxes = RotationAxisWorldAxes * OurRot;
    // MultiplyVectorByRot( RotationAxisObserverAxes, OurRot, RotationAxisWorldAxes );
    VectorNormalize( RotationAxisObserverAxes );
    //Vector3 RotationAxisScreenCoords = RotationAxisObserverAxes * fScalingFromPosToScreen;  // guess this step is a little rendundant :-O
    //VectorNormalize( RotationAxisScreenCoords );

    float DistanceOfRotationPlaneFromOrigin = 0; // Lets move right up to the object
    // we're going to imagine a ray from the MousePoint going down the XAXIS, away from us
    // we'll intersect this ray with the rotation plane to get the point on the rotation plane
    // where we can consider the mouse to be.
    float fVectorDotRotationAxisObserverAxesWithXAXIS = VectorDot( RotationAxisObserverAxes, XAXIS );
    if( fabs( fVectorDotRotationAxisObserverAxesWithXAXIS ) > 0.0005 )
    {
        float fDistanceFromMousePointToRotationPlane = ( DistanceOfRotationPlaneFromOrigin - VectorDot( RotationAxisObserverAxes, VectorObjectToMousePointObserverAxes ) )
                / fVectorDotRotationAxisObserverAxesWithXAXIS;
        //  DEBUG(  " fDistanceFromMousePointToRotationPlane " << fDistanceFromMousePointToRotationPlane ); // DEBUG

        Vector3 VectorMouseClickOnRotationPlaneObserverAxes( fDistanceFromMousePointToRotationPlane, VectorObjectToMousePointObserverAxes.y, VectorObjectToMousePointObserverAxes.z );
        //    DEBUG(  " VectorMouseClickOnRotationPlaneObserverAxes " << VectorMouseClickOnRotationPlaneObserverAxes ); // DEBUG
        // We'll rotate this vector into object axes

        Vector3 VectorMouseClickOnRotationPlaneWorldAxes;
        VectorMouseClickOnRotationPlaneWorldAxes = VectorMouseClickOnRotationPlaneObserverAxes * rInverseOurRot;
        // MultiplyVectorByRot( VectorMouseClickOnRotationPlaneWorldAxes, rInverseOurRot, VectorMouseClickOnRotationPlaneObserverAxes );

        Vector3 VectorMouseClickOnRotationPlaneObjectAxes;
        VectorMouseClickOnRotationPlaneObjectAxes = VectorMouseClickOnRotationPlaneWorldAxes * ObjectRot;
        // MultiplyVectorByRot( VectorMouseClickOnRotationPlaneObjectAxes, ObjectRot, VectorMouseClickOnRotationPlaneWorldAxes );

        //     DEBUG(  " VectorMouseClickOnRotationPlaneObjectAxes " << VectorMouseClickOnRotationPlaneObjectAxes ); // DEBUG

        // now we work out rotation angle
        //    float fRotAngle;
        float fDistanceOfPointFromOrigin;
        switch( Axis )
        {
            case AXIS_X_POS:
            fDistanceOfPointFromOrigin = sqrt( VectorMouseClickOnRotationPlaneObjectAxes.z * VectorMouseClickOnRotationPlaneObjectAxes.z
                                               + VectorMouseClickOnRotationPlaneObjectAxes.y * VectorMouseClickOnRotationPlaneObjectAxes.y );
            //         DEBUG(  "Z axis distnace of point from origin: " << fDistanceOfPointFromOrigin ); // DEBUG

            if( fabs( fDistanceOfPointFromOrigin ) > 0.0005 )
            {
                fRotationAngle = - asin( VectorMouseClickOnRotationPlaneObjectAxes.y / fDistanceOfPointFromOrigin );
                if( VectorMouseClickOnRotationPlaneObjectAxes.z < 0 )
                {
                    fRotationAngle = PI - fRotationAngle;
                }
                //             DEBUG(  "************RotANGLE: " << fRotationAngle ); // DEBUG
                bRotAngleGot = true;
            }
            break;

            case AXIS_Y_POS:
            fDistanceOfPointFromOrigin = sqrt( VectorMouseClickOnRotationPlaneObjectAxes.z * VectorMouseClickOnRotationPlaneObjectAxes.z
                                               + VectorMouseClickOnRotationPlaneObjectAxes.x * VectorMouseClickOnRotationPlaneObjectAxes.x );
            //         DEBUG(  "Z axis distnace of point from origin: " << fDistanceOfPointFromOrigin ); // DEBUG

            if( fabs( fDistanceOfPointFromOrigin ) > 0.0005 )
            {
                fRotationAngle = asin( VectorMouseClickOnRotationPlaneObjectAxes.x / fDistanceOfPointFromOrigin );
                if( VectorMouseClickOnRotationPlaneObjectAxes.z < 0 )
                {
                    fRotationAngle = PI - fRotationAngle;
                }
                //             DEBUG(  "************RotANGLE: " << fRotationAngle ); // DEBUG
                bRotAngleGot = true;
            }
            break;

            case AXIS_Z_POS:
            fDistanceOfPointFromOrigin = sqrt( VectorMouseClickOnRotationPlaneObjectAxes.x * VectorMouseClickOnRotationPlaneObjectAxes.x
                                               + VectorMouseClickOnRotationPlaneObjectAxes.y * VectorMouseClickOnRotationPlaneObjectAxes.y );
            //         DEBUG(  "Z axis distnace of point from origin: " << fDistanceOfPointFromOrigin ); // DEBUG

            if( fabs( fDistanceOfPointFromOrigin ) > 0.0005 )
            {
                fRotationAngle = asin( VectorMouseClickOnRotationPlaneObjectAxes.y / fDistanceOfPointFromOrigin );
                if( VectorMouseClickOnRotationPlaneObjectAxes.x < 0 )
                {
                    fRotationAngle = PI - fRotationAngle;
                }
                //             DEBUG(  "************RotANGLE: " << fRotationAngle ); // DEBUG
                bRotAngleGot = true;
            }
            break;
        }
    }

    if( bRotAngleGot )
    {
        //fRotationAngle = fRotAngle;
        return true;
    }
    else
    {
        return false;
    }
}

// Maths description:
//
// take mousecoordinates, convert to 3d vector in screen coordinates
// take object coordinates, convert to screen coordinates using OpenGL Feedback mode
// now we have a vector, in screen coordinates, from the object to the mouse
// convert this vector into OpenGL observer coordinates by dividing by the scaling factor from OpenGl->screen (see pos handles maths desscdription)
//
// now we imagine a line/ray running along our x-axis, in observer coordinates, through the point we we imagined the mouse clicked
// we put the origin at the position of the object
// we take the object axis according tot he handle we clicked on (XAXIS, YAXIS or ZAXIS) and multiple it by InverseObjectStartRot to convert from Object coordinates into world coordinates
// then we multiple by ObserverRot to convert into observer coordinates/axes
// we normalize, which gives us the normal of a plane running through the origin perpendicular to the rotation axis, in observer coordinates
// we use the maths from http://nehe.gamedev.net collision tutorial to intersect our mouseclick ray with this plane
//
// now we have a vector from the object to a point on the rotation plane
// we multiply by ObjectStartRot to move into object axes/coordinates
// so the vector will now be in the x-y, x-z or y-z plane, depending on which handle we are dragging
// we do a quick asin, trig according to which handle we are dragging,
// and now we have the rotation angle
//
// when we first start dragging, we do this maths and store the start rotation angle
// then we just find the difference between teh current rotation angle and the start one to get the rotation angle chnage
// quick axisangle2rot and rotmultiply and now we have our new rot.

void Editing3DRotClass::InteractiveHandleEdit( mvEditing3DAxisType Axis, int mousex, int mousey )
{
    SelectionIteratorTypedef iterator;
    iterator = Selector.SelectedObjects.begin();
    Object *p_Object = World.GetObjectByReference( iterator->second.iReference );

    Vector3 OurPos;
    Rot OurRot;
    Rot rInverseOurRot;

    if( Camera.bCameraEnabled )
    {
        OurPos = Camera.CurrentCameraPos;
        OurRot = Camera.CurrentCameraRot;
    }
    else
    {
        OurPos = World.GetObjectByReference( MetaverseClient::iMyReference )->pos;
        OurRot = World.GetObjectByReference( MetaverseClient::iMyReference )->rot;
    }
    InverseRot( rInverseOurRot, OurRot );

    if( p_Object != NULL )
    {
        float fCurrentRotationAngle;
        if( GetRotationAngleForObjectAndMouse( fCurrentRotationAngle, Editing3D.StartPos, Editing3D.StartRot, Axis, mousex, mousey ) )
        {
            float fRotateAngleChange = fCurrentRotationAngle - fStartRotationAngle;
            // DEBUG(  "$$$$$$$$$$$$$$$$$$$ Rotation angel change: " << fRotateAngleChange ); // DEBUG

            Vector3 RotationAxisObjectAxes;
            switch( Axis )
            {
                case AXIS_X_POS:
                RotationAxisObjectAxes = XAXIS;
                break;

                case AXIS_Y_POS:
                RotationAxisObjectAxes = YAXIS;
                break;

                case AXIS_Z_POS:
                RotationAxisObjectAxes = ZAXIS;
                break;
            }

            Rot RotationChangeObjectAxes;
            AxisAngle2Rot( RotationChangeObjectAxes, RotationAxisObjectAxes, fRotateAngleChange );
            //DEBUG(  " RotationChangeObjectAxes " << RotationChangeObjectAxes ); // DEBUG

            Rot rInverseStartRot;
            InverseRot( rInverseStartRot, Editing3D.StartRot );

            Rot NewRotation;
            RotMultiply( NewRotation, Editing3D.StartRot, RotationChangeObjectAxes );
            // DEBUG(  " NewRotation " << NewRotation ); // DEBUG

            /*
            DEBUG(  "StartRotWorldAxes: " << StartRot ); // DEBUG

            Rot RotationChangeWorldAxesAxes;
            RotMultiply( RotationChangeWorldAxesAxes, StartRot, RotationChangeObjectAxes );
            DEBUG(  " RotationChangeWorldAxesAxes " << RotationChangeWorldAxesAxes ); // DEBUG

            Rot NewRotationWorldAxes;
            RotMultiply( NewRotationWorldAxes, p_Object->rot, RotationChangeWorldAxesAxes );
            DEBUG(  " NewRotationWorldAxes " << NewRotationWorldAxes ); // DEBUG
            */

            //DEBUG(  " NewRotation " << NewRotationWorldAxes ); // DEBUG
            p_Object->rot = NewRotation;
        }
    }
}

void Editing3DRotClass::InitiateFreeEdit( int mousex, int mousey )
{
    Editing3D.EditingPreliminaries();

    SelectionIteratorTypedef iterator;
    iterator = Selector.SelectedObjects.begin();
    int iSelectedReference = iterator->second.iReference;
    int iSelectedArrayNum = World.GetArrayNumForObjectReference( iSelectedReference );
    Editing3D.StartRot = World.GetObject( iSelectedArrayNum )->rot;
    DEBUG(  "initializing StartRot " << Editing3D.StartRot ); // DEBUG
    //    mvKeyboardAndMouse::bDragging = true;
    Editing3D.iDragStartX = mousex;
    Editing3D.iDragStartY = mousey;

    Editing3D.CurrentEditType = EDITTYPE_ROT;
}

void Editing3DRotClass::InitiateHandleEdit( int mousex, int mousey, mvEditing3DAxisType Axis )
{
    Editing3D.EditingPreliminaries();

    map< int, SELECTION >::iterator iterator = Selector.SelectedObjects.begin();
    int iSelectedReference = iterator->second.iReference;
    Object *p_Object = World.GetObjectByReference( iSelectedReference );

    if( p_Object != NULL )
    {
        Vector3 OurPos;
        Rot OurRot;
        Rot rInverseOurRot;

        if( Camera.bCameraEnabled )
        {
            OurPos = Camera.CurrentCameraPos;
            OurRot = Camera.CurrentCameraRot;
        }
        else
        {
            OurPos = World.GetObjectByReference( MetaverseClient::iMyReference )->pos;
            OurRot = World.GetObjectByReference( MetaverseClient::iMyReference )->rot;
        }
        InverseRot( rInverseOurRot, OurRot );

        float fCurrentRotationAngle;

        Editing3D.StartPos = p_Object->pos;
        Editing3D.StartRot = p_Object->rot;
        if( GetRotationAngleForObjectAndMouse( fCurrentRotationAngle, Editing3D.StartPos, Editing3D.StartRot, Axis, mousex, mousey ) )
        {
            Editing3D.iDragStartX = mousex;
            Editing3D.iDragStartY = mousey;
            fStartRotationAngle = fCurrentRotationAngle;
            DEBUG(  "initializing StartRot " << Editing3D.StartRot <<  " rotation angle " << fCurrentRotationAngle ); // DEBUG

            Editing3D.CurrentAxis = Axis;
            Editing3D.CurrentEditType = EDITTYPE_ROTHANDLE;
        }
    }
}

void Editing3DRotClass::DrawSelectionHandle()
{
    glPushMatrix();

    glScalef( 1.5, 1.5, 1.5 );
    GLUquadricObj *quadratic=gluNewQuadric();   // Create A Pointer To The Quadric Object

    gluQuadricNormals(quadratic, GLU_SMOOTH); // Create Smooth Normals

    gluQuadricOrientation( quadratic, GLU_OUTSIDE );
    glTranslatef(0.0f,0.0f,-0.025f);   // Center The Cylinder
    gluCylinder(quadratic,0.5f,0.5f,0.05f,20,3);

    gluQuadricOrientation( quadratic, GLU_INSIDE );
    gluCylinder(quadratic,0.48f,0.5f,0.05f,20,3);

    gluQuadricOrientation( quadratic, GLU_OUTSIDE );

    glRotatef( 180.0f, 1.0f, 0.0f, 0.0f );
    gluDisk(quadratic,0.48f,0.5f,20,3);

    glRotatef( 180.0f, 1.0f, 0.0f, 0.0f );
    glTranslatef(0.0f,0.0f,0.05f);
    gluDisk(quadratic,0.48f,0.5f,20,3);


    glPopMatrix();
}

void Editing3DRotClass::DrawEditHandles( const Vector3 &ScaleToUse )
{
    mvGraphics.SetColor( 1.0, 0.0, 0.0 );
    // + x
    Selector.AddHitTarget( HITTARGETTYPE_EDITHANDLE, AXIS_X_POS );
    glPushMatrix();
    glScalef( ScaleToUse.x, ScaleToUse.y, ScaleToUse.z );
    glRotatef( 90, 0, 1, 0 );
    DrawSelectionHandle();
    glPopMatrix();

    mvGraphics.SetColor( 0.0, 1.0, 0.0 );
    // + y
    Selector.AddHitTarget( HITTARGETTYPE_EDITHANDLE, AXIS_Y_POS );
    glPushMatrix();
    glScalef( ScaleToUse.x, ScaleToUse.y, ScaleToUse.z );
    glRotatef( 90, 1, 0, 0 );
    DrawSelectionHandle();
    glPopMatrix();

    mvGraphics.SetColor( 0.0, 0.0, 1.0 );
    // + z
    Selector.AddHitTarget( HITTARGETTYPE_EDITHANDLE, AXIS_Z_POS );
    glPushMatrix();
    glScalef( ScaleToUse.x, ScaleToUse.y, ScaleToUse.z );
    //glRotatef( 90, 0, 1, 0 );
    DrawSelectionHandle();
    glPopMatrix();
}
