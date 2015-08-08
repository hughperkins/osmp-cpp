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
//! \brief An ObjectGrouping is a prim that links multiple prims/objects together in the world

// See header file mvobjectstorage.h for documentation

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include <math.h>

//#include <GL/glut.h>
//#include <GL/gl.h>
//#include <GL/glu.h>
#include <iostream>
using namespace std;

#include "tinyxml.h"

#include "BasicTypes.h"
#include "GraphicsInterface.h"
#include "IDBInterface.h"
#include "Diag.h"
#include "XmlHelper.h"
#include "Constants.h"

#include "ObjectGrouping.h"

void ObjectGrouping::GetDeleteSQLFromXMLEx( TiXmlElement *pElement, char *SQL )
{
    Debug( "ObjectGrouping::GetDeleteSQLFromXMLEx\n" );

    //    if( strcmp( pElement->Attribute( "recursivedelete" ), "false" ) == 0 )
    //    {
    DEBUG(  "generating sql to unlink object " << atoi( pElement->Attribute( "ireference" ) ) ); // DEBUG
    sprintf( SQL, "%s update objects a, objects b "
             " set a.i_parentreference=0,"
             "a.pos_x=a.pos_x+b.pos_x,"
             "a.pos_y=a.pos_y+b.pos_y,"
             "a.pos_z=a.pos_z+b.pos_z"
             " where a.i_parentreference=%i and b.i_reference=%i;", SQL,
             atoi( pElement->Attribute( "ireference" ) ),
             atoi( pElement->Attribute( "ireference" ) )
           );
    //   }
    //   else
    //   {
    //       DEBUG(  "not handling recursive delete in db; this shouldnt have arrived here :-O" ); // DEBUG
    // else I dunno.  Maybe metaverseserver should intercept and delete them individually?
    // dbinterface doesnt have access to a World object so it's less easy than in metaverseserver
    // bandwidth doesnt matter once message has reached server
    // sprintf( SQL, "%s delete from objects where i_parentreference=%i;", SQL,
    //     atoi( pElement->Attribute( "ireference" ) ) );
    //   }

    sprintf( SQL, "%s delete from objectgroupings where i_reference=%i;", SQL, atoi( pElement->Attribute( "ireference" ) ) );
    Object::GetDeleteSQLFromXMLEx( pElement, SQL );
}
void ObjectGrouping::GetCreateSQLFromXMLEx( TiXmlElement *pElement, char *SQL )
{
    Debug( "ObjectGrouping::GetCreateSQLFromXMLEx\n" );

    int iGroupObjectReference = atoi( pElement->Attribute("ireference") );

    int iSubObjectSequenceNumber = 0;

    sprintf( SQL, "%s insert into objectgroupings(i_reference,s_objectgrouping_type,i_subobjectsequencenumber,i_subobjectreference)"
             "values(%i,'%s',%i,%i);",
             SQL,
             iGroupObjectReference,
             "OBJECTGROUPING",
             0,
             0
           );

    TiXmlHandle docHandle( pElement );
    if( docHandle.FirstChild("members").FirstChild("member").Element() )
    {
        TiXmlElement *pMemberXml = pElement->FirstChildElement("members")->FirstChildElement( "member" );
        while( pMemberXml != NULL )
        {
            sprintf( SQL, "%s update objects set i_parentreference=%i,pos_x=pos_x-%f,pos_y=pos_y-%f,pos_z=pos_z-%f where i_reference=%i;",
                     SQL, iGroupObjectReference,
                     atof( pElement->FirstChildElement("geometry")->FirstChildElement("pos")->Attribute("x") ),
                     atof( pElement->FirstChildElement("geometry")->FirstChildElement("pos")->Attribute("y") ),
                     atof( pElement->FirstChildElement("geometry")->FirstChildElement("pos")->Attribute("z") ),
                     atoi( pMemberXml->Attribute("ireference") ) );

            pMemberXml = pMemberXml->NextSiblingElement("member" );
            iSubObjectSequenceNumber++;
        }
    }

    Object::GetCreateSQLFromXMLEx( pElement, SQL );
}
void ObjectGrouping::GetUpdateSQLFromXMLEx( TiXmlElement *pElement, char *SQL )
{
    // We'll handle the unlinking/linking here.  We'll let metaverseserver handle the change to position and stuff
    // of children.

    DEBUG(  "GetUpdateSQLFromXMLEx" ); // DEBUG
    int iOurReference = atoi( pElement->Attribute("ireference") );
    TiXmlHandle docHandle( pElement );
    int iChildReference = 0;

    DEBUG(  "checking for members in xml..." ); // DEBUG
    if( docHandle.FirstChild("members").Element() )
    {
        DEBUG(  "removing old children..." ); // DEBUG
        sprintf( SQL, "%s update objects set i_parentreference=0 where i_parentreference=%i;", SQL, iOurReference );

        DEBUG(  "getting first xml member..." ); // DEBUG
        TiXmlElement *pMemberXml = pElement->FirstChildElement("members")->FirstChildElement( "member" );
        while( pMemberXml != NULL )
        {
            DEBUG(  "getting iRef of member..." ); // DEBUG
            iChildReference = atoi( pMemberXml->Attribute("ireference") );
            DEBUG(  "adding sql to update..." ); // DEBUG
            sprintf( SQL, "%s update objects set i_parentreference=%i where i_reference=%i;", SQL, iOurReference, iChildReference );
            pMemberXml = pMemberXml->NextSiblingElement("member" );
        }
    }
    DEBUG(  "done" ); // DEBUG

    Object::GetUpdateSQLFromXMLEx( pElement, SQL );
}
void ObjectGrouping::UpdateFromXML( TiXmlElement *pElement )
{
    Object::UpdateFromXML( pElement );
}
void ObjectGrouping::PopulateFromDBRow()
{
    Debug( "ObjectGrouping::PopulateFromDBRow()\n" );
    iNumSubObjects = 0;

    if(  pdbinterface != 0 )
    {
        sprintf( sObjectGroupingType, pdbinterface->GetFieldValueByName( "s_objectgrouping_type" ) );
    }

    Object::PopulateFromDBRow();
}
char *ObjectGrouping::GetWorldStateRetrieveSQL()
{
    return "select a.i_reference,s_object_name,a.s_object_type, i_owner, i_parentreference, pos_x, pos_y, pos_z,"
           "rot_x,rot_y,rot_z,rot_s,s_objectgrouping_type,i_subobjectsequencenumber,i_subobjectreference,s_script_reference,b_physics_enabled,b_phantom_enabled,b_terrain_enabled,b_gravity_enabled "
           "from objects a,objectgroupings b where a.i_reference = b.i_reference and a.s_object_type='ObjectGrouping' "
           "and b.s_objectgrouping_type='ObjectGrouping';";
}
void ObjectGrouping::LoadFromXML( TiXmlElement *pElement )
{
    Debug( "ObjectGrouping::LoadFromXML\n" );

    // Rem nothing to do here: mvWorldStorage will do it, cos it has pointer to other objects.
    DEBUG(  "(doing nothing here)" ); // DEBUG

    iNumSubObjects = 0;

    Object::LoadFromXML( pElement );
}
void ObjectGrouping::AddSubObject( Object *pSubObject )
{
    if( iNumSubObjects < 10 )
    {
        Debug( "objectgrouping %i adding subobject %i \n", iReference, pSubObject->iReference );
        SubObjectReferences[ iNumSubObjects ] = pSubObject;
        iNumSubObjects++;
    }
    else
    {
        Debug( "WARNING: objectgrouping %i full\n", iReference );
    }
}
void ObjectGrouping::WriteToXMLDoc( TiXmlElement *pElement )
{
    Debug( "ObjectGrouping::WriteToXMLDoc\n" );
    pElement->SetAttribute( "type", sDeepObjectType );
    Object::WriteToXMLDoc( pElement );
}
//void ObjectGrouping::CopyTo( ObjectGrouping *ptarget )
//{
// PLACEHOLDER. TO DO!

