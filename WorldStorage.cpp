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
//! \brief mvworldstorage is the class used to store the world associated with one server
// See header file for documentation

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
#include <sstream>
#include <iostream>
#include <string.h>
#include <math.h>
#include <stdio.h>
using namespace std;

#include "WorldStorage.h"
#include "Terrain.h"
#include "mvMd2Mesh.h"
#include "Math.h"

#include "Cube.h"
#include "Prim.h"
#include "Cone.h"
#include "Sphere.h"
#include "Cylinder.h"
#include "Avatar.h"
#include "ObjectGrouping.h"

mvWorldStorage::mvWorldStorage()
{
    iNumObjects = 0;
}
int mvWorldStorage::GetArrayNumForObjectReference( int iReference )
{
    int iArrayNum = 0;
    for( iArrayNum = 0; iArrayNum < iNumObjects; iArrayNum++ )
    {
        if( p_Objects[ iArrayNum ]->iReference == iReference )
        {
            return iArrayNum;
        }
    }
    DEBUG(  "Couldnt find arraynum for reference " << iReference );
    //SignalCriticalError( "Bounds check problem in GetArrayNumForObjectReference\n" );
    return -1;
}
//Object *mvWorldStorage::GetObject( int iArrayNum )
//{
//   return p_Objects[ iArrayNum ];
//}
char *mvWorldStorage::GetSkyboxChecksum()
{
    return skyboxChecksum;
}
void mvWorldStorage::SetSkyboxChecksum( const char *sNewSkyboxChecksum )
{
    strncpy( skyboxChecksum, sNewSkyboxChecksum, 32);
}
Object *mvWorldStorage::GetObjectByReference( int iReference )
{
    int iArrayNum = GetArrayNumForObjectReference( iReference );
    if( iArrayNum == -1 )
    {
        return NULL;
    }
    else
    {
        return p_Objects[ iArrayNum ];
    }
}
void mvWorldStorage::DeleteObject( int iArrayNum )
{
    if( iArrayNum == -1 )
    {
        DEBUG(  " mvWorldStorage::DeleteObject warning: invalid arraynum passed in" ); // DEBUG
    }

    for( int i =0; i < iNumObjects; i++ )
    {
        DEBUG(  p_Objects[ i ]->iReference << "." << p_Objects[ i ]->sDeepObjectType << " " );
    }

    DEBUG(  "DeleteObject " << iArrayNum ); // DEBUG
    if(iArrayNum < ( iNumObjects - 1 ) )
    {
        delete( p_Objects[ iArrayNum ] );
        p_Objects[ iArrayNum ] = NULL;
        p_Objects[ iArrayNum ] = p_Objects[ iNumObjects - 1 ];
        p_Objects[ iNumObjects - 1 ] = NULL;
        iNumObjects--;
    }
    else
    {
        delete( p_Objects[ iNumObjects - 1 ] );
        p_Objects[ iNumObjects - 1 ] = NULL;
        iNumObjects--;
    }

    for( int i =0; i < iNumObjects; i++ )
    {
        DEBUG(  p_Objects[ i ]->iReference << "." << p_Objects[ i ]->sDeepObjectType << " " );
    }
}

int mvWorldStorage::GetTopLevelParentReference( int iObjectReference )
{
    Object *p_Object = GetObjectByReference( iObjectReference );
    if( p_Object != NULL )
    {
        if( p_Object->iParentReference == 0 )
        {
            return p_Object->iReference;
        }
        else
        {
            return GetTopLevelParentReference( p_Object->iParentReference );
        }
    }
    else
    {
        return -1;
    }
}

