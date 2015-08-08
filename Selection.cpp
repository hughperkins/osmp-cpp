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
//! \brief mvSelection handles object selection within the 3d renderer environment
// see header file for documentation

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
//#include <GL/glut.h>

#include <cstdlib>
#include <iostream>
#include <string.h>
#include <math.h>
#include <stdio.h>
using namespace std;

#include "Diag.h"
//#include "keyandmouse.h"
#include "WorldStorage.h"
#include "Selection.h"
//#include "MetaverseClient.h"
#include "RendererImpl.h"

namespace MetaverseClient
{
    extern int iMyReference;
    extern mvWorldStorage World;
}
using namespace MetaverseClient;


namespace RendererImpl
{
    extern int iViewPoint;
}

typedef pair <int, SELECTION> selection_pair;

void mvSelection::RemoveFromSelectedObjects( int iReference )
{
    SelectedObjects.erase( iReference );
}

bool mvSelection::IsPartOfAvatar( int iReference )
{
    DEBUG(  "IsPartOfAvatar() " << iReference ); // DEBUG
    Object *p_Object = World.GetObjectByReference( iReference );
    if( p_Object != NULL )
    {
        DEBUG(  "object type: " << p_Object->sDeepObjectType << " " << p_Object->iReference << " parentref: " << p_Object->iParentReference ); // DEBUG;
        if( p_Object->iParentReference == 0 )
        {
            DEBUG(  "IsPartOfAvatar() doing comparison " << ( p_Object->iReference == MetaverseClient::iMyReference ) << " " << p_Object->iReference << " " << MetaverseClient::iMyReference );
            return( p_Object->iReference == MetaverseClient::iMyReference );
        }
        else
        {
            DEBUG(  "IsPartOfAvatar() looking up parent " << p_Object->iParentReference ); // DEBUG
            return IsPartOfAvatar( p_Object->iParentReference );
        }
    }
    else
    {
        DEBUG(  "no object found!" ); // DEBUG
        return false;
    }
}

// Based on function from http://nehe.gamedev.net
HITTARGET *mvSelection::GetClickedHitTarget( int iWindowX, int iWindowY )
{
    Debug( "GetSelectedObject()\n" );
    GLuint buffer[512];          // Set Up A Selection Buffer
    GLint hits;            // The Number Of Objects That We Selected

    // The Size Of The Viewport. [0] Is <x>, [1] Is <y>, [2] Is <length>, [3] Is <width>
    GLint viewport[4];

    // This Sets The Array <viewport> To The Size And Location Of The Screen Relative To The Window
    glGetIntegerv(GL_VIEWPORT, viewport);
    glSelectBuffer(512, buffer);        // Tell OpenGL To Use Our Array For Selection

    // Puts OpenGL In Selection Mode. Nothing Will Be Drawn.  Object ID's and Extents Are Stored In The Buffer.
    (void) glRenderMode(GL_SELECT);

    glInitNames();            // Initializes The Name Stack
    glPushName(0);            // Push 0 (At Least One Entry) Onto The Stack

    glMatrixMode(GL_PROJECTION);        // Selects The Projection Matrix
    glPushMatrix();            // Push The Projection Matrix
    glLoadIdentity();           // Resets The Matrix

    // This Creates A Matrix That Will Zoom Up To A Small Portion Of The Screen, Where The Mouse Is.
    gluPickMatrix((GLdouble) iWindowX, (GLdouble) (viewport[3] - iWindowY ), 1.0f, 1.0f, viewport);

    // Apply The Perspective Matrix
    gluPerspective(45.0f, (GLfloat) (viewport[2]-viewport[0])/(GLfloat) (viewport[3]-viewport[1]), 0.1f, 100.0f);
    //  gluPerspective( 45.0, aspect, 1.5, 100.0 );
    //   glFrustum (-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);
    glMatrixMode(GL_MODELVIEW);         // Select The Modelview Matrix
    RendererImpl::DrawWorld();            // Render The Targets To The Selection Buffer
    glMatrixMode(GL_PROJECTION);        // Select The Projection Matrix
    glPopMatrix();            // Pop The Projection Matrix
    glMatrixMode(GL_MODELVIEW);         // Select The Modelview Matrix
    hits=glRenderMode(GL_RENDER);        // Switch To Render Mode, Find Out How Many
    // Objects Were Drawn Where The Mouse Was
    if (hits > 0 )            // If There Were More Than 0 Hits
    {
        DEBUG( "hit" );
        //int choose = buffer[3];         // Make Our Selection The First Object
        //int depth = buffer[1];         // Store How Far Away It Is
        bool bObjectFound = false;
        bool bFinishScan = false;
        int iLoopChoose = 0;
        int choose = 0;
        int depth = 0;
        for (int loop = 0; loop < hits && !bFinishScan; loop++)     // Loop Through All The Detected Hits
        {
            iLoopChoose = buffer[loop*4+3];
            DEBUG(  "choose " << loop << " TargetType: " << HitTargets[ iLoopChoose ].TargetType << " foreignref: " << HitTargets[ iLoopChoose ].iForeignReference ); // DEBUG
            if ( !bObjectFound || buffer[loop*4+1] < GLuint(depth) )
            {
                switch( HitTargets[ iLoopChoose ].TargetType )
                {
                    case HITTARGETTYPE_LANDCOORD:
                    DEBUG(  "object marked as currently selected object" ); // DEBUG
                    choose = buffer[loop*4+3];      // Select The Closer Object
                    depth = buffer[loop*4+1];      // Store How Far Away It Is
                    bObjectFound = true;
                    break;

                    case HITTARGETTYPE_OBJECT:
                    if( !IsPartOfAvatar( HitTargets[ iLoopChoose ].iForeignReference )
                            || RendererImpl::iViewPoint != RendererImpl::VIEWPOINT_MOUSELOOK )
                    {
                        DEBUG(  "object marked as currently selected object" ); // DEBUG
                        choose = buffer[loop*4+3];      // Select The Closer Object
                        depth = buffer[loop*4+1];      // Store How Far Away It Is
                        bObjectFound = true;
                    }
                    break;

                    case HITTARGETTYPE_EDITHANDLE:  // always select this over any other object
                    choose = buffer[loop*4+3];
                    depth = buffer[loop*4+1];
                    bObjectFound = true;
                    bFinishScan = true;
                    break;
                }
            }
        }
        if( bObjectFound )
        {
            return &( HitTargets[ choose ] );
        }
        else
        {
            return NULL;
        }
    }
    return NULL;
}

