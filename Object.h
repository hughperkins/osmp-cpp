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

#ifndef _Object_H
#define _Object_H

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
//#include "modelloader.h"
//#include "ModelFactoryInterface.h"

class Cube;
class Avatar;
class Prim;
class ObjectGrouping;

//! Base class for all prims and objects in the world.  Virtual: cant be instantiated itself
class Object
{
protected:

public:
   static void (*pfCallbackAddName)( int );
   static IDBInterface *pdbinterface;  //!< added this to reduce dependencies somewhat
  // static NeHe::ModelLoader *pMD2ModelLoader;        //!< putting this here to reduce dependencies
  // static ModelFactoryInterface *pModelFactory;    //!< reducing dependencies
   static mvGraphicsInterface *pmvGraphics; //!< added this to reduce dependencies

   static TextureInfoCache *ptextureinfocache;   //!< putting this here to reduce dependencies
   static TerrainCacheClass *pTerrainCache;       //!< putting this here to reduce dependencies
   static MeshInfoCacheClass *pMeshInfoCache;   //!< putting this here to reduce dependencies

   int iReference;            //!< fundamental unique identifier for the object.  Created by dbinterface.  Unique.  (LIke a guiid)
   int iParentReference;      //!< reference for object's parent, or 0 if object is a toplevel object
   char ObjectType[17];       //!< "PRIM" or "OBJECTGROUPING" are only two options here currently
   char sDeepObjectType[17];  //!< The actual class of the object, eg Cube, Avatar etc
   int iownerreference;       //!< iReference of owner/creator of object
   Vector3 pos;                   //!< position of object in sim/world
   Rot rot;                   //!< rotation of object
   char sScriptReference[33]; //!< md5 checksum of assigned script

   char sObjectName[ 65 ];   //!< Object's name

   bool bPhysicsEnabled;  //!< Physics enabled
   bool bPhantomEnabled;  //!< Phantom enabled
   bool bGravityEnabled;  //!< Gravity enabled
   bool bTerrainEnabled;  //!< Terrain enabled (no, I dont remember what this means :-O)
   Vector3 vVelocity;      //!< Current linear velocity
   Vector3 vAngularVelocity;      //!< Current angular velocity
   Vector3 vLocalForce;      //!< Current local linear force spontaneously acting on object
   Vector3 vLocalTorque;      //!< Current local rotational torque spontaneously acting on object

  const char *GetObjectType() const{ return ObjectType; }
  const char *GetDeepObjectType() const{ return sDeepObjectType; }
  const char *GetObjectName() const{ return sObjectName; }

   static void SetAddNameCallback( void (*pfCallbackAddName)( int ) )
   {
      Object::pfCallbackAddName = pfCallbackAddName;
   }

   static void SetDBAbstractionLayer( IDBInterface &rdbinterface )
   {
   	  Object::pdbinterface = &rdbinterface;
   }

   static void SetmvGraphics( mvGraphicsInterface &rmvGraphics )
   {
   	  Object::pmvGraphics = &rmvGraphics;
   }

   static void Settextureinfocache( TextureInfoCache &rtextureinfocache )
   {
   	  Object::ptextureinfocache = &rtextureinfocache;
   }

   static void SetMeshInfoCache( MeshInfoCacheClass &rMeshInfoCache )
   {
   	  Object::pMeshInfoCache = &rMeshInfoCache;
   }

   static void SetTerrainCache( TerrainCacheClass &rTerrainCache )
   {
   	  Object::pTerrainCache = &rTerrainCache;
   }

