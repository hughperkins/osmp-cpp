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
//! \brief Object is the base class for all prims/objects in the world
// see headerfile mvobjectstorage.h for documentation

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif


#ifdef _WIN32
#include <windows.h>
#endif

//#include <GL/gl.h>
//#include <GL/glu.h>
//#include <GL/glut.h>

#include "tinyxml.h"

#include "BasicTypes.h"
#include "GraphicsInterface.h"
#include "IDBInterface.h"
#include "Diag.h"
#include "XmlHelper.h"

#include "Avatar.h"
#include "Sphere.h"
#include "Cube.h"
#include "Cone.h"
#include "Cylinder.h"
#include "ObjectGrouping.h"
#include "Terrain.h"
#include "mvMd2Mesh.h"

#include "Object.h"

void (*Object::pfCallbackAddName)( int ) = NULL;

IDBInterface *Object::pdbinterface = 0;
//NeHe::ModelLoader *Object::pMD2ModelLoader = 0;
//ModelFactoryInterface *Object::pModelFactory = 0;

mvGraphicsInterface *Object::pmvGraphics = 0;
TextureInfoCache *Object::ptextureinfocache = 0;
TerrainCacheClass *Object::pTerrainCache = 0;
MeshInfoCacheClass *Object::pMeshInfoCache = 0;

const char *Object::DeepTypeToObjectType( const char *DeepObjectType )
{
    if( strcmp( DeepObjectType, "CUBE" ) == 0 )
    {
        return "PRIM";
    }
    else if( strcmp( DeepObjectType, "SPHERE" ) == 0 )
    {
        return "PRIM";
    }
    else if( strcmp( DeepObjectType, "CYLINDER" ) == 0 )
    {
        return( "PRIM" );
    }
    else if( strcmp( DeepObjectType, "CONE" ) == 0 )
    {
        return( "PRIM" );
    }
    else if( strcmp( DeepObjectType, "TERRAIN" ) == 0 )
    {
        return( "PRIM" );
    }
    else if( strcmp( DeepObjectType, "mvMd2Mesh" ) == 0 )
    {
        return( "PRIM" );
    }
    else if( strcmp( DeepObjectType, "AVATAR" ) == 0 )
    {
        return( "OBJECTGROUPING" );
    }
    else if( strcmp( DeepObjectType, "OBJECTGROUPING" ) == 0 )
    {
        return( "OBJECTGROUPING" );
    }
    else
    {
        Debug( "DeepTypeToObjectType() WARNING: unknown deep object type type [%s]\n", DeepObjectType );
        return "";
    }
}

