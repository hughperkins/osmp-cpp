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
//! \brief A terrain is a height-mapped prim based on a 128x128 .raw file
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
#include "Terrain.h"
#include "XmlHelper.h"
#include "TextureInfoCache.h"
#include "TerrainInfoCache.h"

void Terrain::CalculateNormals()
{
    for ( int X = 2; X < (iMapSize - 3); X += 1 )
    {
        // DEBUG(  "X " << X ); // DEBUG
        for ( int Y = 2; Y < (iMapSize - 3); Y += 1 )
        {
            Vector3 Normal;
            //   DEBUG(  X << " " << Y ); // DEBUG

            if( X > 5 && X < iMapSize - 5 )
            {
                Normal.x = ( Height( X + 5, Y ) - Height( X - 5, Y ) ) / 10.0f / 2.0f;
            }
            else if( X < iMapSize - 5 )
            {
                Normal.x = ( Height( X + 5, Y ) - Height( X, Y ) ) / 5.0f / 2.0f;
            }
            else
            {
                Normal.x = ( Height( X, Y ) - Height( X - 5, Y ) ) / 5.0f / 2.0f;
            }
            //   DEBUG(  "done x " << Normal.x ); // DEBUG

            if( Y > 5 && Y < iMapSize - 5 )
            {
                Normal.y = ( Height( X, Y + 5 ) - Height( X, Y - 5 ) ) / 10.0f / 2.0f;
            }
            else if( Y < iMapSize - 5 )
            {
                Normal.y = ( Height( X, Y + 5 ) - Height( X, Y ) ) / 5.0f / 2.0f;
            }
            else
            {
                Normal.y = ( Height( X, Y ) - Height( X, Y - 5 ) ) / 5.0f / 2.0f;
            }

            //  DEBUG(  "done y " << Normal.y ); // DEBUG
            Normal.z = 1;

            //  DEBUG(  "assigning to VectorNormals..." ); // DEBUG
            VectorNormals[ X + Y * iMapSize ].x = Normal.x;
            VectorNormals[ X + Y * iMapSize ].y = Normal.y;
            VectorNormals[ X + Y * iMapSize ].z = Normal.z;
            //   DEBUG(  "...done" ); // DEBUG
        }
    }
}

// Loads The .RAW File And Stores It In pHeightMap (based on http://nehe.gamedev.net)
bool Terrain::LoadTerrainFile( const char *sFilePath )
{
    DEBUG(  "LoadRawFile()" ); // DEBUG
    FILE *pFile = NULL;

    pFile = fopen( sFilePath, "rb" );
    if ( pFile == NULL )
    {
        DEBUG(  "Couldn't find height map file " << sFilePath ); // DEBUG
        return false;
    }

    fread( g_HeightMap, 1, iMapSize * iMapSize, pFile );

    // Check If We Received An Error
    int result = ferror( pFile );
    if (result)
    {
        DEBUG(  "Error loading height map file " << sFilePath ); // DEBUG
        fclose(pFile);
        return false;
    }

    fclose(pFile);


    DEBUG(  "LoadRawFile() done" ); // DEBUG

    CalculateNormals();

    bTerrainLoaded = true;
    return true;
}

// from nehe.gamedev.net
int Terrain::Height( const int X, const int Y) const    // This Returns The Height From A Height Map Index
{
    int x = X % iMapSize;        // Error Check Our x Value
    int y = Y % iMapSize;        // Error Check Our y Value

    if(!g_HeightMap)
        return 0;       // Make Sure Our Data Is Valid

    return g_HeightMap[x + (y * iMapSize)];    // Index Into Our Height Array And Return The Height
}

