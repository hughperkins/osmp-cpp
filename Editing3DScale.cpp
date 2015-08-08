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
#include "Editing3DScale.h"
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

//! \file
//! \brief Manages 3d Scale editing
// See Editing3dpos.cpp for a description of roughly how this works

void Editing3DScaleClass::InteractiveHandleEdit( mvEditing3DAxisType Axis, int mousex, int mousey )
{
    // DEBUG(  "InteractiveHandleScaleEdit" ); // DEBUG
    SelectionIteratorTypedef iterator;
    iterator = Selector.SelectedObjects.begin();
    Object *p_Object = World.GetObjectByReference( iterator->second.iReference );

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

    if( p_Object != NULL )
    {
        if( strcmp( p_Object->ObjectType, "PRIM" ) == 0 )
        {
            float fDistanceFromUsToObject = VectorMag( p_Object->pos - OurPos );

            // DEBUG(  "InteractiveHandleScaleEdit 1" ); // DEBUG
            float fScalingFromPosToScreen = mvGraphics.GetScalingFrom3DToScreen( fDistanceFromUsToObject );

            // DEBUG(  "InteractiveHandleScaleEdit 2" ); // DEBUG

            Vector3 ScreenMouseVector;
            ScreenMouseVector.y = - ( (float)(mousex - Editing3D.iDragStartX) );
            ScreenMouseVector.z = - ( (float)( mousey - Editing3D.iDragStartY ) );
            ScreenMouseVector.x = 0;

            //  DEBUG(  "InteractiveHandleScaleEdit 3" ); // DEBUG
            Vector3 PosMouseVectorAvAxes = ScreenMouseVector * ( 1 / fScalingFromPosToScreen );
            // DEBUG(  "InteractiveHandleScaleEdit 4" ); // DEBUG

            Rot rInverseOurRot;
            InverseRot( rInverseOurRot, OurRot );

            Vector3 PosMouseVectorWorldAxes;
            PosMouseVectorWorldAxes = PosMouseVectorAvAxes * rInverseOurRot;
            // MultiplyVectorByRot( PosMouseVectorWorldAxes, rInverseOurRot, PosMouseVectorAvAxes );

            Vector3 PosMouseVectorObjectAxes;
            PosMouseVectorObjectAxes = PosMouseVectorWorldAxes * p_Object->rot;
            // MultiplyVectorByRot( PosMouseVectorObjectAxes, p_Object->rot, PosMouseVectorWorldAxes );
            //   DEBUG(  "screen vector: " << ScreenMouseVector << " PosMouseVectorAvAxes " << PosMouseVectorAvAxes <<
            //      " posmousevectorworldaxes: " << PosMouseVectorWorldAxes << " PosMouseVectorObjectAxes " << PosMouseVectorObjectAxes ); // DEBUG

            //Vector3 vTranslationVectorObjectAxes;
            Vector3 vScaleChangeVectorObjectAxes;
            float fDistanceToScale;

            bool bPositiveAxis = false;

            //   DEBUG(  "InteractiveHandleScaleEdit 5" ); // DEBUG
            switch( Axis )
            {
                case AXIS_X_POS:
                //   DEBUG(  "x +ve" ); // DEBUG
                vScaleChangeVectorObjectAxes.x = VectorDot( PosMouseVectorObjectAxes, XAXIS );
                bPositiveAxis = true;
                break;

                case AXIS_X_NEG:
                //DEBUG(  "x -ve" ); // DEBUG
                vScaleChangeVectorObjectAxes.x = VectorDot( PosMouseVectorObjectAxes, XAXIS );
                break;

                case AXIS_Y_POS:
                //DEBUG(  "y +ve" ); // DEBUG
                vScaleChangeVectorObjectAxes.y = VectorDot( PosMouseVectorObjectAxes, YAXIS );
                break;

                case AXIS_Y_NEG:
                // DEBUG(  "y -ve" ); // DEBUG
                bPositiveAxis = true;
                vScaleChangeVectorObjectAxes.y = VectorDot( PosMouseVectorObjectAxes, YAXIS );
                break;

                case AXIS_Z_POS:
                // DEBUG(  "z +ve" ); // DEBUG
                bPositiveAxis = true;
                vScaleChangeVectorObjectAxes.z = VectorDot( PosMouseVectorObjectAxes, ZAXIS );
                break;

                case AXIS_Z_NEG:
                // DEBUG(  "z -ve" ); // DEBUG
                vScaleChangeVectorObjectAxes.z = VectorDot( PosMouseVectorObjectAxes, ZAXIS );
                break;

            }

            Vector3 vNewScale;
            if( bPositiveAxis )
            {
                vNewScale = Vector3( Editing3D.StartScale ) + Vector3( vScaleChangeVectorObjectAxes );
            }
            else
            {
                vNewScale = Vector3( Editing3D.StartScale ) - Vector3( vScaleChangeVectorObjectAxes );
            }

            if( vNewScale.x < 0.05 )
            {
                vNewScale.x = 0.05;
            }
            else if( vNewScale.y < 0.05 )
            {
                vNewScale.y = 0.05;
            }
            else if( vNewScale.z < 0.05 )
            {
                vNewScale.z = 0.05;
            }

            Vector3 vTranslate;
            vTranslate = ( Vector3( vNewScale ) - Vector3( Editing3D.StartScale ) ) * 0.5f;

            dynamic_cast< Prim *>( p_Object )->scale = vNewScale;

            //    DEBUG(  "vScaleChangeVectorObjectAxes " << vScaleChangeVectorObjectAxes << " Editing3D.StartScale "
            //       << Editing3D.StartScale << " vNewScale " << vNewScale << " vTranslate " << vTranslate ); // DEBUG

            Rot InverseObjectRot;
            InverseRot( InverseObjectRot, p_Object->rot );

            Vector3 vTranslationVectorWorldAxes;
            vTranslationVectorWorldAxes = vTranslate * InverseObjectRot;
            // MultiplyVectorByRot( vTranslationVectorWorldAxes, InverseObjectRot, vTranslate );

            //   DEBUG(  "InteractiveHandleScaleEdit 6" ); // DEBUG
            Vector3 PosTranslationVectorWorldAxes = vTranslationVectorWorldAxes;

            if( bPositiveAxis )
            {
                p_Object->pos = Editing3D.StartPos + PosTranslationVectorWorldAxes;
            }
            else
            {
                p_Object->pos = Editing3D.StartPos - PosTranslationVectorWorldAxes;
            }
        }
    }
    //   DEBUG(  "InteractiveHandleScaleEdit done" ); // DEBUG
}

