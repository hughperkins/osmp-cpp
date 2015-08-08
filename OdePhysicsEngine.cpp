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
//! \brief This module handles liaison with the ode scripting engine
//!
//! This module handles liaison with the ode scripting engine
//! It creates a world at init, then each time HandleCollisionsAndPhysics is called, it adds the non-physical
//! world objects (including moving non-physical objects) into the world as pure geoms
//! and adds the physical objects in as geoms with physical bodies
//! It runs collision detection ("dSpaceCollide") then dynamic collision response ("dWorldQuickStep") then
//! writes the results back to the mv World.
//!
//! Most nonphysical objects are currently mapped to appropriatley scaled and rotated cubic bounding boxes
//! Symmetrical spheres are mapped to spheres
//! terrains do not currently participate in physics, but will do so using trimeshes in the future
//!
//! avatars are mapped to a single object representing the first prim in their linked set, and do not rotate
//!
//! if a lot of time has passed since physics and ocllisiosn were last run, many interpolations are run over this timeinterval
//! to increase accuracy.

#include "ode/ode.h"

#include <map>
#include <set>
#include <iostream>
using namespace std;

#include "Diag.h"
#include "WorldStorage.h"
//#include "TextureInfoCache.h"
//#include "TerrainInfoCache.h"
//#include "MeshInfoCache.h"
#include "Terrain.h"
#include "Config.h"
#include "Collision.h"

/*
#ifdef DYNDEF
#undef DYNDEF
#endif
#ifndef DYNDEF
#define DYNDEF( f ) f
#endif
#ifdef EXTERN
#undef EXTERN
#endif
#define EXTERN extern "C"
*/
#include "OdePhysicsEngine.h"

const float fAvRadius = 1.0;

#define NUM 1000   // max number of objects
#define DENSITY (5.0)  // density of all objects
#define MAX_CONTACTS 4  // maximum number of contact points per body

//TextureInfoCache textureinfocache;  // we dont really need these, but theyre dependencies for now -> something to cleanup sometime
//TerrainCacheClass TerrainCache;
//MeshInfoCacheClass MeshInfoCache;
ConfigClass mvConfig;

dWorldID CollisionAndPhysicsEngineClass::world;
dSpaceID CollisionAndPhysicsEngineClass::permaspace;
dJointGroupID CollisionAndPhysicsEngineClass::contactgroup;

int CollisionAndPhysicsEngineClass::myRefOde;
char CollisionAndPhysicsEngineClass::skyboxOde[33];

float CollisionAndPhysicsEngineClass::fNearestRayIntersectDistance;
bool CollisionAndPhysicsEngineClass::bRayCollided;
Vector3 CollisionAndPhysicsEngineClass::RayNearestPos;

bool CollisionAndPhysicsEngineClass::CollisionFlag;
int CollisionAndPhysicsEngineClass::CollisionBufSizeOde;
COLLISION CollisionAndPhysicsEngineClass::CollisionsOde[2048];

CollisionAndPhysicsEngineClass::CollisionAndPhysicsEngineClass()
{
    cout << "CollisionAndPhysicsEngineClass()" << endl;
    iNumTerrains = 0;
    iDisplayCount = 0;
}

CollisionAndPhysicsEngineClass::~CollisionAndPhysicsEngineClass()
{
    cout << "~CollisionAndPhysicsEngineClass()" << endl;
}

void CollisionAndPhysicsEngineClass::mvRotToOdeQuaternion( dQuaternion &OdeQuat, const Rot &mvQuat )
{
    OdeQuat[0] = mvQuat.s;
    OdeQuat[1] = mvQuat.x;
    OdeQuat[2] = mvQuat.y;
    OdeQuat[3] = mvQuat.z;
}