void Object::PopulateFromDBRow()
{
    //  Debug( "Object::PopulateFromDBRow()\n" );

    if( pdbinterface != 0 )
    {
        iReference = atoi( pdbinterface->GetFieldValueByName( "i_reference" ) );
        iParentReference = atoi( pdbinterface->GetFieldValueByName( "i_parentreference" ) );
        sprintf( ObjectType, pdbinterface->GetFieldValueByName( "s_object_type" ) );

        if( pdbinterface->GetFieldValueByName( "s_object_name" ) != NULL )
        {
            sprintf( sObjectName, "%.64s", pdbinterface->GetFieldValueByName( "s_object_name" ) );
        }

        GetDeepObjectType_From_ObjectTypeAndPrimType();

        iownerreference = atoi( pdbinterface->GetFieldValueByName( "i_owner" ) );
        pos.x = atof( pdbinterface->GetFieldValueByName( "pos_x" ) );
        pos.y = atof( pdbinterface->GetFieldValueByName( "pos_y" ) );
        pos.z = atof( pdbinterface->GetFieldValueByName( "pos_z" ) );

        rot.x = atof( pdbinterface->GetFieldValueByName( "rot_x" ) );
        rot.y = atof( pdbinterface->GetFieldValueByName( "rot_y" ) );
        rot.z = atof( pdbinterface->GetFieldValueByName( "rot_z" ) );
        rot.s = atof( pdbinterface->GetFieldValueByName( "rot_s" ) );

        //  DEBUG(  "reading physicsenabled field..." ); // DEBUG
        if( pdbinterface->GetFieldValueByName( "b_physics_enabled" ) == NULL  )
        {
            bPhysicsEnabled = false;
        }
        else
        {
            //      DEBUG(  "[" << pdbinterface->GetFieldValueByName( "b_physics_enabled" ) << "]" ); // DEBUG
            if( atoi( pdbinterface->GetFieldValueByName( "b_physics_enabled" ) ) == 1 )
            {
                bPhysicsEnabled = true;
            }
            else
            {
                bPhysicsEnabled = false;
            }
        }

        if( pdbinterface->GetFieldValueByName( "b_phantom_enabled" ) == NULL  )
        {
            bPhantomEnabled = false;
        }
        else
        {
            if( atoi( pdbinterface->GetFieldValueByName( "b_phantom_enabled" ) ) == 1 )
            {
                bPhantomEnabled = true;
            }
            else
            {
                bPhantomEnabled = false;
            }
        }

        if( pdbinterface->GetFieldValueByName( "b_terrain_enabled" ) == NULL  )
        {
            bTerrainEnabled = false;
        }
        else
        {
            if( atoi( pdbinterface->GetFieldValueByName( "b_terrain_enabled" ) ) == 1 )
            {
                bTerrainEnabled = true;
            }
            else
            {
                bTerrainEnabled = false;
            }
        }

        if( pdbinterface->GetFieldValueByName( "b_gravity_enabled" ) == NULL  )
        {
            bGravityEnabled = false;
        }
        else
        {
            //DEBUG(  "[" << pdbinterface->GetFieldValueByName( "b_gravity_enabled" ) << "]" ); // DEBUG
            if( atoi( pdbinterface->GetFieldValueByName( "b_gravity_enabled" ) ) == 1 )
            {
                bGravityEnabled = true;
            }
            else
            {
                bGravityEnabled = false;
            }
        }
        //    DEBUG(  "done" ); // DEBUG

        if( pdbinterface->GetFieldValueByName( "s_script_reference" ) != NULL )
        {
            sprintf( sScriptReference, pdbinterface->GetFieldValueByName( "s_script_reference" ) );
        }
        else
        {
            sprintf( sScriptReference, "" );
        }
    }
}
const Object &Object::operator=( const Object &IncomingObject )
{
    this->iReference = IncomingObject.iReference;
    this->bPhysicsEnabled = IncomingObject.bPhysicsEnabled;
    this->bPhantomEnabled = IncomingObject.bPhantomEnabled;
    this->bTerrainEnabled = IncomingObject.bTerrainEnabled;
    this->iParentReference = IncomingObject.iParentReference;
    this->iownerreference = IncomingObject.iownerreference;

    strcpy( this->ObjectType, IncomingObject.ObjectType );
    strcpy( this->sObjectName, IncomingObject.sObjectName );
    this->pos = IncomingObject.pos;
    this->rot = IncomingObject.rot;
    return *this;
}

