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

// see ThreeDeeEditing.h for documentation

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
#include "Editing3D.h"
#include "Editing3DPos.h"
#include "Editing3DScale.h"
#include "Editing3DRot.h"

namespace MetaverseClient
{
    extern mvWorldStorage World;
    extern mvSelection Selector;
    //extern Animation animator;
    extern PlayerMovementClass PlayerMovement;
    extern mvGraphicsClass mvGraphics;
    extern mvCameraClass Camera;

    extern int iMyReference;
}
using namespace MetaverseClient;

//! a 2-d position; used in ThreeDeeEditing.cpp
struct Pos2D
{
    float x;
    float y;
};

//! OpenGL FeedbackbufferStruct, used in ThreeDeeEditing.cpp
struct FeedbackBufferStruct
{
    float type;
    Pos2D Vertices[ 2 ];
};

Vector3 Editing3DClass::GetScreenPos( const Vector3 ObserverPos, const Rot ObserverRot, const Vector3 TargetPos3D )
{
    FeedbackBufferStruct FeedbackBuffer;

    glFeedbackBuffer( 1 * sizeof( Pos2D ), GL_2D, (GLfloat *)&FeedbackBuffer );

    GLint viewport[4];
    // This Sets The Array <viewport> To The Size And Location Of The Screen Relative To The Window
    glGetIntegerv(GL_VIEWPORT, viewport);

    glPushMatrix();

    glRenderMode( GL_FEEDBACK );

    glLoadIdentity();

    // rotate so z axis is up, and x axis is forward
    glRotatef( 90, 0.0, 0.0, 1.0 );
    glRotatef( 90, 0.0, 1.0, 0.0 );

    Rot InverseObserverRot;
    InverseRot( InverseObserverRot, ObserverRot );
    mvGraphics.RotateToRot( InverseObserverRot );

    glTranslatef( - ObserverPos.x, - ObserverPos.y, - ObserverPos.z  );

    glBegin( GL_POINTS );
    glVertex3f( TargetPos3D.x, TargetPos3D.y, TargetPos3D.z );
    glEnd();

    glRenderMode( GL_RENDER );

    glPopMatrix();

    //DEBUG(  "Screencoords of input vertex: " << FeedbackBuffer.Vertices[0].x << " " << FeedbackBuffer.Vertices[0].y ); // DEBUG

    Vector3 ScreenPos;
    ScreenPos.x = 0;
    ScreenPos.y = RendererImpl::GetWindowWidth() - FeedbackBuffer.Vertices[0].x;
    ScreenPos.z = FeedbackBuffer.Vertices[0].y;

    //DEBUG(  "screenpos: " << ScreenPos ); // DEBUG

    return ScreenPos;
}

void Editing3DClass::EditingPreliminaries()
{
    Selector.bSendObjectMoveForSelection = true;
    /*
       Rot rTwist, rPitch;
       AxisAngle2Rot( rTwist, ZAXIS, ((float)PlayerMovement.avatarzrot * mvConstants::piover180 ) );
       AxisAngle2Rot( rPitch, YAXIS, ((float)PlayerMovement.avataryrot * mvConstants::piover180) );
       RotMultiply( rAvatar, rTwist, rPitch );

       InverseRot( rInverseAvatar, rAvatar );
    */
}

void Editing3DClass::InitiateScaleEdit( int mousex, int mousey )
{
    EditingPreliminaries();

    //  mvKeyboardAndMouse::bDragging = true;

    HITTARGET *pHitTarget = Selector.GetClickedHitTarget( mousex, mousey );
    //DEBUG(  "Clicked target type: " << pHitTarget->TargetType ); // DEBUG

    if( pHitTarget == NULL )
    {
        DEBUG(  "Not an edit handle" ); // DEBUG
        Editing3DScale.InitiateFreeEdit( mousex, mousey );
    }
    else if( pHitTarget->TargetType == HITTARGETTYPE_EDITHANDLE )
    {
        DEBUG(  "It's an edit handle :-O" ); // DEBUG
        Editing3DScale.InitiateHandleEdit( mousex, mousey, ( mvEditing3DAxisType) pHitTarget->iForeignReference );
    }
    else
    {
        DEBUG(  "Not an edit handle" ); // DEBUG
        Editing3DScale.InitiateFreeEdit( mousex, mousey );
    }
}

void Editing3DClass::InitiateTranslateEdit( int mousex, int mousey )
{
    EditingPreliminaries();

    //  mvKeyboardAndMouse::bDragging = true;

    HITTARGET *pHitTarget = Selector.GetClickedHitTarget( mousex, mousey );
    //DEBUG(  "Clicked target type: " << pHitTarget->TargetType ); // DEBUG

    if( pHitTarget == NULL )
    {
        DEBUG(  "Not an edit handle" ); // DEBUG
        Editing3DPos.InitiateFreeEdit( mousex, mousey );
    }
    else if( pHitTarget->TargetType == HITTARGETTYPE_EDITHANDLE )
    {
        DEBUG(  "It's an edit handle :-O" ); // DEBUG
        Editing3DPos.InitiateHandleEdit( mousex, mousey, ( mvEditing3DAxisType) pHitTarget->iForeignReference );
    }
    else
    {
        DEBUG(  "Not an edit handle" ); // DEBUG
        Editing3DPos.InitiateFreeEdit( mousex, mousey );
    }
}