   Object()   //!< Constructor.  Initializes object with default values when object is created.  If you add any new properties to object,
              //!< dont forget to add them in here
   {
   	   iReference = 0;
   	   iParentReference = 0;
   	   iownerreference = 0;
   	   pos.x = 0;
   	   pos.y = 0;
   	   pos.z = 0;
   	   rot.x = 0;
   	   rot.y = 0;
   	   rot.z = 0;
   	   rot.s = 1;
   	   sprintf( sScriptReference, "" );
   	   sprintf( sObjectName, "" );

   	   bPhysicsEnabled = false;
   	   bPhantomEnabled = false;
   	   bTerrainEnabled = false;
   	   bGravityEnabled = true;

   	   vAngularVelocity.x = 0.0;
   	   vAngularVelocity.y = 0.0;
   	   vAngularVelocity.z = 0.0;

   	   vVelocity.x = 0;
   	   vVelocity.y = 0;
   	   vVelocity.z = 0;

   	   vLocalForce.x = 0;
   	   vLocalForce.y = 0;
   	   vLocalForce.z = 0;

   	   vLocalTorque.x = 0;
   	   vLocalTorque.y = 0;
   	   vLocalTorque.z = 0;
   }
   static void GetCreateSQLFromXML( TiXmlElement *pElement, char *SQL );    //!< Calls the appropriate GetCreateSQLFromXMlEx function, depending on pElement->Attribute("type")
   static void GetCreateSQLFromXMLEx( TiXmlElement *pElement, char *SQL );  //!< fills string SQL with SQL commands to create the object specified by pElement XML in the database
   static void GetUpdateSQLFromXML( TiXmlElement *pElement, char *SQL );    //!< Calls the appropriate GetUpdateSQLFromXMlEx function, depending on pElement->Attribute("type")
   static void GetUpdateSQLFromXMLEx( TiXmlElement *pElement, char *SQL );  //!< fills string SQL with SQL commands to update the object specified by pElement XML in the database
   static void GetDeleteSQLFromXML( TiXmlElement *pElement, char *SQL );    //!< Calls the appropriate GetDeleteSQLFromXMlEx function, depending on pElement->Attribute("type")
   static void GetDeleteSQLFromXMLEx( TiXmlElement *pElement, char *SQL );   //!< fills string SQL with SQL commands to delete specified by pElement XML from the databse
   static Object *CreateNewObjectFromXML( TiXmlElement *pElement );          //!< Creates a new object/cube/avatar etc from the XMl in pElement
   virtual char *GetWorldStateRetrieveSQL() = 0;                             //!< Gets SQL statement to retrieve objects of type of instantiated object (eg call it on a cube object -> returns SQL to retrieve cubes)
   void GetDeepObjectType_From_ObjectTypeAndPrimType();              //!< populates sDeepObjectType field (same as pElement->Attribute("type") value, or name of object class )
   static const char *DeepTypeToObjectType( const char *DeepObjectType );
   virtual void PopulateFromDBRow();                                       //!< Fills the current object with the values in the currently retrieve DB row in DBAbstractionLayer module
   const Object &Object::operator=( const Object &IncomingObject );
//   virtual void CopyTo( Object *ptargetobject );                           //!< Copies current object to ptargetobject
   virtual void UpdateFromXML( TiXmlElement *pElement );                   //!< updates this object from the XML pElement
   virtual void LoadFromXML( TiXmlElement *pElement );                     //!< loads this object values from the XML pElement
   virtual void WriteToXMLDoc( TiXmlElement *pElement );                   //!< writes the current object to the XML pElement
   virtual void Draw();                                                     //!< Draws this object to OpenGL/GLUT, including applying textures, rotating, and translating (leaves OpenGL Matrices how they were prior to function call)
   virtual void DrawSelected();                                             //!< Draws this object's highlighting to OpenGL/GLUT, so that object appears "Selected" (leaves OpenGL Matrices how they were prior to function call)
   virtual void WriteToDeepXML( TiXmlElement *pElement ) = 0;               //!< Writes object to single recursive XMl structure, removing iReference and iParentReference, which are implicit in the recursion
   friend ostream& operator<< ( ostream& os, Object &object );              //!< serializes object to a stream (eg you can do "cout << *pMyObject;" )
};

#endif // _Object_H