// For now we are doing a shallow copy, which probably isnt what we want (?).
//for( int i= 0; i < iNumSubObjects; i++ )
//{
//ptarget->SubObjectReferences[i] = SubObjectReferences[i];
//}
//ptarget->iNumSubObjects = iNumSubObjects;
//Object::CopyTo( ptarget );
//}
const ObjectGrouping &ObjectGrouping::operator=( const ObjectGrouping &IncomingObject )
{
    // PLACEHOLDER. TO DO!

    // For now we are doing a shallow copy, which probably isnt what we want (?).
    for( int i= 0; i < IncomingObject.iNumSubObjects; i++ )
    {
        this->SubObjectReferences[i] = IncomingObject.SubObjectReferences[i];
    }
    this->iNumSubObjects = IncomingObject.iNumSubObjects;
    Object::operator=( IncomingObject );
    return *this;
}

float GetZRotationAngle( const Rot &InRot  )
{
    float cos_a  = InRot.s;
    float angle  = acos( cos_a ) * 2;

    // Vector3 Axis;
    // float Angle;
    // Rot2AxisAngle( Axis, Angle, InRot );

    if( InRot.z > 0 && InRot.s > 0 )
    {
        //return angle + 3.1415926535;
        return angle;
    }
    else
    {
        //return 3.1415926535 - angle;
        return -angle;
    }

    // DEBUG(  angle << " " << Angle << " " << Axis ); // DEBUG
}