void Editing3DScaleClass::InteractiveFreeEdit( int mousex, int mousey )
{
    // DEBUG(  "InteractiveHandleScaleEdit" ); // DEBUG
    SelectionIteratorTypedef iterator;
    iterator = Selector.SelectedObjects.begin();
    Object *p_Object = World.GetObjectByReference( iterator->second.iReference );

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

    if( p_Object != NULL )
    {
        if( strcmp( p_Object->ObjectType, "PRIM" ) == 0 )
        {
            float fDistanceFromUsToObject = VectorMag( p_Object->pos - OurPos );

            // DEBUG(  "InteractiveHandleScaleEdit 1" ); // DEBUG
            float fScalingFromPosToScreen = mvGraphics.GetScalingFrom3DToScreen( fDistanceFromUsToObject );

            // DEBUG(  "InteractiveHandleScaleEdit 2" ); // DEBUG

            Vector3 ScreenMouseVector;
            ScreenMouseVector.y = - ( (float)(mousex - Editing3D.iDragStartX) );
            ScreenMouseVector.z = - ( (float)( mousey - Editing3D.iDragStartY ) );
            ScreenMouseVector.x = 0;

            //  DEBUG(  "InteractiveHandleScaleEdit 3" ); // DEBUG
            Vector3 PosMouseVectorAvAxes = ScreenMouseVector * ( 1 / fScalingFromPosToScreen );
            // DEBUG(  "InteractiveHandleScaleEdit 4" ); // DEBUG

            Rot rInverseOurRot;
            InverseRot( rInverseOurRot, OurRot );

            Vector3 PosMouseVectorWorldAxes;
            PosMouseVectorWorldAxes = PosMouseVectorAvAxes * rInverseOurRot;
            // MultiplyVectorByRot( PosMouseVectorWorldAxes, rInverseOurRot, PosMouseVectorAvAxes );

            Vector3 PosMouseVectorObjectAxes;
            PosMouseVectorObjectAxes = PosMouseVectorWorldAxes * p_Object->rot;
            // MultiplyVectorByRot( PosMouseVectorObjectAxes, p_Object->rot, PosMouseVectorWorldAxes );
            //   DEBUG(  "screen vector: " << ScreenMouseVector << " PosMouseVectorAvAxes " << PosMouseVectorAvAxes <<
            //      " posmousevectorworldaxes: " << PosMouseVectorWorldAxes << " PosMouseVectorObjectAxes " << PosMouseVectorObjectAxes ); // DEBUG

            //Vector3 vTranslationVectorObjectAxes;
            Vector3 vScaleChangeVectorObjectAxes;
            float fDistanceToScale;

            bool bPositiveAxis = false;

            vScaleChangeVectorObjectAxes = PosMouseVectorObjectAxes;

            Vector3 vNewScale;
            vNewScale = Vector3( Editing3D.StartScale ) + Vector3( vScaleChangeVectorObjectAxes );

            if( vNewScale.x < 0.05 )
            {
                vNewScale.x = 0.05;
            }
            else if( vNewScale.y < 0.05 )
            {
                vNewScale.y = 0.05;
            }
            else if( vNewScale.z < 0.05 )
            {
                vNewScale.z = 0.05;
            }

            //        Vector3 vTranslate;
            //        vTranslate = ( Vector3( vNewScale ) - Vector3( Editing3D.StartScale ) ) * 0.5f;

            dynamic_cast< Prim *>( p_Object )->scale = vNewScale;
        }
    }
    //   DEBUG(  "InteractiveHandleScaleEdit done" ); // DEBUG
}