//void Object::CopyTo( Object *ptargetobject )
//{
//ptargetobject->iReference = iReference;
//    ptargetobject->bPhysicsEnabled = bPhysicsEnabled;
//ptargetobject->bPhantomEnabled = bPhantomEnabled;
//ptargetobject->bTerrainEnabled = bTerrainEnabled;
//ptargetobject->iParentReference = iParentReference;
//sprintf( ptargetobject->ObjectType, ObjectType );
//sprintf( ptargetobject->sObjectName, sObjectName );
//ptargetobject->iownerreference = iownerreference;
//ptargetobject->pos = pos;
//ptargetobject->rot = rot;
//}
void Object::GetDeleteSQLFromXMLEx( TiXmlElement *pElement, char *SQL )
{
    //  Debug( "Object::GetDeleteSQLFromXMLEx\n" );
    sprintf( SQL, "%s delete from objects where i_reference=%i;", SQL, atoi( pElement->Attribute( "ireference" ) ) );
}
void Object::GetCreateSQLFromXMLEx( TiXmlElement *pElement, char *SQL )
{
    //  Debug( "Object::GetCreateSQLFromXMLEx\n" );

    char sObjectType[16] = "";

    sprintf( sObjectType, DeepTypeToObjectType( pElement->Attribute( "type" ) ) );

    sprintf( SQL, "%s insert into objects "
             "(i_reference,i_parentreference,s_object_type,s_object_name,b_physics_enabled,b_gravity_enabled,b_phantom_enabled,b_terrain_enabled,i_owner,pos_x,pos_y,pos_z,rot_x,rot_y,rot_z,rot_s)"
             "select "
             "%i,0,'%s',s_object_name,b_physics_enabled,b_gravity_enabled,b_phantom_enabled,b_terrain_enabled,%i,%f,%f,%f,rot_x,rot_y,rot_z,rot_s from objects where i_reference=0;",
             SQL, atoi( pElement->Attribute( "ireference" ) ), sObjectType, atoi( pElement->Attribute( "owner" ) ),
             atof( pElement->FirstChildElement("geometry")->FirstChildElement("pos")->Attribute("x") ),
             atof( pElement->FirstChildElement("geometry")->FirstChildElement("pos")->Attribute("y") ),
             atof( pElement->FirstChildElement("geometry")->FirstChildElement("pos")->Attribute("z") )
           );

    if( strcmp( pElement->Attribute("type" ), "OBJECTGROUPING" ) == 0 )
    {}
}
void Object::GetUpdateSQLFromXMLEx( TiXmlElement *pElement, char *SQL )
{
    //  Debug( "Object::GetUpdateSQLFromXMLEx\n" );

    TiXmlHandle docHandle( pElement );
    char SetSQL[512] = "";

    //  DEBUG(  pElement ); // DEBUG

    if( pElement->Attribute( "iparentreference" ) != NULL )
    {
        //   DEBUG(  "updating parentref" ); // DEBUG
        sprintf( SetSQL, "%s i_parentreference=%i,", SetSQL, atoi( pElement->Attribute("iparentreference") ) );
    }

    if( pElement->Attribute("objectname" ) != NULL )
    {
        sprintf( SetSQL, "%s s_object_name='%s',", SetSQL, pElement->Attribute("objectname") );
    }

    if( docHandle.FirstChild("physics").Element() )
    {
        if( pElement->FirstChildElement("physics")->Attribute("state") != NULL )
        {
            if( strcmp( pElement->FirstChildElement("physics")->Attribute("state"), "on" ) == 0 )
            {
                sprintf( SetSQL, "%s b_physics_enabled=1,", SetSQL  );
            }
            else
            {
                sprintf( SetSQL, "%s b_physics_enabled=0,", SetSQL  );
            }
        }

        if( pElement->FirstChildElement("physics")->Attribute("gravity") != NULL )
        {
            if( strcmp( pElement->FirstChildElement("physics")->Attribute("gravity"), "on" ) == 0 )
            {
                sprintf( SetSQL, "%s b_gravity_enabled=1,", SetSQL  );
            }
            else
            {
                sprintf( SetSQL, "%s b_gravity_enabled=0,", SetSQL  );
            }
        }
    }

    if( docHandle.FirstChild("phantom").Element() )
    {
        if( pElement->FirstChildElement("phantom")->Attribute("state") != NULL )
        {
            if( strcmp( pElement->FirstChildElement("phantom")->Attribute("state"), "on" ) == 0 )
            {
                sprintf( SetSQL, "%s b_phantom_enabled=1,", SetSQL  );
            }
            else
            {
                sprintf( SetSQL, "%s b_phantom_enabled=0,", SetSQL  );
            }
        }
    }

    if( docHandle.FirstChild("terrain").Element() )
    {
        if( pElement->FirstChildElement("terrain")->Attribute("state") != NULL )
        {
            if( strcmp( pElement->FirstChildElement("terrain")->Attribute("state"), "on" ) == 0 )
            {
                sprintf( SetSQL, "%s b_terrain_enabled=1,", SetSQL  );
            }
            else
            {
                sprintf( SetSQL, "%s b_terrain_enabled=0,", SetSQL  );
            }
        }
    }


    if( docHandle.FirstChild("scripts").FirstChild("script").Element() )
    {
        sprintf( SetSQL, "%s s_script_reference='%s',", SetSQL, pElement->FirstChildElement("scripts")->FirstChildElement("script")->Attribute("sscriptreference") );
    }

    if( pElement->Attribute( "type" ) != NULL )
    {
        // Debug( "updating type (!)\n" );
        sprintf( SetSQL, "%s s_Object_type='%s',", SetSQL, DeepTypeToObjectType( pElement->Attribute("type") ) );
    }
    if( pElement->Attribute( "owner" ) != NULL )
    {
        //   DEBUG(  "updating owner" ); // DEBUG
        sprintf( SetSQL, "%s i_owner=%i,", SetSQL, atoi( pElement->Attribute("owner") ) );
    }
    if( docHandle.FirstChild("geometry").FirstChild("pos").Element() )
    {
        // DEBUG(  "updating pos" ); // DEBUG
        sprintf( SetSQL, "%s pos_x=%f, pos_y=%f, pos_z=%f,", SetSQL,
                 atof( pElement->FirstChildElement("geometry")->FirstChildElement("pos")->Attribute("x") ),
                 atof( pElement->FirstChildElement("geometry")->FirstChildElement("pos")->Attribute("y") ),
                 atof( pElement->FirstChildElement("geometry")->FirstChildElement("pos")->Attribute("z") )
               );
    }
    if( docHandle.FirstChild("geometry").FirstChild("rot").Element() )
    {
        //  DEBUG(  "updating rot" ); // DEBUG
        sprintf( SetSQL, "%s rot_x=%f,rot_y=%f,rot_z=%f,rot_s=%f,", SetSQL,
                 atof( pElement->FirstChildElement("geometry")->FirstChildElement("rot")->Attribute("x") ),
                 atof( pElement->FirstChildElement("geometry")->FirstChildElement("rot")->Attribute("y") ),
                 atof( pElement->FirstChildElement("geometry")->FirstChildElement("rot")->Attribute("z") ),
                 atof( pElement->FirstChildElement("geometry")->FirstChildElement("rot")->Attribute("s") )
               );
    }

    if( strlen( SetSQL ) > 0 )
    {
        DEBUG(  "creating update sql " << SetSQL << "..." ); // DEBUG
        sprintf( SQL, "%s update objects set %.*s where i_reference = %i;", SQL, strlen( SetSQL ) - 1, SetSQL, atoi( pElement->Attribute( "ireference" ) ) );
    }
}
void Object::UpdateFromXML( TiXmlElement *pElement )
{
    //  Debug( "Object::UpdateFromXMLEx\n" );

    //         sprintf( PrimType, pElement->Attribute("type") );
    TiXmlHandle docHandle( pElement );

    if( pElement->Attribute( "iparentreference" ) != NULL )
    {
        //  Debug( "updating parentreference\n" );
        iParentReference = atoi( pElement->Attribute("iparentreference") );
    }

    if( pElement->Attribute("objectname" ) != NULL )
    {
        sprintf( sObjectName, "%.64s", pElement->Attribute("objectname") );
    }

    if( docHandle.FirstChild("phantom").Element() )
    {
        if( pElement->FirstChildElement("phantom")->Attribute("state") != NULL )
        {
            if( strcmp( pElement->FirstChildElement("phantom")->Attribute("state"), "on" ) == 0 )
            {
                bPhantomEnabled = true;
            }
            else
            {
                bPhantomEnabled = false;
            }
        }
    }
    if( docHandle.FirstChild("terrain").Element() )
    {
        if( pElement->FirstChildElement("terrain")->Attribute("state") != NULL )
        {
            if( strcmp( pElement->FirstChildElement("terrain")->Attribute("state"), "on" ) == 0 )
            {
                bTerrainEnabled = true;
            }
            else
            {
                bTerrainEnabled = false;
            }
        }
    }

    if( docHandle.FirstChild("physics").Element() )
    {
        if( pElement->FirstChildElement("physics")->Attribute("state") != NULL )
        {
            if( strcmp( pElement->FirstChildElement("physics")->Attribute("state"), "on" ) == 0 )
            {
                bPhysicsEnabled = true;
            }
            else
            {
                bPhysicsEnabled = false;
            }
        }

        if( pElement->FirstChildElement("physics")->Attribute("gravity") != NULL )
        {
            if( strcmp( pElement->FirstChildElement("physics")->Attribute("gravity"), "on" ) == 0 )
            {
                bGravityEnabled = true;
            }
            else
            {
                bGravityEnabled = false;
            }
        }

        if( docHandle.FirstChild("physics").FirstChild("linearvelocity").Element() )
        {
            vVelocity.x = atof( pElement->FirstChildElement("physics")->FirstChildElement("linearvelocity")->Attribute("x"));
            vVelocity.y = atof( pElement->FirstChildElement("physics")->FirstChildElement("linearvelocity")->Attribute("y"));
            vVelocity.z = atof( pElement->FirstChildElement("physics")->FirstChildElement("linearvelocity")->Attribute("z"));
        }

        if( docHandle.FirstChild("physics").FirstChild("angularvelocity").Element() )
        {
            vAngularVelocity.x = atof( pElement->FirstChildElement("physics")->FirstChildElement("angularvelocity")->Attribute("x"));
            vAngularVelocity.y = atof( pElement->FirstChildElement("physics")->FirstChildElement("angularvelocity")->Attribute("y"));
            vAngularVelocity.z = atof( pElement->FirstChildElement("physics")->FirstChildElement("angularvelocity")->Attribute("z"));
        }

        if( docHandle.FirstChild("physics").FirstChild("localforce").Element() )
        {
            vLocalForce.x = atof( pElement->FirstChildElement("physics")->FirstChildElement("localforce")->Attribute("x"));
            vLocalForce.y = atof( pElement->FirstChildElement("physics")->FirstChildElement("localforce")->Attribute("y"));
            vLocalForce.z = atof( pElement->FirstChildElement("physics")->FirstChildElement("localforce")->Attribute("z"));
        }

        if( docHandle.FirstChild("physics").FirstChild("localtorque").Element() )
        {
            vLocalTorque.x = atof( pElement->FirstChildElement("physics")->FirstChildElement("localtorque")->Attribute("x"));
            vLocalTorque.y = atof( pElement->FirstChildElement("physics")->FirstChildElement("localtorque")->Attribute("y"));
            vLocalTorque.z = atof( pElement->FirstChildElement("physics")->FirstChildElement("localtorque")->Attribute("z"));
        }
    }

    if( pElement->Attribute("objectname" ) != NULL )
    {
        sprintf( sObjectName, "%.64s", pElement->Attribute("objectname") );
    }

    if( pElement->Attribute( "type" ) != NULL )
    {
        //  Debug( "updating type (!)\n" );

        sprintf( ObjectType, DeepTypeToObjectType( pElement->Attribute("type") ) );
        sprintf( sDeepObjectType, pElement->Attribute("type") );
    }
    if( pElement->Attribute( "owner" ) != NULL )
    {
        //  Debug( "updating owner\n" );
        iownerreference = atoi( pElement->Attribute("owner") );
    }

    if( docHandle.FirstChild("scripts").FirstChild("script").Element() )
    {
        sprintf( sScriptReference, pElement->FirstChildElement("scripts")->FirstChildElement("script")->Attribute("sscriptreference") );
    }

    if( docHandle.FirstChild("geometry").FirstChild("pos").Element() )
    {
        //  Debug( "updating pos\n" );
        pos = Vector3( pElement->FirstChildElement("geometry")->FirstChildElement("pos") );
    }
    if( docHandle.FirstChild("geometry").FirstChild("rot").Element() )
    {
        //  Debug( "updating rot\n" );
        rot = Rot( pElement->FirstChildElement("geometry")->FirstChildElement("rot") );
    }


    if( docHandle.FirstChild("scripts").FirstChild("script").Element() )
    {
        DEBUG(  "UpdateXML() loading scriptreference " << pElement->FirstChildElement("scripts")->FirstChildElement("script")->Attribute("sscriptreference" ) ); // DEBUG
        sprintf( sScriptReference, pElement->FirstChildElement("scripts")->FirstChildElement("script")->Attribute("sscriptreference" ) );
    }
}