void CollisionAndPhysicsEngineClass::nearCallback (void *data, dGeomID o1, dGeomID o2)
{
    //DEBUG(  " collision" ); // DEBUG

    int i;
    // if (o1->body && o2->body) return;

    // exit without doing anything if the two bodies are connected by a joint
    dBodyID b1 = dGeomGetBody(o1);
    dBodyID b2 = dGeomGetBody(o2);

    if (b1 && b2 && dAreConnectedExcluding (b1,b2,dJointTypeContact))
    {
        return;
    }

    Object *pObject1 = (Object *)dGeomGetData( o1 );
    Object *pObject2 = (Object *)dGeomGetData( o2 );

    /* if( pObject1 != NULL )
     {
      DEBUG(  "object1:" <<  pObject1->sDeepObjectType << " n: " << pObject1->sObjectName << " r: " << pObject1->iReference << " phan: " << pObject1->bPhantomEnabled << " t:" << pObject1->bTerrainEnabled); // DEBUG
     }
    if( pObject2 != NULL )
     {
      DEBUG(  "object2:" <<  pObject2->sDeepObjectType << " n: " << pObject2->sObjectName << " r: " << pObject2->iReference << " phan: " << pObject2->bPhantomEnabled << " t:" << pObject2->bTerrainEnabled); // DEBUG
     } */

    if ( b1 != NULL)
    {
        pObject1 = (Object *)dBodyGetData( b1 );
    }
    if ( b2 != NULL)
    {
        pObject2 = (Object *)dBodyGetData( b2 );
    }

    /* if( pObject1 != NULL )
     {
      DEBUG(  "bobject1:" <<  pObject1->sDeepObjectType << " n: " << pObject1->sObjectName << " r: " << pObject1->iReference << " phan: " << pObject1->bPhantomEnabled << " t:" << pObject1->bTerrainEnabled); // DEBUG
     }
    if( pObject2 != NULL )
     {
      DEBUG(  "bobject2:" <<  pObject2->sDeepObjectType << " n: " << pObject2->sObjectName << " r: " << pObject2->iReference << " phan: " << pObject2->bPhantomEnabled << " t:" << pObject2->bTerrainEnabled); // DEBUG
     } */

    int iNumberOfTerrain = 0;
    bool bInteractionWithTerrain = false;
    bool bPhantom = false;
    bool bTerrain = false;

    if( pObject1 != NULL )
    {
        if( strcmp( pObject1->sDeepObjectType, "TERRAIN" ) == 0 )
        {
            bInteractionWithTerrain = true;
            iNumberOfTerrain = 1;
        }
        if ( pObject1->bPhantomEnabled )
        {
            bPhantom = true;
        }
        if ( pObject1->bTerrainEnabled )
        {
            bTerrain = true;
        }
    }

    if( pObject2 != NULL )
    {
        if( strcmp( pObject2->sDeepObjectType, "TERRAIN" ) == 0 )
        {
            if( iNumberOfTerrain == 0 )
            {
                iNumberOfTerrain = 2;
                bInteractionWithTerrain = true;

            }
            else // two terrains interacting? not interesting
            {
                return;
            }
        }

        if ( pObject2->bPhantomEnabled )
        {
            bPhantom = true;
        }
        if ( pObject2->bTerrainEnabled )
        {
            bTerrain = true;
        }
    }

    if (CollisionFlag == true)
    {
        // make sure target and colref are not the same
        // make sure itar and iref not already in
        if (pObject2->iReference != pObject1->iReference)
        {
            int f = -1;
            for (i = 0; i < CollisionBufSizeOde; i++)
            {
                if (CollisionsOde[i].iTarget = pObject1->iReference)
                {
                    if (CollisionsOde[i].iCollidingObjectReference = pObject2->iReference)
                    {
                        f = i;
                    }
                }
            }
            if (f == -1)
            {
                CollisionsOde[CollisionBufSizeOde].iTarget = pObject1->iReference;
                CollisionsOde[CollisionBufSizeOde].iCollidingObjectReference = pObject2->iReference;
                CollisionBufSizeOde++;
            }
        }
    }

    //DEBUG("phantom: " << bPhantom << " terrain: " << bTerrain);
    if (( !bPhantom ) || ( bTerrain))
    {
        if ( !bInteractionWithTerrain )
        {
            dContact contact[MAX_CONTACTS];   // up to MAX_CONTACTS contacts per box-box
            for (i=0; i<MAX_CONTACTS; i++)
            {
                contact[i].surface.mode = 0;
                contact[i].surface.mu = dInfinity;
                //contact[i].surface.mu = 0;
                contact[i].surface.mu2 = 0;
                contact[i].surface.bounce = 0.1;
                contact[i].surface.bounce_vel = 0.1;
                contact[i].surface.soft_cfm = 0.01;
            }

            if (int numc = dCollide (o1,o2,MAX_CONTACTS,&contact[0].geom,
                                     sizeof(dContact)))
            {
                dMatrix3 RI;
                dRSetIdentity (RI);
                const dReal ss[3] =
                    {
                        0.02,0.02,0.02
                    };
                for (i=0; i<numc; i++)
                {
                    dJointID c = dJointCreateContact (world,contactgroup,contact+i);
                    dJointAttach (c,b1,b2);
                }
            }

        }
        else
        {
            //DEBUG(  "collision with terrain bounding box" ); // DEBUG

            Terrain *p_Terrain;
            Object *p_CollidingObject;
            if( iNumberOfTerrain == 1 )
            {
                p_Terrain = dynamic_cast< Terrain *>( pObject1 );
                p_CollidingObject = pObject2;
            }
            else
            {
                p_Terrain = dynamic_cast< Terrain *>( pObject2 );
                p_CollidingObject = pObject1;
            }

            //DEBUG("p_CollidingObject " << p_CollidingObject->iReference << " vs myRef : " << myRefOde);
            if (p_CollidingObject->iReference == myRefOde)
            {
                //DEBUG(" collision set skybox " << p_Terrain->sSkyboxReference << " loaded: " << p_Terrain->bTerrainLoaded << " nam: " << p_Terrain->sObjectName);
                strcpy(skyboxOde, p_Terrain->sSkyboxReference);
            }


            if( p_Terrain->bTerrainLoaded )
            {
                dGeomID CollidingGeom;
                if( iNumberOfTerrain == 1 )
                {
                    CollidingGeom = o2;
                    // contactposition = dGeomGetPosition( o2 );
                }
                else
                {
                    CollidingGeom = o1;
                    //  contactposition = dGeomGetPosition( o1 );
                }
                const dReal *odecollidingbodypos = dGeomGetPosition( CollidingGeom );
                Vector3 mvcollidingbodypos;
                mvcollidingbodypos.x = odecollidingbodypos[0];
                mvcollidingbodypos.y = odecollidingbodypos[1];
                mvcollidingbodypos.z = odecollidingbodypos[2];

                //DEBUG(  "pos of colliding body: " << mvcollidingbodypos ); // DEBUG
                // DEBUG(  "terrain pos: " << p_Terrain->pos << " rot: " << p_Terrain->rot << " scale: " << p_Terrain->scale ); // DEBUG

                Vector3 PosRelativeTerrainRootWorldAxes;
                PosRelativeTerrainRootWorldAxes.x = mvcollidingbodypos.x - p_Terrain->pos.x;
                PosRelativeTerrainRootWorldAxes.y = mvcollidingbodypos.y - p_Terrain->pos.y;
                PosRelativeTerrainRootWorldAxes.z = mvcollidingbodypos.z - p_Terrain->pos.z;

                // DEBUG(  "pos relative terrainrootworldaxes: " << PosRelativeTerrainRootWorldAxes ); // DEBUG

                Vector3 PosRelativeTerrainRootRotatedToTerrainAxes;
                Rot InverseTerrainRot;
                InverseRot( InverseTerrainRot, p_Terrain->rot );
                PosRelativeTerrainRootRotatedToTerrainAxes = PosRelativeTerrainRootWorldAxes * p_Terrain->rot;
                // MultiplyVectorByRot( PosRelativeTerrainRootRotatedToTerrainAxes, p_Terrain->rot, PosRelativeTerrainRootWorldAxes );

                Vector3 PosInTerrain;
                PosInTerrain.x = PosRelativeTerrainRootRotatedToTerrainAxes.x / p_Terrain->scale.x * (float)p_Terrain->iMapSize + (float)p_Terrain->iMapSize / 2;
                PosInTerrain.y = PosRelativeTerrainRootRotatedToTerrainAxes.y / p_Terrain->scale.y * (float)p_Terrain->iMapSize + (float)p_Terrain->iMapSize / 2;
                //      PosInTerrain.z = PosRelativeTerrainRootRotatedToTerrainAxes.z / p_Terrain->scale.z * (float)p_Terrain->iMapSize;
                int ixPosInTerrain = (int)PosInTerrain.x;
                int iyPosInTerrain = (int)PosInTerrain.y;
                if( ixPosInTerrain >= 3 && ixPosInTerrain <= p_Terrain->iMapSize - 3 && iyPosInTerrain >= 3 && iyPosInTerrain <= p_Terrain->iMapSize - 3 )
                {
                    //DEBUG(  "Getting height... " << PosInTerrain.x << " " << PosInTerrain.y ); // DEBUG
                    float fHeightOfTerrainTerrainAxes = p_Terrain->Height( ixPosInTerrain, iyPosInTerrain );

                    Vector3 PosOfTerrainTerrainAxesScaledGlobalAxes;
                    PosOfTerrainTerrainAxesScaledGlobalAxes.x = PosRelativeTerrainRootRotatedToTerrainAxes.x;
                    PosOfTerrainTerrainAxesScaledGlobalAxes.y = PosRelativeTerrainRootRotatedToTerrainAxes.y;
                    PosOfTerrainTerrainAxesScaledGlobalAxes.z = ( fHeightOfTerrainTerrainAxes - 127.5f ) * p_Terrain->scale.z / 255.0f;

                    Vector3 PosOfTerrainRelativeTerrainRootRotatedGlobalAxes;
                    PosOfTerrainRelativeTerrainRootRotatedGlobalAxes = PosOfTerrainTerrainAxesScaledGlobalAxes * InverseTerrainRot;
                    // MultiplyVectorByRot( PosOfTerrainRelativeTerrainRootRotatedGlobalAxes, InverseTerrainRot, PosOfTerrainTerrainAxesScaledGlobalAxes );

                    Vector3 PosOfTerrainGlobalAxes;
                    PosOfTerrainGlobalAxes.x = PosOfTerrainRelativeTerrainRootRotatedGlobalAxes.x + p_Terrain->pos.x;
                    PosOfTerrainGlobalAxes.y = PosOfTerrainRelativeTerrainRootRotatedGlobalAxes.y + p_Terrain->pos.y;
                    PosOfTerrainGlobalAxes.z = PosOfTerrainRelativeTerrainRootRotatedGlobalAxes.z + p_Terrain->pos.z;

                    dContact terraincontact;

                    terraincontact.surface.mode = 0;
                    terraincontact.surface.mu = dInfinity;
                    //terraincontact.surface.mu = 0;
                    terraincontact.surface.mu2 = 0;
                    terraincontact.surface.bounce = 0.1;
                    terraincontact.surface.bounce_vel = 0.1;
                    terraincontact.surface.soft_cfm = 0.01;

                    //  DEBUG(  "creating temp geom" ); // DEBUG
                    dGeomID TempTerrainGeom = dCreateBox ( permaspace, 10.0, 10.0, 5.0 );
                    dGeomSetPosition( TempTerrainGeom, PosOfTerrainGlobalAxes.x, PosOfTerrainGlobalAxes.y, PosOfTerrainGlobalAxes.z - 2.5 );

                    int numcollisions = dCollide (CollidingGeom,TempTerrainGeom,1,&terraincontact.geom, sizeof(dContact));

                    if( numcollisions > 0 )
                    {
                        //     DEBUG(  "creating joint..." ); // DEBUG
                        terraincontact.geom.g2 = 0;
                        //Vector3 NormalVector = p_Terrain->VectorNormals[ ixPosInTerrain + p_Terrain->iMapSize * iyPosInTerrain ];
                        //terraincontact.geom.normal[0] = NormalVector.x;
                        //terraincontact.geom.normal[1] = NormalVector.y;
                        //terraincontact.geom.normal[2] = NormalVector.z;
                        // DEBUG(  "vectornormal " << ixPosInTerrain << " " << iyPosInTerrain << NormalVector ); // DEBUG
                        dJointID c = dJointCreateContact( world, contactgroup, &terraincontact );
                        dJointAttach( c, b1, b2 );
                    }
                    dGeomDestroy( TempTerrainGeom );

                    //  if( iDisplayCount == 0 )
                    //  {
                    //   DEBUG(  "pos of colliding body: " << mvcollidingbodypos ); // DEBUG
                    //  DEBUG(  "terrain pos: " << p_Terrain->pos << " rot: " << p_Terrain->rot << " scale: " << p_Terrain->scale ); // DEBUG
                    // DEBUG(  "pos relative terrainrootworldaxes: " << PosRelativeTerrainRootWorldAxes ); // DEBUG
                    // DEBUG(  "PosInTerrain = " << PosInTerrain << " heightofterrain " << fHeightOfTerrainTerrainAxes ); // DEBUG
                    //  }
                }
            }
        }
    }
    //DEBUG(  "...processed" ); // DEBUG
}