void mvWorldStorage::DereferenceParent( Object *p_Object )
{
    DEBUG(  "DereferenceParent" ); // DEBUG

    Object *p_ParentObject = GetObjectByReference( p_Object->iParentReference );

    if( p_ParentObject == NULL )
    {
        DEBUG(  "mvWorldStorage::DereferenceParent() parentobject is null" ); // DEBUG
        return;
    }

    if( strcmp( p_ParentObject->ObjectType, "OBJECTGROUPING" ) != 0 )
    {
        DEBUG(  "mvWorldStorage::DereferenceParent() WARNING: function called for non-objectgroup" ); // DEBUG
        return;
    }

    ObjectGrouping *p_Parent = dynamic_cast< ObjectGrouping *>( p_ParentObject );
    if( p_Parent != NULL )
    {
        bool bRootWasDeleted = false;
        DEBUG(  "Scanning subobjects..." ); // DEBUG
        for( int i = 0; i < p_Parent->iNumSubObjects; i++ )
        {
            DEBUG(  "child ref is " << p_Parent->SubObjectReferences[ i ]->iReference ); // DEBUG
            if( p_Parent->SubObjectReferences[ i ] == p_Object )
            {
                DEBUG(  "Removing subobject arraynum " << i << " of " << p_Parent->iNumSubObjects << " ..." ); // DEBUG
                p_Parent->SubObjectReferences[ i ] = p_Parent->SubObjectReferences[ p_Parent->iNumSubObjects - 1 ];
                p_Parent->iNumSubObjects--;
                if( i == 0 )
                {
                    bRootWasDeleted = true;
                }
            }
        }
        if( bRootWasDeleted )
        {
            DEBUG(  "root of linked set was deleted, relinking to move root..." ); // DEBUG

            ostringstream documentxml;
            documentxml << "<childrenstore><members>";
            for( int i = 0; i < p_Parent->iNumSubObjects; i++ )
            {
                documentxml << "<member ireference=\"" << p_Parent->SubObjectReferences[ i ]->iReference << "\"/>";
            }
            documentxml << "</members></childrenstore>";

            TiXmlDocument ChildrenStore;
            ChildrenStore.Parse( documentxml.str().c_str() );
            UnlinkChildren( p_Parent );
            DEBUG(  "relinking from doc " << ChildrenStore ); // DEBUG
            LinkFromXML( p_Parent, ChildrenStore.RootElement() );
        }
    }
}

bool mvWorldStorage::IsLastPrimInGroup( int iGroupReference )
{
    DEBUG(  "IsLastPrimInGroup()" ); // DEBUG

    Object *p_Object = GetObjectByReference( iGroupReference );

    if( p_Object == NULL )
    {
        DEBUG(  "mvWorldStorage::IsLastPrimInGroup() parentobject is null" ); // DEBUG
        return false;
    }

    if( strcmp( p_Object->ObjectType, "OBJECTGROUPING" ) != 0 )
    {
        DEBUG(  "mvWorldStorage::IsLastPrimInGroup() WARNING: function called for non-objectgroup" ); // DEBUG
        return false;
    }

    ObjectGrouping *p_Group = dynamic_cast< ObjectGrouping *>( p_Object );
    if( p_Group != NULL )
    {
        if( p_Group->iNumSubObjects > 1 )
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }
}

void mvWorldStorage::DeleteObjectByObjectReference( int iReference, bool bRecursive, bool bForceDeleteIfLastPrim )
{
    DEBUG(  "DeleteObjectByObjectReference " << iReference << " " << bRecursive << " " << bForceDeleteIfLastPrim ); // DEBUG
    Object *p_TargetObject = GetObjectByReference( iReference );

    if( p_TargetObject != NULL )
    {
        if( p_TargetObject->iParentReference != 0 )
        {
            //DEBUG(  "checking if is last prim in group..." ); // DEBUG
            // if( bForceDeleteIfLastPrim || !IsLastPrimInGroup( p_TargetObject->iParentReference ) )
            // {
            DEBUG(  "dereferncing parent..." ); // DEBUG
            DereferenceParent( p_TargetObject );
            // }
            //else
            // {
            //   DEBUG(  "Cant delete this prim from group because it is last prim in group" ); // DEBUG
            //   return;
            // }
        }
        if( strcmp( p_TargetObject->ObjectType, "OBJECTGROUPING" ) == 0 )
        {
            DEBUG(  "objectgrouping, handling child objects before deleting objectgrouping object..." );
            for( int i = 0; i < iNumObjects; i++ )
            {
                Object *p_ChildObject = GetObject( i );
                if( p_ChildObject->iParentReference == iReference )
                {
                    if( bRecursive )
                    {
                        DEBUG(  "deleting suboejct recurisvely " << p_ChildObject->iReference ); // DEBUG
                        DeleteObjectByObjectReference( p_ChildObject->iReference, true, true );
                    }
                    else
                    {
                        DEBUG(  "Unlinking child object, putting pos back " << p_Objects[ i ]->iReference ); // DEBUG
                        p_ChildObject->iParentReference = 0;
                        p_ChildObject->pos.x = p_TargetObject->pos.x + p_ChildObject->pos.x;
                        p_ChildObject->pos.y = p_TargetObject->pos.y + p_ChildObject->pos.y;
                        p_ChildObject->pos.z = p_TargetObject->pos.z + p_ChildObject->pos.z;
                    }
                }
            }
        }
        DEBUG(  "deleting object ref " << iReference ); // DEBUG
        DeleteObject( GetArrayNumForObjectReference( iReference ) );
    }
}
int mvWorldStorage::AddObject( Object *p_Object )
{
    p_Objects[ iNumObjects ] = p_Object;
    iNumObjects++;
    return( iNumObjects - 1 );
}
void mvWorldStorage::CrossReferenceParentIfNecessary( int SubObjectArrayPos )
{
    int ParentObjectArrayPos;

    DEBUG(  "CrossReferenceParentIfNecessary() Getting parent reference ..." );
    int iParentReference = p_Objects[ SubObjectArrayPos ]->iParentReference;
    ParentObjectArrayPos = GetArrayNumForObjectReference( iParentReference );
    if( ParentObjectArrayPos != -1 )
    {
        dynamic_cast< ObjectGrouping * >( p_Objects[ ParentObjectArrayPos ])->AddSubObject( p_Objects[ SubObjectArrayPos ] );
    }
    else
    {
        DEBUG(  "No parent found at moment for object iReference " << p_Objects[ SubObjectArrayPos ]->iReference );
    }
}
void mvWorldStorage::CrossReferenceChildrenIfNecessary( ObjectGrouping *p_Group )
{
    int iArrayPos = 0;
    for( iArrayPos = 0; iArrayPos < iNumObjects; iArrayPos++ )
    {
        Object *p_ChildObject = p_Objects[ iArrayPos ];
        if( p_ChildObject->iParentReference == p_Group->iReference )
        {
            DEBUG(  "Child of iReference " << p_ChildObject->iParentReference << " is " << p_ChildObject->iReference ); // DEBUG
            p_Group->AddSubObject( p_ChildObject );
        }
    }
}
void mvWorldStorage::DeleteObjectXML( TiXmlElement *pElement )
{
    DEBUG(  "DeleteObjectXML" ); // DEBUG
    int iReference = atoi( pElement->Attribute("ireference") );

    if( pElement->Attribute("recursivedelete" ) == NULL )
    {
        DeleteObjectByObjectReference( iReference );
    }
    else
    {
        if( strcmp( pElement->Attribute("recursivedelete"), "true" ) == 0 )
        {
            DeleteObjectByObjectReference( iReference, true );
        }
    }
}

