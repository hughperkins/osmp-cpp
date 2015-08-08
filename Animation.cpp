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
//! \brief The animation module is responsible for making the world move.
//!
//! The animation module is responsible for making the world move.
//! See header file for documentation

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif


#ifdef _WIN32
#include <windows.h>
#endif

//#include <GL/gl.h>
//#include <GL/glu.h>
//#include <GL/glut.h>

#include <cstdlib>
#include <iostream>
#include <string.h>
#include <math.h>
#include <stdio.h>
using namespace std;

//#include "keyandmouse.h"
#include "WorldStorage.h"
#include "Constants.h"
#include "Animation.h"
#include "TickCount.h"

#include "Prim.h"

void Animation::RemoveMovingObject( const int iMoveArrayNum )
{
    DEBUG(  "RemoveMovingObject imovearraynum=" << iMoveArrayNum << " inummovingobjects=" << iNumMovingObjects ); // DEBUG
    MovingObjects[ iMoveArrayNum ] = MovingObjects[ iNumMovingObjects - 1 ];
    iNumMovingObjects--;
}

void Animation::RemoveFromMovingObjects( const int iReference )
{
    int iMovingObjectNum = 0;
    for( iMovingObjectNum = 0; iMovingObjectNum < iNumMovingObjects; iMovingObjectNum++ )
    {
        if( MovingObjects[ iMovingObjectNum ].iReference == iReference )
        {
            //          DEBUG( "Removing moving object num" << iMovingObjectNum  ); // DEBUG
            RemoveMovingObject( iMovingObjectNum );
            return;
        }
    }
}

const int Animation::ReferenceToMovingObjectArrayNum( const int iReference )
{
    int iMovingObjectNum = 0;
    for( iMovingObjectNum = 0; iMovingObjectNum < iNumMovingObjects; iMovingObjectNum++ )
    {
        if( MovingObjects[ iMovingObjectNum ].iReference == iReference )
        {
            return iMovingObjectNum;
        }
    }
    return -1;
}