void CollisionAndPhysicsEngineClass::CreateSphere( dSpaceID targetspace, OdeObject &rOdeObject, const Prim *p_Prim, const Vector3 &TranslationContext, const Rot &RotationContext )
{
    float fAverageSphereRadius = ( p_Prim->scale.x + p_Prim->scale.y + p_Prim->scale.z ) / 3.0;
    rOdeObject.geom = dCreateSphere ( targetspace, fAverageSphereRadius );
    dGeomSetData ( rOdeObject.geom,(void*)p_Prim );
}

void CollisionAndPhysicsEngineClass::CreateCube( dSpaceID targetspace, OdeObject &rOdeObject, const Prim *p_Prim, const Vector3 &TranslationContext, const Rot &RotationContext )
{
    rOdeObject.geom = dCreateBox ( targetspace, p_Prim->scale.x, p_Prim->scale.y, p_Prim->scale.z );
    dGeomSetData ( rOdeObject.geom,(void*)p_Prim );
}

void CollisionAndPhysicsEngineClass::CreateCylinder( dSpaceID targetspace, OdeObject &rOdeObject, const Prim *p_Prim, const Vector3 &TranslationContext, const Rot &RotationContext )
{
    rOdeObject.geom = dCreateCCylinder ( targetspace, p_Prim->scale.x, ( p_Prim->scale.y + p_Prim->scale.z ) / 2 );
    dGeomSetData ( rOdeObject.geom,(void*)p_Prim );
}

