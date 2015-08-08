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
//! \brief see header file for documentation

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#include <mysql.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>
#include <sstream>
using namespace std;

#include "tinyxml.h"

#include "MySQLDBInterface.h"
#include "IDBInterface.h"
#include "Parse.h"
#include "Diag.h"
#include "Config.h"
#include "System.h"

#include "SocketsClass.h"
#include "TextureInfoCache.h"
#include "ScriptInfoCache.h"
#include "TerrainInfoCache.h"
#include "Terrain.h"
#include "mvMd2Mesh.h"
#include "MeshInfoCache.h"

#include "Object.h"
#include "Avatar.h"
#include "Prim.h"
#include "Cube.h"
#include "Cylinder.h"
#include "Cone.h"
#include "Sphere.h"
#include "ObjectGrouping.h"

mvsocket SocketMetaverseServer;
int iMetaverseServerPort = 22170;

#define BUFSIZE 2047
char SendBuffer[BUFSIZE + 1];  //!< buffer for socket sends
char ReadBuffer[4097];  //!< buffer for socket reads

char SQLCommands[10][512];  //!< buffer to parse a list of SQLCommands into

bool bRunningWithDB = true;  //!< are we actually using a db (true)? or just faking it? (false)

ConfigClass mvConfig; //!< reads config data from config.xml file and makes it available to program

int iWithoutDBNextObjectReference = 1; //!< when we're not actually using a db, this stores the reference for next generated object

map <int, int, less<int> > TempRefCache;  //<! reference cache, if we're not using a database
typedef map <int, int, less<int> >::iterator TempRefCacheIteratorTypedef;
typedef pair <int, int> temprefpair;

IDBInterface *pdbinterface;  //!< abstracts RDBMS-specified functions

//! returns next free object iReference;  this is a sim-unique identifier number that is stable for the life of the database
int GetNextFreeObjectReference()
{
    int iNextReference = 0;

    if( bRunningWithDB )
    {
        pdbinterface->RunOneRowQuery( "SELECT i_next_reference FROM nextobjectreference;" );
        if( pdbinterface->RowAvailable() )
        {
            iNextReference = atoi( pdbinterface->GetFieldValueByName( "i_next_reference" ) );
            DEBUG(  "Next free object reference: " << iNextReference ); // DEBUG
        }
        else
        {
            DEBUG(  "Error getting next free object reference!" ); // DEBUG
            mvSystem::mvExit(1);
        }

        pdbinterface->ExecuteSQL( "update nextobjectreference set i_next_reference = %i;", iNextReference + 1 );

        return iNextReference;
    }
    else
    {
        iWithoutDBNextObjectReference++;
        return( iWithoutDBNextObjectReference - 1 );
    }
}

//! Updates the object iReference to the database according to the values in pElement
void UpdateObject( int iReference, TiXmlElement *pElement )
{
    if( bRunningWithDB )
    {
        DEBUG(  "updateobject" ); // DEBUG
        char UpdateSQL[2048] = "";
        Object::GetUpdateSQLFromXML( pElement, UpdateSQL );

        int iNumCommands = Parse( SQLCommands, UpdateSQL, ";" );

        for( int i = 0; i < iNumCommands; i++ )
        {
            if( strlen( SQLCommands[i] ) > 0 )
            {
                pdbinterface->ExecuteSQL( "%s;", SQLCommands[i] );
            }
        }
    }
    DEBUG(  "updateobject finished" ); // DEBUG
}

//! Updates the skybox info specified by pElement to the database
void UpdateSkybox( TiXmlElement *pElement )
{
    if( bRunningWithDB )
    {
        string ref = pElement->Attribute("stexturereference");
        pdbinterface->ExecuteSQL( "delete from skybox;");
        pdbinterface->ExecuteSQL( "insert into skybox values ('%s');", ref.c_str() );
    }
    DEBUG(  "updateskybox finished" ); // DEBUG
}

