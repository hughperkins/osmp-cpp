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
//! \brief Manages 3d Position editing

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
    extern mvSelection Selector;
    //extern Animation animator;
    extern PlayerMovementClass PlayerMovement;
    extern mvGraphicsClass mvGraphics;
    extern mvCameraClass Camera;

    extern int iMyReference;
    extern Editing3DClass Editing3D;
}

// Example implementation: 3d Pos editing with handles
// ===================================================
//
// How this works:
//
// - the keyandmousesdl.cpp class tells us when someone is holding down button "editmode" (configured in config.xml),
//   or releasing it
// - we toggle bEditMode at this point
// - RendererImpl calls our function to draw Edit Handles
// - if bEditMode we draw them, registering them as HitTargets with mvSelection
//
// When someone clicks on our handle:
// - keyandmousesdl sends us an EditPosInitiate event
// - we call mvSelection.GetSelectObjectReference to get a HitTarget struct - if any - for clicked target, passing
//   in the mouse coordinates
// - if the HitTarget has a TargetType of TARGETTYPE_EDITHANDLE, then we know someone just clicked an edit handle
//   and can react accordingly
//
// ...skip some, to the maths...
//
// We know the distance moved on the screen by the mouse.
//
// We use OpenGL feedback mode to get the scaling from screen coordinates to 3d world coordinates at the distance
// that the object is from us. We do this by rendering two vertices 1.0 distance apart at the same distance as the object is, and measuring the distance apart that they appear on the screen. We assume that scaling is the same in both screen-y and screen-x directions.
//
// Now we can translate the mousemove screen vector into a realworld vector, in the avatar axes, by simply dividing
// by the scaling vector we just calculated.
//
// Now we multiply by InverseAvRot to get us the mousemove vector in WorldAxes at the point that the object is.
//
// Finally we multiply by the object rotation to into object axes.
//
// We know which axis handle the user clicked on, because we stored this in the HitTarget and retrieved it when the
// user clicked it.
//
// We dot product the mousemove vector, in object axes, with the appropriate unit vector. This gives us how far
// the object has been translated.
//
// Then we multiply this vector by InverseObjectRot to get back into World axes,
// and now we just add this to the Object's position at the start of the drag.
//