void _InteractiveFreeScaleEdit_Old( bool bAltAxes, int x, int y )
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

    SelectionIteratorTypedef iterator;
    iterator = Selector.SelectedObjects.begin();
    int iSelectedReference = iterator->second.iReference;
    int iSelectedArrayNum = World.GetArrayNumForObjectReference( iSelectedReference );

    // DEBUG(  "interactive scale edit objectype=[" << World.GetObject( iSelectedArrayNum )->ObjectType << "]" ); // DEBUG
    if( strcmp( World.GetObject( iSelectedArrayNum )->ObjectType, "PRIM" ) == 0 )
    {
        float HalfWinWidth = RendererImpl::GetWindowWidth() / 2;
        float HalfWinHeight = RendererImpl::GetWindowHeight() / 2;

        Vector3 ScaleAvAxes;
        if( bAltAxes )
        {
            ScaleAvAxes.y = ((float)(x - Editing3D.iDragStartX)) / HalfWinWidth * 1.0;
            ScaleAvAxes.x = - ((float)(y - Editing3D.iDragStartY)) / HalfWinWidth * 1.0;
            ScaleAvAxes.z = 0;
        }
        else
        {
            ScaleAvAxes.y = ((float)(x - Editing3D.iDragStartX)) / HalfWinWidth * 1.0;
            ScaleAvAxes.z = - ((float)(y - Editing3D.iDragStartY)) / HalfWinWidth * 1.0;
            ScaleAvAxes.x = 0;
        }

        Rot PrimRot = dynamic_cast<Prim *>(World.GetObject( iSelectedArrayNum ))->rot;
        Rot InversePrimRot;
        InverseRot( InversePrimRot, PrimRot );

        // currentscaleseenbyav = avrot * inverseprim * currentprimscale
        Vector3 CurrentScaleWorldAxes;
        CurrentScaleWorldAxes = Editing3D.StartScale * rInverseOurRot;
        // MultiplyVectorByRot( CurrentScaleWorldAxes, rInverseOurRot, Editing3D.StartScale );
        Vector3 CurrentScaleAvatarAxes;
        CurrentScaleAvatarAxes = CurrentScaleWorldAxes * OurRot;
        // MultiplyVectorByRot( CurrentScaleAvatarAxes, OurRot, CurrentScaleWorldAxes );

        // newscaleseenbyava = ( <1,1,1> + screenvector ) * currentscaleseenbyav
        Vector3 ScaleNewScaleAvAxes;
        ScaleNewScaleAvAxes.x = ( 1 + ScaleAvAxes.x ) * CurrentScaleAvatarAxes.x;
        ScaleNewScaleAvAxes.y = ( 1 + ScaleAvAxes.y ) * CurrentScaleAvatarAxes.y;
        ScaleNewScaleAvAxes.z = ( 1 + ScaleAvAxes.z ) * CurrentScaleAvatarAxes.z;

        // newscaleseenbyprim = primrot * inverseav * newscaleseenbyav
        Vector3 NewScaleWorldAxes;
        NewScaleWorldAxes = ScaleNewScaleAvAxes * rInverseOurRot;
        // MultiplyVectorByRot( NewScaleWorldAxes, rInverseOurRot, ScaleNewScaleAvAxes );
        Vector3 NewScaleSeenByPrim;
        NewScaleSeenByPrim = NewScaleWorldAxes * PrimRot;
        // MultiplyVectorByRot( NewScaleSeenByPrim, PrimRot, NewScaleWorldAxes );

        if( NewScaleSeenByPrim.x < 0 )
        {
            NewScaleSeenByPrim.x = 0.05;
        }
        if( NewScaleSeenByPrim.y < 0 )
        {
            NewScaleSeenByPrim.y = 0.05;
        }
        if( NewScaleSeenByPrim.z < 0 )
        {
            NewScaleSeenByPrim.z = 0.05;
        }

        SelectionIteratorTypedef iterator;
        iterator = Selector.SelectedObjects.begin();
        int iSelectedReference = iterator->second.iReference;
        int iSelectedArrayNum = World.GetArrayNumForObjectReference( iSelectedReference );
        dynamic_cast<Prim *>(World.GetObject( iSelectedArrayNum ))->scale = NewScaleSeenByPrim;
        // DEBUG(  "setting new scale " << NewScaleSeenByPrim ); // DEBUG
    }
}

