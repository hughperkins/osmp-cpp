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
//! \brief Member definitions for Avatar primtype

// For documentation see header file mvobjectstorage.h

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#ifdef _WIN32
#include <windows.h>
#endif

// #include <GL/glut.h>
//#include <GL/gl.h>
//#include <GL/glu.h>

#include "tinyxml.h"

#include "BasicTypes.h"
#include "GraphicsInterface.h"
#include "IDBInterface.h"
#include "Diag.h"
#include "XmlHelper.h"

#include "ObjectGrouping.h"

#include "Avatar.h"

void Avatar::GetUpdateSQLFromXMLEx( TiXmlElement *pElement, char *SQL )
{
    // Need to populate this TODO
    //sprintf( SQL, "update avatars set
    ObjectGrouping::GetUpdateSQLFromXMLEx( pElement, SQL );
}
char *Avatar::GetWorldStateRetrieveSQL()
{
    return   "SELECT a.i_reference,s_object_name,a.s_object_type, b.s_objectgrouping_type, a.i_owner, i_parentreference, "
             "pos_x, pos_y, pos_z, rot_x, rot_y, rot_z, rot_s, s_avatar_name,s_script_reference,b_physics_enabled,b_gravity_enabled,b_phantom_enabled,b_terrain_enabled "
             "FROM objects a, objectgroupings b, avatars c "
             "where a.i_reference = b.i_reference and a.i_reference = c.i_reference and "
             "a.s_object_type = 'ObjectGrouping' and b.s_objectgrouping_type = 'AVATAR';";
}
void Avatar::PopulateFromDBRow()
{
    Debug( "Avatar::PopulateFromDBRow()\n" );

    if( pdbinterface != 0 )
    {
        sprintf( avatarname, pdbinterface->GetFieldValueByName( "s_avatar_name" ) );
    }

    ObjectGrouping::PopulateFromDBRow();
}
void Avatar::LoadFromXML( TiXmlElement *pElement )
{
    Debug( "Avatar::LoadFromXML\n" );
    sprintf( avatarname, pElement->FirstChildElement("meta")->FirstChildElement("avatar")->Attribute("name") );
    ObjectGrouping::LoadFromXML( pElement );
}
void Avatar::WriteToXMLDoc( TiXmlElement *pElement )
{
    Debug( "Avatar::WriteToXMLDocEx\n" );
    pElement->SetAttribute( "type", "AVATAR" );
    AddXmlChildIfNecessary( pElement, "meta" );
    AddXmlChildIfNecessary( pElement, "avatar" );
    pElement->FirstChildElement("meta")->FirstChildElement("avatar")->SetAttribute( "name", avatarname );
    ObjectGrouping::WriteToXMLDoc( pElement );
}
void Avatar::UpdateFromXML( TiXmlElement *pElement )
{
    ObjectGrouping::UpdateFromXML( pElement );
}
const Avatar &Avatar::operator=( const Avatar &IncomingPrim )
{
    strcpy( this->avatarname, IncomingPrim.avatarname );
    ObjectGrouping::operator=( IncomingPrim );
    return *this;
}
void Avatar::Draw()
{
    //        Debug( "Avatar::DrawEx\n" );
    //      glPrint(avatarname); // Print GL Text To The Screen


    pmvGraphics->PushMatrix();
    pmvGraphics->RasterPos3f(pos.x, pos.y, pos.z + 0.8 );
    if( pmvGraphics != 0 )
    {
        pmvGraphics->printtext( avatarname );
    }

    pmvGraphics->PopMatrix();

    ObjectGrouping::Draw();
}