void Editing3DPosClass::InteractiveHandleEdit( mvEditing3DAxisType Axis, int mousex, int mousey )
{
    SelectionIteratorTypedef iterator;
    iterator = Selector.SelectedObjects.begin();
    Object *p_Object = MetaverseClient::World.GetObjectByReference( iterator->second.iReference );

    Vector3 OurPos;
    Rot OurRot;

    if( Camera.bCameraEnabled )
    {
        OurPos = Camera.CurrentCameraPos;
        OurRot = Camera.CurrentCameraRot;
    }
    else
    {
        Object *p_Object = MetaverseClient::World.GetObjectByReference( MetaverseClient::iMyReference );
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

    if( p_Object != NULL )
    {
        float fDistanceFromUsToObject = VectorMag( p_Object->pos - OurPos );

        float fScalingFromPosToScreen = graphics.GetScalingFrom3DToScreen( fDistanceFromUsToObject );
        //float fScalingFromPosToScreen = 172;
        //DEBUG(  "Screen scaling: " << fScalingFromPosToScreen ); // DEBUG

        Vector3 ScreenMouseVector;
        ScreenMouseVector.y = - ( (float)(mousex - Editing3D.iDragStartX) );
        ScreenMouseVector.z = - ( (float)( mousey - Editing3D.iDragStartY ) );
        ScreenMouseVector.x = 0;

        Vector3 PosMouseVectorAvAxes = ScreenMouseVector * ( 1 / fScalingFromPosToScreen );

        Vector3 PosMouseVectorWorldAxes;
        PosMouseVectorWorldAxes = PosMouseVectorAvAxes * rInverseOurRot;
        // MultiplyVectorByRot( PosMouseVectorWorldAxes, rInverseOurRot, PosMouseVectorAvAxes );

        Vector3 PosMouseVectorObjectAxes;
        PosMouseVectorObjectAxes = PosMouseVectorWorldAxes * p_Object->rot;
        // MultiplyVectorByRot( PosMouseVectorObjectAxes, p_Object->rot, PosMouseVectorWorldAxes );
        // DEBUG(  "screen vector: " << ScreenMouseVector << " PosMouseVectorAvAxes " << PosMouseVectorAvAxes <<
        //    " posmousevectorworldaxes: " << PosMouseVectorWorldAxes << " PosMouseVectorObjectAxes " << PosMouseVectorObjectAxes ); // DEBUG

        Vector3 vTranslationVectorObjectAxes;
        float fDistanceToTranslate;

        // float fModDragSquared = PosMouseVectorObjectAxes.x * PosMouseVectorObjectAxes.x
        //     + PosMouseVectorObjectAxes.y * PosMouseVectorObjectAxes.y
        //     + PosMouseVectorObjectAxes.z * PosMouseVectorObjectAxes.z;
        // float fAxisUnitDotDrag;

        switch( Axis )
        {
            case AXIS_X_POS:
            //   DEBUG(  "x +ve" ); // DEBUG
            //   fAxisUnitDotDrag = VectorDot( PosMouseVectorObjectAxes, XAXIS );
            //   if( fabs( fAxisUnitDotDrag ) > 0.0001 )
            //   {
            //      vTranslationVectorObjectAxes.x = fModDragSquared / fAxisUnitDotDrag;
            //  }
            vTranslationVectorObjectAxes.x = VectorDot( PosMouseVectorObjectAxes, XAXIS );
            break;

            case AXIS_X_NEG:
            //   DEBUG(  "x -ve" ); // DEBUG
            vTranslationVectorObjectAxes.x = VectorDot( PosMouseVectorObjectAxes, XAXIS );
            break;

            case AXIS_Y_POS:
            //   DEBUG(  "y +ve" ); // DEBUG
            vTranslationVectorObjectAxes.y = VectorDot( PosMouseVectorObjectAxes, YAXIS );
            break;

            case AXIS_Y_NEG:
            //     DEBUG(  "y -ve" ); // DEBUG
            vTranslationVectorObjectAxes.y = VectorDot( PosMouseVectorObjectAxes, YAXIS );
            break;

            case AXIS_Z_POS:
            //   DEBUG(  "z +ve" ); // DEBUG
            vTranslationVectorObjectAxes.z = VectorDot( PosMouseVectorObjectAxes, ZAXIS );
            break;

            case AXIS_Z_NEG:
            //   DEBUG(  "z -ve" ); // DEBUG
            vTranslationVectorObjectAxes.z = VectorDot( PosMouseVectorObjectAxes, ZAXIS );
            break;

        }

        Rot InverseObjectRot;
        InverseRot( InverseObjectRot, p_Object->rot );

        Vector3 vTranslationVectorWorldAxes;
        vTranslationVectorWorldAxes = vTranslationVectorObjectAxes * InverseObjectRot;
        // MultiplyVectorByRot( vTranslationVectorWorldAxes, InverseObjectRot, vTranslationVectorObjectAxes );

        Vector3 PosTranslationVectorWorldAxes = vTranslationVectorWorldAxes;

        p_Object->pos = Editing3D.StartPos + PosTranslationVectorWorldAxes;
        //DEBUG(  "pobjectpos: " << p_Object->pos ); // DEBUG
    }
}

void Editing3DPosClass::InteractiveFreeEdit( bool bAltAxes, int x, int y )
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

    Vector3 pTranslate;
    if( bAltAxes )
    {
        pTranslate.y = - ((float)(x - Editing3D.iDragStartX)) / HalfWinWidth * 3.0;
        pTranslate.x = - ((float)(y - Editing3D.iDragStartY)) / HalfWinWidth * 3.0;
        pTranslate.z = 0;
    }
    else
    {
        pTranslate.y = - ((float)(x - Editing3D.iDragStartX)) / HalfWinWidth * 3.0;
        pTranslate.z = - ((float)(y - Editing3D.iDragStartY)) / HalfWinWidth * 3.0;
        pTranslate.x = 0;
    }

    Vector3 PosChangeWorldAxes;
    PosChangeWorldAxes = pTranslate * rInverseOurRot;
    // MultiplyVectorByRot( PosChangeWorldAxes, rInverseOurRot, pTranslate  );

    Vector3 PosNewPosWorldAxes;
    PosNewPosWorldAxes.x = PosChangeWorldAxes.x + Editing3D.StartPos.x;
    PosNewPosWorldAxes.y = PosChangeWorldAxes.y + Editing3D.StartPos.y;
    PosNewPosWorldAxes.z = PosChangeWorldAxes.z + Editing3D.StartPos.z;


    SelectionIteratorTypedef iterator;
    iterator = Selector.SelectedObjects.begin();
    int iSelectedReference = iterator->second.iReference;
    int iSelectedArrayNum = World.GetArrayNumForObjectReference( iSelectedReference );
    World.GetObject( iSelectedArrayNum )->pos =PosNewPosWorldAxes;
}