void Object::LoadFromXML( TiXmlElement *pElement )
{
    TiXmlHandle docHandle( pElement );

    if( docHandle.FirstChild("scripts").FirstChild("script").Element() )
    {
        DEBUG(  "LoadFromXMl() loading scriptreference " << pElement->FirstChildElement("scripts")->FirstChildElement("script")->Attribute("sscriptreference" ) ); // DEBUG
        sprintf( sScriptReference, pElement->FirstChildElement("scripts")->FirstChildElement("script")->Attribute("sscriptreference" ) );
    }

    if( pElement->Attribute("objectname" ) != NULL )
    {
        sprintf( sObjectName, "%.64s", pElement->Attribute("objectname") );
    }
    else
    {
        sprintf( sObjectName, "" );
    }

    if( docHandle.FirstChild("physics").Element() )
    {
        if( pElement->FirstChildElement("physics")->Attribute("state") != NULL )
        {
            if( strcmp( pElement->FirstChildElement("physics")->Attribute("state"), "on" ) == 0 )
            {
                bPhysicsEnabled = true;
            }
            else
            {
                bPhysicsEnabled = false;
            }
        }
        else
        {
            bPhysicsEnabled = false;
        }
    }
    else
    {
        bPhysicsEnabled = false;
    }

    if( docHandle.FirstChild("phantom").Element() )
    {
        if( pElement->FirstChildElement("phantom")->Attribute("state") != NULL )
        {
            if( strcmp( pElement->FirstChildElement("phantom")->Attribute("state"), "on" ) == 0 )
            {
                bPhantomEnabled = true;
            }
            else
            {
                bPhantomEnabled = false;
            }
        }
        else
        {
            bPhantomEnabled = false;
        }
    }
    else
    {
        bPhantomEnabled = false;
    }

    if( docHandle.FirstChild("terrain").Element() )
    {
        if( pElement->FirstChildElement("terrain")->Attribute("state") != NULL )
        {
            if( strcmp( pElement->FirstChildElement("terrain")->Attribute("state"), "on" ) == 0 )
            {
                bTerrainEnabled = true;
            }
            else
            {
                bTerrainEnabled = false;
            }
        }
        else
        {
            bTerrainEnabled = false;
        }
    }
    else
    {
        bTerrainEnabled = false;
    }

    if( docHandle.FirstChild("physics").Element() )
    {
        //DEBUG(  "mvobject loadfromxml reading physicenabled from xml " ); // DEBUG
        //DEBUG(  "gravity attribute is: " << pElement->FirstChildElement("physics")->Attribute("gravity") ); // DEBUG
        if( pElement->FirstChildElement("physics")->Attribute("gravity") != NULL )
        {
            if( strcmp( pElement->FirstChildElement("physics")->Attribute("gravity"), "on" ) == 0 )
            {
                bGravityEnabled = true;
            }
            else
            {
                bGravityEnabled = false;
            }
        }
        else
        {
            bGravityEnabled = false;
        }
    }
    else
    {
        bGravityEnabled = false;
    }
    DEBUG(  "mvobject loadfromxml gravity enabled is: " << bGravityEnabled ); // DEBUG

    //  Debug( "Object::LoadFromXML\n" );
    if( pElement->Attribute("ireference") != NULL )
    {
        iReference = atoi( pElement->Attribute("ireference") );
    }
    if( pElement->Attribute("iparentreference") != NULL )
    {
        iParentReference = atoi( pElement->Attribute("iparentreference") );
    }

    sprintf( ObjectType, DeepTypeToObjectType( pElement->Attribute("type") ) );
    sprintf( sDeepObjectType, pElement->Attribute("type") );
    // Debug( "object type [%s] [%s]\n", ObjectType, sDeepObjectType );
    if( pElement->Attribute("owner") != NULL )
    {
        iownerreference = atoi( pElement->Attribute("owner") );
    }
    if( docHandle.FirstChild("geometry").FirstChild("pos").Element() )
    {
        pos = Vector3( pElement->FirstChildElement("geometry")->FirstChildElement("pos") );
    }
    else
    {
        pos.x = 0.0;
        pos.y = 0.0;
        pos.z = 0.0;
    }
    if( docHandle.FirstChild("geometry").FirstChild("rot").Element() )
    {
        rot = Rot( pElement->FirstChildElement("geometry")->FirstChildElement("rot") );
    }
    else
    {
        rot.x = 0.0;
        rot.y = 0.0;
        rot.z = 0.0;
        rot.s = 1.0;
    }
    //   Debug( "Object::LoadFromXML done\n" );
}
void Object::WriteToXMLDoc( TiXmlElement *pElement )
{
    //  Debug( "Object::WriteToXMLDoc\n" );
    pElement->SetAttribute( "ireference", iReference );
    pElement->SetAttribute( "iparentreference", iParentReference );
    pElement->SetAttribute( "owner", iownerreference );

    if( strcmp( sObjectName, "" ) != 0 )
    {
        pElement->SetAttribute( "objectname", sObjectName );
    }

    if( strcmp( sScriptReference, "" ) != 0 )
    {
        AddXmlChildIfNecessary( pElement, "scripts" );
        AddXmlChildIfNecessary( pElement->FirstChildElement("scripts"), "script" );
        pElement->FirstChildElement("scripts")->FirstChildElement("script")->SetAttribute("sscriptreference", sScriptReference );
    }

    if( bPhysicsEnabled )
    {
        AddXmlChildIfNecessary( pElement, "physics" );
        pElement->FirstChildElement("physics")->SetAttribute("state","on" );
    }

    if( bPhantomEnabled )
    {
        AddXmlChildIfNecessary( pElement, "phantom" );
        pElement->FirstChildElement("phantom")->SetAttribute("state","on" );
    }

    if( bTerrainEnabled )
    {
        AddXmlChildIfNecessary( pElement, "terrain" );
        pElement->FirstChildElement("terrain")->SetAttribute("state","on" );
    }

    if( bGravityEnabled )
    {
        AddXmlChildIfNecessary( pElement, "physics" );
        pElement->FirstChildElement("physics")->SetAttribute("gravity","on" );
        DEBUG(  "mvobject writetoxmldoc writing physicenabled to xml " ); // DEBUG
        DEBUG(  "mvobject writetoxmldoc currentxml: " << pElement ); // DEBUG
    }

    AddXmlChildIfNecessary( pElement, "geometry" );
    AddXmlChildIfNecessary( pElement->FirstChildElement("geometry"), "pos" );
    AddXmlChildIfNecessary( pElement->FirstChildElement("geometry"), "rot" );

    pos.WriteToXMLElement( pElement->FirstChildElement("geometry")->FirstChildElement("pos") );
    rot.WriteToXMLElement( pElement->FirstChildElement("geometry")->FirstChildElement("rot") );
    //  Debug( "Object::WriteToXMLDoc done\n" );
}
void Object::Draw()
{
    //     Debug( "Object::DrawEx\n" );
}
void Object::DrawSelected()
{
    //     Debug( "Object::DrawEx\n" );
}