void Terrain::GetCreateSQLFromXMLEx( TiXmlElement *pElement, char *SQL )
{
    //    Debug( "Terrain::GetCreateSQLFromXMLEx\n" );

    sprintf( SQL, "insert into terrains (i_reference, color_0_r,color_0_g,color_0_b)select %i,color_0_r,color_0_g,color_0_b from terrains where i_reference = 0;",
             atoi( pElement->Attribute( "ireference" ) ) );

    Prim::GetCreateSQLFromXMLEx( pElement, SQL );
}
void Terrain::GetUpdateSQLFromXMLEx( TiXmlElement *pElement, char *SQL )
{
    //   Debug( "Terrain::GetUpdateSQLFromXMLEx\n" );

    TiXmlHandle docHandle( pElement );
    if( docHandle.FirstChild("faces").FirstChild("face").FirstChild("color").Element() )
    {
        sprintf( SQL, "update terrains set color_0_r=%f, color_0_g=%f, color_0_b=%f where i_reference = %i;",
                 atof( pElement->FirstChildElement("faces")->FirstChildElement("face")->FirstChildElement("color")->Attribute("r") ),
                 atof( pElement->FirstChildElement("faces")->FirstChildElement("face")->FirstChildElement("color")->Attribute("g") ),
                 atof( pElement->FirstChildElement("faces")->FirstChildElement("face")->FirstChildElement("color")->Attribute("b") ),
                 atoi( pElement->Attribute( "ireference" ) )  );
    }
    if( docHandle.FirstChild("faces").FirstChild("face").FirstChild("texture").Element() )
    {
        sprintf( SQL, "%s update terrains set s_texture_reference='%s' where i_reference = %i;",
                 SQL,
                 pElement->FirstChildElement("faces")->FirstChildElement("face")->FirstChildElement("texture")->Attribute("stexturereference"), atoi( pElement->Attribute( "ireference" ) ) );
    }

    if( docHandle.FirstChild("faces").FirstChild("face").FirstChild("skybox").Element() )
    {
        sprintf( SQL, "%s update terrains set s_skybox_reference='%s' where i_reference = %i;",
                 SQL,
                 pElement->FirstChildElement("faces")->FirstChildElement("face")->FirstChildElement("skybox")->Attribute("stexturereference"), atoi( pElement->Attribute( "ireference" ) ) );
    }

    if( docHandle.FirstChild("geometry").FirstChild("terrain").Element() )
    {
        sprintf( SQL, "%s update terrains set s_terrain_reference='%s' where i_reference = %i;",
                 SQL,
                 pElement->FirstChildElement("geometry")->FirstChildElement("terrain")->Attribute("sterrainreference" ),
                 atoi( pElement->Attribute( "ireference" ) ) );
    }

    Prim::GetUpdateSQLFromXMLEx( pElement, SQL );
}
char *Terrain::GetWorldStateRetrieveSQL()
{
    return   "SELECT a.i_reference,a.s_object_type,b.s_prim_type,a.i_owner,i_parentreference,s_object_name,"
             "pos_x,pos_y,pos_z,rot_x,rot_y,rot_z,rot_s,scale_x,scale_y,scale_z,s_texture_reference,s_skybox_reference,b_physics_enabled,b_phantom_enabled,b_terrain_enabled,b_gravity_enabled,s_script_reference,s_terrain_reference,"
             "color_0_r,color_0_g,color_0_b FROM objects a, prims b, terrains c "
             "where a.i_reference=b.i_reference and a.i_reference=c.i_reference and "
             "a.s_object_type='Prim' and b.s_prim_type='Terrain';";
}
void Terrain::PopulateFromDBRow()
{
    // Debug( "Terrain::PopulateFromDBRow()\n" );
    // DEBUG(  "Terrain::Populatefromdbrow, color0_r = " << GetFieldValueByName( "color_0_r" ) ); // DEBUG

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

        if( pdbinterface->GetFieldValueByName( "s_skybox_reference" ) != NULL )
        {
            sprintf( sSkyboxReference, pdbinterface->GetFieldValueByName( "s_skybox_reference" ) );
        }
        else
        {
            sprintf( sSkyboxReference, "" );
        }

        sprintf( sTerrainReference, pdbinterface->GetFieldValueByName( "s_terrain_reference" ) );
    }

    Prim::PopulateFromDBRow();
}
void Terrain::UpdateFromXML( TiXmlElement *pElement )
{
    // Debug( "Terrain::UpdateFromXMLEx\n" );

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

    if( docHandle.FirstChild("faces").FirstChild("face").FirstChild("skybox").Element() )
    {
        //   DEBUG(  "loading texture reference..." ); // DEBUG
        sprintf( sSkyboxReference, pElement->FirstChildElement("faces")->FirstChildElement("face")->FirstChildElement("skybox")->Attribute("stexturereference") );
    }

    if( docHandle.FirstChild("geometry").FirstChild("terrain").Element() )
    {
        sprintf( sTerrainReference, pElement->FirstChildElement("geometry")->FirstChildElement("terrain")->Attribute("sterrainreference" ) );
    }

    Prim::UpdateFromXML( pElement );
}
void Terrain::LoadFromXML( TiXmlElement *pElement )
{
    //  Debug( "Terrain::LoadFromXML\n" );
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

    if( docHandle.FirstChild("faces").FirstChild("face").FirstChild("skybox").Element() )
    {
        // DEBUG(  "loading texture reference..." ); // DEBUG
        sprintf( sSkyboxReference, pElement->FirstChildElement("faces")->FirstChildElement("face")->FirstChildElement("skybox")->Attribute("stexturereference") );
    }
    else
    {
        sprintf( sSkyboxReference, "" );
    }

    if( docHandle.FirstChild("geometry").FirstChild("terrain").Element() )
    {
        sprintf( sTerrainReference, pElement->FirstChildElement("geometry")->FirstChildElement("terrain")->Attribute("sterrainreference" ) );
    }

    // DEBUG(  "end of Terrain::LoadFromXML" ); // DEBUG
    Prim::LoadFromXML( pElement );
}
void Terrain::WriteToXMLDoc( TiXmlElement *pElement )
{
    //  Debug( "Terrain::WriteToXMLDoc\n" );
    pElement->SetAttribute( "type", "TERRAIN" );

    AddXmlChildIfNecessary( pElement, "geometry" );
    AddXmlChildIfNecessary( pElement->FirstChildElement("geometry"), "terrain" );
    pElement->FirstChildElement("geometry")->FirstChildElement("terrain")->SetAttribute("sterrainreference", sTerrainReference );

    AddXmlChildIfNecessary( pElement, "faces" );
    AddXmlChildIfNecessary( pElement->FirstChildElement("faces"), "face" );
    AddXmlChildIfNecessary( pElement->FirstChildElement("faces")->FirstChildElement("face"), "color" );
    color0.WriteToXMLElement( pElement->FirstChildElement("faces")->FirstChildElement("face")->FirstChildElement("color") );
    AddXmlChildIfNecessary( pElement->FirstChildElement("faces")->FirstChildElement("face"), "texture" );
    pElement->FirstChildElement("faces")->FirstChildElement("face")->FirstChildElement("texture")->SetAttribute("stexturereference", sTextureReference );
    AddXmlChildIfNecessary( pElement->FirstChildElement("faces")->FirstChildElement("face"), "skybox" );
    pElement->FirstChildElement("faces")->FirstChildElement("face")->FirstChildElement("skybox")->SetAttribute("stexturereference", sSkyboxReference );
    Prim::WriteToXMLDoc( pElement );
}
const Terrain &Terrain::operator=( const Terrain &IncomingPrim )
{
    this->color0 = IncomingPrim.color0;
    strcpy( this->sTextureReference, IncomingPrim.sTextureReference );
    strcpy( this->sSkyboxReference, IncomingPrim.sSkyboxReference );
    strcpy( this->sTerrainReference, IncomingPrim.sTerrainReference );
    Prim::operator=( IncomingPrim );
    return *this;
}