void mvWorldStorage::UnlinkChildren( ObjectGrouping *p_Group )
{
    Vector3 ParentPos = p_Group->pos;
    Rot ParentRot = p_Group->rot;
    Rot InverseParentRot;
    InverseRot( InverseParentRot, ParentRot );

    for( int i = 0; i < p_Group->iNumSubObjects; i++ )
    {
        DEBUG(  "unlinking child " << p_Group->SubObjectReferences[i]->iReference <<
                " group pos " << p_Group->pos << " child pos " << p_Group->SubObjectReferences[i]->pos
                << " " << p_Group->SubObjectReferences[i]->rot ); // DEBUG

        Object *p_ChildObject = p_Group->SubObjectReferences[i];

        Rot OldChildRot = p_ChildObject->rot;
        Rot NewChildRot;
        RotMultiply( NewChildRot, ParentRot, OldChildRot );

        Vector3 OldChildPos = p_ChildObject->pos;
        Vector3 NewChildPos;
        Vector3 GroupAxesVectorFromParentToChild = OldChildPos;
        Vector3 GlobalAxesVectorFromParentToChild;
        GlobalAxesVectorFromParentToChild = GroupAxesVectorFromParentToChild * InverseParentRot;
        //MultiplyVectorByRot( GlobalAxesVectorFromParentToChild, InverseParentRot, GroupAxesVectorFromParentToChild );
        NewChildPos = GlobalAxesVectorFromParentToChild + ParentPos;

        p_ChildObject->iParentReference = 0;
        p_ChildObject->pos = NewChildPos;
        p_ChildObject->rot = NewChildRot;
        DEBUG(  "child after unlinking: " << p_ChildObject->pos << " " << p_ChildObject->rot ); // DEBUG

        p_Group->SubObjectReferences[i] = NULL;
    }
    p_Group->iNumSubObjects = 0;
}