ostream& operator<< ( ostream& os, Object &object )
{
    TiXmlDocument IPC;
    IPC.Parse( "<objectcreate>"
               "<meta><avatar /></meta>"
               "<geometry><pos /><rot /><scale /></geometry>"
               "<faces><face num=\"0\"><color/></face></faces>"
               "</objectcreate>" );
    object.WriteToXMLDoc( IPC.RootElement() );

    if( strcmp( object.ObjectType, "OBJECTGROUPING" ) == 0 )
    {
        // DEBUG(  "objectgoruping detected" ); // DEBUG
        for( int i= 0; i < dynamic_cast< ObjectGrouping &>(object).iNumSubObjects;i++ )
        {
            // DEBUG(  "subobject detected" ); // DEBUG
            os << *(dynamic_cast< ObjectGrouping &>(object).SubObjectReferences[i]);
        }
    }

    os << IPC << endl;

    return os;
}

void Object::GetCreateSQLFromXML( TiXmlElement *pElement, char *SQL )
{
    Debug( "Object::GetCreateSQLFromXML\n" );
    const char *sDeepObjectType = pElement->Attribute( "type" );

    if( strcmp( sDeepObjectType, "CUBE" ) == 0 )
    {
        Cube::GetCreateSQLFromXMLEx( pElement, SQL );
    }
    else if( strcmp( sDeepObjectType, "SPHERE" ) == 0 )
    {
        Sphere::GetCreateSQLFromXMLEx( pElement, SQL );
    }
    else if( strcmp( sDeepObjectType, "CYLINDER" ) == 0 )
    {
        Cylinder::GetCreateSQLFromXMLEx( pElement, SQL );
    }
    else if( strcmp( sDeepObjectType, "CONE" ) == 0 )
    {
        Cone::GetCreateSQLFromXMLEx( pElement, SQL );
    }
    else if( strcmp( sDeepObjectType, "TERRAIN" ) == 0 )
    {
        Terrain::GetCreateSQLFromXMLEx( pElement, SQL );
    }
    else if( strcmp( sDeepObjectType, "mvMd2Mesh" ) == 0 )
    {
        mvMd2Mesh::GetCreateSQLFromXMLEx( pElement, SQL );
    }
    else if( strcmp( sDeepObjectType, "AVATAR" ) == 0 )
    {
        Avatar::GetCreateSQLFromXMLEx( pElement, SQL );
    }
    else if( strcmp( sDeepObjectType, "OBJECTGROUPING" ) == 0 )
    {
        ObjectGrouping::GetCreateSQLFromXMLEx( pElement, SQL );
    }
}