//! Deletes the object specified by pElement from the database; then confirms to the metaverseserver component
void DeleteObjectXML( TiXmlElement *pElement )
{
    int iReference = atoi( pElement->Attribute("ireference") );
    if( iReference != 0 )
    {
        if( strcmp( pElement->Attribute("type"), "AVATAR" ) == 0 )
        {
            DEBUG(  "WARNING: attempt to delete avatar! :-O" ); // DEBUG
            return;
        }
        else if( strcmp( pElement->Attribute("type"), "OBJECTGROUPING" ) == 0 )
        {
            DEBUG(  "deleting objectgrouping..." ); // DEBUG
            if( bRunningWithDB )
            {
                char DeleteSQL[2048] = "";
                Object::GetDeleteSQLFromXML( pElement, DeleteSQL );

                int iNumCommands = Parse( SQLCommands, DeleteSQL, ";" );
                DEBUG(  "deleteSQL: " << DeleteSQL ); // DEBUG

                for( int i = 0; i < iNumCommands; i++ )
                {
                    if( strlen( SQLCommands[i] ) > 0 )
                    {
                        pdbinterface->ExecuteSQL( "%s;", SQLCommands[i] );
                    }
                }
            }

        }
        else
        {
            DEBUG(  "DeleteObject() " << iReference ); // DEBUG

            if( bRunningWithDB )
            {
                pdbinterface->ExecuteSQL( "delete from objects where i_reference = %i;", iReference );
                pdbinterface->ExecuteSQL( "delete from prims where i_reference = %i;", iReference );
                pdbinterface->ExecuteSQL( "delete from objectgroupings where i_reference = %i;", iReference );
                pdbinterface->ExecuteSQL( "delete from avatars where i_reference = %i;", iReference );
                pdbinterface->ExecuteSQL( "delete from meshes where i_reference = %i;", iReference );
                pdbinterface->ExecuteSQL( "delete from md2meshes where i_reference = %i;", iReference );
                pdbinterface->ExecuteSQL( "delete from cubes where i_reference = %i;", iReference );
                pdbinterface->ExecuteSQL( "delete from cylinders where i_reference = %i;", iReference );
                pdbinterface->ExecuteSQL( "delete from spheres where i_reference = %i;", iReference );
                pdbinterface->ExecuteSQL( "delete from cones where i_reference = %i;", iReference );
            }

        }
        std::string IPCString;
        IPCString << *pElement;
        sprintf( SendBuffer, "%s\n", IPCString.c_str() );

        DEBUG(  "Sending " << SendBuffer ); // DEBUG
        SocketMetaverseServer.Send( SendBuffer );
    }
    else
    {
        DEBUG(  "WARNING: trying to delete object 0 (default) !" ); // DEBUG
    }
}

//! Creates a new object specified by pElement, then confirms to the metaverseserver component
void CreateObjectFromXML( TiXmlElement *pElement )
{
    int iReference;
    int iParentReference;
    TempRefCacheIteratorTypedef iterator;

    if( strcmp( pElement->Value(), "objectimport" ) == 0 )
    {
        iterator = TempRefCache.find( atoi( pElement->Attribute( "i_temp_reference" ) ) );
        if( iterator != TempRefCache.end() )
        {
            DEBUG(  "tempref found in cache " << atoi( pElement->Attribute( "i_temp_reference" ) ) << " " << iterator->second ); // DEBUG
            iReference = iterator->second;
        }
        else
        {
            iReference = GetNextFreeObjectReference();
            DEBUG(  "generating new object ref " << iReference << " for " << atoi( pElement->Attribute( "i_temp_reference" ) ) ); // DEBUG
            TempRefCache.insert( temprefpair( atoi( pElement->Attribute( "i_temp_reference" ) ), iReference ) );
        }

        if( atoi( pElement->Attribute( "i_temp_parentreference" ) ) != 0 )
        {
            iterator = TempRefCache.find( atoi( pElement->Attribute( "i_temp_parentreference" ) ) );
            if( iterator != TempRefCache.end() )
            {
                DEBUG(  "parenttempref found in cache " << iterator->second ); // DEBUG
                iParentReference = iterator->second;
            }
            else
            {
                iParentReference = GetNextFreeObjectReference();
                DEBUG(  "generating new object ref " << atoi( pElement->Attribute( "i_temp_parentreference" ) ) << " " << iParentReference ); // DEBUG
                TempRefCache.insert( temprefpair( atoi( pElement->Attribute( "i_temp_parentreference" ) ), iParentReference ) );
            }
        }
        else
        {
            iParentReference = 0;
        }
    }
    else
    {
        iReference = GetNextFreeObjectReference();
        iParentReference = 0;  // since we never create objects within alinked set, but create them first then link them, for now
    }

    pElement->SetAttribute( "ireference", iReference );
    pElement->SetAttribute( "iparentreference", iParentReference );
    if( pElement->Attribute("i_temp_reference" ) != NULL )
    {
        pElement->RemoveAttribute("i_temp_reference" );
    }
    if( pElement->Attribute("i_temp_parentreference" ) != NULL )
    {
        pElement->RemoveAttribute("i_temp_parentreference" );
    }

    if( bRunningWithDB )
    {

        char CreateSQL[2048] = "";
        Object::GetCreateSQLFromXML( pElement, CreateSQL );

        int iNumCommands = Parse( SQLCommands, CreateSQL, ";" );

        int i;
        for( i = 0; i < iNumCommands; i++ )
        {
            if( strlen( SQLCommands[i] ) > 0 )
            {
                pdbinterface->ExecuteSQL( "%s;", SQLCommands[i] );
            }
        }

        sprintf( CreateSQL, "" );
        Object::GetUpdateSQLFromXML( pElement, CreateSQL );

        iNumCommands = Parse( SQLCommands, CreateSQL, ";" );

        for( i = 0; i < iNumCommands; i++ )
        {
            if( strlen( SQLCommands[i] ) > 0 )
            {
                pdbinterface->ExecuteSQL( "%s;", SQLCommands[i] );
            }
        }


    }

    pElement->SetValue( "objectcreate" );

    std::string IPCString;
    IPCString << *pElement;
    sprintf( SendBuffer, "%s\n", IPCString.c_str() );

    DEBUG(  "Sending " << SendBuffer ); // DEBUG
    SocketMetaverseServer.Send( SendBuffer );
}

