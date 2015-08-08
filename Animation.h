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
//! the animation module is responsible for making the world move:
//!  - clientside retrospective interpolation of other avatar movements
//!  - prospective smooth interpolation of object movements wrt size, position, rotation and even colour
//!
//! animator uses, and manipulates the World object
//! some other objects (selection and keyandmouse) access objects in animator directlry, for now; probably should clean this up sometime

#ifndef _ANIMATION_H
#define _ANIMATION_H

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif


#ifdef _WIN32
#include <windows.h>
#endif

//#include <GL/glut.h>

#include <cstdlib>
#include <iostream>
#include <string.h>
#include <math.h>
#include <stdio.h>
using namespace std;

// #include "keyandmouse.h"
#include "WorldStorage.h"
#include "Math.h"
#include "TickCount.h"

//! a MovingObject struct holds the data for a single animated object (used by Animation)
struct MovingObject
{
    int iReference;  //!< Object's iReference number (unique within sim and database)
    //  int iObjectArrayNum;

    bool bPosChange;   //!< true if position is being animated
    Vector3 StartPos;     //!< start position for animation
    Vector3 EndPos;        //!< end position for animation

    bool bRotChange;   //!< true if rotation is being animated
    Rot StartRot;      //!< start rotation for rotation animation
    Rot EndRot;        //!< end rotation for rotation animation
    Vector3 Axis;       //!< axis of rotation for rotation animation
    float fAngleFromEndToStart; //!< Total angular change for rotation animation

    bool bScaleChange;  //!< true if scale is being animated
    Vector3 StartScale;   //!< scale before animation started
    Vector3 EndScale;     //!< scale once animation will have finished

    bool bColorChange;  //!< true if color is being animated
    Color StartColor;   //!< color before animation started
    Color EndColor;     //!< color after animatino will have finished

    int iStartTickCount; //!< tick count at start of animation (tickcount is basically a measure of time of day)
    int iEndTickCount;   //!< tick count when animation will have finished
};

//! Animation handles interpolated non-physical movement

//! Animation handles interpolated non-physical movement
//! the animation module is responsible for making the world move:
//!  - clientside retrospective interpolation of other avatar movements
//!  - prospective smooth interpolation of object movements wrt size, position, rotation and even colour
//!
//! animator uses, and manipulates the World object
//! some other objects (selection and keyandmouse) access objects in animator directlry, for now; probably should clean this up sometime
//!
//! How to use this class:
//! - in constructor, pass in a reference to your mvWorldStorage object
//! - call MoveObject for each animated object; passing in an XML element with structure:
//!
//!   <pre>
//!   <someelement>
//!   <dynamics>
//!   <duration milliseconds="..."/>
//!   </dynamics>
//!   <geometry>
//!   <pos x="..." y="..." z="..."/>  (optional)
//!   <scale x="..." y="..." z="..."/>  (optional)
//!   <color r="..." g="..." b="..."/>  (optional)
//!   <rot x="..." y="..." z="..." s="..."/>  (optional)
//!   </geometry>
//!   </someelement>
//!   </pre>
//!
//! - each display/animation frame, call AnimateWorld to animate the world; animated objects will be removed
//!   from MovingObjects list automatically once their animation has finished
//! - you can call RemoveFromMovingObjects to stop animating an object, passing in the iReference of the object
//! - you can call GetThisTimeInterval to obtain ThisTimeInterval
class Animation
{
public:
    Animation( mvWorldStorage &WorldStorage ) :
            World( WorldStorage )
    {
        iNumMovingObjects = 0;
        ThisTimeInterval = 0;
        iLastElapsedTime = MVGetTickCount();
        //World = WorldStorage;
    }

    void MoveObjectFromXMLString( const char *XMLString );         //!< call this to signal a new animated object, input is an XML objectmove command
    void MoveObject( const TiXmlElement *pElement );         //!< call this to signal a new animated object, input is an XML objectmove command
    void AnimateWorld();                                   //!< call this to animate world for one frame

    void RemoveFromMovingObjects( const int iReference );  //!< removes an animated object given its iReference
    //! Obtain ThisTimeInterval
    int GetThisTimeInterval()
    {
        return ThisTimeInterval;
    }

    //! Obtain number of moving objects
    int GetNumMovingObjects()
    {
        return iNumMovingObjects;
    }

    //! Obtain iReference for moving object number iMovingObjectNumber
    int GetMovingObjectiReference( int iMovingObjectNumber )
    {
        return MovingObjects[ iMovingObjectNumber ].iReference;
    }

protected:
    mvWorldStorage &World;  //!< Reference to world storage object, which stores all objects in world

    long ThisTimeInterval;  //!< Length of time since last animation frame, in milliseconds
    long iLastElapsedTime;  //!< Ticktime for last frame (measure of time of day)

    int iNumMovingObjects;   //!< Number of animated objects currently
    MovingObject MovingObjects[200];  //!< Stores movement properties for all animated objects

    void RemoveMovingObject( const int iMoveArrayNum );  //!< Removes an animated object given its array number
    const int ReferenceToMovingObjectArrayNum( const int iReference ); //!< Obtains the animated object's array number, given the object's iReference number
    void UpdateMovingObjectFromXML( const int iReference, const int iObjectArrayPos, const TiXmlElement *pElement );   //!< Updates the movement properties of an object from a passed-in XML message
    void MoveWorld();  //!< Runs one frame of animation; moving all objects to next position
    void MovePlayer(); //!< Erm, that's strange; this function doesnt actually exist :-O

};

#endif // _ANIMATION_H
