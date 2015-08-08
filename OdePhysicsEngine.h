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
//! \brief odephysicsengine handles liaison with the ode scripting engine

#ifndef _ODEPHYSICSENGINE_H
#define _ODEPHYSICSENGINE_H

#include "ode/ode.h"
#include "BasicTypes.h"
#include "Object.h"
#include "ObjectGrouping.h"
#include "WorldStorage.h"

#if defined(_WIN32) && !defined(BUILDING_PYTHONINTERFACES)
 #ifdef BUILDING_PHYSICSENGINE
    #define EXPCL_PHYSICSENGINE __declspec(dllexport)
 #else
    #define EXPCL_PHYSICSENGINE __declspec(dllimport)
 #endif
#else
  #define EXPCL_PHYSICSENGINE
#endif

#ifndef BUILDING_PYTHONINTERFACES
//! Stores triangles for one 128 x 128 terrain, used in odephysicsengine.cpp
class TerrainTriangleData
{
public:
    dTriMeshDataID dataid;
    dVector3 Size;
    dVector3 Vertices[ 129 * 129 * 3];
    int Indices[128 * 128 * 2 * 3];
    int iNumIndices;
    bool bTrianglesInitialized;
};
#endif

//! Information about a single object in ode, using in odephysicsengine.cpp

//! Information about a single object in ode, associating body, gemo, p_Object and so on
#ifndef BUILDING_PYTHONINTERFACES
struct OdeObject
{
    dBodyID body;
    dGeomID geom;
    bool bPhysics;
    dMass fMass;
    bool bEnabled;
    int iReference;
    Object *p_Object;
};
#endif

class EXPCL_PHYSICSENGINE CollisionAndPhysicsEngineClass
{
public:
    CollisionAndPhysicsEngineClass();
    ~CollisionAndPhysicsEngineClass();

    virtual void HandleCollisionsAndPhysics( int iElapsedTimeMilliseconds, mvWorldStorage &World, COLLISION Collisions[], int &ColiisionBufSize, char sSkyboxReference[33], int myRef);  //!< call this function to handle one collision/physics frame, passing in World and the time since last frame
    virtual void Init();        //!< call once to initialize dll
    virtual void Cleanup();      //!< call once at end to cleanup

    virtual void ObjectCreate( Object *p_Object );      //!< Signal new object
    virtual void ObjectModify( Object *p_Object );      //!< Signal destroyed object
    virtual void ObjectDestroy( int iReference );      //!< Signal destroyed object

    virtual bool CollideSphereAsRayWithObject( Vector3 &rPosCollidePos, const Vector3 PosRayOrigin, const Vector3 vRayVector, const float fSphereRadius, const Object *p_Object );  // Collides a ray with an object, for example to see intersect your mouse pointer with an object when you click on it, to find out where you clicked
protected:
#ifndef BUILDING_PYTHONINTERFACES

    static dWorldID world;
    static dSpaceID permaspace;
    static dJointGroupID contactgroup;

    static int myRefOde;
    static char skyboxOde[33];

    static float fNearestRayIntersectDistance;
    static bool bRayCollided;
    static Vector3 RayNearestPos;

    static bool CollisionFlag;
    static int CollisionBufSizeOde;
    static COLLISION CollisionsOde[2048];

    TerrainTriangleData Terrains[5];
    int iNumTerrains;

    int iDisplayCount; // just used so we dont spam output when displaying diag suff

    map < int, OdeObject, less< int > > PhysicalObjects;
    map < int, OdeObject, less< int > > StaticObjects;
    //OdeObject OdeObjects[ NUM ];
    //int iNumOdeObjects = 0;

    static void nearCallback (void *data, dGeomID o1, dGeomID o2);
    static void RayCallback(void *data, dGeomID o1, dGeomID o2);

    virtual void mvRotToOdeQuaternion( dQuaternion &OdeQuat, const Rot &mvQuat );
    virtual void CreateSphere( dSpaceID targetspace, OdeObject &rOdeObject, const Prim *p_Prim, const Vector3 &TranslationContext, const Rot &RotationContext );
    virtual void CreateCube( dSpaceID targetspace, OdeObject &rOdeObject, const Prim *p_Prim, const Vector3 &TranslationContext, const Rot &RotationContext );
    virtual void CreateCylinder( dSpaceID targetspace, OdeObject &rOdeObject, const Prim *p_Prim, const Vector3 &TranslationContext, const Rot &RotationContext );
    virtual void CreateGeom( dSpaceID targetspace, const Object *p_Object, const Vector3 &TranslationContext, const Rot &RotationContext );
    virtual void DeleteGeom( int iReference );
    virtual void CreatePhysicalGeom( OdeObject &rOdeObject, const Object *p_Object );
    virtual void SimulationLoop( float fTimeStepSeconds );
#endif
};

/*
// Note: Please dont add include statements here!  DYNDEF function declarations only
// More documentation in odephysicsengine.cpp

EXTERN void DYNDEF(HandleCollisionsAndPhysics)( int iElapsedTimeMilliseconds, mvWorldStorage &World, COLLISION Collisions[], int &ColiisionBufSize, char sSkyboxReference[33], int myRef);  //!< call this function to handle one collision/physics frame, passing in World and the time since last frame
EXTERN void DYNDEF(mvDllInit)();        //!< call once to initialize dll
EXTERN void DYNDEF(mvDllCleanup)();      //!< call once at end to cleanup

EXTERN void DYNDEF(ObjectCreate)( Object *p_Object );      //!< Signal new object
EXTERN void DYNDEF(ObjectModify)( Object *p_Object );      //!< Signal destroyed object
EXTERN void DYNDEF(ObjectDestroy)( int iReference );      //!< Signal destroyed object

EXTERN bool DYNDEF(CollideSphereAsRayWithObject)( Vector3 &rPosCollidePos, const Vector3 PosRayOrigin, const Vector3 vRayVector, const float fSphereRadius, const Object *p_Object );  // Collides a ray with an object, for example to see intersect your mouse pointer with an object when you click on it, to find out where you clicked
*/

#endif // _ODEPHYSICSENGINE_H
