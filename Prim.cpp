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
//! \brief A Prim is the baseclass for all discrete prim classes in the world; derives from Object
// For documentation see header file mvobjectstorage.h

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#ifdef _WIN32
#include <windows.h>
#endif

//#include <GL/glut.h>

#include "tinyxml.h"

#include "BasicTypes.h"
#include "GraphicsInterface.h"
#include "IDBInterface.h"
#include "Diag.h"
#include "XmlHelper.h"

#include "Prim.h"

void Prim::GetCreateSQLFromXMLEx( TiXmlElement *pElement, char *SQL )
{
    //   Debug( "Prim::GetCreateSQLFromXMLEx\n" );

    sprintf( SQL, "%s insert into prims (i_reference,s_prim_type,scale_x,scale_y,scale_z)select %i,s_prim_type,scale_x,scale_y,scale_z from prims where i_reference = 0;",
             SQL, atoi( pElement->Attribute( "ireference" ) ) );

    Object::GetCreateSQLFromXMLEx( pElement, SQL );
}
void Prim::GetUpdateSQLFromXMLEx( TiXmlElement *pElement, char *SQL )
{
    //   Debug( "Prim::GetUpdateSQLFromXMLEx\n" );

    TiXmlHandle docHandle( pElement );
    char SetSQL[512] = "";
    if( pElement->Attribute( "type" ) != NULL )
    {
        sprintf( SetSQL, "%s s_prim_type='%s',", SetSQL, pElement->Attribute("type") );
    }
    if( docHandle.FirstChild("geometry").FirstChild("scale").Element() )
    {
        sprintf( SetSQL, "%s scale_x=%f,scale_y=%f,scale_z=%f,", SetSQL,
                 atof( pElement->FirstChildElement("geometry")->FirstChildElement("scale")->Attribute("x") ),
                 atof( pElement->FirstChildElement("geometry")->FirstChildElement("scale")->Attribute("y") ),
                 atof( pElement->FirstChildElement("geometry")->FirstChildElement("scale")->Attribute("z") ) );
    }

    if( strlen( SetSQL ) > 0 )
    {
        sprintf( SQL, "%s update prims set %.*s where i_reference = %i;", SQL, strlen( SetSQL ) - 1, SetSQL, atoi( pElement->Attribute( "ireference" ) ) );
    }

    Object::GetUpdateSQLFromXMLEx( pElement, SQL );
}
void Prim::PopulateFromDBRow()
{
    //  Debug( "Prim::PopulateFromDBRow()\n" );

    //   DEBUG(  "Prim:: Populatefromdbrow, color0.b = " << color0.b ); // DEBUG

    if( pdbinterface != NULL )
    {
        scale.x = atof( pdbinterface->GetFieldValueByName( "scale_x" ) );
        scale.y = atof( pdbinterface->GetFieldValueByName( "scale_y" ) );
        scale.z = atof( pdbinterface->GetFieldValueByName( "scale_z" ) );
        sprintf( PrimType, pdbinterface->GetFieldValueByName( "s_prim_type" ) );
    }

    Object::PopulateFromDBRow();
}
void Prim::UpdateFromXML( TiXmlElement *pElement )
{
    //  Debug( "Prim::UpdateFromXMLEx\n" );

    TiXmlHandle docHandle( pElement );
    if( pElement->Attribute( "type" ) != NULL )
    {
        sprintf( PrimType, pElement->Attribute("type") );
    }
    if( docHandle.FirstChild("geometry").FirstChild("scale").Element() )
    {
        scale = Vector3( pElement->FirstChildElement("geometry")->FirstChildElement("scale") );
    }

    Object::UpdateFromXML( pElement );
}
void Prim::LoadFromXML( TiXmlElement *pElement )
{
    TiXmlHandle docHandle( pElement );
    //  Debug( "Prim::LoadFromXML\n" );
    sprintf( PrimType, pElement->Attribute("type") );
    if( docHandle.FirstChild("geometry").FirstChild("scale").Element() )
    {
        scale = Vector3( pElement->FirstChildElement("geometry")->FirstChildElement("scale") );
    }
    // else
    // {
    //Debug( "WARNING
    //  scale.x = 0.2; scale.y = 0.2; scale.z = 0.2;
    // }
    Object::LoadFromXML( pElement );
}
void Prim::WriteToXMLDoc( TiXmlElement *pElement )
{
    //  Debug( "Prim::WriteToXMLDoc\n" );

    AddXmlChildIfNecessary( pElement, "geometry" );
    AddXmlChildIfNecessary( pElement->FirstChildElement("geometry"), "scale" );

    scale.WriteToXMLElement( pElement->FirstChildElement("geometry")->FirstChildElement("scale") );
    //  Debug( "Prim::WriteToXMLDoc done\n" );
    Object::WriteToXMLDoc( pElement );
}
//void Prim::CopyTo( Prim *ptargetprim )
//{
//sprintf( ptargetprim->PrimType, PrimType );
//ptargetprim->scale = scale;
//Object::CopyTo( ptargetprim );
//}
const Prim &Prim::operator=( const Prim &IncomingPrim )
{
    strcpy( this->PrimType, IncomingPrim.PrimType );
    this->scale = IncomingPrim.scale;
    Object::operator=( IncomingPrim );
    return *this;
}
void Prim::Draw()
{
    //  Debug( "Prim::DrawEx\n" );
    Object::Draw();
}