void CollisionAndPhysicsEngineClass::CreateGeom( dSpaceID targetspace, const Object *p_Object, const Vector3 &TranslationContext, const Rot &RotationContext )
{
    //DEBUG(  "CreateGeom " << p_Object->ObjectType << " r: " << p_Object->iReference << " n: " << p_Object->sObjectName << "..." ); // DEBUG
    bool bObjectCreated = false;
    if( strcmp( p_Object->ObjectType, "PRIM" ) == 0 )
    {
        // DEBUG(  "its a prim" ); // DEBUG
        const Prim *p_Prim = dynamic_cast< const Prim *>( p_Object );
        if( strcmp( p_Object->sDeepObjectType, "SPHERE" ) == 0 )
        {
            OdeObject NewOdeObject;

            // if its more or less symmetrical, draw as a sphere, otherwise use a cubic bounding box:
            float fAverageSphereRadius = ( p_Prim->scale.x + p_Prim->scale.y + p_Prim->scale.z ) / 3.0;
            if( p_Prim->scale.x / fAverageSphereRadius < 1.4
                    && p_Prim->scale.y / fAverageSphereRadius < 1.4
                    && p_Prim->scale.z / fAverageSphereRadius < 1.4 )
            {
                CreateSphere( targetspace, NewOdeObject, p_Prim, TranslationContext, RotationContext );
            }
            else
            {
                CreateCube( targetspace, NewOdeObject, p_Prim, TranslationContext, RotationContext );
            }

            //  DEBUG(  "about to do insert.." ); // DEBUG
            StaticObjects.insert( pair< int, OdeObject >( p_Object->iReference, NewOdeObject ) );
            //  DEBUG(  "...done" ); // DEBUG

            bObjectCreated = true;
        }
        else if( strcmp( p_Object->sDeepObjectType, "CUBE" ) == 0 )
        {
            OdeObject NewOdeObject;

            CreateCube( targetspace, NewOdeObject, p_Prim, TranslationContext, RotationContext );

            //  DEBUG(  "about to do insert.." ); // DEBUG
            StaticObjects.insert( pair< int, OdeObject >( p_Object->iReference, NewOdeObject ) );
            //  DEBUG(  "...done" ); // DEBUG

            bObjectCreated = true;
        }
        else if( strcmp( p_Object->sDeepObjectType, "CONE" ) == 0 )
        {
            OdeObject NewOdeObject;

            // map to cubic bounding box
            CreateCube( targetspace, NewOdeObject, p_Prim, TranslationContext, RotationContext );

            //  DEBUG(  "about to do insert.." ); // DEBUG
            StaticObjects.insert( pair< int, OdeObject >( p_Object->iReference, NewOdeObject ) );
            //  DEBUG(  "...done" ); // DEBUG

            bObjectCreated = true;
        }
        else if( strcmp( p_Object->sDeepObjectType, "mvMd2Mesh" ) == 0 )
        {
            OdeObject NewOdeObject;

            CreateCube( targetspace, NewOdeObject, p_Prim, TranslationContext, RotationContext );

            //    DEBUG(  "about to do insert.." ); // DEBUG
            StaticObjects.insert( pair< int, OdeObject >( p_Object->iReference, NewOdeObject ) );
            //    DEBUG(  "...done" ); // DEBUG

            bObjectCreated = true;
        }
        else if( strcmp( p_Object->sDeepObjectType, "CYLINDER" ) == 0 )
        {
            OdeObject NewOdeObject;

            CreateCylinder( targetspace, NewOdeObject, p_Prim, TranslationContext, RotationContext );

            //      DEBUG(  "about to do insert.." ); // DEBUG
            StaticObjects.insert( pair< int, OdeObject >( p_Object->iReference, NewOdeObject ) );
            //      DEBUG(  "...done" ); // DEBUG

            bObjectCreated = true;
        }
        else if( strcmp( p_Object->sDeepObjectType, "TERRAIN" ) == 0 )
        {
            OdeObject NewOdeObject;

            // map to cubic bounding box
            CreateCube( targetspace, NewOdeObject, p_Prim, TranslationContext, RotationContext );

            //DEBUG(  "about to do insert.." ); // DEBUG
            StaticObjects.insert( pair< int, OdeObject >( p_Object->iReference, NewOdeObject ) );
            //DEBUG(  "...done" ); // DEBUG

            bObjectCreated = true;
        }
    }
    else if( strcmp( p_Object->sDeepObjectType, "OBJECTGROUPING" ) == 0 )
    {
        const ObjectGrouping *p_Group = dynamic_cast< const ObjectGrouping *>( p_Object );
        for( int i = 0; i < p_Group->iNumSubObjects; i++ )
        {
            Vector3 GlobalAxisPosOffset;

            //    Rot InverseRotationContext;
            //    InverseRot( InverseRotationContext, RotationContext );

            GlobalAxisPosOffset = p_Group->pos * RotationContext;
            // MultiplyVectorByRot( GlobalAxisPosOffset, RotationContext, p_Group->pos );
            Vector3 NewPosContext = TranslationContext + GlobalAxisPosOffset;

            Rot InverseGroupRot;
            InverseRot( InverseGroupRot, p_Group->rot );

            Rot NewRotContext;
            RotMultiply( NewRotContext, InverseGroupRot, RotationContext );

            CreateGeom( targetspace, p_Group->SubObjectReferences[ i ], NewPosContext, NewRotContext );
        }
    }

    if( bObjectCreated )
    {
        OdeObject &rNewObject = StaticObjects.find( p_Object->iReference )->second;

        Rot InverseRotationContext;
        InverseRot( InverseRotationContext, RotationContext );

        Vector3 GlobalAxisPosOffset;
        //MultiplyVectorByRot( GlobalAxisPosOffset, RotationContext, p_Object->pos );
        GlobalAxisPosOffset = p_Object->pos * RotationContext;
        // MultiplyVectorByRot( GlobalAxisPosOffset, RotationContext, p_Object->pos );

        Vector3 GlobalAxisPos;
        GlobalAxisPos = TranslationContext + GlobalAxisPosOffset;

        dGeomSetPosition( rNewObject.geom, GlobalAxisPos.x, GlobalAxisPos.y, GlobalAxisPos.z );

        Rot GlobalAxisRot;
        RotMultiply( GlobalAxisRot, p_Object->rot, RotationContext );
        dQuaternion BodyRotation;
        mvRotToOdeQuaternion( BodyRotation, GlobalAxisRot );
        dGeomSetQuaternion( rNewObject.geom, BodyRotation );

        rNewObject.bPhysics = false;
    }
    // DEBUG(  "...done" ); // DEBUG
}

