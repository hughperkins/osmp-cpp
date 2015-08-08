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
//!
//! The MVmvMd2Mesh Class is a storage class for md2 Quake 2 animated meshes
//! It is a derivative of Prim, via MVMESH, because meshes are considered a type of prim
//! They have scale, position, rotation and texture.
//! the actual md2 loader comes from http://nehe.gamedev.net

// see mvobject.h for documetatnion on most of the functions in this class

#ifndef _MVmvMd2Mesh_H
#define _MVmvMd2Mesh_H

#include "MD2Mesh.h"

#include "Prim.h"
#include "Mesh.h"

//! The MVmvMd2Mesh Class is a storage class for md2 Quake 2 animated meshes

//! The MVmvMd2Mesh Class is a storage class for md2 Quake 2 animated meshes
//! It is a derivative of Prim, via MVMESH, because meshes are considered a type of prim
//! They have scale, position, rotation and texture.
//! the actual md2 loader comes from http://nehe.gamedev.net
//!
//! For documentaiton on member functions, see doucmentaiton of Object class
class mvMd2Mesh : public MESH
{
public:
   char sMeshReference[33];
   bool bMeshLoaded;

   mvMd2Mesh();
   ~mvMd2Mesh();
   static void GetCreateSQLFromXMLEx( TiXmlElement *pElement, char *SQL );
   static void GetUpdateSQLFromXMLEx( TiXmlElement *pElement, char *SQL );
   virtual char *GetWorldStateRetrieveSQL();
   virtual void PopulateFromDBRow();
   virtual void UpdateFromXML( TiXmlElement *pElement );
   virtual void LoadFromXML( TiXmlElement *pElement );
   virtual void WriteToXMLDoc( TiXmlElement *pElement );
   const mvMd2Mesh &mvMd2Mesh::operator=( const mvMd2Mesh &IncomingPrim );
   //virtual void CopyTo( mvMd2Mesh *ptargetmd2mesh );
   virtual void Draw();
   virtual void DrawSelected();

      
protected:
   LaminarChaos::MD2Mesh MD2MeshObject;
   //virtual void RenderMesh();                      //!< renders the mesh to opengl
   virtual bool LoadMeshFile( string sFilePath );  //!< loads the .md2 meshfile whose path is passed-in
};

#endif // _MVmvMd2Mesh_H