void mvWorldStorage::LinkFromXML( ObjectGrouping *p_Group, TiXmlElement *pElement )
{
    TiXmlHandle docHandle( pElement );
    if( docHandle.FirstChild("members").Element() )
    {
        //      int iSubObjectSequenceNumber = 0;
        int iChildReference;

        bool bIsFirstChild = true;
        Object *p_ChildObject;
        TiXmlElement *pMemberXml = pElement->FirstChildElement("members")->FirstChildElement( "member" );
        while( pMemberXml != NULL )
        {
            iChildReference = atoi( pMemberXml->Attribute("ireference") );
            p_ChildObject = GetObjectByReference( iChildReference );

            if( p_ChildObject != NULL )
            {
                // tell child it has a parent now
                DEBUG(  " linking child " << p_ChildObject->iReference << " currentpos " << p_ChildObject->pos
                        << " group pos " << p_Group->pos ); // DEBUG

                p_ChildObject->iParentReference = p_Group->iReference;

                Vector3 NewChildPos;
                Rot NewChildRot;

                if( bIsFirstChild )
                {
                    p_Group->pos = p_ChildObject->pos;
                    p_Group->rot = p_ChildObject->rot;

                    NewChildPos.x = 0;
                    NewChildPos.y = 0;
                    NewChildPos.z = 0;

                    NewChildRot.x = 0;
                    NewChildRot.y = 0;
                    NewChildRot.z = 0;
                    NewChildRot.s = 1;

                    bIsFirstChild = false;
                }
                else
                {
                    Vector3 ParentPos = p_Group->pos;
                    Rot ParentRot = p_Group->rot;
                    Rot InverseParentRot;
                    InverseRot( InverseParentRot, ParentRot );

                    Rot OldChildRot = p_ChildObject->rot;

                    RotMultiply( NewChildRot, InverseParentRot, OldChildRot );

                    Vector3 OldChildPos = p_ChildObject->pos;
                    Vector3 GlobalAxesVectorFromParentToChild;
                    GlobalAxesVectorFromParentToChild = OldChildPos - ParentPos;
                    Vector3 GroupAxesVectorFromParentToChild;
                    GroupAxesVectorFromParentToChild = GlobalAxesVectorFromParentToChild * ParentRot;
                    // MultiplyVectorByRot( GroupAxesVectorFromParentToChild, ParentRot, GlobalAxesVectorFromParentToChild );
                    NewChildPos = GroupAxesVectorFromParentToChild;
                }

                p_ChildObject->pos = NewChildPos;
                p_ChildObject->rot = NewChildRot;

                p_ChildObject->iParentReference = p_Group->iReference;

                DEBUG(  "new child pos: " << p_ChildObject->pos ); // DEBUG

                // Add pointer to child in SubObjectReferences
                p_Group->AddSubObject( p_ChildObject );

                //           iSubObjectSequenceNumber++;
            }
            else
            {
                DEBUG(  "Warning: Not found in object array." ); // DEBUG
            }
            pMemberXml = pMemberXml->NextSiblingElement("member" );
        }
    }
}

Object *mvWorldStorage::UpdateObjectXML( TiXmlElement *pElement )
{
    cout<< "update request received" <<endl;
    int iReference = atoi( pElement->Attribute("ireference" ) );
    DEBUG(  "objectgrouping rot=" << GetObjectByReference( iReference )->rot ); // DEBUG

    Object *p_Object = GetObjectByReference( iReference );

    DEBUG(  "UPdateObjectXML() updating object in memory..." ); // DEBUG
    if( p_Object != NULL )
    {
        p_Object->UpdateFromXML( pElement );

        if( strcmp( p_Object->ObjectType, "OBJECTGROUPING" ) == 0  )
        {
            ObjectGrouping *pGroup = dynamic_cast<ObjectGrouping *>( p_Object );
            DEBUG(  "UpdateObjectXML() updating objectgrouping... numchildren before update = " << pGroup->iNumSubObjects ); // DEBUG

            TiXmlHandle docHandle( pElement );
            if( docHandle.FirstChild("members").Element() )
            {
                DEBUG(  "UpdateObjectXML() relinking children..." ); // DEBUG
                UnlinkChildren( pGroup );
                LinkFromXML( pGroup, pElement );
                // CrossReferenceChildrenIfNecessary( p_Object );  // Only do this when first create, in case children
                // existed prior to creation
            }
            DEBUG(  "mvWorldStorage::UpdateObjectXML num children after update: " << pGroup->iNumSubObjects ); // DEBUG
        }
    }
    else
    {
        DEBUG(  "object ref " << iReference << " not found " ); // DEBUG
    }
    DEBUG(  "objectgrouping rot=" << GetObjectByReference( iReference )->rot ); // DEBUG
    return p_Object;
}
void mvWorldStorage::MoveObjectXML( TiXmlElement *pElement )
{
    int iReference = atoi( pElement->Attribute("ireference" ) );
    int iObjectArrayPos = GetArrayNumForObjectReference( iReference );
    if( iObjectArrayPos != -1 )
    {
        //  cout<< "got update for position for object ref "<< iReference;
        //p_Objects[ iObjectArrayPos ]->pos = Vector3::Vector3FromXMLElement( pElement->FirstChildElement("geometry")->FirstChildElement("pos") );
    }
}
Object *mvWorldStorage::StoreObjectXMLString( const char *XMLString )
{
    TiXmlDocument IPC;
    IPC.Parse( XMLString );

    TiXmlElement *pElement = IPC.RootElement();
    return StoreObjectXML( pElement );
}