//! retrieves all objects of type p_Object (eg Cube, Cone etc) from the database, and sends them to the metaverseserver component
void RetrieveWorldStateForOneObjectType( Object *p_Object )
{
    if( bRunningWithDB )
    {
        pdbinterface->RunMultiRowQuery( p_Object->GetWorldStateRetrieveSQL() );
        while( pdbinterface->RowAvailable() )
        {
            DEBUG( "got row" );

            DEBUG( "populating object from db row..." );
            p_Object->PopulateFromDBRow();
            DEBUG( "object populated from db row..." );

            TiXmlDocument IPC;
            IPC.Parse( "<objectrefreshdata>"
                       "<meta><avatar /></meta>"
                       "<geometry><pos /><rot /><scale /></geometry>"
                       "<faces><face num=\"0\"><color/></faces>"
                       "</objectrefreshdata>" );

            p_Object->WriteToXMLDoc( IPC.RootElement() );

            std::string IPCString;
            IPCString << IPC;
            sprintf( SendBuffer, "%s\n", IPCString.c_str() );

            DEBUG( "Sending " << SendBuffer );
            SocketMetaverseServer.Send( SendBuffer );
            pdbinterface->NextRow();
        }
        pdbinterface->EndMultiRowQuery();
    }
}

//! Loads skybox info from the database, and sends it to the metaverseserver component
void LoadSkyboxInfo()
{
    if( bRunningWithDB )
    {
        pdbinterface->RunOneRowQuery( "select s_texture_reference from skybox;");
        if( pdbinterface->RowAvailable() )
        {
            string sChecksum = pdbinterface->GetFieldValueByName( "s_texture_reference" );

            TiXmlDocument IPC;
            IPC.Parse( "<skyboxupdate/>" );
            IPC.RootElement()->SetAttribute("stexturereference", sChecksum);

            std::string IPCString;
            IPCString << IPC;
            sprintf( SendBuffer, "%s\n", IPCString.c_str() );

            printf( "Sending [%s]", SendBuffer );
            SocketMetaverseServer.Send( SendBuffer );
        }
    }
}

//! Loads texture info from the database, and sends it to the metaverseserver component
void LoadTextureInfo()
{
    if( bRunningWithDB )
    {
        pdbinterface->RunMultiRowQuery( TextureInfoCache::GetTextureRetrievalSQL().c_str() );
        while( pdbinterface->RowAvailable() )
        {
            TEXTUREINFO TextureInfo;
            TiXmlDocument IPC;
            IPC.Parse( "<texture/>" );
            TextureInfoCache::LoadTextureInfoFromDBRow( TextureInfo );
            TextureInfoCache::WriteTextureInfoToXML( IPC.RootElement(), TextureInfo );

            std::string IPCString;
            IPCString << IPC;
            sprintf( SendBuffer, "%s\n", IPCString.c_str() );

            printf( "Sending [%s]", SendBuffer );
            SocketMetaverseServer.Send( SendBuffer );

            pdbinterface->NextRow();
        }
        pdbinterface->EndMultiRowQuery();
    }
}