void CollisionAndPhysicsEngineClass::DeleteGeom( int iReference )
{
    if( StaticObjects.find( iReference ) != StaticObjects.end() )
    {
        OdeObject &rOdeObject = StaticObjects.find( iReference )->second;
        dGeomDestroy( rOdeObject.geom );
        StaticObjects.erase( iReference );
    }
    // StaticObjects.erase( iReference );
}

void CollisionAndPhysicsEngineClass::CreatePhysicalGeom( OdeObject &rOdeObject, const Object *p_Object )
{
    if( strcmp( p_Object->ObjectType, "PRIM" ) == 0 )
    {
        const Prim *p_Prim = dynamic_cast< const Prim *>( p_Object );
        if( strcmp( p_Object->sDeepObjectType, "SPHERE" ) == 0 )
        {
            //rOdeObject.geom = dCreateSphere( permaspace, ( p_Prim->scale.x + p_Prim->scale.y + p_Prim->scale.z ) / 3 );
            //dMassSetSphere( &( rOdeObject.fMass ), DENSITY, ( p_Prim->scale.x + p_Prim->scale.y + p_Prim->scale.z ) / 3 );

            // map sphere to box
            rOdeObject.geom = dCreateBox( permaspace, p_Prim->scale.x, p_Prim->scale.y, p_Prim->scale.z );
            dMassSetBox( &( rOdeObject.fMass ), DENSITY, p_Prim->scale.x, p_Prim->scale.y, p_Prim->scale.z );
            rOdeObject.bEnabled = true;
        }
        else if( strcmp( p_Object->sDeepObjectType, "CUBE" ) == 0 )
        {
            rOdeObject.geom = dCreateBox( permaspace, p_Prim->scale.x, p_Prim->scale.y, p_Prim->scale.z );
            dMassSetBox( &( rOdeObject.fMass ), DENSITY, p_Prim->scale.x, p_Prim->scale.y, p_Prim->scale.z );
            rOdeObject.bEnabled = true;
        }
        else if( strcmp( p_Object->sDeepObjectType, "CONE" ) == 0 )
        {
            // map cone to box
            rOdeObject.geom = dCreateBox( permaspace, p_Prim->scale.x, p_Prim->scale.y, p_Prim->scale.z );
            dMassSetBox( &( rOdeObject.fMass ), DENSITY, p_Prim->scale.x, p_Prim->scale.y, p_Prim->scale.z );
            rOdeObject.bEnabled = true;
        }
        else if( strcmp( p_Object->sDeepObjectType, "mvMd2Mesh" ) == 0 )
        {
            // map md2mesh to box
            rOdeObject.geom = dCreateBox( permaspace, p_Prim->scale.x, p_Prim->scale.y, p_Prim->scale.z );
            dMassSetBox( &( rOdeObject.fMass ), DENSITY, p_Prim->scale.x, p_Prim->scale.y, p_Prim->scale.z );
            rOdeObject.bEnabled = true;
        }
        else if( strcmp( p_Object->sDeepObjectType, "CYLINDER" ) == 0 )
        {
            rOdeObject.geom = dCreateCCylinder( permaspace, p_Prim->scale.x, ( p_Prim->scale.y + p_Prim->scale.z ) / 2 );
            dMassSetCylinder( &( rOdeObject.fMass ), DENSITY, 3, p_Prim->scale.x, ( p_Prim->scale.y + p_Prim->scale.z ) / 2 );
            rOdeObject.bEnabled = true;
        }
    }
    else if( strcmp( p_Object->ObjectType, "OBJECTGROUPING" ) == 0 )
    {
        //cout<< " physical objectgrouping "  ); // DEBUG
        const ObjectGrouping *p_Group = dynamic_cast< const ObjectGrouping *>( p_Object );
        if( p_Group->iNumSubObjects >= 1 )
        {
            //cout<< " creating physical subobject... "  ); // DEBUG
            CreatePhysicalGeom( rOdeObject, p_Group->SubObjectReferences[ 0 ] );
        }
    }
}