void Animation::UpdateMovingObjectFromXML( const int iReference, const int iObjectArrayPos, const TiXmlElement *pElement )
{
    TiXmlHandle docHandle((TiXmlElement *)pElement );
    if( !docHandle.FirstChild("dynamics").FirstChild("duration").Element() )
    {
        return;
    }

    if( !docHandle.FirstChild("geometry").Element() )
    {
        return;
    }

    // DEBUG(  "inummovingobjects=" << iNumMovingObjects ); // DEBUG
    MovingObject *pMovingObject;
    int iMovingObjectArrayNum = ReferenceToMovingObjectArrayNum( iReference );
    //  if( !Selector.IsSelected( iReference ) )
    //  {
    if( iMovingObjectArrayNum == -1 )
    {
        DEBUG(  "Adding new moving object..." ); // DEBUG
        iMovingObjectArrayNum = iNumMovingObjects;
        pMovingObject = &(MovingObjects[ iMovingObjectArrayNum ]);
        pMovingObject->iReference = iReference;
        // pMovingObject->iObjectArrayNum = iObjectArrayPos;

        iNumMovingObjects++;
    }
    DEBUG(  "Populating moving object..." ); // DEBUG
    pMovingObject = &(MovingObjects[ iMovingObjectArrayNum ]);

    pMovingObject->bPosChange = false;
    pMovingObject->bRotChange = false;
    pMovingObject->bScaleChange = false;
    pMovingObject->bColorChange = false;

    const TiXmlElement *pDynamics = pElement->FirstChildElement("dynamics");
    pMovingObject->iStartTickCount = MVGetTickCount();
    //   DEBUG(  "Getting end tick count..." ); // DEBUG
    pMovingObject->iEndTickCount = MVGetTickCount() + atoi( pDynamics->FirstChildElement("duration")->Attribute("milliseconds") );

    //  DEBUG(  "Doing pos..." ); // DEBUG
    Object *pObject = World.GetObject( iObjectArrayPos );

    const TiXmlElement *pGeometry = pElement->FirstChildElement("geometry");
    //DEBUG(  "inummovingobjects=" << iNumMovingObjects ); // DEBUG
    if( pGeometry != NULL )
    {
        if( pGeometry->FirstChildElement("pos") != NULL )
        {
            //   DEBUG(  "pos change" ); // DEBUG
            pMovingObject->bPosChange = true;
            pMovingObject->StartPos = pObject->pos;
            pMovingObject->EndPos = Vector3( pElement->FirstChildElement("geometry")->FirstChildElement("pos") );
        }
        //   DEBUG(  "Doing rot..." ); // DEBUG
        //DEBUG(  "inummovingobjects=" << iNumMovingObjects ); // DEBUG
        if( pGeometry->FirstChildElement("rot") != NULL )
        {
            //   DEBUG(  "rot change" ); // DEBUG
            pMovingObject->bRotChange = true;
            pMovingObject->StartRot = pObject->rot;
            pMovingObject->EndRot = Rot( pElement->FirstChildElement("geometry")->FirstChildElement("rot") );

            Rot InverseEndRot;
            InverseRot( InverseEndRot, pMovingObject->EndRot );
            Rot RotFromEndToStart;
            RotMultiply( RotFromEndToStart, pMovingObject->StartRot, InverseEndRot );

            Rot2AxisAngle( pMovingObject->Axis, pMovingObject->fAngleFromEndToStart, RotFromEndToStart );

        }
        //  DEBUG(  "Doing scale..." ); // DEBUG
        // DEBUG(  "inummovingobjects=" << iNumMovingObjects ); // DEBUG
        if( pGeometry->FirstChildElement("scale") != NULL )
        {
            //    DEBUG(  "scale change" ); // DEBUG
            if( strcmp( pObject->ObjectType, "PRIM" ) == 0 )
            {
                pMovingObject->bScaleChange = true;
                pMovingObject->StartScale = dynamic_cast<Prim *>(pObject)->scale;
                pMovingObject->EndScale = Vector3( pElement->FirstChildElement("geometry")->FirstChildElement("scale") );
            }
        }
    }
    //  DEBUG(  "Doing color..." ); // DEBUG
    // DEBUG(  "inummovingobjects=" << iNumMovingObjects ); // DEBUG
    if( docHandle.FirstChild("faces").FirstChild("face").FirstChild("color").Element() )
    {
        if( strcmp( pObject->ObjectType, "PRIM" ) == 0 )
        {
            pMovingObject->StartColor = dynamic_cast<Prim *>(pObject)->GetColor(0);
            pMovingObject->bColorChange = true;
            pMovingObject->EndColor = Color( pElement->FirstChildElement("faces")->FirstChildElement("face")->FirstChildElement("color") );
        }
    }
    //     DEBUG(  "moving object updated" ); // DEBUG
    DEBUG(  "inummovingobjects=" << iNumMovingObjects ); // DEBUG
    //}
}

void Animation::MoveObjectFromXMLString( const char *XMLString )
{
    TiXmlDocument IPC;
    IPC.Parse( XMLString );
    MoveObject( IPC.RootElement() );
}

void Animation::MoveObject( const TiXmlElement *pElement )
{
    int iReference = atoi( pElement->Attribute("ireference" ) );
    int iObjectArrayPos = World.GetArrayNumForObjectReference( iReference );
    if( iObjectArrayPos != -1 )
    {
        UpdateMovingObjectFromXML( iReference, iObjectArrayPos, pElement );
        //World.p_Objects[ iObjectArrayPos ]->pos = Vector3::Vector3FromXMLElement( pIPC->RootElement()->FirstChildElement("geometry")->FirstChildElement("pos") );
    }
}

