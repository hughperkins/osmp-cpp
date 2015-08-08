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
//! \brief A mesh is a base class for animated mesh prims

#ifndef _MVMESH_H
#define _MVMESH_H

#include "Prim.h"

//! A mesh is a base class for animated mesh prims

//! A mesh is a special type of prim for holding meshes
//! For documentaiton on member functions, see doucmentaiton of Object class
//! A MESH is never instantiated itself: instead a specified type of mesh - eg MVmvMd2Mesh - is derived
class MESH : public Prim
{
public:
   Color color0;  
   char sTextureReference[33];

   MESH()
   {  
   	   DEBUG( "MESH::MESH" );
   	   sprintf( ObjectType, "PRIM" );
   	   sprintf( sDeepObjectType, "MESH" );
   	   sprintf( PrimType, "mvMd2Mesh" );
   	   color0.r = 0.8;
   	   color0.g = 0.2;
   	   color0.b = 0.4;
   	   sprintf( sTextureReference, "" );
  }
   static void GetCreateSQLFromXMLEx( TiXmlElement *pElement, char *SQL );
   static void GetUpdateSQLFromXMLEx( TiXmlElement *pElement, char *SQL );
   virtual char *GetWorldStateRetrieveSQL();
   virtual void PopulateFromDBRow();
   virtual void UpdateFromXML( TiXmlElement *pElement );
   virtual void LoadFromXML( TiXmlElement *pElement );
   virtual void WriteToXMLDoc( TiXmlElement *pElement );
   //virtual void CopyTo( MESH *ptargetmesh );
   const MESH &MESH::operator=( const MESH &IncomingPrim );
   virtual void Draw();
   virtual void DrawSelected();
   virtual void WriteToDeepXML( TiXmlElement *pElement );
   virtual void SetColor( int face, const Color &newcolor );
   virtual Color &GetColor( int face );
   virtual void SetTexture( int face, const char *sTextureReference );
   virtual const char *GetTexture( int face );

   virtual bool LoadMeshFile( string sFilePath ) = 0;
      
protected:
};

#endif // _MVMESH_H