void Editing3DScaleClass::InitiateFreeEdit(int mousex, int mousey )
{
    Editing3D.EditingPreliminaries();

    SelectionIteratorTypedef iterator;
    iterator = Selector.SelectedObjects.begin();
    int iSelectedReference = iterator->second.iReference;
    Object *p_Object = World.GetObjectByReference( iSelectedReference );

    if( p_Object != NULL )
    {
        if( strcmp( p_Object->ObjectType, "PRIM" ) == 0 )
        {
            Editing3D.iDragStartX = mousex;
            Editing3D.iDragStartY = mousey;
            Editing3D.StartScale = dynamic_cast< Prim * >( p_Object )->scale;

            Editing3D.CurrentEditType = EDITTYPE_SCALE;
        }
    }
    //DEBUG(  "initializing StartPos " << Editing3D.StartScale ); // DEBUG
}

void Editing3DScaleClass::InitiateHandleEdit( int mousex, int mousey, mvEditing3DAxisType Axis )
{
    Editing3D.EditingPreliminaries();

    map< int, SELECTION >::iterator iterator = Selector.SelectedObjects.begin();
    int iSelectedReference = iterator->second.iReference;
    Object *p_Object = World.GetObjectByReference( iSelectedReference );

    if( p_Object != NULL )
    {
        if( strcmp( p_Object->ObjectType, "PRIM" ) == 0 )
        {
            Editing3D.iDragStartX = mousex;
            Editing3D.iDragStartY = mousey;
            Editing3D.StartScale = dynamic_cast< Prim *>( p_Object )->scale;
            Editing3D.StartPos = p_Object->pos;
            //DEBUG(  "initializing Editing3D.StartScale " << Editing3D.StartScale ); // DEBUG

            Editing3D.CurrentAxis = Axis;
            Editing3D.CurrentEditType = EDITTYPE_SCALEHANDLE;
        }
    }
}

