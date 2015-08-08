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
//! \brief The MVmvMd2Mesh Class is a storage class for md2 Quake 2 animated meshes

// For documentation see header file mvobjectstorage.h and mvMd2Mesh.h

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
#include "MeshInfoCache.h"

#include "MD2Mesh.h"

#include "mvMD2Mesh.h"

/*
void mvMd2Mesh::RenderMesh()
{
 if( pModel != 0 )
 {
 // DEBUG(  "rendering mesh.." ); // DEBUG
  pmvGraphics->PushMatrix();

  const Vector3 &boxmin = pModel->GetBoundingBoxMin();
  const Vector3 &boxmax = pModel->GetBoundingBoxMax();

 // DEBUG(  "boundingboxes " << boxmin << " " << boxmax ); // DEBUG

//  glTranslatef( - ( boxmax.x - boxmin.x ) / 2.0f, - ( boxmax.y - boxmin.y ) / 2.0f, - ( boxmax.z - boxmin.z ) / 2.0f );

  pmvGraphics->Scalef( 1.0f / (boxmax.x - boxmin.x), 1.0f / (boxmax.y - boxmin.y), 1.0f / (boxmax.z - boxmin.z) );

  pmvGraphics->Rotatef( 90, 1,0,0);

  pModel->DrawTimeFrame(0,framepos);
  framepos+= 0.125;
  if( framepos >= pModel->GetNumFrames() )
  {
   framepos = 0;
  }
  pmvGraphics->PopMatrix();

 // DEBUG(  "...done" ); // DEBUG
 }
}
*/

bool mvMd2Mesh::LoadMeshFile( string sMeshFile )
{
    DEBUG(  "loading mesh file " << sMeshFile << " ... " ); // DEBUG
    char FilePath[255];
    sprintf( FilePath, "%s", sMeshFile.c_str() );  // We should probably add consts into the laminarchaos interface; something for the future
    if( MD2MeshObject.LoadMeshFile( FilePath ) )
    {
        bMeshLoaded = true;
    }
    else
    {
        bMeshLoaded = false;
    }
    return bMeshLoaded;

    // if( Object::pMD2ModelLoader != 0 )
    // {
    //   DEBUG(  "MD2ModelLoader present" ); // DEBUG
    //   if( pModel != 0 )
    //   {
    //     DEBUG(  "Model class has been created" ); // DEBUG
    //    //DEBUG(  "number of frames prior to load is " << pModel->GetNumFrames() ); // DEBUG
    //   //DEBUG(  "number of frames prior to load is " << pMD2ModelLoader->GetNumFrames() ); // DEBUG
    //       if( !pModel->Load( sMeshFile, *( Object::pMD2ModelLoader ) ) )
    //       {
    //         DEBUG(  "couldnt load modelfile " << sMeshFile ); // DEBUG
    //         return false;
    //       }
    //       else
    //       {
    //      numframes= pModel->GetNumFrames();
    //     // boundingboxmin = Object::pMD2ModelLoader->GetBoundingBoxMin();  // htis should really be shifted into the model
    //     // boundingboxmax = Object::pMD2ModelLoader->GetBoundingBoxMax();
    //      bMeshLoaded = true;
    //      return true;
    //       }
    //    }
    //    else
    //    {
    //      DEBUG(  "ERROR: Model class not loaded" ); // DEBUG
    ///    }
    //  }
    //  else
    //  {
    //   DEBUG(  "ERROR: MD2ModelLoader not present" ); // DEBUG
    //  }
}