//! Loads terrain info from the database, and sends it to the metaverseserver component
void LoadTerrainInfo()
{
    if( bRunningWithDB )
    {
        pdbinterface->RunMultiRowQuery( TerrainCacheClass::GetTerrainRetrievalSQL().c_str() );
        while( pdbinterface->RowAvailable() )
        {
            TerrainINFO TerrainInfo;
            TiXmlDocument IPC;
            IPC.Parse( "<terrain/>" );
            TerrainCacheClass::LoadTerrainInfoFromDBRow( TerrainInfo );
            TerrainCacheClass::WriteTerrainInfoToXML( IPC.RootElement(), TerrainInfo );

            std::string IPCString;
            IPCString << IPC;
            sprintf( SendBuffer, "%s\n", IPCString.c_str() );

            printf( "Sending [%s]", SendBuffer );
            SocketMetaverseServer.Send( SendBuffer );

            pdbinterface->NextRow();
        }
        pdbinterface->EndMultiRowQuery();
    }
}

//! Loads mesh info from the database, and sends it to the metaverseserver component
void LoadMeshInfo()
{
    if( bRunningWithDB )
    {
        pdbinterface->RunMultiRowQuery( MeshInfoCacheClass::GetFileInfoRetrievalSQL().c_str() );
        while( pdbinterface->RowAvailable() )
        {
            FILEINFO FileInfo;
            TiXmlDocument IPC;
            IPC.Parse( "<meshfile/>" );
            MeshInfoCacheClass::LoadFileInfoFromDBRow( FileInfo );
            MeshInfoCacheClass::WriteFileInfoToXML( IPC.RootElement(), FileInfo );

            std::string IPCString;
            IPCString << IPC;
            sprintf( SendBuffer, "%s\n", IPCString.c_str() );

            printf( "Sending [%s]", SendBuffer );
            SocketMetaverseServer.Send( SendBuffer );

            pdbinterface->NextRow();
        }
        pdbinterface->EndMultiRowQuery();
    }
}

//! Loads script info from the database, and sends it to the metaverseserver component
void LoadScriptInfo()
{
    if( bRunningWithDB )
    {
        pdbinterface->RunMultiRowQuery( ScriptInfoCacheClass::GetRetrievalSQL().c_str() );
        while( pdbinterface->RowAvailable() )
        {
            SCRIPTINFO ScriptInfo;
            TiXmlDocument IPC;
            IPC.Parse( "<script/>" );
            ScriptInfoCacheClass::LoadInfoFromDBRow( ScriptInfo );
            ScriptInfoCacheClass::WriteInfoToXML( IPC.RootElement(), ScriptInfo );

            std::string IPCString;
            IPCString << IPC;
            sprintf( SendBuffer, "%s\n", IPCString.c_str() );

            printf( "Sending [%s]", SendBuffer );
            SocketMetaverseServer.Send( SendBuffer );

            pdbinterface->NextRow();
        }
        pdbinterface->EndMultiRowQuery();
    }
}

//! Sets a value in the script database storage area, according to pElement
void SetInfoFromXML( TiXmlElement *pElement )
{
    int iOwner = atoi( pElement->Attribute("iowner" ) );
    string sStore = pElement->Attribute("store" );
    string sKey = pElement->Attribute("valuename" );
    string sData = pElement->Attribute("value" );

    pdbinterface->ExecuteSQL( "delete from userdata where i_owner=%i and s_store='%s' and s_key='%s';",
                                   iOwner, sStore.c_str(), sKey.c_str() );
    pdbinterface->ExecuteSQL( "INSERT INTO userdata "
                                   "( i_owner, s_store, s_key, s_data )"
                                   " values "
                                   " ( %i, '%s', '%s', '%s' );", iOwner, sStore.c_str(), sKey.c_str(), sData.c_str() );
}

//! Retrieves a value in the script database storage area, according to pElement, and sends it to the metaverseserver component
void RequestInfoXML( TiXmlElement *pElement )
{
    int iOwner = atoi( pElement->Attribute("idataowner" ) );
    string sStore = pElement->Attribute("store" );
    string sKey = pElement->Attribute("valuename" );

    pdbinterface->RunOneRowQuery( "select s_data from userdata where i_owner=%i and s_store='%s' and s_key='%s';",
                                       iOwner, sStore.c_str(), sKey.c_str() );
    if( pdbinterface->RowAvailable() )
    {
        string sResult = pdbinterface->GetFieldValueByName( "s_data" );

        pElement->SetAttribute( "value", sResult );
        pElement->SetValue( "inforesponse" );
        ostringstream ResponseStream;
        ResponseStream << *pElement << endl;
        DEBUG(  "sending to server: " << ResponseStream.str() ); // DEBUG
        SocketMetaverseServer.Send( ResponseStream.str().c_str() );
    }
}

