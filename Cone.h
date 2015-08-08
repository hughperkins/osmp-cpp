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
//! \brief Contains all the prim/object classes that can be rezed in the world/sim
//!
//! Contains all the prim / object classes that can be rezed in the world/sim, except Terrain which has its own
//! headerfile (Terrain.h)
//!
//! Object is the pure virtual base class of all objects
//! there are two main derivative branches currently : Prim and ObjectGrouping
//! Prims is stuff you can see, like cubes and spheres (when we implement spheres)
//! ObjectGroupings are groups of objects - could be prims or other objectgroupings
//! groupings can be hierarhical
//! currently max 10 objects per group, but this is easily extended (probably use a map class for this)
//!
//! This module uses BasicTypes.h which defines Vector3, Rot, and Color
//!
//! Objects are responsible for drawing themselves when called with Draw()
//! similarly, a lot of database stuff is in the objects themselves when its object-specific
//! (data-oriented architecture)
//!
//! Object contains static pointers to certain objects, stored in interface pointers, which helps
//! reduce the dependencies that a data-centric architecture encourages

#ifndef _Cone_H
#define _Cone_H

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif


#ifdef _WIN32
#include <windows.h>
#endif

//#include <GL/gl.h>
//#include <GL/glu.h>
//#include <GL/glut.h>

#include <string>

#include "tinyxml.h"

#include "BasicTypes.h"
#include "Collision.h"
#include "GraphicsInterface.h"
#include "IDBInterface.h"
#include "Diag.h"
#include "Math.h"
#include "TextureInfoCache.h"
#include "TerrainInfoCache.h"
#include "MeshInfoCache.h"

#include "Prim.h"

//! A cone is a special type of prim.
//
// For documentaiton on member functions, see doucmentaiton of Object class
class Cone : public Prim
{
public:
   Color color0;
   //int iTextureReference;
   char sTextureReference[33];
   // int iTextureID;

   Cone()
   {
   	   DEBUG( "Cone::Cone" );
   	   sprintf( ObjectType, "PRIM" );
   	   sprintf( sDeepObjectType, "CONE" );
   	   sprintf( PrimType, "CONE" );
   	   color0.r = 0.2;
   	   color0.g = 0.2;
   	   color0.b = 0.2;
   	   sprintf( sTextureReference, "" );

   	  // iTextureID = 0;  // temp session for fast drawing
  }
   static void GetCreateSQLFromXMLEx( TiXmlElement *pElement, char *SQL );
   static void GetUpdateSQLFromXMLEx( TiXmlElement *pElement, char *SQL );
   virtual char *GetWorldStateRetrieveSQL();
   virtual void PopulateFromDBRow();
   virtual void UpdateFromXML( TiXmlElement *pElement );
   void LoadFromXML( TiXmlElement *pElement );
   virtual void WriteToXMLDoc( TiXmlElement *pElement );
   //virtual void CopyTo( Cone *ptargetcube );
   const Cone &Cone::operator=( const Cone &IncomingPrim );
   virtual void Draw();
   virtual void DrawSelected();
   virtual void WriteToDeepXML( TiXmlElement *pElement );
   virtual void SetColor( int face, const Color &newcolor );
   virtual Color &GetColor( int face );
   virtual void SetTexture( int face, const char *sTextureReference );
   virtual const char *GetTexture( int face );
};

#endif // _Cone_H