void CollisionAndPhysicsEngineClass::SimulationLoop( float fTimeStepSeconds )
{
    dSpaceCollide (permaspace,0,&nearCallback);
    dWorldQuickStep ( world, fTimeStepSeconds );
    dJointGroupEmpty (contactgroup);
}

void CollisionAndPhysicsEngineClass::HandleCollisionsAndPhysics( int iElapsedTimeMilliseconds, mvWorldStorage &World, COLLISION Collisions[], int &CollisionBufSize, char sSkyboxReference[33], int myRef)
{
    // just going to copy world each time for now.... later we can add functions to dll so that
    // renderer notifies us of new objects and stuff?

    // DEBUG(  "handlecollisionsandphsyics" ); // DEBUG

    bool bArePhysicalObjects = false;

    float fTimeslotmultiplier = (float)iElapsedTimeMilliseconds / (float)1000.0;
    //DEBUG( "fTimeslotmultiplier " << fTimeslotmultiplier );

    // added check to animation.cpp to make sure it's ThisTimeInterval variable
    // is never 0.
    //if( fTimeslotmultiplier < 0.001 )
    //{
    //   DEBUG( "aborting for too small timeslot : " << iElapsedTimeMilliseconds );
    //   return;
    //}

    //iNumOdeObjects = 0;
    //iNumTerrains = 0;

    if( iDisplayCount > 50 )
    {
        //exit(1);
        iDisplayCount = 0;
    }

    // DEBUG(  "loading phsycisal objects ..." ); // DEBUG
    int i;
    for( i = 0; i < World.iNumObjects; i++ )
    {
        // DEBUG(  "arraynum " << i ); // DEBUG
        Object *p_Object = World.GetObject( i );

        if( p_Object->iParentReference == 0 )
        {
            if( p_Object->bPhysicsEnabled  )
            {
                OdeObject NewOdeObject;

                NewOdeObject.body = dBodyCreate (world);
                //  const dReal *pNewRot;
                // pNewRot = dBodyGetRotation( rOdeObject.body );
                //DEBUG(  pNewRot[0] << " " << pNewRot[1] << " " << pNewRot[2] << " " << pNewRot[3] ); // DEBUG
                dBodySetPosition( NewOdeObject.body, p_Object->pos.x, p_Object->pos.y, p_Object->pos.z );


                if( strcmp( p_Object->sDeepObjectType, "AVATAR" ) != 0 )
                {
                    bArePhysicalObjects = true;
                    //DEBUG(  "physical body: pos: " << p_Object->pos << " vel: " << p_Object->vVelocity << " rot: " << p_Object->rot << " grav: " << p_Object->bGravityEnabled ); // DEBUG

                    dBodySetAngularVel( NewOdeObject.body, p_Object->vAngularVelocity.x, p_Object->vAngularVelocity.y, p_Object->vAngularVelocity.z );

                    dQuaternion BodyRotation;
                    mvRotToOdeQuaternion( BodyRotation, p_Object->rot );
                    dBodySetQuaternion( NewOdeObject.body, BodyRotation );

                    if( !p_Object->bGravityEnabled )
                    {
                        dBodySetGravityMode( NewOdeObject.body, 0 );
                    }

                    dBodyAddRelForce( NewOdeObject.body, p_Object->vLocalForce.x, p_Object->vLocalForce.y, p_Object->vLocalForce.z );
                    dBodyAddRelTorque( NewOdeObject.body, p_Object->vLocalTorque.x, p_Object->vLocalTorque.y, p_Object->vLocalTorque.z );
                }
                else
                {}

                //          DEBUG(  " velocity " << p_Object->vVelocity.x << " " << p_Object->vVelocity.y << " " << p_Object->vVelocity.z << " " ); // DEBUG
                dBodySetLinearVel  ( NewOdeObject.body, p_Object->vVelocity.x, p_Object->vVelocity.y, p_Object->vVelocity.z );

                NewOdeObject.bEnabled = false;
                CreatePhysicalGeom( NewOdeObject, p_Object );

                if( NewOdeObject.bEnabled )
                {
                    dGeomSetBody ( NewOdeObject.geom, NewOdeObject.body );

                    dBodySetMass( NewOdeObject.body, &NewOdeObject.fMass );

                    dBodySetData ( NewOdeObject.body,(void*)p_Object );
                    NewOdeObject.bPhysics = true;
                }
                else
                {
                    DEBUG(  "HandleCollisionAndPhysics() rOdeObject.bEnabled is false!" ); // DEBUG
                    dBodyDestroy( NewOdeObject.body );
                    dGeomDestroy( NewOdeObject.geom );
                }

                PhysicalObjects.insert( pair< int, OdeObject >( p_Object->iReference, NewOdeObject ) );
            }
        }
    }

    //DEBUG(  "physical objects loaded" ); // DEBUG

    //DEBUG(  "num physical objects: " << PhysicalObjects.size() << " nonphys objects: " << StaticObjects.size() ); // DEBUG

    int iIterations = ((int)( fTimeslotmultiplier / 0.02 )) + 1;
    if( iIterations > 5 )
    {
        DEBUG(  "running " << iIterations << " iterations timeslot: " << fTimeslotmultiplier ); // DEBUG
    }

    if (CollisionBufSize == 0)
    {
        CollisionBufSizeOde = 0;
        CollisionFlag = true;
    }
    myRefOde = myRef;
    strcpy(skyboxOde, "");
    for( i = 0; i < iIterations; i++ )
        ;
    {
        SimulationLoop( fTimeslotmultiplier / (float)iIterations  );
    }
    if (CollisionBufSize == 0)
    {
        for (i = 0; i < CollisionBufSizeOde; i++)
        {
            //DEBUG("*** SCRIPT coll target: " << CollisionsOde[i].iTarget << " collref: " << CollisionsOde[i].iCollidingObjectReference);
            Collisions[i].iTarget = CollisionsOde[i].iTarget;
            Collisions[i].iCollidingObjectReference = CollisionsOde[i].iCollidingObjectReference;
        }
        CollisionBufSize = CollisionBufSizeOde;
        //DEBUG("CollisionBufSize2 " << CollisionBufSize );

        CollisionFlag = false;
    }
    if (strcmp(skyboxOde, "") != 0)
    {
        //DEBUG("SENDING BACK SKYBOX " << skyboxOde);
        strcpy(sSkyboxReference, skyboxOde);
    }

    // DEBUG(  "clean up etc" ); // DEBUG
    for( map < int, OdeObject >::iterator iterator = PhysicalObjects.begin(); iterator != PhysicalObjects.end(); iterator++ )
        //for( i = 0; i < iNumOdeObjects; i++ )
    {
        Object *p_Object = (Object *)dBodyGetData( iterator->second.body );

        const dReal *pNewPos;
        pNewPos = dGeomGetPosition( iterator->second.geom );
        p_Object->pos.x = pNewPos[0];
        p_Object->pos.y = pNewPos[1];
        p_Object->pos.z = pNewPos[2];

        const dReal *pNewVel;
        pNewVel = dBodyGetLinearVel( iterator->second.body );
        p_Object->vVelocity.x = pNewVel[0];
        p_Object->vVelocity.y = pNewVel[1];
        p_Object->vVelocity.z = pNewVel[2];

        if( strcmp( p_Object->sDeepObjectType, "AVATAR" ) != 0 )
        {

            const dReal *pNewRot;
            pNewRot = dBodyGetQuaternion( iterator->second.body );
            p_Object->rot.s = pNewRot[0];
            p_Object->rot.x = pNewRot[1];
            p_Object->rot.y = pNewRot[2];
            p_Object->rot.z = pNewRot[3];

            const dReal *pAngularVelocity;
            pAngularVelocity = dBodyGetAngularVel( iterator->second.body );
            p_Object->vAngularVelocity.x = pAngularVelocity[0];
            p_Object->vAngularVelocity.y = pAngularVelocity[1];
            p_Object->vAngularVelocity.z = pAngularVelocity[2];
        }

        dBodyDestroy( iterator->second.body );
        dGeomDestroy( iterator->second.geom );
    }

    PhysicalObjects.clear();

    dJointGroupEmpty (contactgroup);

    // DEBUG(  "done" ); // DEBUG
    //iNumOdeObjects = 0;

    if( bArePhysicalObjects )
    {
        iDisplayCount++;
    }
}