//! Loads everythign in the world from db, and forwards it to the metaverseserver component
void RetrieveWorldState()
{
    Cube Cube;
    Sphere Sphere;
    Cone Cone;
    Cylinder Cylinder;
    mvMd2Mesh md2mesh;
    Avatar Avatar;
    Terrain Terrain;
    ObjectGrouping ObjectGrouping;

    if( bRunningWithDB )
    {
        RetrieveWorldStateForOneObjectType( &Cube );
        RetrieveWorldStateForOneObjectType( &Avatar );
        RetrieveWorldStateForOneObjectType( &Sphere );
        RetrieveWorldStateForOneObjectType( &Cylinder );
        RetrieveWorldStateForOneObjectType( &md2mesh );
        RetrieveWorldStateForOneObjectType( &Cone );
        RetrieveWorldStateForOneObjectType( &Terrain );
        RetrieveWorldStateForOneObjectType( &ObjectGrouping );
        LoadTextureInfo();
        LoadSkyboxInfo();
        LoadScriptInfo();
        LoadTerrainInfo();
        LoadMeshInfo();
    }
    else
    {
        int iPlatformreference = GetNextFreeObjectReference();
        std::ostringstream ss;
        ss <<
        "<objectrefreshdata " <<
        "ireference=\"" << iPlatformreference << "\" " <<
        "owner=\"0\" " <<
        "iparentreference=\"0\" " <<
        "objectname=\"Platform\" " <<
        "type=\"Cube\" " <<
        ">" <<
        "<geometry><pos x=\"0\" y=\"0\" z=\"-1.3\"/>" <<
        "<rot x=\"0\" y=\"0\" z=\"0\" s=\"1\" />" <<
        "<scale x=\"20\" y=\"20\" z=\"0.5\"/>" <<
        "</geometry>" <<
        "<faces>" <<
        "<face num=\"0\"><color r=\"0.2\" g=\"0.5\" b=\"0.2\"/></face>" <<
        "</faces>" <<
        "<terrain state=\"on\" />" <<
        "</objectrefreshdata>" << endl;
        sprintf( SendBuffer, "%s\n", ss.str().c_str() );

        DEBUG( "Sending " << SendBuffer );
        SocketMetaverseServer.Send( SendBuffer );
    }
}

//! Registers a file (terrain, script, mesh etc), specified by pElement, into the database
void RegisterFileXML( TiXmlElement *pElement )
{
    if( bRunningWithDB )
    {
        DEBUG(" *** RegisterFileXML");
        if( strcmp( pElement->Attribute("type"), "TEXTURE" ) == 0 )
        {
            ostringstream sqlstream;
            // gotta fill in owner field in serverfileagent/metaverseserver.
            sqlstream << "insert into textures ( s_texture_reference, i_owner, s_source_filename,s_server_filename ) "
            << "values ( '" <<pElement->Attribute("checksum") << "', 0, "
            << "'" << pElement->Attribute("sourcefilename") << "', '" << pElement->Attribute("serverfilename")
            << "');";
            pdbinterface->ExecuteSQL( sqlstream.str().c_str() );
        }
        else if( strcmp( pElement->Attribute("type"), "SCRIPT" ) == 0 )
        {
            ostringstream sqlstream;
            // gotta fill in owner field in serverfileagent/metaverseserver.
            sqlstream << "insert into scripts ( s_script_reference, i_owner, s_source_filename,s_server_filename ) "
            << "values ( '" <<pElement->Attribute("checksum") << "', 0, "
            << "'" << pElement->Attribute("sourcefilename") << "', '" << pElement->Attribute("serverfilename")
            << "');";
            pdbinterface->ExecuteSQL( sqlstream.str().c_str() );
        }
        else if( strcmp( pElement->Attribute("type"), "TERRAIN" ) == 0 )
        {
            ostringstream sqlstream;
            // gotta fill in owner field in serverfileagent/metaverseserver.
            sqlstream << "insert into terrainfiles ( s_terrain_reference, i_owner, s_source_filename,s_server_filename ) "
            << "values ( '" <<pElement->Attribute("checksum") << "', 0, "
            << "'" << pElement->Attribute("sourcefilename") << "', '" << pElement->Attribute("serverfilename")
            << "');";
            pdbinterface->ExecuteSQL( sqlstream.str().c_str() );
        }
        else if( strcmp( pElement->Attribute("type"), "MESHFILE" ) == 0 )
        {
            ostringstream sqlstream;
            // gotta fill in owner field in serverfileagent/metaverseserver.
            sqlstream << "insert into meshfiles ( s_file_reference, i_owner, s_source_filename,s_server_filename ) "
            << "values ( '" <<pElement->Attribute("checksum") << "', 0, "
            << "'" << pElement->Attribute("sourcefilename") << "', '" << pElement->Attribute("serverfilename")
            << "');";
            pdbinterface->ExecuteSQL( sqlstream.str().c_str() );
        }
        else
        {
            DEBUG(  "Warning: unknown type: " << pElement->Attribute("type") ); // DEBUG
        }
    }
}