void Animation::MoveWorld()
{
    float fCurrentTickCount = (float)MVGetTickCount();
    int iMoveArrayNum;
    MovingObject *pMovingObject;

    // DEBUG(  "MoveWorld() start inummovingobjects=" << iNumMovingObjects ); // DEBUG
    for( iMoveArrayNum = 0; iMoveArrayNum < iNumMovingObjects; iMoveArrayNum++ )
    {
        //DEBUG(  "Moving object " << iNumMovingObjects ); // DEBUG
        pMovingObject = &MovingObjects[ iMoveArrayNum ];
        if( pMovingObject->iEndTickCount < (int)fCurrentTickCount )
        {
            DEBUG(  "Descheduling moving object " << iMoveArrayNum ); // DEBUG
            RemoveMovingObject( iMoveArrayNum );
            iMoveArrayNum--;
            //        DEBUG(  "Deschedule done " << iMoveArrayNum ); // DEBUG
        }
        else
        {
            int iObjectArrayNum = World.GetArrayNumForObjectReference( pMovingObject->iReference );
            if( iObjectArrayNum != -1 )
            {
                Object *pObject = World.GetObject( iObjectArrayNum );

                float fMultiplier = (fCurrentTickCount - (float)(pMovingObject->iStartTickCount)) / (float)(pMovingObject->iEndTickCount - pMovingObject->iStartTickCount);
                //  DEBUG(  "fMultiplier = " << fMultiplier ); // DEBUG
                if( pMovingObject->bPosChange )
                {
                    pObject->pos.x = ( 1 - fMultiplier ) * pMovingObject->StartPos.x + fMultiplier * pMovingObject->EndPos.x;
                    pObject->pos.y = ( 1 - fMultiplier ) * pMovingObject->StartPos.y + fMultiplier * pMovingObject->EndPos.y;
                    pObject->pos.z = ( 1 - fMultiplier ) * pMovingObject->StartPos.z + fMultiplier * pMovingObject->EndPos.z;
                }
                if( pMovingObject->bScaleChange )
                {
                    dynamic_cast<Prim *>(pObject)->scale.x = ( 1 - fMultiplier ) * pMovingObject->StartScale.x + fMultiplier * pMovingObject->EndScale.x;
                    dynamic_cast<Prim *>(pObject)->scale.y = ( 1 - fMultiplier ) * pMovingObject->StartScale.y + fMultiplier * pMovingObject->EndScale.y;
                    dynamic_cast<Prim *>(pObject)->scale.z = ( 1 - fMultiplier ) * pMovingObject->StartScale.z + fMultiplier * pMovingObject->EndScale.z;
                }
                if( pMovingObject->bRotChange )
                {
                    //  DEBUG(  "rot update\n" ); // DEBUG
                    Rot RotFromEndToCurrent;
                    AxisAngle2Rot( RotFromEndToCurrent,  pMovingObject->Axis, pMovingObject->fAngleFromEndToStart * ( 1 - fMultiplier ) );
                    RotMultiply( pObject->rot, RotFromEndToCurrent, pMovingObject->EndRot );
                    //  DEBUG(  "rot update done\n" ); // DEBUG
                }
                if( pMovingObject->bColorChange )
                {
                    if( strcmp( pObject->ObjectType, "PRIM" ) == 0 )
                    {
                        Color newcolor;
                        newcolor.r = ( 1 - fMultiplier ) * pMovingObject->StartColor.r + fMultiplier * pMovingObject->EndColor.r;
                        newcolor.g = ( 1 - fMultiplier ) * pMovingObject->StartColor.g + fMultiplier * pMovingObject->EndColor.g;
                        newcolor.b = ( 1 - fMultiplier ) * pMovingObject->StartColor.b + fMultiplier * pMovingObject->EndColor.b;

                        dynamic_cast<Prim *>(pObject)->SetColor( 0, newcolor );
                    }
                }
            }
        }
    }
    // DEBUG(  "MoveWorld()done" ); // DEBUG
}

void Animation::AnimateWorld()
{
    ThisTimeInterval = MVGetTickCount() - iLastElapsedTime;
    if (ThisTimeInterval == 0)
    {
        ThisTimeInterval = 1; // Not sure why this goes to 0.  10 seems to be lowest it can get on win32
    }
    iLastElapsedTime = MVGetTickCount();

    // MovePlayer();
    MoveWorld();
}
