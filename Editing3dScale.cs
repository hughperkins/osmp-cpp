// Copyright Hugh Perkins 2004,2005,2006
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License version 2 as published by the
// Free Software Foundation;
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
//  more details.
//
// You should have received a copy of the GNU General Public License along
// with this program in the file licence.txt; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-
// 1307 USA
// You can find the licence also on the web at:
// http://www.opensource.org/licenses/gpl-license.php
//

using System;
using System.Collections;
using System.Windows.Forms;

//! \file
//! \brief Manages 3d Scale editing

namespace OSMP
{
        
        void InteractiveHandleEdit( Editing3d.AxisType axis, int mousex, int mousey )
        {
            // DEBUG(  "InteractiveHandleScaleEdit" ); // DEBUG
            Entity entity = selector.GetFirstSelectedEntity();
        
            Vector3 OurPos = null;
            Rot OurRot = null;
        
            if( camera.bCameraEnabled )
            {
                OurPos = camera.CurrentCameraPos;
                OurRot = camera.CurrentCameraRot;
            }
            else
            {
                Avatar ouravatar = MetaverseClient.GetInstance().myavatar;
                if( ouravatar != null )
                {
                    OurPos = ouravatar.pos;
                    OurRot = ouravatar.rot;
                }
                else
                {
                    return;
                }
            }
        
            if( entity != null )
            {
                float fDistanceFromUsToEntity = ( entity.pos - OurPos ).Det();
    
                // DEBUG(  "InteractiveHandleScaleEdit 1" ); // DEBUG
                float fScalingFromPosToScreen = graphics.GetScalingFrom3DToScreen( fDistanceFromUsToEntity );
    
                // DEBUG(  "InteractiveHandleScaleEdit 2" ); // DEBUG
    
                Vector3 ScreenMouseVector = new Vector3(
                    0,
                    ScreenMouseVector.y = - ( (float)( mousex - editing3d.iDragStartX) ),
                    ScreenMouseVector.z = - ( (float)( mousey - editing3d.iDragStartY ) );
    
                //  DEBUG(  "InteractiveHandleScaleEdit 3" ); // DEBUG
                Vector3 PosMouseVectorAvAxes = ScreenMouseVector * ( 1 / fScalingFromPosToScreen );
                // DEBUG(  "InteractiveHandleScaleEdit 4" ); // DEBUG
    
                Rot rInverseOurRot;
                InverseRot( rInverseOurRot, OurRot );
    
                Vector3 PosMouseVectorWorldAxes = PosMouseVectorAvAxes * rInverseOurRot;    
                Vector3 PosMouseVectorEntityAxes = PosMouseVectorWorldAxes * entity.rot;
                //   DEBUG(  "screen vector: " << ScreenMouseVector << " PosMouseVectorAvAxes " << PosMouseVectorAvAxes <<
                //      " posmousevectorworldaxes: " << PosMouseVectorWorldAxes << " PosMouseVectorEntityAxes " << PosMouseVectorEntityAxes ); // DEBUG
    
                Vector3 vScaleChangeVectorEntityAxes = null;
                float fDistanceToScale;
    
                bool bPositiveAxis = false;
    
                //   DEBUG(  "InteractiveHandleScaleEdit 5" ); // DEBUG
                switch( axis )
                {
                    case Editing3d.AxisType.PosX:
                    //   DEBUG(  "x +ve" ); // DEBUG
                    vScaleChangeVectorEntityAxes.x = Vector3.DotProduct( PosMouseVectorEntityAxes, XAXIS );
                    bPositiveAxis = true;
                    break;
    
                    case Editing3d.AxisType.NegX:
                    //DEBUG(  "x -ve" ); // DEBUG
                    vScaleChangeVectorEntityAxes.x = Vector3.DotProduct( PosMouseVectorEntityAxes, XAXIS );
                    break;
    
                    case Editing3d.AxisType.PosY:
                    //DEBUG(  "y +ve" ); // DEBUG
                    vScaleChangeVectorEntityAxes.y = Vector3.DotProduct( PosMouseVectorEntityAxes, YAXIS );
                    break;
    
                    case Editing3d.AxisType.NegY:
                    // DEBUG(  "y -ve" ); // DEBUG
                    bPositiveAxis = true;
                    vScaleChangeVectorEntityAxes.y = Vector3.DotProduct( PosMouseVectorEntityAxes, YAXIS );
                    break;
    
                    case Editing3d.AxisType.PosZ:
                    // DEBUG(  "z +ve" ); // DEBUG
                    bPositiveAxis = true;
                    vScaleChangeVectorEntityAxes.z = Vector3.DotProduct( PosMouseVectorEntityAxes, ZAXIS );
                    break;
    
                    case Editing3d.AxisType.NegZ:
                    // DEBUG(  "z -ve" ); // DEBUG
                    vScaleChangeVectorEntityAxes.z = Vector3.DotProduct( PosMouseVectorEntityAxes, ZAXIS );
                    break;
    
                }
    
                Vector3 vNewScale = null;
                if( bPositiveAxis )
                {
                    vNewScale = editing3d.StartScale + vScaleChangeVectorEntityAxes;
                }
                else
                {
                    vNewScale = editing3d.StartScale - vScaleChangeVectorEntityAxes;
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
    
                Vector3 vTranslate = ( vNewScale - editing3d.StartScale ) * 0.5f;    
                entity.scale = vNewScale;
    
                //    DEBUG(  "vScaleChangeVectorEntityAxes " << vScaleChangeVectorEntityAxes << " editing3d.StartScale "
                //       << editing3d.StartScale << " vNewScale " << vNewScale << " vTranslate " << vTranslate ); // DEBUG
    
                Rot InverseEntityRot = entity.rot.Inverse();
                Vector3 vTranslationVectorWorldAxes = vTranslate * InverseEntityRot;
                //   DEBUG(  "InteractiveHandleScaleEdit 6" ); // DEBUG
                Vector3 PosTranslationVectorWorldAxes = vTranslationVectorWorldAxes;
    
                if( bPositiveAxis )
                {
                    entity.pos = editing3d.StartPos + PosTranslationVectorWorldAxes;
                }
                else
                {
                    entity.pos = editing3d.StartPos - PosTranslationVectorWorldAxes;
                }
            }
            //   DEBUG(  "InteractiveHandleScaleEdit done" ); // DEBUG
        }
        
        void InteractiveFreeEdit( int mousex, int mousey )
        {
            // DEBUG(  "InteractiveHandleScaleEdit" ); // DEBUG
            Entity entity = selector.GetFirstSelectedEntity();
        
            Vector3 OurPos;
            Rot OurRot;
        
            if( camera.bCameraEnabled )
            {
                OurPos = camera.CurrentCameraPos;
                OurRot = camera.CurrentCameraRot;
            }
            else
            {
                Entity *entity = World.GetEntityByReference( MetaverseClient::iMyReference );
                if( entity != null )
                {
                    OurPos = entity.pos;
                    OurRot = entity.rot;
                }
                else
                {
                    return;
                }
            }
        
            if( entity != null )
            {
                if( strcmp( entity.EntityType, "PRIM" ) == 0 )
                {
                    float fDistanceFromUsToEntity = VectorMag( entity.pos - OurPos );
        
                    // DEBUG(  "InteractiveHandleScaleEdit 1" ); // DEBUG
                    float fScalingFromPosToScreen = graphics.GetScalingFrom3DToScreen( fDistanceFromUsToEntity );
        
                    // DEBUG(  "InteractiveHandleScaleEdit 2" ); // DEBUG
        
                    Vector3 ScreenMouseVector;
                    ScreenMouseVector.y = - ( (float)(mousex - editing3d.iDragStartX) );
                    ScreenMouseVector.z = - ( (float)( mousey - editing3d.iDragStartY ) );
                    ScreenMouseVector.x = 0;
        
                    //  DEBUG(  "InteractiveHandleScaleEdit 3" ); // DEBUG
                    Vector3 PosMouseVectorAvAxes = ScreenMouseVector * ( 1 / fScalingFromPosToScreen );
                    // DEBUG(  "InteractiveHandleScaleEdit 4" ); // DEBUG
        
                    Rot rInverseOurRot;
                    InverseRot( rInverseOurRot, OurRot );
        
                    Vector3 PosMouseVectorWorldAxes;
                    PosMouseVectorWorldAxes = PosMouseVectorAvAxes * rInverseOurRot;
                    // MultiplyVectorByRot( PosMouseVectorWorldAxes, rInverseOurRot, PosMouseVectorAvAxes );
        
                    Vector3 PosMouseVectorEntityAxes;
                    PosMouseVectorEntityAxes = PosMouseVectorWorldAxes * entity.rot;
                    // MultiplyVectorByRot( PosMouseVectorEntityAxes, entity.rot, PosMouseVectorWorldAxes );
                    //   DEBUG(  "screen vector: " << ScreenMouseVector << " PosMouseVectorAvAxes " << PosMouseVectorAvAxes <<
                    //      " posmousevectorworldaxes: " << PosMouseVectorWorldAxes << " PosMouseVectorEntityAxes " << PosMouseVectorEntityAxes ); // DEBUG
        
                    //Vector3 vTranslationVectorEntityAxes;
                    Vector3 vScaleChangeVectorEntityAxes;
                    float fDistanceToScale;
        
                    bool bPositiveAxis = false;
        
                    vScaleChangeVectorEntityAxes = PosMouseVectorEntityAxes;
        
                    Vector3 vNewScale;
                    vNewScale = Vector3( editing3d.StartScale ) + Vector3( vScaleChangeVectorEntityAxes );
        
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
                    //        vTranslate = ( Vector3( vNewScale ) - Vector3( editing3d.StartScale ) ) * 0.5f;
        
                    dynamic_cast< Prim *>( entity ).scale = vNewScale;
                }
            }
            //   DEBUG(  "InteractiveHandleScaleEdit done" ); // DEBUG
        }
        
        void _InteractiveFreeScaleEdit_Old( bool bAltAxes, int x, int y )
        {
            Vector3 OurPos = null;
            Rot OurRot = null;
        
            if( camera.bCameraEnabled )
            {
                OurPos = camera.CurrentCameraPos;
                OurRot = camera.CurrentCameraRot;
            }
            else
            {
                Avatar ouravatar = MetaverseClient.GetInstance().myavatar;
                if( ouravatar != null )
                {
                    OurPos = ouravatar.pos;
                    OurRot = ouravatar.rot;
                }
                else
                {
                    return;
                }
            }
        
            Rot rInverseOurRot = OurRot.Inverse();
        
            Entity entity = selector.GetFirstSelectedEntity();
        
            // DEBUG(  "interactive scale edit objectype=[" << World.GetEntity( iSelectedArrayNum ).EntityType << "]" ); // DEBUG
            float HalfWinWidth = RendererFactory.GetInstance().WindowWidth / 2;
            float HalfWinHeight = RendererFactory.GetInstance().WindowHeight / 2;
    
            Vector3 ScaleAvAxes;
            if( bAltAxes )
            {
                ScaleAvAxes.y = ((float)(x - editing3d.iDragStartX)) / HalfWinWidth * 1.0;
                ScaleAvAxes.x = - ((float)(y - editing3d.iDragStartY)) / HalfWinWidth * 1.0;
                ScaleAvAxes.z = 0;
            }
            else
            {
                ScaleAvAxes.y = ((float)(x - editing3d.iDragStartX)) / HalfWinWidth * 1.0;
                ScaleAvAxes.z = - ((float)(y - editing3d.iDragStartY)) / HalfWinWidth * 1.0;
                ScaleAvAxes.x = 0;
            }
    
            Rot PrimRot = entity.rot;
            Rot InversePrimRot = PrimRot.Inverse();
    
            Vector3 CurrentScaleWorldAxes = editing3d.StartScale * rInverseOurRot;
            Vector3 CurrentScaleAvatarAxes = CurrentScaleWorldAxes * OurRot;
    
            Vector3 ScaleNewScaleAvAxes = new Vector3(
                ( 1 + ScaleAvAxes.x ) * CurrentScaleAvatarAxes.x,
                ( 1 + ScaleAvAxes.y ) * CurrentScaleAvatarAxes.y,
                ( 1 + ScaleAvAxes.z ) * CurrentScaleAvatarAxes.z );
    
            Vector3 NewScaleWorldAxes = ScaleNewScaleAvAxes * rInverseOurRot;
            Vector3 NewScaleSeenByPrim = NewScaleWorldAxes * PrimRot;
    
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
    
            entity.scale = NewScaleSeenByPrim;
            // DEBUG(  "setting new scale " << NewScaleSeenByPrim ); // DEBUG
        }
        
        void InitiateFreeEdit(int mousex, int mousey )
        {
            editing3d.EditingPreliminaries();
        
            Entity entity = selector.GetFirstSelectedEntity();
        
            if( entity != null )
            {
                editing3d.iDragStartX = mousex;
                editing3d.iDragStartY = mousey;
                editing3d.StartScale = entity.scale;
                editing3d.CurrentEditType = Editing3d.EditType.ScaleFree;
            }
            //DEBUG(  "initializing StartPos " << editing3d.StartScale ); // DEBUG
        }
        
        void InitiateHandleEdit( int mousex, int mousey, Editing3d.AxisType Axis )
        {
            editing3d.EditingPreliminaries();
        
            Entity entity = selector.GetFirstSelectedEntity();
        
            if( entity != null )
            {
                editing3d.iDragStartX = mousex;
                editing3d.iDragStartY = mousey;
                editing3d.StartScale = entity.scale;
                editing3d.StartPos = entity.pos;
                //DEBUG(  "initializing editing3d.StartScale " << editing3d.StartScale ); // DEBUG
    
                editing3d.CurrentAxis = Axis;
                editing3d.CurrentEditType = Editing3d.EditType.ScaleHandle;
            }
        }
        
        void DrawEditHandles( const Vector3 &ScaleToUse )
        {
            graphics.SetColor( 1.0, 0.0, 0.0 );
        
            Vector3 HandleScale( 0.14, 0.14, 0.14 );
        
            // + x
            selector.AddHitTarget( new HitTargetEditHandle( Editing3d.AxisType.PosX ) );
            graphics.PushMatrix();
            graphics.Translate( ScaleToUse.x / 2, 0, 0 );
            graphics.Scale( HandleScale.x, HandleScale.y, HandleScale.z );
            graphics.Rotate( 90, 0, 1, 0 );
            graphics.DoCube();
            graphics.PopMatrix();
        
            // - x
            selector.AddHitTarget( new HitTargetEditHandle( Editing3d.AxisType.NegX ) );
            graphics.PushMatrix();
            graphics.Translate( - ScaleToUse.x / 2, 0, 0 );
            graphics.Scale( HandleScale.x, HandleScale.y, HandleScale.z );
            graphics.Rotate( -90, 0, 1, 0 );
            graphics.DoCube();
            graphics.PopMatrix();
        
            graphics.SetColor( 0.0, 1.0, 0.0 );
        
            // + y
            selector.AddHitTarget( new HitTargetEditHandle( Editing3d.AxisType.PosY ) );
            graphics.PushMatrix();
            graphics.Translate( 0, - ScaleToUse.y / 2, 0 );
            graphics.Scale( HandleScale.x, HandleScale.y, HandleScale.z );
            graphics.Rotate( 90, 1, 0, 0 );
            graphics.DoCube();
            graphics.PopMatrix();
        
            // - y
            selector.AddHitTarget( new HitTargetEditHandle( Editing3d.AxisType.NegY ) );
            graphics.PushMatrix();
            graphics.Translate( 0, ScaleToUse.y / 2, 0 );
            graphics.Scale( HandleScale.x, HandleScale.y, HandleScale.z );
            graphics.Rotate( -90, 1, 0, 0 );
            graphics.DoCube();
            graphics.PopMatrix();
        
            graphics.SetColor( 0.0, 0.0, 1.0 );
        
            // + z
            selector.AddHitTarget( new HitTargetEditHandle( Editing3d.AxisType.PosZ ) );
            graphics.PushMatrix();
            graphics.Translate( 0, 0, ScaleToUse.z / 2 );
            graphics.Scale( HandleScale.x, HandleScale.y, HandleScale.z );
            //graphics.Rotate( 90, 0, 1, 0 );
            graphics.DoCube();
            graphics.PopMatrix();
        
            // - z
            selector.AddHitTarget( new HitTargetEditHandle( Editing3d.AxisType.NegZ ) );
            graphics.PushMatrix();
            graphics.Translate( 0, 0, - ScaleToUse.z / 2 );
            graphics.Scale( HandleScale.x, HandleScale.y, HandleScale.z );
            graphics.Rotate( 180, 0, 1, 0 );
            graphics.DoCube();
            graphics.PopMatrix();
        }
    }
}