void Editing3DPosClass::DrawEditHandles( const Vector3 &ScaleToUse )
{
    mvGraphics.SetColor( 1.0, 0.0, 0.0 );

    Vector3 HandleScale( 0.2, 0.2, 0.2 );

    // + x
    Selector.AddHitTarget( HITTARGETTYPE_EDITHANDLE, AXIS_X_POS );
    glPushMatrix();
    glTranslatef( ScaleToUse.x / 2, 0, 0 );
    glScalef( HandleScale.x, HandleScale.y, HandleScale.z );
    glRotatef( 90, 0, 1, 0 );
    mvGraphics.DoCone();
    glPopMatrix();

    // - x
    Selector.AddHitTarget( HITTARGETTYPE_EDITHANDLE, AXIS_X_NEG );
    glPushMatrix();
    glTranslatef( - ScaleToUse.x / 2, 0, 0 );
    glScalef( HandleScale.x, HandleScale.y, HandleScale.z );
    glRotatef( -90, 0, 1, 0 );
    mvGraphics.DoCone();
    glPopMatrix();

    mvGraphics.SetColor( 0.0, 1.0, 0.0 );

    // + y
    Selector.AddHitTarget( HITTARGETTYPE_EDITHANDLE, AXIS_Y_POS );
    glPushMatrix();
    glTranslatef( 0, - ScaleToUse.y / 2, 0 );
    glScalef( HandleScale.x, HandleScale.y, HandleScale.z );
    glRotatef( 90, 1, 0, 0 );
    mvGraphics.DoCone();
    glPopMatrix();

    // - y
    Selector.AddHitTarget( HITTARGETTYPE_EDITHANDLE, AXIS_Y_NEG );
    glPushMatrix();
    glTranslatef( 0, ScaleToUse.y / 2, 0 );
    glScalef( HandleScale.x, HandleScale.y, HandleScale.z );
    glRotatef( -90, 1, 0, 0 );
    mvGraphics.DoCone();
    glPopMatrix();

    mvGraphics.SetColor( 0.0, 0.0, 1.0 );

    // + z
    Selector.AddHitTarget( HITTARGETTYPE_EDITHANDLE, AXIS_Z_POS );
    glPushMatrix();
    glTranslatef( 0, 0, ScaleToUse.z / 2 );
    glScalef( HandleScale.x, HandleScale.y, HandleScale.z );
    //glRotatef( 90, 0, 1, 0 );
    mvGraphics.DoCone();
    glPopMatrix();

    // - z
    Selector.AddHitTarget( HITTARGETTYPE_EDITHANDLE, AXIS_Z_NEG );
    glPushMatrix();
    glTranslatef( 0, 0, - ScaleToUse.z / 2 );
    glScalef( HandleScale.x, HandleScale.y, HandleScale.z );
    glRotatef( 180, 0, 1, 0 );
    mvGraphics.DoCone();
    glPopMatrix();
}

void Editing3DPosClass::InitiateHandleEdit( int mousex, int mousey, mvEditing3DAxisType Axis )
{
    Editing3D.EditingPreliminaries();

    map< int, SELECTION >::iterator iterator = Selector.SelectedObjects.begin();
    int iSelectedReference = iterator->second.iReference;
    Object *p_Object = World.GetObjectByReference( iSelectedReference );

    if( p_Object != NULL )
    {
        Editing3D.iDragStartX = mousex;
        Editing3D.iDragStartY = mousey;
        Editing3D.StartPos = p_Object->pos;
        DEBUG(  "initializing StartPos " << Editing3D.StartPos ); // DEBUG

        Editing3D.CurrentAxis = Axis;
        Editing3D.CurrentEditType = EDITTYPE_POSHANDLE;
    }
}

void Editing3DPosClass::InitiateFreeEdit( int mousex, int mousey )
{
    SelectionIteratorTypedef iterator;
    iterator = Selector.SelectedObjects.begin();
    int iSelectedReference = iterator->second.iReference;
    Object *p_Object = World.GetObjectByReference( iSelectedReference );

    if( p_Object != NULL )
    {
        Editing3D.iDragStartX = mousex;
        Editing3D.iDragStartY = mousey;
        Editing3D.StartPos = p_Object->pos;
        DEBUG(  "initializing StartPos " << Editing3D.StartPos ); // DEBUG

        Editing3D.CurrentEditType = EDITTYPE_POS;
    }
}