//! Maybe obsolete?
void Authenticate( int iConnectionRef, const char *AvatarName, const char *AvatarPassword )
{
    int iAvatarReference;
    int iPrimReference;

    Debug( "Authenticating %s on connectionref %i\n", AvatarName, iConnectionRef );

    if( bRunningWithDB )
    {
        pdbinterface->RunOneRowQuery( "select password from accounts where account_name = '%s';", AvatarName  );
        if( pdbinterface->RowAvailable() )
        {
            if( strcmp( AvatarPassword, pdbinterface->GetFieldValueByName( "password" ) ) == 0 )
            {
                pdbinterface->RunOneRowQuery( "select i_reference from avatars where s_avatar_name = '%s';", AvatarName );

                int iReference = atoi( pdbinterface->GetFieldValueByName( "i_reference" ) );

                Debug( "avatar %s authenticated\n", AvatarName );
                sprintf( SendBuffer, "<loginaccept name=\"%s\" iconnectionref=\"%i\" ireference=\"%i\"/>\n", AvatarName, iConnectionRef, iReference );
                DEBUG( "Sending to server " << SendBuffer );
                SocketMetaverseServer.Send( SendBuffer );
            }
            else
            {
                Debug( "avatar %s failed authentication!\n", AvatarName );
                sprintf( SendBuffer, "<loginreject name=\"%s\" iconnectionref=\"%i\"/>\n", AvatarName, iConnectionRef );
                DEBUG( "Sending to server " << SendBuffer );
                SocketMetaverseServer.Send( SendBuffer );
            }
        }
        else
        {
            Debug( "new user [%s].  Registering...\n", AvatarName );
            iAvatarReference = GetNextFreeObjectReference();
            iPrimReference = GetNextFreeObjectReference();
            pdbinterface->ExecuteSQL( "insert into accounts ( account_name, password ) values ( '%s', '%s' );", AvatarName, AvatarPassword );
            pdbinterface->ExecuteSQL( "insert into avatars ( i_reference, s_avatar_name, account_name ) values ( %i, '%s', '%s' );",
                                           iAvatarReference, AvatarName, AvatarName );
            pdbinterface->ExecuteSQL( "insert into objects ( i_reference, s_object_type, i_owner, i_parentreference, pos_x, pos_y, pos_z, rot_x, rot_y, rot_z, rot_s ) values ( %i, '%s', %i, %i,%f,%f,%f,%f,%f,%f, %f);",
                                           iAvatarReference, "OBJECTGROUPING", iAvatarReference, 0, 0.0,0.0,0.0, 0.0,0.0,0.0,1.0 );
            pdbinterface->ExecuteSQL( "insert into objectgroupings ( i_reference, i_subobjectsequencenumber,i_subobjectreference, s_objectgrouping_type ) values ( %i, %i, %i, '%s' );",
                                           iAvatarReference, 0, iPrimReference, "AVATAR" );

            pdbinterface->ExecuteSQL( "INSERT INTO objects ( i_reference, s_object_type, i_owner, i_parentreference, pos_x, pos_y, pos_z, rot_x, rot_y, rot_z, rot_s ) values ( %i, '%s', %i, %i,%f,%f,%f,%f,%f,%f,%f);",
                                           iPrimReference, "PRIM", iAvatarReference, iAvatarReference, 0.0,0.0,0.0, 0.0,0.0,0.0,1.0 );
            pdbinterface->ExecuteSQL( "INSERT INTO prims ( i_reference, s_prim_type, scale_x, scale_y, scale_z ) values ( %i, '%s',%f,%f,%f );",
                                           iPrimReference, "CUBE", 0.5,0.5,2.0 );
            pdbinterface->ExecuteSQL( "INSERT INTO cubes ( i_reference, color_0_r, color_0_g, color_0_b ) values ( %i, %f, %f, %f );", iPrimReference, 1.0,1.0,1.0 );
            sprintf( SendBuffer, "<objectcreate type=\"Cube\" ireference=\"%i\" owner=\"%i\" iparentreference=\"%i\"><geometry><pos x=\"%f\" y=\"%f\" z=\"%f=\"/>"
                     "<scale x=\"0.5\" y=\"0.5\" z=\"2.0\"/></geometry></objectcreate>\n",
                     iPrimReference, iAvatarReference, iAvatarReference, 0.0,0.0, 0.0 );
            SocketMetaverseServer.Send( SendBuffer );
            sprintf( SendBuffer, "<objectcreate type=\"Avatar\" ireference=\"%i\" owner=\"%i\" iparentreference=\"%i\"><meta><avatar name=\"%s\"/></meta><geometry><pos x=\"%f\" y=\"%f\" z=\"%f=\"/></geometry></objectcreate>\n",
                     iAvatarReference, iAvatarReference, 0, AvatarName, 0.0,0.0, 0.0 );
            SocketMetaverseServer.Send( SendBuffer );

            sprintf( SendBuffer, "<loginaccept name=\"%s\" iconnectionref=\"%i\" ireference=\"%i\"/>\n", AvatarName, iConnectionRef, iAvatarReference );
            DEBUG( "Sending to server " << SendBuffer );
            SocketMetaverseServer.Send( SendBuffer );
        }
    }
    else
    {
        Debug( "new user [%s].  Registering...\n", AvatarName );
        iAvatarReference = GetNextFreeObjectReference();
        iPrimReference = GetNextFreeObjectReference();

        sprintf( SendBuffer, "<objectcreate type=\"Cube\" ireference=\"%i\" owner=\"%i\" iparentreference=\"%i\"><geometry><pos x=\"%f\" y=\"%f\" z=\"%f=\"/><scale x=\"0.5\" y=\"0.5\" z=\"2.0\"/></geometry></objectcreate>\n",
                 iPrimReference, iAvatarReference, iAvatarReference, 0.0,0.0, 0.0 );
        SocketMetaverseServer.Send( SendBuffer );
        sprintf( SendBuffer, "<objectcreate type=\"Avatar\" ireference=\"%i\" owner=\"%i\" iparentreference=\"%i\"><meta><avatar name=\"%s\"/></meta><geometry><pos x=\"%f\" y=\"%f\" z=\"%f=\"/></geometry></objectcreate>\n",
                 iAvatarReference, iAvatarReference, 0, AvatarName, 0.0,0.0, 0.0 );
        SocketMetaverseServer.Send( SendBuffer );

        sprintf( SendBuffer, "<loginaccept name=\"%s\" iconnectionref=\"%i\" ireference=\"%i\"/>\n", AvatarName, iConnectionRef, iAvatarReference );
        DEBUG( "Sending to server " << SendBuffer );
        SocketMetaverseServer.Send( SendBuffer );
    }

}