void Object::GetDeleteSQLFromXML( TiXmlElement *pElement, char *SQL )
{
    Debug( "Object::GetDeleteSQLFromXML\n" );
    const char *sDeepObjectType = pElement->Attribute( "type" );

    if( strcmp( sDeepObjectType, "OBJECTGROUPING" ) == 0 )
    {
        ObjectGrouping::GetDeleteSQLFromXMLEx( pElement, SQL );
    }
}

void Object::GetUpdateSQLFromXML( TiXmlElement *pElement, char *SQL )
{
    Debug( "Object::GetUpdateSQLFromXML\n" );
    const char *sDeepObjectType = pElement->Attribute( "type" );

    if( strcmp( sDeepObjectType, "CUBE" ) == 0 )
    {
        Cube::GetUpdateSQLFromXMLEx( pElement, SQL );
    }
    else if( strcmp( sDeepObjectType, "SPHERE" ) == 0 )
    {
        Sphere::GetUpdateSQLFromXMLEx( pElement, SQL );
    }
    else if( strcmp( sDeepObjectType, "CYLINDER" ) == 0 )
    {
        Cylinder::GetUpdateSQLFromXMLEx( pElement, SQL );
    }
    else if( strcmp( sDeepObjectType, "CONE" ) == 0 )
    {
        Cone::GetUpdateSQLFromXMLEx( pElement, SQL );
    }
    else if( strcmp( sDeepObjectType, "TERRAIN" ) == 0 )
    {
        Terrain::GetUpdateSQLFromXMLEx( pElement, SQL );
    }
    else if( strcmp( sDeepObjectType, "MD2MESH" ) == 0 )
    {
        mvMd2Mesh::GetUpdateSQLFromXMLEx( pElement, SQL );
    }
    else if( strcmp( sDeepObjectType, "AVATAR" ) == 0 )
    {
        Avatar::GetUpdateSQLFromXMLEx( pElement, SQL );
    }
    else if( strcmp( sDeepObjectType, "OBJECTGROUPING" ) == 0 )
    {
        ObjectGrouping::GetUpdateSQLFromXMLEx( pElement, SQL );
    }
}