Object *mvWorldStorage::UpdateObjectXMLString( const char *XMLString )
{
    TiXmlDocument IPC;
    IPC.Parse( XMLString );

    TiXmlElement *pElement = IPC.RootElement();
    return UpdateObjectXML( pElement );
}

Object *mvWorldStorage::StoreObjectXML( TiXmlElement *pElement )
{
    int iObjectArrayNum;
    Object *p_Object = NULL;
    int iReference = atoi( pElement->Attribute("ireference") );

    if( GetObjectByReference( iReference ) == NULL )
    {

        if( iNumObjects < iMaxObjects )
        {
            DEBUG( "storing " << pElement->Attribute( "type") << " ..." );

            if( strcmp( pElement->Attribute( "type"), "CUBE" ) == 0 )
            {
                iObjectArrayNum = AddObject( new Cube );
                p_Object = GetObject( iObjectArrayNum );
                p_Object->LoadFromXML( pElement );
            }
            else if( strcmp( pElement->Attribute( "type"), "SPHERE" ) == 0 )
            {
                iObjectArrayNum = AddObject( new Sphere );
                p_Object = GetObject( iObjectArrayNum );
                p_Object->LoadFromXML( pElement );
            }
            else if( strcmp( pElement->Attribute( "type"), "CYLINDER" ) == 0 )
            {
                iObjectArrayNum = AddObject( new Cylinder );
                p_Object = GetObject( iObjectArrayNum );
                p_Object->LoadFromXML( pElement );
            }
            else if( strcmp( pElement->Attribute( "type"), "CONE" ) == 0 )
            {
                iObjectArrayNum = AddObject( new Cone );
                p_Object = GetObject( iObjectArrayNum );
                p_Object->LoadFromXML( pElement );
            }
            else if( strcmp( pElement->Attribute( "type"), "TERRAIN" ) == 0 )
            {
                iObjectArrayNum = AddObject( new Terrain );
                p_Object = GetObject( iObjectArrayNum );
                p_Object->LoadFromXML( pElement );
            }
            else if( strcmp( pElement->Attribute( "type"), "MD2MESH" ) == 0 )
            {
                iObjectArrayNum = AddObject( new mvMd2Mesh );
                p_Object = GetObject( iObjectArrayNum );
                p_Object->LoadFromXML( pElement );
            }
            else if( strcmp( pElement->Attribute( "type"), "AVATAR" ) == 0 )
            {
                iObjectArrayNum = AddObject( new Avatar );
                p_Objects[ iObjectArrayNum ]->LoadFromXML( pElement );
                ObjectGrouping *pGroup = dynamic_cast<ObjectGrouping *>(p_Objects[ iObjectArrayNum ]);
                CrossReferenceChildrenIfNecessary( pGroup );
            }
            else if( strcmp( pElement->Attribute( "type"), "OBJECTGROUPING" ) == 0 )
            {
                iObjectArrayNum = AddObject( new ObjectGrouping );
                ObjectGrouping *pGroup = dynamic_cast<ObjectGrouping *>(p_Objects[ iObjectArrayNum ]);
                pGroup->LoadFromXML( pElement );

                TiXmlHandle docHandle( pElement );
                CrossReferenceChildrenIfNecessary( pGroup );
                if( docHandle.FirstChild("members").Element() )
                {
                    LinkFromXML( pGroup, pElement );
                }
            }
            else
            {
                DEBUG(  "WARNING: mvWorldStorage, StoreObject type " << pElement->Attribute( "type") << " not recognised" ); // DEBUG
            }

            DEBUG( "done " ); // DEBUG
        }

        if( atoi( pElement->Attribute( "iparentreference") ) != 0 )
        {
            //        Debug( "linking with parent...\n" );
            CrossReferenceParentIfNecessary( iObjectArrayNum );
        }

        DEBUG(  "object ref " << iReference << " stored" ); // DEBUG
    }
    else
    {
        DEBUG(   "Ignoring object refresh ref " << iReference << " Already have." ); // DEBUG
    }

    return p_Object;
}

void mvWorldStorage::Clear()
{
    for( int i = 0; i < iNumObjects; i++ )
    {
        delete GetObject( i );
    }
    iNumObjects = 0;
}