void mvMd2Mesh::GetCreateSQLFromXMLEx( TiXmlElement *pElement, char *SQL )
{
    //    Debug( "mvMd2Mesh::GetCreateSQLFromXMLEx\n" );

    sprintf( SQL, "insert into md2meshes (i_reference, s_mesh_reference )select %i,s_mesh_reference from md2meshes where i_reference = 0;",
             atoi( pElement->Attribute( "ireference" ) ) );

    MESH::GetCreateSQLFromXMLEx( pElement, SQL );
}
void mvMd2Mesh::GetUpdateSQLFromXMLEx( TiXmlElement *pElement, char *SQL )
{
    //   Debug( "mvMd2Mesh::GetUpdateSQLFromXMLEx\n" );

    TiXmlHandle docHandle( pElement );
    if( docHandle.FirstChild("geometry").FirstChild("mesh").Element() )
    {
        sprintf( SQL, "%s update md2meshes set s_mesh_reference='%s' where i_reference = %i;",
                 SQL,
                 pElement->FirstChildElement("geometry")->FirstChildElement("mesh")->Attribute("smeshreference" ),
                 atoi( pElement->Attribute( "ireference" ) ) );
    }
    MESH::GetUpdateSQLFromXMLEx( pElement, SQL );
}
char *mvMd2Mesh::GetWorldStateRetrieveSQL()
{
    return   "SELECT a.i_reference,s_object_name,a.s_object_type,b.s_prim_type,a.i_owner,i_parentreference,"
             "pos_x,pos_y,pos_z,rot_x,rot_y,rot_z,rot_s,scale_x,scale_y,scale_z,s_texture_reference,b_physics_enabled,b_phantom_enabled,b_terrain_enabled,b_gravity_enabled,s_script_reference,s_mesh_reference,"
             "color_0_r,color_0_g,color_0_b FROM objects a, prims b, meshes c, md2meshes d "
             "where a.i_reference=b.i_reference and a.i_reference=c.i_reference and a.i_reference = d.i_reference and "
             "a.s_object_type='Prim' and b.s_prim_type='mvMd2Mesh';";
}
void mvMd2Mesh::PopulateFromDBRow()
{
    if( pdbinterface != 0 )
    {
        if( pdbinterface->GetFieldValueByName( "s_mesh_reference" ) != NULL )
        {
            sprintf( sMeshReference, pdbinterface->GetFieldValueByName( "s_mesh_reference" ) );
        }
        else
        {
            sprintf( sMeshReference, "" );
        }
    }

    MESH::PopulateFromDBRow();
}
void mvMd2Mesh::UpdateFromXML( TiXmlElement *pElement )
{
    // Debug( "mvMd2Mesh::UpdateFromXMLEx\n" );

    TiXmlHandle docHandle( pElement );
    if( docHandle.FirstChild("geometry").FirstChild("mesh").Element() )
    {
        sprintf( sMeshReference, pElement->FirstChildElement("geometry")->FirstChildElement("mesh")->Attribute("smeshreference" ) );
    }

    MESH::UpdateFromXML( pElement );
}
void mvMd2Mesh::LoadFromXML( TiXmlElement *pElement )
{
    //  Debug( "mvMd2Mesh::LoadFromXML\n" );
    //     color0.r = 1.0; color0.g = 0; color0.b = 1.0;

    TiXmlHandle docHandle( pElement );
    if( docHandle.FirstChild("geometry").FirstChild("mesh").Element() )
    {
        sprintf( sMeshReference, pElement->FirstChildElement("geometry")->FirstChildElement("mesh")->Attribute("smeshreference" ) );
    }

    // DEBUG(  "end of mvMd2Mesh::LoadFromXML" ); // DEBUG
    MESH::LoadFromXML( pElement );
}
void mvMd2Mesh::WriteToXMLDoc( TiXmlElement *pElement )
{
    //  Debug( "mvMd2Mesh::WriteToXMLDoc\n" );
    pElement->SetAttribute( "type", "mvMd2Mesh" );

    AddXmlChildIfNecessary( pElement, "geometry" );
    AddXmlChildIfNecessary( pElement->FirstChildElement("geometry"), "mesh" );
    pElement->FirstChildElement("geometry")->FirstChildElement("mesh")->SetAttribute("smeshreference", sMeshReference );

    MESH::WriteToXMLDoc( pElement );
}
const mvMd2Mesh &mvMd2Mesh::operator=( const mvMd2Mesh &IncomingPrim )
{
    strcpy( this->sMeshReference, IncomingPrim.sMeshReference );
    MESH::operator=( IncomingPrim );
    return *this;
}

void mvMd2Mesh::Draw()
{
    if( !bMeshLoaded )
    {
        //DEBUG(  "mvmd2mesh.cpp Draw() mesh not loaded yet" ); // DEBUG
        if( pMeshInfoCache != 0 )
        {
            // DEBUG(  "mvmd2mesh.cpp pMeshInfoCache loaded ok" ); // DEBUG
            if( pMeshInfoCache->Files.find( sMeshReference ) != pMeshInfoCache->Files.end() )
            {
                static int iCount = 0;
                if( iCount > 50 )
                {
                    DEBUG(  "mvmd2mesh.cpp found refernce in cache " << sMeshReference ); // DEBUG
                    iCount = 0;
                }
                iCount++;
                if( pMeshInfoCache->Files.find( sMeshReference )->second.bFilePresent )
                {
                    DEBUG(  "mvmd2mesh.cpp file is available" ); // DEBUG
                    LoadMeshFile( pMeshInfoCache->Files.find( sMeshReference )->second.sOurFilePath.c_str() );
                }
            }
        }
        else
        {
            DEBUG(  "mvmd2mesh: ERROR: pMeshInfoCache not loaded" ); // DEBUG
        }

        if( !bMeshLoaded )
        {
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
    //DEBUG(  "Drawing md2mesh..." ); // DEBUG
    MD2MeshObject.Render();
    // DEBUG(  "mvMd2Mesh drawn" ); // DEBUG
    pmvGraphics->Bind2DTexture( 0 );

    pmvGraphics->PopMatrix();

    //      Debug( "mvMd2Mesh::Draw done\n" );
    MESH::Draw();
}
void mvMd2Mesh::DrawSelected()
{
    if( !bMeshLoaded )
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
        pmvGraphics->DrawWireframeBox( 10 );
    }

    pmvGraphics->PopMatrix();

    //      Debug( "mvMd2Mesh::DrawEx done\n" );
    MESH::Draw();
}

mvMd2Mesh::mvMd2Mesh()
{
    DEBUG( "mvMd2Mesh::mvMd2Mesh" );
    sprintf( sDeepObjectType, "mvMd2Mesh" );

    sprintf( sMeshReference, "" );
    bMeshLoaded = false;

    //  pModel = 0;

    //   if( pModelFactory != 0 )
    //  {
    //     DEBUG(  "making new model..." ); // DEBUG
    //     pModel = pModelFactory->NewModel();
    //  }
    //  else
    // {
    //    DEBUG(  "ERROR mvmd2mesh.cpp md2mesh() ModelFactory not present!" ); // DEBUG
    // }

    //  framepos = 0;
}

mvMd2Mesh::~mvMd2Mesh()
{
    //if( pModel != 0 )
    //{
    //  delete( pModel );
    //  pModel = 0;
    //}
}