//! Mainloop for dbinterface; checks for new messages from metaverseserver, and processes them
void MainLoop()
{
    int ReadResult = 0;

    while( 1 )
    {
        DEBUG( "Waiting for message from mv server..." );
        ReadResult = SocketMetaverseServer.ReceiveLineBlocking( ReadBuffer );
        if( ReadResult == SOCKETS_READ_OK )
        {
            if( ReadBuffer[0] =='<' )
            {
                DEBUG( "received xml from server " << ReadBuffer );
                TiXmlDocument IPC;
                IPC.Parse( ReadBuffer );
                if( strcmp( IPC.RootElement()->Value(), "login" ) == 0 )
                {
                    Debug( "received authentication request from %s\n", IPC.RootElement()->Attribute("name") );
                    Authenticate( atoi( IPC.RootElement()->Attribute("iconnectionref") ),
                                  IPC.RootElement()->Attribute("name"),
                                  IPC.RootElement()->Attribute("password")
                                );
                }
                else if( strcmp( IPC.RootElement()->Value(), "objectcreate" ) == 0 )
                {
                    DEBUG( "received object create request" );
                    CreateObjectFromXML( IPC.RootElement() );
                }
                else if( strcmp( IPC.RootElement()->Value(), "objectimport" ) == 0 )
                {
                    DEBUG( "received object create request" );
                    CreateObjectFromXML( IPC.RootElement() );
                }
                else if( strcmp( IPC.RootElement()->Value(), "objectupdate" ) == 0 )
                {
                    DEBUG(  "received object update request" << ReadBuffer ); // DEBUG
                    UpdateObject( atoi( IPC.RootElement()->Attribute("ireference" ) ), IPC.RootElement() );
                }
                else if( strcmp( IPC.RootElement()->Value(), "skyboxupdate" ) == 0 )
                {
                    DEBUG(  "received skybox update request" << ReadBuffer ); // DEBUG
                    UpdateSkybox(  IPC.RootElement() );
                }
                else if( strcmp( IPC.RootElement()->Value(), "objectdelete" ) == 0 )
                {
                    DEBUG( "received object delete request" );
                    DeleteObjectXML( IPC.RootElement() );
                }
                else if( strcmp( IPC.RootElement()->Value(), "setinfo" ) == 0 )
                {
                    DEBUG( "received setinfo request" );
                    SetInfoFromXML( IPC.RootElement() );
                }
                else if( strcmp( IPC.RootElement()->Value(), "requestinfo" ) == 0 )
                {
                    DEBUG( "received requestinfo request" );
                    RequestInfoXML( IPC.RootElement() );
                }
                else if( strcmp( IPC.RootElement()->Value(), "registerfile" ) == 0 )
                {
                    DEBUG( "received registerfile request" );
                    RegisterFileXML( IPC.RootElement() );
                }
                else if( strcmp( IPC.RootElement()->Value(), "requestworldstate" ) == 0 )
                {
                    DEBUG( "Retrieve world command received" );
                    RetrieveWorldState();
                }
            }
            else
            {
                DEBUG( "received legacy IPC from server " << ReadBuffer );
            }
        }
        else if( ReadResult == SOCKETS_READ_SOCKETGONE )
        {
            printf( "Server socket gone.  Dieing...\n" );

            if( bRunningWithDB )
            {
                pdbinterface->DisconnectDB();
            }
            mvSystem::mvExit(1);
        }
    }
}