void CollisionAndPhysicsEngineClass::RayCallback(void *data, dGeomID o1, dGeomID o2)
{
    DEBUG(  "RayCallback" ); // DEBUG
    dContact raycontact;

    if( dGeomGetClass( o1 ) == dRayClass || dGeomGetClass( o2 ) == dRayClass )
    {
        int numc = dCollide ( o1, o2, 1, &raycontact.geom, sizeof( raycontact ) );
        if( numc > 0 )
        {
            DEBUG(  raycontact.geom.depth ); // DEBUG
            if( raycontact.geom.depth < fNearestRayIntersectDistance )
            {
                DEBUG(  "It's the nearest yet! -> adding" ); // DEBUG
                bRayCollided = true;
                RayNearestPos.x = raycontact.geom.pos[0];
                RayNearestPos.y = raycontact.geom.pos[1];
                RayNearestPos.z = raycontact.geom.pos[2];
                DEBUG(  RayNearestPos ); // DEBUG
            }
        }
    }
}

bool CollisionAndPhysicsEngineClass::CollideSphereAsRayWithObject( Vector3 &rPosCollidePos, const Vector3 PosRayOrigin, const Vector3 vRayVector, const float fSphereRadius, const Object *p_Object )
{
    //    dSpaceID testcollidespace = dHashSpaceCreate (0);

    Vector3 vNormalizedRay = vRayVector;
    VectorNormalize( vNormalizedRay );

    DEBUG(  "physics ray collider " << PosRayOrigin << " " << vRayVector ); // DEBUG

    dGeomID RayID = dCreateRay ( permaspace, 20.0 );
    dGeomRaySet (RayID, PosRayOrigin.x, PosRayOrigin.y, PosRayOrigin.z, vNormalizedRay.x, vNormalizedRay.y, vNormalizedRay.z );

    RayNearestPos = PosRayOrigin;

    //RayNearestPos;
    bRayCollided = false;
    fNearestRayIntersectDistance = 100.0f;
    dSpaceCollide (permaspace,0,&RayCallback);

    rPosCollidePos = RayNearestPos - vNormalizedRay * fSphereRadius;
    DEBUG(  rPosCollidePos ); // DEBUG

    dGeomDestroy( RayID );

    return bRayCollided;
}