void Terrain::Draw()
{
    if( !bTerrainLoaded )
    {
        if( pTerrainCache != 0 )
        {
            //DEBUG(" Look for terrain " << sTerrainReference);
            if( pTerrainCache->Terrains.find( sTerrainReference ) != pTerrainCache->Terrains.end() )
            {
                //DEBUG(" Found terrain " << sTerrainReference);
                if( pTerrainCache->Terrains.find( sTerrainReference )->second.bFilePresent )
                {
                    DEBUG(" Calling LoadTerrainFile() ");
                    LoadTerrainFile( pTerrainCache->Terrains.find( sTerrainReference )->second.sOurFilePath.c_str() );
                }
            }
        }

        if( !bTerrainLoaded )
        {
            //DEBUG(" Terrain not loaded");
            return;
        }
    }


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

    pmvGraphics->Bind2DTexture( iTextureID );
    //DEBUG(  "Drawing terrain..." ); // DEBUG
    pmvGraphics->RenderTerrain( g_HeightMap, iMapSize );
    // DEBUG(  "Terrain drawn" ); // DEBUG
    pmvGraphics->Bind2DTexture( 0 );

    pmvGraphics->PopMatrix();

    //      Debug( "Terrain::Draw done\n" );
    Prim::Draw();
}
void Terrain::DrawSelected()
{
    if( !bTerrainLoaded )
    {
        return;
    }

    float mcolor[4];
    mcolor[0] = 1.0 - color0.r;
    mcolor[1] = 1.0 - color0.g;
    mcolor[2] = 1.0 - color0.b;
    mcolor[3] = 0.4;
    pmvGraphics->SetMaterialColor( mcolor);

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
        pmvGraphics->DrawWireframeBox( 20 );
    }

    pmvGraphics->PopMatrix();

    //Debug( "Terrain::DrawEx done\n" );
    Prim::Draw();
}

void Terrain::WriteToDeepXML( TiXmlElement *pElement )
{
    WriteToXMLDoc( pElement );
    pElement->RemoveAttribute("ireference");
    pElement->RemoveAttribute("iparentreference");
}

void Terrain::SetColor( int face, const Color &newcolor )
{
    color0 = newcolor;
}

void Terrain::SetTexture( int face, const char *sTextureReference )
{
    sprintf( this->sTextureReference, sTextureReference );
}

void Terrain::SetSkybox( const char *sSkyboxReference )
{
    sprintf( this->sSkyboxReference, sSkyboxReference );
}

Color &Terrain::GetColor( int face )
{
    return color0;
}

const char *Terrain::GetTexture( int face )
{
    return sTextureReference;
}

const char *Terrain::GetSkybox()
{
    return sSkyboxReference;
}
