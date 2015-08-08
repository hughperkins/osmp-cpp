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
//! \brief function definitions for Cylinder primtype

// For documentation see header file mvobjectstorage.h

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif


#ifdef _WIN32
#include <windows.h>
#endif

//#include <GL/gl.h>
//#include <GL/glu.h>
//#include <GL/glut.h>
#include <iostream>
using namespace std;
#include "math.h"

#include "Math.h"
#include "tinyxml.h"

#include "BasicTypes.h"
#include "GraphicsInterface.h"
#include "IDBInterface.h"
#include "Diag.h"
#include "XmlHelper.h"
#include "TextureInfoCache.h"

#include "Cylinder.h"

void Cylinder::GetCreateSQLFromXMLEx( TiXmlElement *pElement, char *SQL )
{
    //    Debug( "Cylinder::GetCreateSQLFromXMLEx\n" );

    sprintf( SQL, "insert into cylinders (i_reference, color_0_r,color_0_g,color_0_b)select %i,color_0_r,color_0_g,color_0_b from cylinders where i_reference = 0;",
             atoi( pElement->Attribute( "ireference" ) ) );

    Prim::GetCreateSQLFromXMLEx( pElement, SQL );
}
void Cylinder::GetUpdateSQLFromXMLEx( TiXmlElement *pElement, char *SQL )
{
    //   Debug( "Cylinder::GetUpdateSQLFromXMLEx\n" );

    TiXmlHandle docHandle( pElement );
    if( docHandle.FirstChild("faces").FirstChild("face").FirstChild("color").Element() )
    {
        sprintf( SQL, "update cylinders set color_0_r=%f, color_0_g=%f, color_0_b=%f where i_reference = %i;",
                 atof( pElement->FirstChildElement("faces")->FirstChildElement("face")->FirstChildElement("color")->Attribute("r") ),
                 atof( pElement->FirstChildElement("faces")->FirstChildElement("face")->FirstChildElement("color")->Attribute("g") ),
                 atof( pElement->FirstChildElement("faces")->FirstChildElement("face")->FirstChildElement("color")->Attribute("b") ),
                 atoi( pElement->Attribute( "ireference" ) )  );
    }
    if( docHandle.FirstChild("faces").FirstChild("face").FirstChild("texture").Element() )
    {
        sprintf( SQL, "%s update cylinders set s_texture_reference='%s' where i_reference = %i;",
                 SQL,
                 pElement->FirstChildElement("faces")->FirstChildElement("face")->FirstChildElement("texture")->Attribute("stexturereference"), atoi( pElement->Attribute( "ireference" ) ) );
    }

    Prim::GetUpdateSQLFromXMLEx( pElement, SQL );
}
char *Cylinder::GetWorldStateRetrieveSQL()
{
    return   "SELECT a.i_reference,s_object_name,a.s_object_type,b.s_prim_type,a.i_owner,i_parentreference,"
             "pos_x,pos_y,pos_z,rot_x,rot_y,rot_z,rot_s,scale_x,scale_y,scale_z,s_texture_reference,b_physics_enabled,b_phantom_enabled,b_terrain_enabled,b_gravity_enabled,s_script_reference,"
             "color_0_r,color_0_g,color_0_b FROM objects a, prims b, cylinders c "
             "where a.i_reference=b.i_reference and a.i_reference=c.i_reference and "
             "a.s_object_type='Prim' and b.s_prim_type='Cylinder';";
}
void Cylinder::PopulateFromDBRow()
{
    // Debug( "Cylinder::PopulateFromDBRow()\n" );
    // DEBUG(  "Cylinder::Populatefromdbrow, color0_r = " << GetFieldValueByName( "color_0_r" ) ); // DEBUG

    if( pdbinterface != 0 )
    {
        color0.r = atof( pdbinterface->GetFieldValueByName( "color_0_r" ) );
        color0.g = atof( pdbinterface->GetFieldValueByName( "color_0_g" ) );
        color0.b = atof( pdbinterface->GetFieldValueByName( "color_0_b" ) );
        if( pdbinterface->GetFieldValueByName( "s_texture_reference" ) != NULL )
        {
            sprintf( sTextureReference, pdbinterface->GetFieldValueByName( "s_texture_reference" ) );
        }
        else
        {
            sprintf( sTextureReference, "" );
        }
    }

    Prim::PopulateFromDBRow();
}
void Cylinder::UpdateFromXML( TiXmlElement *pElement )
{
    // Debug( "Cylinder::UpdateFromXMLEx\n" );

    TiXmlHandle docHandle( pElement );
    if( docHandle.FirstChild("faces").FirstChild("face").FirstChild("color").Element() )
    {
        color0 = Color( pElement->FirstChildElement("faces")->FirstChildElement("face")->FirstChildElement("color") );
    }
    if( docHandle.FirstChild("faces").FirstChild("face").FirstChild("texture").Element() )
    {
        //   DEBUG(  "loading texture reference..." ); // DEBUG
        sprintf( sTextureReference, pElement->FirstChildElement("faces")->FirstChildElement("face")->FirstChildElement("texture")->Attribute("stexturereference") );
    }

    Prim::UpdateFromXML( pElement );
}
void Cylinder::LoadFromXML( TiXmlElement *pElement )
{
    //  Debug( "Cylinder::LoadFromXML\n" );
    //     color0.r = 1.0; color0.g = 0; color0.b = 1.0;

    TiXmlHandle docHandle( pElement );
    if( docHandle.FirstChild("faces").FirstChild("face").FirstChild("color").Element() )
    {
        //   DEBUG(  "reading color from XML IPC" ); // DEBUG
        color0 = Color( pElement->FirstChildElement("faces")->FirstChildElement("face")->FirstChildElement("color") );
    }
    else
    {
        //   DEBUG(  "No colour information in XML IPC" ); // DEBUG
    }

    if( docHandle.FirstChild("faces").FirstChild("face").FirstChild("texture").Element() )
    {
        // DEBUG(  "loading texture reference..." ); // DEBUG
        sprintf( sTextureReference, pElement->FirstChildElement("faces")->FirstChildElement("face")->FirstChildElement("texture")->Attribute("stexturereference") );
    }
    else
    {
        sprintf( sTextureReference, "" );
    }
    // DEBUG(  "end of Cylinder::LoadFromXML" ); // DEBUG
    Prim::LoadFromXML( pElement );
}
void Cylinder::WriteToXMLDoc( TiXmlElement *pElement )
{
    //  Debug( "Cylinder::WriteToXMLDoc\n" );
    pElement->SetAttribute( "type", "CYLINDER" );
    AddXmlChildIfNecessary( pElement, "faces" );
    AddXmlChildIfNecessary( pElement->FirstChildElement("faces"), "face" );
    AddXmlChildIfNecessary( pElement->FirstChildElement("faces")->FirstChildElement("face"), "color" );
    color0.WriteToXMLElement( pElement->FirstChildElement("faces")->FirstChildElement("face")->FirstChildElement("color") );
    AddXmlChildIfNecessary( pElement->FirstChildElement("faces")->FirstChildElement("face"), "texture" );
    pElement->FirstChildElement("faces")->FirstChildElement("face")->FirstChildElement("texture")->SetAttribute("stexturereference", sTextureReference );
    Prim::WriteToXMLDoc( pElement );
}
//void Cylinder::CopyTo( Cylinder *ptargetcylinder )
//{
//ptargetcylinder->color0 = color0;
//strcpy( ptargetcylinder->sTextureReference, sTextureReference );
//Prim::CopyTo( ptargetcylinder );
//}
const Cylinder &Cylinder::operator=( const Cylinder &IncomingPrim )
{
    this->color0 = IncomingPrim.color0;
    strcpy( this->sTextureReference, IncomingPrim.sTextureReference );
    Prim::operator=( IncomingPrim );
    return *this;
}
void Cylinder::Draw()
{
    float mcolor[4];
    mcolor[0] = color0.r;
    mcolor[1] = color0.g;
    mcolor[2] = color0.b;
    mcolor[3] = 1.0;
    pmvGraphics->SetMaterialColor( mcolor);

    float x,y,z,sx,sy,sz;
    x = pos.x;
    y= pos.y;
    z = pos.z;
    sx = scale.x;
    sy = scale.y;
    sz = scale.z;

    //    Debug( "%f %f %f %f %f %f\n", x, y, z, sx, sy, sz );

    pmvGraphics->PushMatrix();

    pmvGraphics->Translatef( x,y,z);


    //DEBUG(  ">>> rot stuff" ); // DEBUG
    if( pmvGraphics != 0 )
    {
        pmvGraphics->RotateToRot( rot );
    }

    pmvGraphics->Scalef( scale.x, scale.y, scale.z );

    int iTextureID = 0;
    if( ptextureinfocache != 0 )
    {
        iTextureID = ptextureinfocache->TextureReferenceToTextureId( sTextureReference );
    }
    else
    {
        DEBUG(  "WARNING: Object::ptextureinfocache is not initialized" ); // DEBUG
    }

    pmvGraphics->Bind2DTexture(iTextureID );
    pmvGraphics->DoCylinder();
    pmvGraphics->Bind2DTexture( 0 );

    pmvGraphics->PopMatrix();

    //      Debug( "Cylinder::Draw done\n" );
    Prim::Draw();
}
void Cylinder::DrawSelected()
{
    float mcolor[4];
    mcolor[0] = 1.0 - color0.r;
    mcolor[1] = 1.0 - color0.g;
    mcolor[2] = 1.0 - color0.b;
    mcolor[3] = 0.4;
    pmvGraphics->SetMaterialColor(  mcolor);

    float x,y,z,sx,sy,sz;
    x = pos.x;
    y= pos.y;
    z = pos.z;
    sx = scale.x;
    sy = scale.y;
    sz = scale.z;


    pmvGraphics->PushMatrix();

    pmvGraphics->Translatef( x,y,z);

    if( pmvGraphics != 0 )
    {
        pmvGraphics->RotateToRot( rot );
    }

    pmvGraphics->Scalef( scale.x, scale.y, scale.z );
    if( pmvGraphics != 0 )
    {
        pmvGraphics->DrawWireframeBox( 10 );
    }

    pmvGraphics->PopMatrix();

    //      Debug( "Cylinder::DrawEx done\n" );
    Prim::Draw();
}

void Cylinder::WriteToDeepXML( TiXmlElement *pElement )
{
    WriteToXMLDoc( pElement );
    pElement->RemoveAttribute("ireference");
    pElement->RemoveAttribute("iparentreference");
}

void Cylinder::SetColor( int face, const Color &newcolor )
{
    color0 = newcolor;
}

void Cylinder::SetTexture( int face, const char *sTextureReference )
{
    sprintf( this->sTextureReference, sTextureReference );
}

Color &Cylinder::GetColor( int face )
{
    return color0;
}

const char *Cylinder::GetTexture( int face )
{
    return sTextureReference;
}