//! Point of entry for DatabaseManager process; handles commandline arguments, connects to database, then runs main loop
int main( int argc, char *argv[] )
{
    INFO( "" );
    INFO( "DatabaseManager" );
    INFO( "===============" );
    INFO( "" );
    INFO( "    Options available:" );
    INFO( "       --nodb    runs without db; no post-session persistance" );
    INFO( "" );
    INFO( "" );

    bRunningWithDB = true;

    pdbinterface = new MySQLDBInterface();
    mvConfig.ReadConfig();

#ifndef _NOLOGGINGLIB

    CMessageGroup::disableAllMsgGroups();
    if( mvConfig.DebugLevel == "3" )
    {
        CMessageGroup::enableMsgGroups( "DEBUG" );
    }
#endif

    TempRefCache.clear();
    Object::SetDBAbstractionLayer( *pdbinterface );
    FileInfoCacheClass::SetDBAbstractionLayer( *pdbinterface );

    float fLoadFloat = 1.0 * 5.0;

    mvsocket::InitSocketSystem();
    printf( "Connecting to Metaverse Server on port %i...\n", iMetaverseServerPort );
    SocketMetaverseServer.ConnectToServer( inet_addr( "127.0.0.1" ), iMetaverseServerPort );
    printf( "Connected to Metaverse Server.\n" );

    if( argc>1 )
    {
        if( strcmp( argv[1], "--nodb" ) == 0 )
        {
            printf( "Option --nodb selected: we will not be using the database for this session\n" );
            printf( "WARNING: objects will not be persistent beyond this session\n" );
            bRunningWithDB = false;
        }
    }

    if( bRunningWithDB )
    {
        cout << "Connecting to local MySQL database \"" << mvConfig.SimDatabaseInfo.DatabaseName << "\"..." << endl;
        cout << "username: [" << mvConfig.SimDatabaseInfo.UserName << "] Password: [" << mvConfig.SimDatabaseInfo.Password << "]" << endl;
        pdbinterface->DBConnect( mvConfig.SimDatabaseInfo.Host.c_str(),
                                      mvConfig.SimDatabaseInfo.DatabaseName.c_str(),
                                      mvConfig.SimDatabaseInfo.UserName.c_str(),
                                      mvConfig.SimDatabaseInfo.Password.c_str() );
    }

    printf( "Initialization completed\n" );

    MainLoop();



    if( bRunningWithDB )
    {
        pdbinterface->DisconnectDB();
    }

    return 0;
}