void mvSelection::ToggleObjectInSelection( int iReference, bool bSelectParentObject )
{
    DEBUG(  "trying to toggle selection for object refrenence " << iReference );
    int iObjectArrayNum = World.GetArrayNumForObjectReference( iReference );
    if( iObjectArrayNum != -1 )
    {
        int iParentReference = World.GetObject( iObjectArrayNum )->iParentReference;
        if( iParentReference != 0 && bSelectParentObject )
        {
            DEBUG(  "This object has a parent " << iParentReference << " trying that..." );
            ToggleObjectInSelection( iParentReference, true );
        }
        else
        {
            // check it's not an avatar, or, if it is, that its us :-O
            if( strcmp( World.GetObject( iObjectArrayNum )->sDeepObjectType, "AVATAR" ) != 0 || World.GetObject( iObjectArrayNum )->iReference == MetaverseClient::iMyReference )
            {
                SelectedObjectIterator = SelectedObjects.find( iReference );
                if( SelectedObjectIterator != SelectedObjects.end() )
                {
                    SelectedObjects.erase( iReference );
                    DEBUG( "Removing object from selection" );
                }
                else
                {
                    SELECTION NewSelection;
                    NewSelection.iReference = iReference;
                    DEBUG( "Adding object to selection" );
                    SelectedObjects.insert( selection_pair ( iReference, NewSelection ) );
                    DEBUG( "object added" );
                }
            }
        }
    }
}

bool mvSelection::IsSelected( int iReference )
{
    DEBUG(  "IsSelected()" ); // DEBUG
    SelectedObjectIterator = SelectedObjects.find( iReference );
    if( SelectedObjectIterator != SelectedObjects.end() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

// remove stuff that has a parent now (its linked) or doesnt exist any more
void mvSelection::PurgeThingsThatShouldntBeHere()
{
    bool bNothingDone = true;
    int iSelectedObjectWorldArrayNum;
    SelectionIteratorTypedef iterator;
    DEBUG(  "purge" ); // DEBUG
    for ( iterator = SelectedObjects.begin( ) ; iterator != SelectedObjects.end( ); iterator++ )
    {
        DEBUG(  "loop" ); // DEBUG
        DEBUG(  iterator->second.iReference ); // DEBUG
        iSelectedObjectWorldArrayNum = World.GetArrayNumForObjectReference( iterator->second.iReference );

        if( iSelectedObjectWorldArrayNum != -1 )
        {}
        else
        {
            DEBUG(  "purging object because doesnt exist " ); // DEBUG
            SelectedObjects.erase( iterator );
            iterator++;
            bNothingDone = false;
        }
    }
    if( !bNothingDone )
    {
        PurgeThingsThatShouldntBeHere();
    }
    DEBUG(  "purge done" ); // DEBUG
}

void mvSelection::ToggleClickedInSelection( bool bSelectParentObject, int iMouseX, int iMouseY )
{
    int iClickedReference;
    if( bSelectParentObject )
    {
        iClickedReference = GetClickedTopLevelObjectReference( iMouseX, iMouseY );
    }
    else
    {
        iClickedReference = GetClickedPrimReference( iMouseX, iMouseY );
    }

    if( iClickedReference != -1 )
    {
        DEBUG(  "selected has reference " << iClickedReference );
        ToggleObjectInSelection( iClickedReference, bSelectParentObject );
    }

    PurgeThingsThatShouldntBeHere();
}

int mvSelection::GetNumSelected()
{
    return SelectedObjects.size();
}

int mvSelection::GetClickedTopLevelObjectReference( int iMouseX, int iMouseY )
{
    int iClickedPrim = GetClickedPrimReference( iMouseX, iMouseY );
    return World.GetTopLevelParentReference( iClickedPrim );
}

int mvSelection::GetClickedPrimReference( int iMouseX, int iMouseY )
{
    HITTARGET *pHitTarget = GetClickedHitTarget( iMouseX, iMouseY );

    if( pHitTarget != NULL )
    {
        if( pHitTarget->TargetType == HITTARGETTYPE_OBJECT )
        {
            DEBUG(  "selected has reference " << pHitTarget->iForeignReference );
            return pHitTarget->iForeignReference;
        }
    }

    return 0;
}

void mvSelection::AddHitTarget( enumHitTargetType TargetType, int iForeignReference )
{
    //HITTARGET NewHitTarget;
    HitTargets[ iTargetReference ].TargetType = TargetType;
    HitTargets[ iTargetReference ].iForeignReference = iForeignReference;
    glLoadName( iTargetReference );
    iTargetReference++;
}

void mvSelection::Clear()
{
    SelectedObjects.clear();
}

vector<int> mvSelection::GetSelectedObjectReferences()
{
    vector<int> iReferences;
    iReferences.clear();

    SelectionIteratorTypedef it;
    for( it = SelectedObjects.begin(); it != SelectedObjects.end(); it++ )
    {
        iReferences.push_back( it->second.iReference );
    }
    return iReferences;
}