void CollisionAndPhysicsEngineClass::Init()
{
    cout << "dllinit" << endl;
    DEBUG(  "collisionandphysicsdll.mvDllinit()" ); // DEBUG

    mvConfig.ReadConfig();
#ifndef _NOLOGGINGLIB

    CMessageGroup::disableAllMsgGroups();
    if( mvConfig.DebugLevel == "3" )
    {
        CMessageGroup::enableMsgGroups( "DEBUG" );
    }
#endif

    CollisionFlag = false;
    world = dWorldCreate();
    permaspace = dHashSpaceCreate (0);
    contactgroup = dJointGroupCreate (0);
    dWorldSetGravity (world,0,0, -30.0);
    dWorldSetCFM (world,1e-4);
    dWorldSetAutoDisableFlag (world,0);
    //dWorldSetContactMaxCorrectingVel (world,10.0);
    dWorldSetContactSurfaceLayer (world,0.001);

    // dGeomID box = dCreateBox (permaspace, 21.0, 21.0, 1.0);
    // dGeomSetPosition( box, -10.0, -10.0, -1.5 );
}

void CollisionAndPhysicsEngineClass::Cleanup()
{
    dJointGroupDestroy (contactgroup);
    dSpaceDestroy (permaspace);
    dWorldDestroy (world);
}

void CollisionAndPhysicsEngineClass::ObjectCreate( Object *p_Object )
{
    // DEBUG(  "objectcreate" ); // DEBUG
    if( p_Object != NULL )
    {
        if( p_Object->iParentReference == 0 )
        {
            if( !p_Object->bPhysicsEnabled  )
            {
                if( !p_Object->bPhantomEnabled  )
                {
                    Vector3 PosContext;
                    Rot RotContext;
                    PosContext.x = 0;
                    PosContext.y = 0;
                    PosContext.z = 0;
                    RotContext.x = 0;
                    RotContext.y = 0;
                    RotContext.z = 0;
                    RotContext.s = 1;
                    DEBUG(  "Creating geom for " << p_Object->iReference); // DEBUG
                    CreateGeom( permaspace, p_Object, PosContext, RotContext );
                }
            }
        }
    }
    // DEBUG(  "....done" ); // DEBUG
}

void CollisionAndPhysicsEngineClass::ObjectModify( Object *p_Object )
{
    //DEBUG(  "objectmodify" ); // DEBUG
    if( p_Object != NULL )
    {
        //DEBUG(  "number static objects before erase: " << StaticObjects.size());
        DeleteGeom( p_Object->iReference );
        //DEBUG(  "after: " << StaticObjects.size());
        if( p_Object->iParentReference == 0 )
        {
            if( !p_Object->bPhysicsEnabled  )
            {
                if( !p_Object->bPhantomEnabled  )
                {
                    //DEBUG("add: " << p_Object->iReference << " ph: " << p_Object->bPhantomEnabled);
                    Vector3 PosContext;
                    Rot RotContext;
                    PosContext.x = 0;
                    PosContext.y = 0;
                    PosContext.z = 0;
                    RotContext.x = 0;
                    RotContext.y = 0;
                    RotContext.z = 0;
                    RotContext.s = 1;
                    CreateGeom( permaspace, p_Object, PosContext, RotContext );
                }
            }
        }
    }
    // DEBUG(  "....done" ); // DEBUG
}

void CollisionAndPhysicsEngineClass::ObjectDestroy( int iReference )
{
    DeleteGeom( iReference );
    // StaticObjects.erase( iReference );
}
