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

#ifndef _MVTerrain_H
#define _MVTerrain_H

#include "Object.h"
#include "Prim.h"

//! A terrain is a height-mapped prim based on a 128x128 .raw file

//! A terrain is a special type of prim.
//! It is a height-mapped prim based on a 128x128 .raw file

// For documentaiton on member functions, see doucmentaiton of Object class
class Terrain : public Prim
{
public:
  // static int iNextTerrainListNumber;

   Color color0;  //!< terrain color
   char sTextureReference[33];  //!< reference of texture on terrain
   char sSkyboxReference[33];  //!< reference of skybox associated with terrain

   char sTerrainReference[33];  //!< md5 checksum of terrain file to load and display
  // int iOurTerrainListNumber;
   
   bool bTerrainLoaded;        //!< has the terrain been loaded into g_HeightMap yet?
   
  // bool bTerrainListInitialized;
   
   static const int iMapSize = 128;  //!< size of terrain map; always 128
   unsigned char g_HeightMap[ iMapSize * iMapSize ];  //!< Heightmap data
   Vector3 VectorNormals[ iMapSize * iMapSize ];  //!< normals data

   Terrain()
   {  
   	   DEBUG( "mvTerrain Terrain::Terrain()" );
   	   sprintf( ObjectType, "PRIM" );
   	   sprintf( sDeepObjectType, "TERRAIN" );
   	   sprintf( PrimType, "TERRAIN" );
   	   color0.r = 0.8;
   	   color0.g = 0.2;
   	   color0.b = 0.4;
   	   sprintf( sTextureReference, "" );
   	   sprintf( sSkyboxReference, "" );
   	   sprintf( sTerrainReference, "" );
   	   //iOurTerrainListNumber = 0;
   	   bTerrainEnabled = true;
   	   bTerrainLoaded = false;
   	   //bTerrainListInitialized = false;
  }
   static void GetCreateSQLFromXMLEx( TiXmlElement *pElement, char *SQL );
   static void GetUpdateSQLFromXMLEx( TiXmlElement *pElement, char *SQL );
   virtual char *GetWorldStateRetrieveSQL();
   virtual void PopulateFromDBRow();
   virtual void UpdateFromXML( TiXmlElement *pElement );
   void LoadFromXML( TiXmlElement *pElement );
   virtual void WriteToXMLDoc( TiXmlElement *pElement );
   //virtual void CopyTo( Terrain *ptargetterrain );
   const Terrain &Terrain::operator=( const Terrain &IncomingPrim );
   virtual void Draw();
   virtual void DrawSelected();
   virtual void WriteToDeepXML( TiXmlElement *pElement );
   virtual void SetColor( int face, const Color &newcolor );
   virtual Color &GetColor( int face );
   virtual void SetTexture( int face, const char *sTextureReference );
   virtual void SetSkybox( const char *sSkyboxReference );
   virtual const char *GetTexture( int face );
   virtual const char *GetSkybox( );

   bool LoadTerrainFile( const char *sFilePath );   //!< loads a 128x128 8-bit raw file as the terrain data
   void CalculateNormals();  //!< calculates normals to terrain
      
   void RenderTerrain();          //!< renders terrain to opengl
   virtual int Height( const int X, const int Y) const;    //!< gets the height of a piece of terrain at position X,Y
   void RenderHeightMap();         //!< called by RenderTerrain
protected:
};

#endif // _MVTerrain_H