void Object::GetDeepObjectType_From_ObjectTypeAndPrimType()
{
    if( strcmp( ObjectType, "PRIM" ) == 0 )
    {
        sprintf( sDeepObjectType, dynamic_cast<Prim *>(this)->PrimType );
    }
    else if( strcmp( ObjectType, "OBJECTGROUPING" ) == 0 )
    {
        sprintf( sDeepObjectType, dynamic_cast<ObjectGrouping *>(this)->sObjectGroupingType );
    }
}

Object *Object::CreateNewObjectFromXML( TiXmlElement *pElement )
{
    Object *pNewObject = NULL;
    Debug( "Object::CreateNewObjectFromXML\n" );
    const char *sDeepObjectType = pElement->Attribute( "type" );

    if( strcmp( sDeepObjectType, "CUBE" ) == 0 )
    {
        pNewObject = new Cube;
    }
    else if( strcmp( sDeepObjectType, "SPHERE" ) == 0 )
    {
        pNewObject = new Sphere;
    }
    else if( strcmp( sDeepObjectType, "CYLINDER" ) == 0 )
    {
        pNewObject = new Cylinder;
    }
    else if( strcmp( sDeepObjectType, "CONE" ) == 0 )
    {
        pNewObject = new Cone;
    }
    else if( strcmp( sDeepObjectType, "TERRAIN" ) == 0 )
    {
        pNewObject = new Terrain;
    }
    else if( strcmp( sDeepObjectType, "MD2MESH" ) == 0 )
    {
        pNewObject = new mvMd2Mesh;
    }
    else if( strcmp( sDeepObjectType, "AVATAR" ) == 0 )
    {
        pNewObject = new Avatar;
    }
    else if( strcmp( sDeepObjectType, "OBJECTGROUPING" ) == 0 )
    {
        pNewObject = new ObjectGrouping;
    }

    if( pNewObject != NULL )
    {
        pNewObject->LoadFromXML( pElement );
    }
    return pNewObject;
}