void Editing3DClass::InitiateRotateEdit( int mousex, int mousey )
{
    EditingPreliminaries();

    //  mvKeyboardAndMouse::bDragging = true;

    HITTARGET *pHitTarget = Selector.GetClickedHitTarget( mousex, mousey );
    //DEBUG(  "Clicked target type: " << pHitTarget->TargetType ); // DEBUG

    if( pHitTarget == NULL )
    {
        DEBUG(  "Not an edit handle" ); // DEBUG
        Editing3DRot.InitiateFreeEdit( mousex, mousey );
    }
    else if( pHitTarget->TargetType == HITTARGETTYPE_EDITHANDLE )
    {
        DEBUG(  "It's an edit handle :-O" ); // DEBUG
        Editing3DRot.InitiateHandleEdit( mousex, mousey, ( mvEditing3DAxisType) pHitTarget->iForeignReference );
    }
    else
    {
        DEBUG(  "Not an edit handle" ); // DEBUG
        Editing3DRot.InitiateFreeEdit( mousex, mousey );
    }
}

void Editing3DClass::UpdateTranslateEdit( bool bAltAxes, int mousex, int mousey )
{
    EditingPreliminaries();

    if( CurrentEditType == EDITTYPE_POS && bAxisModifierOn == bAltAxes )
    {
        Editing3DPos.InteractiveFreeEdit( bAltAxes, mousex, mousey );
    }
    else if( CurrentEditType == EDITTYPE_POSHANDLE )
    {
        Editing3DPos.InteractiveHandleEdit( CurrentAxis, mousex, mousey );
    }
    else
    {
        InitiateTranslateEdit( mousex, mousey );
        bAxisModifierOn = bAltAxes;
    }
}

void Editing3DClass::UpdateScaleEdit( bool bAltAxes, int mousex, int mousey )
{
    EditingPreliminaries();

    if( CurrentEditType == EDITTYPE_SCALE && bAxisModifierOn == bAltAxes )
    {
        Editing3DScale.InteractiveFreeEdit( mousex, mousey );
    }
    else if( CurrentEditType == EDITTYPE_SCALEHANDLE )
    {
        Editing3DScale.InteractiveHandleEdit( CurrentAxis, mousex, mousey );
    }
    else
    {
        InitiateScaleEdit( mousex, mousey );
        bAxisModifierOn = bAltAxes;
    }
}

void Editing3DClass::UpdateRotateEdit( bool bAltAxes, int mousex, int mousey )
{
    EditingPreliminaries();

    if( CurrentEditType == EDITTYPE_ROT && bAxisModifierOn == bAltAxes )
    {
        Editing3DRot.InteractiveFreeEdit( mousex, mousey );
    }
    else if( CurrentEditType == EDITTYPE_ROTHANDLE )
    {
        Editing3DRot.InteractiveHandleEdit( CurrentAxis, mousex, mousey );
    }
    else
    {
        InitiateRotateEdit( mousex, mousey );
        bAxisModifierOn = bAltAxes;
    }
}

void Editing3DClass::EditDone()
{
    CurrentEditType = EDITTYPE_NONE;
}

void Editing3DClass::DrawEditBarsToOpenGL()
{
    if( bShowEditBars )
    {
        SelectionIteratorTypedef iterator;
        iterator = Selector.SelectedObjects.begin();
        int iSelectedReference = iterator->second.iReference;
        Object *p_Object = World.GetObjectByReference( iSelectedReference );

        if( p_Object != NULL )
        {
            glPushMatrix();

            if( p_Object->iParentReference == 0 )
            {
                //p_Object->DrawSelected();
            }
            else
            {
                RendererImpl::SetupAxes( World.GetObjectByReference ( p_Object->iParentReference ) );
            }

            glTranslatef( p_Object->pos.x, p_Object->pos.y, p_Object->pos.z );
            mvGraphics.RotateToRot( p_Object->rot );

            Vector3 FaceCentreOffsets[6];

            Vector3 ScaleToUse;

            if( strcmp( p_Object->ObjectType, "PRIM" ) == 0 )
            {
                Prim *p_Prim = dynamic_cast< Prim *>( p_Object );
                ScaleToUse = p_Prim->scale;
            }
            else
            {
                ScaleToUse.x = 2.0;
                ScaleToUse.y = 2.0;
                ScaleToUse.z = 2.0;
            }

            switch( EditBarType )
            {
                case EDITBAR_POS:
                Editing3DPos.DrawEditHandles( ScaleToUse );
                break;

                case EDITBAR_SCALE:
                Editing3DScale.DrawEditHandles( ScaleToUse );
                break;

                case EDITBAR_ROT:
                Editing3DRot.DrawEditHandles( ScaleToUse );
                break;

            }

            glPopMatrix();
        }
    }
}

void Editing3DClass::ShowEditPosBars()
{
    EditBarType = EDITBAR_POS;
    bShowEditBars = true;
}

void Editing3DClass::ShowEditRotBars()
{
    EditBarType = EDITBAR_ROT;
    bShowEditBars = true;
}

void Editing3DClass::ShowEditScaleBars()
{
    EditBarType = EDITBAR_SCALE;
    bShowEditBars = true;
}

void Editing3DClass::HideEditBars()
{
    EditBarType = EDITBAR_NONE;
    bShowEditBars = false;
}