void ObjectGrouping::Draw()
{
    //    Debug( "ObjectGrouping::DrawEx\n" );
    pmvGraphics->PushMatrix();
    pmvGraphics->Translatef( pos.x, pos.y, pos.z );

    bool bNeedToPopMatrix = false;

    bool bRotatedToGroupRot = false;
    for( int iSubObjectRef = 0; iSubObjectRef < iNumSubObjects; iSubObjectRef++ )
    {
        if( !bRotatedToGroupRot )
        {
            // dont rotate first prim in elevation for avatars (looks better like this)
            //if( strcmp( sDeepObjectType, "AVATAR" ) != 0 )
            if( true ) // Just display all avatars as is for now( we should put this back in though probably)
            {
                if( pmvGraphics != 0 )
                {
                    pmvGraphics->RotateToRot( rot );
                }
                bRotatedToGroupRot = true;
            }
            else
            {
                if( iSubObjectRef == 0 )
                    //if( false )
                {
                    pmvGraphics->PushMatrix();
                    bNeedToPopMatrix = true;
                    float zRotationAngle = GetZRotationAngle( rot );
                    // DEBUG(  "objectgrouping.draw zRotationAngle " << zRotationAngle ); // DEBUG
                    Rot rZRotationOnlyRot;
                    Vector3 Axis;
                    Axis.x = 0;
                    Axis.y = 0;
                    Axis.z = 1;
                    AxisAngle2Rot( rZRotationOnlyRot, Axis, zRotationAngle );

                    if( pmvGraphics != 0 )
                    {
                        pmvGraphics->RotateToRot( rZRotationOnlyRot );
                    }
                }
                else
                {
                    pmvGraphics->PopMatrix();
                    bNeedToPopMatrix = false;

                    if( pmvGraphics != 0 )
                    {
                        pmvGraphics->RotateToRot( rot );
                    }
                    bRotatedToGroupRot = true;
                }
            }
        }

        if( pfCallbackAddName != NULL )
        {
            pfCallbackAddName( SubObjectReferences[ iSubObjectRef ]->iReference );
        }
        SubObjectReferences[ iSubObjectRef ]->Draw();
    }

    if( bNeedToPopMatrix )
    {
        pmvGraphics->PopMatrix();
    }

    pmvGraphics->PopMatrix();

    Object::Draw();
}
void ObjectGrouping::DrawSelected()
{
    //Debug( "ObjectGrouping::DrawEx\n" );
    pmvGraphics->PushMatrix();
    pmvGraphics->Translatef( pos.x, pos.y, pos.z );
    if( pmvGraphics != 0 )
    {
        pmvGraphics->RotateToRot( rot );
    }
    //DEBUG(  "objectgrouping drawing selection ..., pos" << pos << " There are " << iNumSubObjects << " subobjects" ); // DEBUG
    for( int iSubObjectRef = 0; iSubObjectRef < iNumSubObjects; iSubObjectRef++ )
    {
        //DEBUG(  "Drawing suboject " << iSubObjectRef << pos ); // DEBUG
        SubObjectReferences[ iSubObjectRef ]->DrawSelected();
    }
    pmvGraphics->PopMatrix();

    Object::DrawSelected();
}

void ObjectGrouping::WriteToDeepXML( TiXmlElement *pElement )
{
    WriteToXMLDoc( pElement );
    pElement->RemoveAttribute("ireference");
    pElement->RemoveAttribute("iparentreference");

    TiXmlElement *pSubObjectsElement = new TiXmlElement("subobjects");
    pElement->InsertEndChild( *pSubObjectsElement );
    for( int i = 0; i < iNumSubObjects; i++ )
    {
        DEBUG(  "for loop" ); // DEBUG
        TiXmlElement *pSubObjectElement = new TiXmlElement( "object" );
        this->SubObjectReferences[ i ]->WriteToDeepXML( pSubObjectElement );
        pElement->FirstChildElement("subobjects")->InsertEndChild( *pSubObjectElement );
    }
    DEBUG(  "done object write to deepxml" ); // DEBUG
}