void Editing3DScaleClass::DrawEditHandles( const Vector3 &ScaleToUse )
{
    mvGraphics.SetColor( 1.0, 0.0, 0.0 );

    Vector3 HandleScale( 0.14, 0.14, 0.14 );

    // + x
    Selector.AddHitTarget( HITTARGETTYPE_EDITHANDLE, AXIS_X_POS );
    glPushMatrix();
    glTranslatef( ScaleToUse.x / 2, 0, 0 );
    glScalef( HandleScale.x, HandleScale.y, HandleScale.z );
    glRotatef( 90, 0, 1, 0 );
    mvGraphics.DoCube();
    glPopMatrix();

    // - x
    Selector.AddHitTarget( HITTARGETTYPE_EDITHANDLE, AXIS_X_NEG );
    glPushMatrix();
    glTranslatef( - ScaleToUse.x / 2, 0, 0 );
    glScalef( HandleScale.x, HandleScale.y, HandleScale.z );
    glRotatef( -90, 0, 1, 0 );
    mvGraphics.DoCube();
    glPopMatrix();

    mvGraphics.SetColor( 0.0, 1.0, 0.0 );

    // + y
    Selector.AddHitTarget( HITTARGETTYPE_EDITHANDLE, AXIS_Y_POS );
    glPushMatrix();
    glTranslatef( 0, - ScaleToUse.y / 2, 0 );
    glScalef( HandleScale.x, HandleScale.y, HandleScale.z );
    glRotatef( 90, 1, 0, 0 );
    mvGraphics.DoCube();
    glPopMatrix();

    // - y
    Selector.AddHitTarget( HITTARGETTYPE_EDITHANDLE, AXIS_Y_NEG );
    glPushMatrix();
    glTranslatef( 0, ScaleToUse.y / 2, 0 );
    glScalef( HandleScale.x, HandleScale.y, HandleScale.z );
    glRotatef( -90, 1, 0, 0 );
    mvGraphics.DoCube();
    glPopMatrix();

    mvGraphics.SetColor( 0.0, 0.0, 1.0 );

    // + z
    Selector.AddHitTarget( HITTARGETTYPE_EDITHANDLE, AXIS_Z_POS );
    glPushMatrix();
    glTranslatef( 0, 0, ScaleToUse.z / 2 );
    glScalef( HandleScale.x, HandleScale.y, HandleScale.z );
    //glRotatef( 90, 0, 1, 0 );
    mvGraphics.DoCube();
    glPopMatrix();

    // - z
    Selector.AddHitTarget( HITTARGETTYPE_EDITHANDLE, AXIS_Z_NEG );
    glPushMatrix();
    glTranslatef( 0, 0, - ScaleToUse.z / 2 );
    glScalef( HandleScale.x, HandleScale.y, HandleScale.z );
    glRotatef( 180, 0, 1, 0 );
    mvGraphics.DoCube();
    glPopMatrix();
}
