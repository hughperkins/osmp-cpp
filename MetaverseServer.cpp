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
//! \brief This module is the metaverseserver module which is responsible for managing the server-side parts of OSMP.
//!
//! This module is the metaverseserver module which is responsible for managing the server-side parts of OSMP.
//! It connects to DBInterface in order to read and write to a database
//! ScriptingEngines can connect to it in order to read and write to the world
//! A server console/gui can also connect to this to obtain informatoin on current connections and so on
//!
//! The server has a copy of the world
//! It interfaces with a collision and physics dll
//! Scripting engines link with the metaverseserver via XML sockets IPC.
//! The server handles communicatiosn with the clients, and with the database.
//! we might possibly put a WAn compression module in between the server and the Internet/WAN connection

// 20050330 Mark Wagner - Modified to use mvsocket as a class, not a glorified struct

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#ifdef _WIN32
//#include <process.h>
#include <io.h>
#endif

#include <stdio.h>
#include <string>
#include <algorithm>
#include <list>
#include <set>
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

#ifndef _WIN32  // Linux stuff
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#endif

#include "tinyxml.h"

#include "port_list.h"

#include "TickCount.h"

#include "Object.h"
#include "ObjectGrouping.h"
#include "Avatar.h"

#include "WorldStorage.h"
#include "SocketsClass.h"
#include "SocketsConnectionManager.h"
#include "TextureInfoCache.h"
// #include "Parse.h"
#include "OdePhysicsEngine.h"
#include "Math.h"
#include "Diag.h"
#include "Checksum.h"
#include "ObjectImportExport.h"
#include "ScriptInfoCache.h"
#include "TerrainInfoCache.h"
//#include "CollisionAndPhysicsDllLoader.h"
#include "Animation.h"
#include "Config.h"
#include "MeshInfoCache.h"
#include "System.h"
#include "SpawnWrap.h"
#include "Collision.h"
#include "Terrain.h"

#define BUFSIZE 2047

const int iTicksPerFrame = 17;         //!< we assume the server is running at around 75fps, and if the server hasnothing better to do, it'll sleep this number of milliseconds
int LastTickCount = 0;   //!< tickcount of last frame

float fSayDistance = 10.0;            //!< How far Says travel

char ReadBuffer[4098];  //!< data buffer for socket reads
char SendBuffer[BUFSIZE + 1];  //!< data buffer for socket writes

mvsocket SocketDBInterfaceListener;  //!< socket for dbinterface component to connect on
mvsocket SocketDBInterface;   //!< socket for comms with dbinterface component
mvsocket SocketFileServerAgentListener;  //!< socket for serverfileagent component to connect on
mvsocket SocketFileServerAgent;  //!< socket for comms with serverfileagent component

mvsocket AuthServerConnection;   //!< socket to connect to AuthServer on

SocketsConnectionManagerClass ServerConsoleConnectionManager;  //!< Manages connections from server console (not yet implemented)
SocketsConnectionManagerClass MetaverseServerConnectionManager;  //!< Manages connections from OSMP clients (?)

CollisionAndPhysicsEngineClass CollisionAndPhysicsEngine;

mvWorldStorage World;                    //!< The World and container for all objects in it (except the hardcoded land)
TextureInfoCache textureinfocache;   //!< manages upload/download etc of textures
ScriptInfoCacheClass ScriptInfoCache;  //!< stores information about available scripts
TerrainCacheClass TerrainCache;   //!< stores information about available textures
//CollisionAndPhysicsDllLoaderClass CollisionAndPhysics;   //!< Physics dll loader
Animation animator( World );   //!< Used to animate the world (non-physics movement)
ConfigClass mvConfig;   //!< Load configuration from config.xml, and exposes it as properties
MeshInfoCacheClass MeshInfoCache;   //!< stores information about available meshfiles

COLLISION Collisions[2048]; //!< Active collisions; this is used to pass collision information from the physics engine to the scripting engines
COLLISION Colliding[2048]; //!< collision data from last frame; this is used to pass collision information from the physics engine to the scripting engines
int CollidingBufSize = 0;
int CollisionBufSize = 0;


typedef set<int>
SET_INT;
SET_INT DirtyCache;            //!< set of all objects which have been changed but not yet written to db (mostly for objectmove stuff)
int iDirtyCacheWriteDelaySeconds = 10;   //!< Interval between writing objects that have moved to db
int iLastDirtyCacheWriteTickCount = 0;   //!< Last dirty cache write tickcount (careful, tickcount is in milliseconds)

//! Returns true or false according to whether rConnection is a local client or not. Used for privilege assignment to local scripting engines
bool IsLocalClient( const CONNECTION &rConnection )
{
    cout << "inet: " << inet_ntoa( rConnection.connectionsocket.GetPeer() ) << endl;
    if( strcmp( inet_ntoa( rConnection.connectionsocket.GetPeer() ), "0.0.0.0" ) == 0 )
    {
        return true;
    }
    else if( strcmp( inet_ntoa( rConnection.connectionsocket.GetPeer() ), "127.0.0.1" ) == 0 )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//! Waits for something to happen on a socket, or for iTicksPerFrame mseconds to pass since last frame
void SocketsBlockTillNextFrame()
{
    vector<const mvsocket *> sockets;
    int iTicksSinceLastTime = MVGetTickCount() - LastTickCount;

    LastTickCount = MVGetTickCount();

    int iTicksTillNextFrame = iTicksPerFrame - iTicksSinceLastTime;
    if( iTicksTillNextFrame < 0 )
    {
        iTicksTillNextFrame = 0;
    }

    ServerConsoleConnectionManager.GetSocketList( sockets );
    MetaverseServerConnectionManager.GetSocketList( sockets );
    sockets.push_back(&SocketDBInterface);
    sockets.push_back(&SocketDBInterfaceListener);

    // DEBUG("Blocking for " << iTicksTillNextFrame << " ticks");

    SocketsReadBlock(iTicksTillNextFrame, sockets);
}

//! Sends out updates to clients for all objects that have changed in the world
void ManageDirtyCache()   // could maybe use a separate thread for this???
{
    int iArrayNum;
    int iReference;
    if( MVGetTickCount() - iLastDirtyCacheWriteTickCount > 1000 * iDirtyCacheWriteDelaySeconds )
    {
        SET_INT::iterator iterator;
        for( iterator = DirtyCache.begin(); iterator != DirtyCache.end(); iterator++ )
        {
            iReference = *iterator;
            iArrayNum = World.GetArrayNumForObjectReference( iReference );
            {
                if( iArrayNum != -1 )
                {
                    TiXmlDocument IPC;
                    IPC.Parse( "<objectupdate><meta><avatar/></meta><geometry><pos/><rot/><scale/></geometry><faces><face num=\"0\"><color/></face></faces></objectcreate>" );
                    World.GetObject( iArrayNum )->WriteToXMLDoc( IPC.RootElement() );

                    ostringstream messagestream;
                    messagestream << IPC << endl;
                    string IPCText = messagestream.str();
                    DEBUG( "Updating dirty object to db [" << IPCText << "]" );
                    SocketDBInterface.Send( IPCText.c_str() );
                }
            }
        }
        DirtyCache.clear();
        iLastDirtyCacheWriteTickCount = MVGetTickCount();
    }
}

//! Sends Message to all connected metaverse clients
void BroadcastToAllClients( const char *Message )
{
    MetaverseServerConnectionManager.Broadcast( Message );
}

//! Sends message to all local connected clients; this will be primarily scripting engines

//! Sends message to all local connected clients; this will be primarily scripting engines
//! These messages will tend to be higher security than the general internet client messages
//! and / or higher bandwidth.  For example, information about scripts is generally
//! only sent to local clients
void BroadcastToLocalClients( const char *Message )
{
    ConnectionsIteratorTypedef iterator;
    for( iterator = MetaverseServerConnectionManager.Connections.begin(); iterator != MetaverseServerConnectionManager.Connections.end(); iterator++ )
    {
        if( IsLocalClient( iterator->second ) )
            //if( iterator->second.bInternet == 0 )
        {
            DEBUG(  "sending to local client " << inet_ntoa( iterator->second.connectionsocket.GetPeer() ) <<  " " << Message ); // DEBUG
            MetaverseServerConnectionManager.SendThruConnection( iterator->second, Message );
        }
    }
}

//! Sends Message to all non-local connected client; ie users out on the Internet
void BroadcastToInternetClients( const char *Message )
{
    ConnectionsIteratorTypedef iterator;
    for( iterator = MetaverseServerConnectionManager.Connections.begin(); iterator != MetaverseServerConnectionManager.Connections.end(); iterator++ )
    {
        if( !IsLocalClient( iterator->second ) )
            //if( iterator->second.bInternet == 1)
        {
            DEBUG(  "sending to Internet client " << inet_ntoa( iterator->second.connectionsocket.GetPeer() ) << " " << Message ); // DEBUG
            MetaverseServerConnectionManager.SendThruConnection( iterator->second, Message );
        }
    }
}

//! Broadcasts the existing object specified by iObjectIndex to all connected clients
void BroadcastExistingObject( int iObjectIndex )
{
    if( World.GetObject( iObjectIndex ) != NULL )
    {
        printf( "sending object %i reference %i\n", iObjectIndex, World.GetObject( iObjectIndex )->iReference );
        TiXmlDocument IPC;
        IPC.Parse( "<objectrefreshdata>"
                   "<meta><avatar /></meta>"
                   "<geometry><pos /><rot /><scale /></geometry>"
                   "<faces><face num=\"0\"/></faces>"
                   "</objectrefreshdata>" );
        DEBUG( "about to write to xml doc object type " << World.GetObject(iObjectIndex)->ObjectType );
        World.GetObject(iObjectIndex)->WriteToXMLDoc( IPC.RootElement() );
        DEBUG( "xml doc done" );

        std::string IPCText;
        IPCText << IPC;
        sprintf( SendBuffer, "%.2046s\n", IPCText.c_str() );
        DEBUG( "Broadcasting to clients " << SendBuffer  );

        BroadcastToLocalClients( SendBuffer );

        string InternetIPC;
        if( IPC.RootElement()->FirstChildElement("scripts") != NULL )
        {
            IPC.RootElement()->RemoveChild( IPC.RootElement()->FirstChildElement( "scripts" ) );
        }
        InternetIPC << IPC;
        sprintf( SendBuffer, "%.2046s\n", InternetIPC.c_str() );

        BroadcastToInternetClients( SendBuffer );
    }
}

//! Sends the object specified by iObjectIndex to the client specified by rConnection
void SendExistingObjectToOneConnection( CONNECTION &rConnection, int iObjectIndex )
{
    Object *p_Object = World.GetObject( iObjectIndex );
    if( p_Object != NULL )
    {
        printf( "sending object %i reference %i\n", iObjectIndex, p_Object->iReference );
        TiXmlDocument IPC;
        IPC.Parse( "<objectrefreshdata>"
                   "<meta><avatar /></meta>"
                   "<geometry><pos /><rot /><scale /></geometry>"
                   "<faces><face num=\"0\"><color/></face></faces>"
                   "</objectrefreshdata>" );
        DEBUG( "about to write to xml doc object type " << p_Object->ObjectType );
        p_Object->WriteToXMLDoc( IPC.RootElement() );
        DEBUG( "xml doc done" );

        if( strcmp( p_Object->sScriptReference, "" ) != 0 )
        {
            if( strcmp( inet_ntoa( rConnection.connectionsocket.GetPeer() ), "0.0.0.0" ) != 0 )
            {
                IPC.RootElement()->RemoveChild( IPC.RootElement()->FirstChildElement( "scripts" ) );
            }
        }

        std::string IPCText;
        IPCText << IPC;
        char Message[2047];
        sprintf( Message, "%.2046s\n", IPCText.c_str() );
        DEBUG(  "Sending[" << Message << "] to client ireference " << rConnection.iForeignReference ); // DEBUG

        MetaverseServerConnectionManager.SendThruConnection( rConnection, Message );
    }
}

//! Tells the dbinterface component to delete object p_Object
void DeleteObjectFromDBEx( Object *p_Object )
{
    ostringstream messagestream;
    messagestream << "<objectdelete ireference=\"" << p_Object->iReference << "\" type=\"" << p_Object->sDeepObjectType << "\"/>" << endl;
    string message = messagestream.str();
    DEBUG( " sending to db: " << message );
    SocketDBInterface.Send( message.c_str() );
}

//! Loops through p_Object and all of its children, and asks the dbinterface component to delete them
void RecursiveDelete( Object *p_Object )
{
    DEBUG(  "recursivedelete{}" ); // DEBUG
    if( dynamic_cast< ObjectGrouping * >(p_Object)->
            iNumSubObjects > 0 )
    {
        DEBUG(  "deleting subobjects.." ); // DEBUG
        for( int i = 0; i < dynamic_cast< ObjectGrouping * >(p_Object)->
                iNumSubObjects;
                i++ )
        {
            if( strcmp( dynamic_cast< ObjectGrouping * >(p_Object)->SubObjectReferences[ i ]->
                        sDeepObjectType, "OBJECTGROUPING" ) == 0 )
            {
                RecursiveDelete( dynamic_cast< ObjectGrouping * >(p_Object)->SubObjectReferences[ i ] );
            }
            else
            {
                DeleteObjectFromDBEx( dynamic_cast< ObjectGrouping * >(p_Object)->SubObjectReferences[ i ] );
            }
        }
    }
    DeleteObjectFromDBEx( p_Object );
}

//! Asks the dbinterface to delete the object specified by pElement (could be an objectgrouping)
void DeleteObjectFromDB( int iownerreference, TiXmlElement *pElement )
{
    DEBUG(  "DeleteObjectFromDB" ); // DEBUG
    int iReference = atoi( pElement->Attribute("ireference" ) );

    Object *p_Object = World.GetObjectByReference( iReference );
    if( p_Object != NULL )
    {
        if( strcmp( p_Object->sDeepObjectType, "AVATAR" ) == 0 )
        {
            DEBUG(  "Attempt to delete avatar object, ignoring" ); // DEBUG
            return;
        }

        if( p_Object->iParentReference != 0 )
        {
            if( !World.IsLastPrimInGroup( p_Object->iParentReference ) )
            {
                DEBUG(  "dereferencing parent..." ); // DEBUG
                World.DereferenceParent( p_Object );
            }
            else
            {
                DEBUG(  "attempting to delete last prim in group -> aborting" ); // DEBUG
                return;
            }
        }

        if( strcmp( p_Object->ObjectType, "OBJECTGROUPING" ) == 0 )
        {
            DEBUG( "received demand to delete object ref %i,sending to db..." << iReference );

            if( pElement->Attribute("recursivedelete" ) == NULL
                    || strcmp( pElement->Attribute("recursivedelete" ), "true" ) == 0 )
            {
                DEBUG(  "recusrive delete" ); // DEBUG
                RecursiveDelete( p_Object );
            }
            else
            {
                DEBUG(  "just remove group, to unlink" ); // DEBUG
                SocketDBInterface.Send( "<objectdelete ireference=\"%i\" type=\"%s\"/>\n", iReference, p_Object->sDeepObjectType );
            }
        }
        else
        {
            DEBUG(  "not an object grouping, sending directly to db..." ); // DEBUG
            SocketDBInterface.Send( "<objectdelete ireference=\"%i\" type=\"%s\"/>\n", iReference, p_Object->sDeepObjectType );
        }
    }
}

//! Sends the object specified by pElement to the dbinterface for storage in the db
void StoreObjectInDB( int iownerreference, TiXmlElement *pElement )
{
    if( World.iNumObjects < World.GetMaxObjects() )
    {
        DEBUG( "StoreObjectinDB()" );

        pElement->SetAttribute( "owner", iownerreference );

        std::string IPCString;
        IPCString << *pElement;
        sprintf( SendBuffer, "%s\n", IPCString.c_str() );

        SocketDBInterface.Send( SendBuffer );
    }
    else
    {
        DEBUG( "No more object space available!\n" );
    }
}

//! Sends entire world state to the client specified by rConnection
void SendCurrentDataToConnection( CONNECTION rConnection )
{
    DEBUG( "Sending world state to client ref" << rConnection.iForeignReference );

    ostringstream clientmessagestream;
    clientmessagestream << "<skyboxupdate stexturereference=\"" << World.GetSkyboxChecksum() << "\" />" << endl;
    MetaverseServerConnectionManager.SendThruConnection( rConnection, clientmessagestream.str().c_str() );



    int i;
    for( i = 0; i < World.iNumObjects; i++ )
    {
        SendExistingObjectToOneConnection( rConnection, i );
    }


    TextureIteratorTypedef iterator;
    for( iterator = textureinfocache.Textures.begin(); iterator != textureinfocache.Textures.end(); iterator++ )
    {
        DEBUG(  "sending texture " << iterator->second.sSourceFilename ); // DEBUG
        TEXTUREINFO &rTextureInfo = iterator->second;
        ostringstream clientmessagestream;
        clientmessagestream << "<texture checksum=\"" << rTextureInfo.sChecksum << "\" serverfilename=\"" << rTextureInfo.sServerFilename << "\" sourcefilename=\"" << rTextureInfo.sSourceFilename << "\"/>" << endl;
        MetaverseServerConnectionManager.SendThruConnection( rConnection, clientmessagestream.str().c_str() );
    }

    TerrainIteratorTypedef terrainiterator;
    for( terrainiterator = TerrainCache.Terrains.begin(); terrainiterator != TerrainCache.Terrains.end(); terrainiterator++ )
    {
        DEBUG(  "sending terrain " << terrainiterator->second.sSourceFilename ); // DEBUG
        TerrainINFO &rTerrainInfo = terrainiterator->second;
        ostringstream clientmessagestream;
        clientmessagestream << "<terrain checksum=\"" << rTerrainInfo.sChecksum << "\" serverfilename=\""
        << rTerrainInfo.sServerFilename << "\" sourcefilename=\"" << rTerrainInfo.sSourceFilename << "\"/>" << endl;
        MetaverseServerConnectionManager.SendThruConnection( rConnection, clientmessagestream.str().c_str() );
    }

    FileInfoIteratorTypedef meshiterator;
    for( meshiterator = MeshInfoCache.Files.begin(); meshiterator != MeshInfoCache.Files.end(); meshiterator++ )
    {
        DEBUG(  "sending mesh " << meshiterator->second.sSourceFilename ); // DEBUG
        FILEINFO &rMeshInfo = meshiterator->second;
        ostringstream clientmessagestream;
        clientmessagestream << "<meshfile checksum=\"" << rMeshInfo.sChecksum << "\" serverfilename=\""
        << rMeshInfo.sServerFilename << "\" sourcefilename=\"" << rMeshInfo.sSourceFilename << "\"/>" << endl;
        MetaverseServerConnectionManager.SendThruConnection( rConnection, clientmessagestream.str().c_str() );
    }

    if( strcmp( inet_ntoa( rConnection.connectionsocket.GetPeer() ), "0.0.0.0" ) == 0 )
    {
        ScriptIteratorTypedef scriptiterator;
        for( scriptiterator = ScriptInfoCache.Scripts.begin(); scriptiterator != ScriptInfoCache.Scripts.end(); scriptiterator++ )
        {
            DEBUG(  "sending script " << scriptiterator->second.sSourceFilename ); // DEBUG
            SCRIPTINFO &rScriptInfo = scriptiterator->second;
            ostringstream clientmessagestream;
            clientmessagestream << "<script checksum=\"" << rScriptInfo.sChecksum << "\" serverfilename=\"" << rScriptInfo.sServerFilename << "\" sourcefilename=\"" << rScriptInfo.sSourceFilename << "\"/>" << endl;
            MetaverseServerConnectionManager.SendThruConnection( rConnection, clientmessagestream.str().c_str() );
        }
    }
    DEBUG("Done SendCurrentDataToConnection");
}

//! Removes the object specified by pElement from internal world, and broadcasts delete to all clients
//!
//! Removes the object specified by pElement from internal world, and broadcasts delete to all clients
//! database has already deleted object by this time
void CacheAndBroadcastObjectDeleteFromDB( TiXmlElement *pElement )
{
    World.DeleteObjectXML( pElement );

    char sMessage[256];
    sprintf( sMessage, "<objectdelete ireference=\"%i\"/>\n", atoi( pElement->Attribute("ireference") ) );
    BroadcastToAllClients( sMessage );
    CollisionAndPhysicsEngine.ObjectDestroy( atoi( pElement->Attribute("ireference" ) ) );
}

//! Adds the object specified by pElement to internal world, and broadcasts create/update to all clients
//!
//! Adds the object specified by pElement to internal world, and broadcasts create/update to all clients
//! database has already created/updated object by this time
void CacheAndBroadcastObjectFromDB( TiXmlElement *pElement )
{
    Object *p_Object = World.StoreObjectXML( pElement );
    CollisionAndPhysicsEngine.ObjectModify( p_Object );

    std::string IPCText;
    IPCText << *pElement;
    sprintf( SendBuffer, "%.2046s\n", IPCText.c_str() );

    BroadcastToLocalClients( SendBuffer );

    string InternetIPC;
    if( pElement->FirstChildElement("scripts") != NULL )
    {
        pElement->RemoveChild( pElement->FirstChildElement( "scripts" ) );
    }
    InternetIPC << *pElement;
    sprintf( SendBuffer, "%.2046s\n", InternetIPC.c_str() );

    BroadcastToInternetClients( SendBuffer );
}

//! Adds the skybox specified by pElement to internal world, and broadcasts to all clients
//!
//! Adds the skybox specified by pElement to internal world, and broadcasts to all clients
//! database has already been updated by this time
void CacheAndBroadcastSkyboxFromDB( TiXmlElement *pElement )
{
    char t[33];
    strcpy(t,pElement->Attribute("stexturereference"));
    World.SetSkyboxChecksum( t );
    DEBUG("** SET WORD SKYBOX " << t);

    std::string IPCText;
    IPCText << *pElement;
    sprintf( SendBuffer, "%.2046s\n", IPCText.c_str() );

    BroadcastToLocalClients( SendBuffer );

    string InternetIPC;
    InternetIPC << *pElement;
    sprintf( SendBuffer, "%.2046s\n", InternetIPC.c_str() );

    BroadcastToInternetClients( SendBuffer );
}

//! Adds the texture specified by pElement to internal texture fileinfocache, and broadcasts to all clients
//!
//! Adds the texture specified by pElement to internal texture fileinfocache, and broadcasts to all clients
//! database has already been updated by this time
void CacheAndBroadcastTextureFromDB( TiXmlElement *p_Element )
{
    textureinfocache.AddTextureInfoFromXML( p_Element );
    ostringstream clientmessagestream;
    clientmessagestream << "<texture checksum=\"" << p_Element->Attribute("checksum") << "\" serverfilename=\"" << p_Element->Attribute("sourcefilename") << "\" sourcefilename=\"" << p_Element->Attribute("sourcefilename") << "\"/>" << endl;
    BroadcastToAllClients( clientmessagestream.str().c_str() );
}

//! Adds the terrain specified by pElement to internal terrain fileinfocache, and broadcasts to all clients
//!
//! Adds the terrain specified by pElement to internal terrain fileinfocache, and broadcasts to all clients
//! database has already been updated by this time
void CacheAndBroadcastTerrainFromDB( TiXmlElement *p_Element )
{
    TerrainCache.AddTerrainInfoFromXML( p_Element );
    ostringstream clientmessagestream;
    clientmessagestream << "<terrain checksum=\"" << p_Element->Attribute("checksum") << "\" serverfilename=\"" << p_Element->Attribute("sourcefilename") << "\" sourcefilename=\"" << p_Element->Attribute("sourcefilename") << "\"/>" << endl;
    BroadcastToAllClients( clientmessagestream.str().c_str() );
}

//! Adds the meshfile specified by pElement to internal meshfile fileinfocache, and broadcasts to all clients
//!
//! Adds the meshfile specified by pElement to internal meshfile fileinfocache, and broadcasts to all clients
//! database has already been updated by this time
void CacheAndBroadcastMeshfileFromDB( TiXmlElement *p_Element )
{
    MeshInfoCache.AddFileInfoFromXML( p_Element );
    ostringstream clientmessagestream;
    clientmessagestream << "<meshfile checksum=\"" << p_Element->Attribute("checksum") << "\" serverfilename=\"" << p_Element->Attribute("sourcefilename") << "\" sourcefilename=\"" << p_Element->Attribute("sourcefilename") << "\"/>" << endl;
    BroadcastToAllClients( clientmessagestream.str().c_str() );
}

//! Adds the script specified by pElement to internal script fileinfocache, and broadcasts to all clients
//!
//! Adds the script specified by pElement to internal script fileinfocache, and broadcasts to all clients
//! database has already been updated by this time
void CacheAndBroadcastScriptFromDB( TiXmlElement *p_Element )
{
    DEBUG(  "CacheAndBroadcastScriptFromDB..." ); // DEBUG
    ScriptInfoCache.AddInfoFromXML( p_Element );
    ostringstream clientmessagestream;
    clientmessagestream << "<script checksum=\"" << p_Element->Attribute("checksum") << "\" serverfilename=\"" << p_Element->Attribute("sourcefilename") << "\" sourcefilename=\"" << p_Element->Attribute("sourcefilename") << "\"/>" << endl;
    BroadcastToLocalClients( clientmessagestream.str().c_str() );
    DEBUG(  "...done" ); // DEBUG
}

//! Handles event originating from a connected client, currently means a mouse click (I think?)
void HandleEvent( int iOwnerReference, TiXmlElement *pElement )
{
    string sEventType = pElement->Attribute("type");
    if( sEventType == "clickdown" )
    {
        ostringstream messagestream;

        pElement->SetAttribute("iowner", iOwnerReference );

        messagestream << *pElement << endl;
        DEBUG(  "broadcasting to local clients: " << messagestream.str() ); // DEBUG
        BroadcastToLocalClients( messagestream.str().c_str() );
    }
}

//! Handles an object update request from an object; first step is to ask dbinterface to update database
//!
//! Handles an object update request from an object; first step is to ask dbinterface to update database
//! Once database has been updated, dbinterface will send us back the confirmation, and we'll update it in
//! our internal representation and broadcast to all clients (including original initiator client)
void UpdateObjectCachetoDBAndBroadcast( int iOwnerReference, TiXmlElement *pElement )
{
    DEBUG(  "update request received from clientref " << iOwnerReference ); // DEBUG
    int iReference = atoi( pElement->Attribute("ireference" ) );
    Object *p_Object;
    p_Object = World.GetObjectByReference( iReference );

    if( p_Object != NULL )
    {
        if( strcmp( p_Object->ObjectType, "OBJECTGROUPING" ) != 0 )
        {
            World.UpdateObjectXML( pElement );
        }
        else
        {
            ObjectGrouping *pGroup = dynamic_cast< ObjectGrouping *>( p_Object );
            set < int >
            PreviousMembers;
            PreviousMembers.clear();
            for( int i = 0; i < pGroup->iNumSubObjects; i++ )
            {
                PreviousMembers.insert( pGroup->SubObjectReferences[ i ]->iReference );
            }

            World.UpdateObjectXML( pElement );

            for( set < int >::iterator iterator = PreviousMembers.begin(); iterator != PreviousMembers.end(); iterator++ )
            {
                int iMemberReference = *iterator;
                Object *p_ChildObject = World.GetObjectByReference( iMemberReference );
                ostringstream DBUpdateStream;
                DBUpdateStream << "<objectupdate ireference=\"" << p_ChildObject->iReference << "\" type=\"" << p_ChildObject->sDeepObjectType << "\">"
                << "<geometry>"
                << p_ChildObject->pos
                << p_ChildObject->rot
                << "</geometry>"
                << "</objectupdate>" << endl;
                DEBUG(  "sending to db " << DBUpdateStream.str() ); // DEBUG
                SocketDBInterface.Send( DBUpdateStream.str().c_str() );
            }

            for( int i = 0; i < pGroup->iNumSubObjects; i++ )
            {
                Object *p_ChildObject = pGroup->SubObjectReferences[ i ];
                ostringstream DBUpdateStream;
                DBUpdateStream << "<objectupdate ireference=\"" << p_ChildObject->iReference << "\" type=\"" << p_ChildObject->sDeepObjectType << "\">"
                << "<geometry>"
                << p_ChildObject->pos
                << p_ChildObject->rot
                << "</geometry>"
                << "</objectupdate>" << endl;
                DEBUG(  "sending to db " << DBUpdateStream.str() ); // DEBUG
                SocketDBInterface.Send( DBUpdateStream.str().c_str() );
            }
        }

        pElement->SetAttribute( "type", p_Object->sDeepObjectType );

        std::string IPCString;
        IPCString << *pElement;
        sprintf( SendBuffer, "%s\n", IPCString.c_str() );

        SocketDBInterface.Send( SendBuffer );

        BroadcastToLocalClients( SendBuffer );

        string InternetIPC;
        if( pElement->FirstChildElement("scripts") != NULL )
        {
            pElement->RemoveChild( pElement->FirstChildElement( "scripts" ) );
        }
        InternetIPC << *pElement;
        sprintf( SendBuffer, "%.2046s\n", InternetIPC.c_str() );

        BroadcastToInternetClients( SendBuffer );
    }
}

//! Handles a skybox request from an object; first step is to ask dbinterface to update database
//!
//! Handles a skybox request from an object; first step is to ask dbinterface to update database
//! Once database has been updated, dbinterface will send us back the confirmation, and we'll update it in
//! our internal representation and broadcast to all clients (including original initiator client)
void UpdateSkyboxCachetoDBAndBroadcast( int iOwnerReference, TiXmlElement *pElement )
{
    DEBUG(  "update skybox request received from clientref " << iOwnerReference ); // DEBUG

    std::string IPCString;
    IPCString << *pElement;
    sprintf( SendBuffer, "%s\n", IPCString.c_str() );

    SocketDBInterface.Send( SendBuffer );

    string InternetIPC;
    InternetIPC << *pElement;
    sprintf( SendBuffer, "%.2046s\n", InternetIPC.c_str() );

    DEBUG("Sending to Inet clients : " << SendBuffer);
    BroadcastToInternetClients( SendBuffer );
}

//! moves an object specified by pElement (contains the ireference and the move instructions); and broadcasts change to all connected clients
void MoveObjectAndBroadcast( TiXmlElement *pElement )
{
    int iReference = atoi( pElement->Attribute("ireference" ) );
    int iArrayNum = World.GetArrayNumForObjectReference( iReference );
    if( iArrayNum != -1 )
    {

        World.GetObject( iArrayNum )->UpdateFromXML( pElement );
        CollisionAndPhysicsEngine.ObjectModify( World.GetObject( iArrayNum ) );

        DirtyCache.insert( iReference );

        std::string IPCString;
        IPCString << *pElement;
        sprintf( SendBuffer, "%s\n", IPCString.c_str() );

        printf( "Broadcasting [%s]\n", SendBuffer );
        BroadcastToAllClients( SendBuffer );
    }
}

//! Returns the number of currently connected metaverse clients
int NumClientConnections( const char *avname )
{
    return MetaverseServerConnectionManager.NumClientConnectionsWithName( avname );
}

//! Processes a say, ooc etc received from rConnection, and specified by pElement
void ProcessComm( CONNECTION &rConnection, TiXmlElement *pElement )
{
    int iSenderReference;

    if( pElement->Attribute("iowner") == NULL || !IsLocalClient( rConnection ) )
    {
        iSenderReference = rConnection.iForeignReference;
    }
    else
    {
        iSenderReference = atoi( pElement->Attribute("iowner") );
    }

    DEBUG( "Sender ref is: " <<  iSenderReference ); // DEBUG

    int iArrayNum = World.GetArrayNumForObjectReference( iSenderReference );

    if( iArrayNum != -1 )
    {
        char Message[512];
        //sprintf( Message, pIPC->RootElement()

        pElement->SetAttribute( "ireference", iSenderReference );

        if( pElement->Attribute("type") == NULL )
        {
            pElement->SetAttribute( "type", "say" );
        }

        char ObjectName[65];
        if( strcmp( World.GetObject( iArrayNum )->sDeepObjectType, "AVATAR" ) == 0 )
        {
            sprintf( ObjectName, dynamic_cast< Avatar *>(World.GetObject( iArrayNum ))->avatarname );
        }
        else
        {
            string strObjectName = World.GetObject( iArrayNum )->sObjectName ;
            if (strObjectName == "")
            {
                strObjectName = World.GetObject( iArrayNum )->sDeepObjectType ;
            }
            sprintf( ObjectName, strObjectName.c_str() );

        }

        if( strcmp( pElement->Attribute("type"), "ooc" ) == 0 )
        {
            DEBUG( "received ooc comm, broadcasting... " );
            sprintf( SendBuffer, "<comm type=\"ooc\" ireference=\"%i\" objectname=\"%s\" message=\"%s\"/>\n",
                     iSenderReference, ObjectName, pElement->Attribute("message") );
            BroadcastToAllClients( SendBuffer );
        }
        else if( strcmp( pElement->Attribute("type"), "say" ) == 0 )
        {
            DEBUG( "received say comm, sending... " );

            float fSayDistanceSquared = fSayDistance * fSayDistance;

            ConnectionsIteratorTypedef iterator;
            for( iterator = MetaverseServerConnectionManager.Connections.begin(); iterator != MetaverseServerConnectionManager.Connections.end(); iterator++ )
            {
                int iTargetObjectReference = iterator->second.iForeignReference;
                int iArrayNumForThisTarget = World.GetArrayNumForObjectReference( iTargetObjectReference );
                if( iArrayNumForThisTarget != -1 )
                {
                    DEBUG( "targetreference " << iTargetObjectReference << " arrynum for target " << iTargetObjectReference << " arraynum "
                           "for speaker " << iArrayNum );
                    float TargetSquareVectorDistance = SquareVectorDistance( World.GetObject( iArrayNumForThisTarget )->pos, World.GetObject( iArrayNum )-> pos );
                    if( TargetSquareVectorDistance < fSayDistanceSquared )
                    {
                        DEBUG( "sending to " << iterator->second.name.c_str() );
                        sprintf( SendBuffer, "<comm type=\"say\" ireference=\"%i\" objectname=\"%s\" message=\"%s\"/>\n",
                                 iSenderReference, ObjectName, pElement->Attribute("message") );
                        MetaverseServerConnectionManager.SendThruConnection( iterator->second, SendBuffer );
                    }
                    else
                    {
                        DEBUG(  iterator->second.name << " is out of range" ); // DEBUG
                    }
                }
            }
        }
    }
}

//! Replies to an inforequest, using internal information, for example providing information about a specific script to a client
void SendInfoResponse( CONNECTION &rConnection, TiXmlElement *pElement )
{
    string sType = pElement->Attribute("type");
    if( sType == "SCRIPT" )
    {
        int iReference = atoi( pElement->Attribute("ireference") );
        int iClientEditReference = atoi( pElement->Attribute("clienteditreference") );

        Object *p_Object = World.GetObjectByReference( iReference );
        if( p_Object != NULL )
        {
            string sScriptReference = p_Object->sScriptReference;
            if( ScriptInfoCache.Scripts.find( sScriptReference ) != ScriptInfoCache.Scripts.end() )
            {
                SCRIPTINFO &rScriptInfo = ScriptInfoCache.Scripts.find( sScriptReference )->second;

                pElement->SetValue( "inforesponse" );
                pElement->SetAttribute( "checksum", rScriptInfo.sChecksum );
                pElement->SetAttribute( "sourcefilename", rScriptInfo.sSourceFilename );
                pElement->SetAttribute( "serverfilename", rScriptInfo.sServerFilename );

                ostringstream messagestream;
                messagestream << *pElement << endl;

                DEBUG(  "SEnding to client " << messagestream.str() ); // DEBUG
                MetaverseServerConnectionManager.SendThruConnection( rConnection, messagestream.str().c_str() );
            }
            else
            {
                DEBUG(  "script not found for object ref " << iReference ); // DEBUG
            }
        }
        else
        {
            DEBUG(  "object ref " << iReference << " not found" ); // DEBUG
        }
    }
    else if( sType == "DBUSERDATA" )
    {
        ostringstream DBStream;
        DBStream << *pElement << endl;
        DEBUG(  "sending to db: " << DBStream.str() ); // DEBUG
        SocketDBInterface.Send( DBStream.str().c_str() );
    }
}

//! Processes a reply to inforequest received from the dbinterface component
void ProcessInfoResponseFromDB( TiXmlElement *pElement )
{
    string sType = pElement->Attribute("type");
    if( sType == "DBUSERDATA" )
    {
        int iReference = atoi( pElement->Attribute( "ireference" ) );
        int iReplyToReference;
        if( pElement->Attribute( "ireplytoreference" ) != NULL )
        {
            iReplyToReference = atoi( pElement->Attribute( "ireplytoreference" ) );
        }
        else
        {
            iReplyToReference = iReference;
        }

        CONNECTION ClientConnection;
        if( MetaverseServerConnectionManager.GetConnectionForForeignReference( ClientConnection, iReplyToReference ) )
        {
            ostringstream messagestream;
            messagestream << *pElement << endl;
            DEBUG(  "sending to client " << iReplyToReference << " " << messagestream.str() ); // DEBUG
            MetaverseServerConnectionManager.SendThruConnection( ClientConnection, messagestream.str().c_str() );
        }
    }
}

//! Sends a hyperlink request message to the client specified by rConnection to connect to the server specified by pElement
void HyperlinkAgentFromXML( CONNECTION &rConnection, TiXmlElement *pElement )
{
    if( IsLocalClient( rConnection ) )
    {
        int iTargetReference = atoi( pElement->Attribute("itargetreference" ) );

        CONNECTION ClientConnection;
        if( MetaverseServerConnectionManager.GetConnectionForForeignReference( ClientConnection, iTargetReference ) )
        {
            ostringstream messagestream;
            messagestream << *pElement << endl;
            DEBUG(  "sending to client " << iTargetReference << " " << messagestream.str() ); // DEBUG
            MetaverseServerConnectionManager.SendThruConnection( ClientConnection, messagestream.str().c_str() );
        }
    }
}

//! Processes a database script data setinfo request specified by pElement, and sent by the client specified in rConnection
void SetInfoFromXML( CONNECTION &rConnection, TiXmlElement *pElement )
{
    string sType = pElement->Attribute("type" );

    if( sType == "DBUSERDATA" )
    {
        if( pElement->Attribute("iowner") == NULL || !IsLocalClient( rConnection ) )
        {
            pElement->SetAttribute("iowner", rConnection.iForeignReference );
        }

        ostringstream DBStream;
        DBStream << *pElement << endl;
        DEBUG(  "sending to db: " << DBStream.str() ); // DEBUG
        SocketDBInterface.Send( DBStream.str().c_str() );
    }
}

//! Processed messages received from the dbinterface component
void ProcessDBInterfaceMessages()
{
    int bytesRecv = 0;

    //  DEBUG( "CheckdBInfo()\n" );
    sprintf( ReadBuffer, "" );
    int ReadResult = SocketDBInterface.ReceiveLineIfAvailable( ReadBuffer );
    //   DEBUG( "GotReadREsult\n" );
    //   DEBUG( "Its: [%s]\n", ReadBuffer );

    if( ReadResult == SOCKETS_READ_OK )
    {
        if( ReadBuffer[0] == '<' )
        {
            DEBUG( "received xml from dbinterface, socket " << SocketDBInterface.GetSocket() << " " << ReadBuffer );
            TiXmlDocument IPC;
            IPC.Parse( ReadBuffer );
            if( strcmp( IPC.RootElement()->Value(), "loginaccept" ) == 0 )
            {
                ConnectionsIteratorTypedef iterator;
                iterator = MetaverseServerConnectionManager.Connections.find( atoi( IPC.RootElement()->Attribute("iconnectionref") ) );
                if( iterator != MetaverseServerConnectionManager.Connections.end() )
                {
                    CONNECTION &rConnection = iterator->second;

                    if( MetaverseServerConnectionManager.NumClientConnectionsWithName( IPC.RootElement()->Attribute("name") ) == 0 )
                    {
                        rConnection.name = IPC.RootElement()->Attribute("name");
                        rConnection.bAuthenticated = true;
                        rConnection.iForeignReference = atoi( IPC.RootElement()->Attribute("ireference" ) );
                        sprintf( SendBuffer, "<loginaccept name=\"%s\" ireference=\"%i\"/>\n", IPC.RootElement()->Attribute("name"), rConnection.iForeignReference );
                        DEBUG(  "sending to client " << SendBuffer ); // DEBUG
                        MetaverseServerConnectionManager.SendThruConnection( rConnection, SendBuffer );
                    }
                    else
                    {
                        sprintf( SendBuffer, "<loginreject name=\"%s\" reason=\"alreadyconnected\"/>\n", IPC.RootElement()->Attribute("name") );
                        printf( "sending to client [%s]\n", SendBuffer );
                        MetaverseServerConnectionManager.SendThruConnection( rConnection, SendBuffer );

                        DEBUG(  "Kicking client " << IPC.RootElement()->Attribute("name") << " : avatar already connected" ); // DEBUG
                        MetaverseServerConnectionManager.CloseConnectionNow( rConnection );
                    }
                }
            }
            else if( strcmp( IPC.RootElement()->Value(), "loginreject" ) == 0 )
            {
                ConnectionsIteratorTypedef iterator;
                iterator = MetaverseServerConnectionManager.Connections.find( atoi( IPC.RootElement()->Attribute("iconnectionref") ) );
                if( iterator != MetaverseServerConnectionManager.Connections.end() )
                {
                    CONNECTION &rConnection = iterator->second;

                    sprintf( SendBuffer, "<loginreject name=\"%s\" reason=\"failedauth\"/>\n", IPC.RootElement()->Attribute("name") );
                    DEBUG(  "sending to client " << SendBuffer ); // DEBUG
                    MetaverseServerConnectionManager.SendThruConnection( rConnection, SendBuffer );

                    DEBUG(  "Kicking client " << IPC.RootElement()->Attribute("name") << " for failed auth" ); // DEBUG
                    MetaverseServerConnectionManager.CloseConnectionNow( rConnection );
                }
            }
            else if( strcmp( IPC.RootElement()->Value(), "objectcreate" ) == 0 )
            {
                DEBUG( "received object create from db" );

                CacheAndBroadcastObjectFromDB( IPC.RootElement() );
            }
            else if( strcmp( IPC.RootElement()->Value(), "skyboxupdate" ) == 0 )
            {
                DEBUG( "received skybox update from db");
                CacheAndBroadcastSkyboxFromDB( IPC.RootElement() );
            }
            else if( strcmp( IPC.RootElement()->Value(), "objectdelete" ) == 0 )
            {
                DEBUG( "received object delete from db" );

                CacheAndBroadcastObjectDeleteFromDB( IPC.RootElement() );
            }
            else if( strcmp( IPC.RootElement()->Value(), "texture" ) == 0 )
            {
                DEBUG( "received texture info from db" );

                CacheAndBroadcastTextureFromDB( IPC.RootElement() );
            }
            else if( strcmp( IPC.RootElement()->Value(), "meshfile" ) == 0 )
            {
                DEBUG( "received meshfile info from db" );

                CacheAndBroadcastMeshfileFromDB( IPC.RootElement() );
            }
            else if( strcmp( IPC.RootElement()->Value(), "terrain" ) == 0 )
            {
                DEBUG( "received terrain info from db" );

                CacheAndBroadcastTerrainFromDB( IPC.RootElement() );
            }
            else if( strcmp( IPC.RootElement()->Value(), "script" ) == 0 )
            {
                DEBUG( "received script info from db" );

                CacheAndBroadcastScriptFromDB( IPC.RootElement() );
            }
            else if( strcmp( IPC.RootElement()->Value(), "inforesponse" ) == 0 )
            {
                DEBUG( "received inforesponse from db" );

                ProcessInfoResponseFromDB( IPC.RootElement() );
            }
            else if( strcmp( IPC.RootElement()->Value(), "objectrefreshdata" ) == 0 )
            {
                DEBUG( "received object refresh from db" );
                char Type[17];
                sprintf( Type, "%.16s", IPC.RootElement()->Attribute( "type") );

                CacheAndBroadcastObjectFromDB( IPC.RootElement() );
            }
        }
        else
        {
            DEBUG( "received legacy IPC from dbinterface " << SendBuffer );
        }
    }
    else if( ReadResult == SOCKETS_READ_SOCKETGONE )
    {
        printf( "Lost DBInterface connection, dieing..." );
        mvSystem::mvExit(1);
        // printf( "Lost DBInterface connection, waiting for new connection...\n" );
        // SocketDBInterface.CopyFrom( SocketDBInterfaceListener.AcceptNewConnection() );
        // printf( "DBInterface connection restored\n" );
    }
    //  DEBUG( "End of DBInfo function\n" );
}

//! Processes a message received from a connected client
void HandleClientInput( int iConnectionRef, CONNECTION &rConnection, char *Message )
{
    DEBUG( "client " << iConnectionRef << " says [" << Message << "]");
    if( Message[0] == '<' )  // XML IPC
    {
        string a = inet_ntoa(rConnection.connectionsocket.GetPeer());
        DEBUG(  "received xml from client ref " << rConnection.iForeignReference << " socket " << rConnection.connectionsocket.GetSocket() << " ip " << a << " [" << Message << "]" ); // DEBUG
        TiXmlDocument IPC;
        IPC.Parse( Message );
        DEBUG(  "bauthenticated for this client = " << rConnection.bAuthenticated ); // DEBUG
        if( !rConnection.bAuthenticated )
        {
            if( strcmp( IPC.RootElement()->Value(), "login" ) == 0 )
            {
                sprintf( SendBuffer, "<login iconnectionref=\"%i\" name=\"%s\" password=\"%s\"/>\n",
                         iConnectionRef,
                         IPC.RootElement()->Attribute("name"),
                         IPC.RootElement()->Attribute("password") );
                DEBUG( "Sending to db " << SendBuffer );
                SocketDBInterface.Send( SendBuffer );

                // Commented out; poor security; never trust the client in an MMORPG
                // inet clients pass internet=1
                //if ( atoi(IPC.RootElement()->Attribute("internet")) == 1)
                //{
                //   rConnection.bInternet = 1;
                //}
                //else
                //{
                //   rConnection.bInternet = 0;
                //}
            }
        }
        else
        {
            if( strcmp( IPC.RootElement()->Value(), "requestworldstate" ) == 0 )
            {
                SendCurrentDataToConnection( rConnection );
            }
            else if( strcmp( IPC.RootElement()->Value(), "objectcreate" ) == 0 )
            {
                StoreObjectInDB( rConnection.iForeignReference, IPC.RootElement() );
            }
            else if( strcmp( IPC.RootElement()->Value(), "objectimport" ) == 0 )
            {
                StoreObjectInDB( rConnection.iForeignReference, IPC.RootElement() );
            }
            else if( strcmp( IPC.RootElement()->Value(), "objectupdate" ) == 0 )
            {
                UpdateObjectCachetoDBAndBroadcast( rConnection.iForeignReference, IPC.RootElement() );
            }
            else if( strcmp( IPC.RootElement()->Value(), "skyboxupdate" ) == 0 )
            {
                UpdateSkyboxCachetoDBAndBroadcast( rConnection.iForeignReference, IPC.RootElement() );
            }
            else if( strcmp( IPC.RootElement()->Value(), "requestinfo" ) == 0 )
            {
                SendInfoResponse( rConnection, IPC.RootElement() );
            }
            else if( strcmp( IPC.RootElement()->Value(), "setinfo" ) == 0 )
            {
                SetInfoFromXML( rConnection, IPC.RootElement() );
            }
            else if( strcmp( IPC.RootElement()->Value(), "event" ) == 0 )
            {
                HandleEvent( rConnection.iForeignReference, IPC.RootElement() );
            }
            else if( strcmp( IPC.RootElement()->Value(), "hyperlinkagent" ) == 0 )
            {
                HyperlinkAgentFromXML( rConnection, IPC.RootElement() );
            }
            else if( strcmp( IPC.RootElement()->Value(), "objectdelete" ) == 0 )
            {
                DeleteObjectFromDB( rConnection.iForeignReference, IPC.RootElement() );
            }
            else if( strcmp( IPC.RootElement()->Value(), "objectmove" ) == 0 )
            {
                MoveObjectAndBroadcast( IPC.RootElement() );
            }
            else if( strcmp( IPC.RootElement()->Value(), "comm" ) == 0 )
            {
                DEBUG("Handle Input, Process Comm");
                ProcessComm( rConnection, IPC.RootElement() );
            }
            else if( strcmp( IPC.RootElement()->Value(), "capture" ) == 0 )
            {
                if (IsLocalClient(rConnection))
                {
                    int iReference = atoi(IPC.RootElement()->Attribute("iowner"));
                    DEBUG("Sending capture to client: " << iReference);
                    ostringstream clientmessagestream;
                    clientmessagestream << *IPC.RootElement() << endl;
                    CONNECTION rConnection2;
                    MetaverseServerConnectionManager.GetConnectionForForeignReference(rConnection2, iReference);
                    MetaverseServerConnectionManager.SendThruConnection( rConnection2 , clientmessagestream.str().c_str());

                }
            }
            else if( strcmp( IPC.RootElement()->Value(), "keyup" ) == 0 )
            {
                ostringstream messagestream;

                IPC.RootElement()->SetAttribute("iowner", rConnection.iForeignReference );

                messagestream << *IPC.RootElement() << endl;
                DEBUG("broadcasting to local clients: " << messagestream.str() ); // DEBUG
                BroadcastToLocalClients( messagestream.str().c_str() );
            }
            else if( strcmp( IPC.RootElement()->Value(), "keydown" ) == 0 )
            {
                ostringstream messagestream;

                IPC.RootElement()->SetAttribute("iowner", rConnection.iForeignReference );

                messagestream << *IPC.RootElement() << endl;
                DEBUG("broadcasting to local clients: " << messagestream.str() ); // DEBUG
                BroadcastToLocalClients( messagestream.str().c_str() );
            }
        }
    }
    else  // legacy IPC
    {
        DEBUG( "received legacy IPC from client " << iConnectionRef << " " << Message );
    }
}

//! Checks for any incoming messages from connected metaverse clients
void CheckForClientMessages()
{
    int bytesRecv = 0;
    for ( MetaverseServerConnectionManager.ConnectionsIterator = MetaverseServerConnectionManager.Connections.begin( ) ;
            MetaverseServerConnectionManager.ConnectionsIterator != MetaverseServerConnectionManager.Connections.end( );
            MetaverseServerConnectionManager.ConnectionsIterator++ )
    {
        sprintf( ReadBuffer, "" );
        int ReadResult = MetaverseServerConnectionManager.ReceiveIteratorLineIfAvailable( ReadBuffer );
        if( ReadResult == SOCKETS_READ_OK )
        {
            DEBUG( "client Read buffer " << ReadBuffer );
            HandleClientInput( MetaverseServerConnectionManager.ConnectionsIterator->first,
                               MetaverseServerConnectionManager.ConnectionsIterator->second,
                               ReadBuffer );
        }

    }
}

//! Handles messages from server console components; not currently implemented
void HandleConsoleInput( int iConnectionRef, CONNECTION &rConnection, char *Message )
{}

//! Registeres a file (texture, script, mesh etc) with the database, by sending the register message to the dbinterface component
void RegisterNewFileWithDB( TiXmlElement *p_Element )
{
    string type = p_Element->Attribute("type");
    ostringstream dbmessagestream;
    dbmessagestream << "<registerfile type=\"" << type << "\" sourcefilename=\"" << p_Element->Attribute("sourcefilename")
    << "\" checksum=\"" << p_Element->Attribute("checksum") << "\" serverfilename=\"" << p_Element->Attribute("serverfilename") << "\"/>" << endl;
    DEBUG( "sending to db " << dbmessagestream.str() );
    SocketDBInterface.Send( dbmessagestream.str().c_str() );

    if( type == "TEXTURE" )
    {
        DEBUG(  "received texture" ); // DEBUG
        textureinfocache.AddTextureInfoFromXML( p_Element );
        ostringstream clientmessagestream;
        //    clientmessagestream << "<texture checksum=\"" << p_Element->Attribute("checksum") << "\" sourcefilename=\"" << p_Element->Attribute("sourcefilename") << "\"/>" ); // DEBUG
        clientmessagestream << "<texture checksum=\"" << p_Element->Attribute("checksum") << "\" serverfilename=\"" << p_Element->Attribute("sourcefilename") << "\" sourcefilename=\"" << p_Element->Attribute("sourcefilename") << "\"/>" << endl;
        BroadcastToAllClients( clientmessagestream.str().c_str() );
    }
    else if( type == "TERRAIN" )
    {
        DEBUG(  "received terrain" ); // DEBUG
        TerrainCache.AddTerrainInfoFromXML( p_Element );
        ostringstream clientmessagestream;
        //    clientmessagestream << "<texture checksum=\"" << p_Element->Attribute("checksum") << "\" sourcefilename=\"" << p_Element->Attribute("sourcefilename") << "\"/>" ); // DEBUG
        clientmessagestream << "<terrain checksum=\"" << p_Element->Attribute("checksum") << "\" serverfilename=\"" << p_Element->Attribute("sourcefilename") << "\" sourcefilename=\"" << p_Element->Attribute("sourcefilename") << "\"/>" << endl;
        BroadcastToAllClients( clientmessagestream.str().c_str() );
    }
    else if( type == "MESHFILE" )
    {
        DEBUG(  "received md2mesh" ); // DEBUG
        MeshInfoCache.AddFileInfoFromXML( p_Element );
        ostringstream clientmessagestream;
        clientmessagestream << "<meshfile checksum=\"" << p_Element->Attribute("checksum") << "\" serverfilename=\"" << p_Element->Attribute("sourcefilename") << "\" sourcefilename=\"" << p_Element->Attribute("sourcefilename") << "\"/>" << endl;
        DEBUG(  "sending to clients " << clientmessagestream.str() ); // DEBUG
        BroadcastToAllClients( clientmessagestream.str().c_str() );
    }
    else if( type == "SCRIPT" )
    {
        DEBUG(  "received script" ); // DEBUG
        ScriptInfoCache.AddInfoFromXML( p_Element );
        ostringstream clientmessagestream;
        clientmessagestream << "<script checksum=\"" << p_Element->Attribute("checksum") << "\" serverfilename=\"" << p_Element->Attribute("sourcefilename") << "\" sourcefilename=\"" << p_Element->Attribute("sourcefilename") << "\"/>" << endl;
        BroadcastToLocalClients( clientmessagestream.str().c_str() );
    }
    else
    {
        DEBUG(  "WARNING: RegisterNewFileWithDB unknown file type [" << type << "]" ); // DEBUG
    }
}

//! Checks for messages from server console components; not really used currently
void CheckForConsoleMessages()
{
    int bytesRecv = 0;
    for ( ServerConsoleConnectionManager.ConnectionsIterator = ServerConsoleConnectionManager.Connections.begin( ) ;
            ServerConsoleConnectionManager.ConnectionsIterator != ServerConsoleConnectionManager.Connections.end( );
            ServerConsoleConnectionManager.ConnectionsIterator++ )
    {
        sprintf( ReadBuffer, "" );
        int ReadResult = ServerConsoleConnectionManager.ReceiveIteratorLineIfAvailable( ReadBuffer );
        if( ReadResult == SOCKETS_READ_OK )
        {
            DEBUG( "server console Read buffer " << ReadBuffer );
            HandleConsoleInput( ServerConsoleConnectionManager.ConnectionsIterator->first,
                                ServerConsoleConnectionManager.ConnectionsIterator->second,
                                ReadBuffer );
        }

    }
}

//! Checks for messages received from serverfileagent component
void CheckForServerFileAgentMessage()
{
    int ReadResult = 0;

    if( SocketFileServerAgent.DataAvailable() )
    {
        ReadResult = SocketFileServerAgent.ReceiveLineIfAvailable( ReadBuffer );
        if( ReadResult == SOCKETS_READ_OK )
        {
            if( ReadBuffer[0] == '<' )
            {
                DEBUG( "XML IPC received from SocketFileServerAgent " << ReadBuffer << endl );

                TiXmlDocument IPC;
                IPC.Parse( ReadBuffer );
                TiXmlElement *pElement = IPC.RootElement();
                if( strcmp( pElement->Value(), "newfileupload" ) == 0 )
                {
                    RegisterNewFileWithDB( IPC.RootElement() );
                }
            }
            else
            {
                DEBUG( "Legacy IPC received from Server " << ReadBuffer );
            }
        }
        else if( ReadResult == SOCKETS_READ_SOCKETGONE )
        {
            printf( "Lost server file agent connection, dieing...\n" );
            mvSystem::mvExit(1);
        }
    }
}

//! Connects to the authentication server specified in the config.xml file
bool ConnectToAuthServer()
{
    bool bResult = false;

    char sAuthServerIP[129];
    sprintf( sAuthServerIP, "%.128s", mvConfig.AuthServers[0].sIPAddress.c_str() );

    if( isalpha( sAuthServerIP[0] ) )
    {
        hostent* remoteHost;
        remoteHost = gethostbyname( sAuthServerIP );
        ostringstream ipaddressstream;
        ipaddressstream << (int)(unsigned char)remoteHost->h_addr_list[0][0] << "."
        << (int)(unsigned char)remoteHost->h_addr_list[0][1] << "."
        << (int)(unsigned char)remoteHost->h_addr_list[0][2] << "."
        << (int)(unsigned char)remoteHost->h_addr_list[0][3];
        DEBUG(  ipaddressstream.str() ); // DEBUG
        sprintf( sAuthServerIP, ipaddressstream.str().c_str() );
        //exit(1);
    }

    AuthServerConnection.Init();

    DEBUG( "connecting to auth server [" << sAuthServerIP << "] [" << mvConfig.AuthServers[0].iPort << "]" );
    AuthServerConnection.ConnectToServer( inet_addr( sAuthServerIP ), mvConfig.AuthServers[0].iPort );

    INFO( "Attempting to login to authserver as " << mvConfig.sSimName );
    ostringstream messagestream;
    messagestream << "<login name=\"" << mvConfig.sSimName << "\" "
    "password=\"" << mvConfig.AuthServers[0].sPassword << "\"/>" << endl;
    AuthServerConnection.Send( messagestream.str().c_str() );

    bool bAuthenticated = false;
    int ReadResult = SOCKETS_READ_OK;
    while( ReadResult == SOCKETS_READ_OK && bAuthenticated == false )
    {
        DEBUG( "reading from auth server..." );
        ReadResult = AuthServerConnection.ReceiveLineBlocking( ReadBuffer );
        DEBUG( "read " << ReadBuffer );

        if( ReadResult == SOCKETS_READ_OK )
        {
            TiXmlDocument IPC;
            IPC.Parse( ReadBuffer );
            if( strcmp( IPC.RootElement()->Value(), "loginaccept" ) == 0 )
            {
                INFO( "Successfully logged in to authserver" );
                bAuthenticated = true;
                //result = "LOGINOK";
                bResult = true ;
            }
            else if( strncmp( ReadBuffer, "<loginreject", strlen( "<loginreject" ) ) == 0 )
            {
                INFO( "Auth server Login failed." );
                //result = "Login failed.  Please try again.";
                bResult = false;
            }
        }
        else
        {
            INFO( "Failed to connect to auth server, or login failed." );
            //result = "Failed to connect to server, or login failed. Please try again.";
            bResult = false;
        }
    }

    if( bAuthenticated == false )
    {
        INFO( "Failed to connect to auth server, or login failed." );
        //result = "Failed to connect to server, or login failed. Please try again.";
        bResult = false;
    }
    return bResult;
}

//! Sends information about objects collisions to the scripting engines (ie, to locally connected clients)
void SendCollisionsToScripts()
{
    // new entries in Collisions get collisionstart
    // entries in Colliding that are not in Collisions get collisionend
    // colliding = collisions

    list< int > started;
    list< int >::iterator istarted;

    int i;
    int j;
    int f;
    //DEBUG("CollisionBufSize " << CollisionBufSize << " CollidingBufSize " << CollidingBufSize);
    for (i = 0; i < CollisionBufSize; i++)
    {
        //DEBUG("1 CsiT : " << Collisions[i].iTarget << " vs CsiR " << Collisions[i].iCollidingObjectReference);
        f = -1;
        for (j = 0; j < CollidingBufSize; j++)
        {
            //DEBUG("1 CdjT : " << Colliding[j].iTarget << " vs CdjR " << Colliding[j].iCollidingObjectReference);
            if (Collisions[i].iTarget == Colliding[j].iTarget)
            {
                if (Collisions[i].iCollidingObjectReference == Colliding[j].iCollidingObjectReference )
                {
                    f = j;
                    break;
                }
            }
        }
        //DEBUG("f1: " << f);
        if (f == -1)
        {
            started.push_back(Collisions[i].iTarget);
            ostringstream clientmessagestream;
            clientmessagestream << "<collisionstart itarget=\"" << Collisions[i].iTarget << "\" ireference=\"" << Collisions[i].iCollidingObjectReference << "\"/>" << endl;
            BroadcastToLocalClients( clientmessagestream.str().c_str() );
        }
    }

    for (i = 0; i < CollidingBufSize; i++)
    {
        //DEBUG("2 CdiT : " << Colliding[i].iTarget << " vs CdiR " << Colliding[i].iCollidingObjectReference);
        istarted = find( started.begin(), started.end(), Colliding[i].iTarget);
        if ( istarted == started.end() )
        {
            f = -1;
            for (j = 0; j < CollisionBufSize; j++)
            {
                //DEBUG("2 CsjT : " << Collisions[j].iTarget << " vs CsjR " << Collisions[j].iCollidingObjectReference);
                if (Colliding[i].iTarget == Collisions[j].iTarget)
                {
                    if (Colliding[i].iCollidingObjectReference == Collisions[j].iCollidingObjectReference)
                    {
                        f = j;
                        break;
                    }
                }
            }
            //DEBUG("f2: " << f);
            if (f == -1)
            {
                ostringstream clientmessagestream;
                clientmessagestream << "<collisionend itarget=\"" << Collisions[i].iTarget << "\" ireference=\"" << Collisions[i].iCollidingObjectReference << "\"/>" << endl;
                BroadcastToLocalClients( clientmessagestream.str().c_str() );
            }
        }
    }

    for (i = 0; i < CollisionBufSize; i++)
    {
        Colliding[i].iTarget = Collisions[i].iTarget;
        Colliding[i].iCollidingObjectReference = Collisions[i].iCollidingObjectReference;
    }
    CollidingBufSize = CollisionBufSize;

}

//! Runs the metaverseserver.  Processes incoming messages, animates world, processes physics ...
void MainLoop()
{
    while(1)
    {
        SocketsBlockTillNextFrame();

        ServerConsoleConnectionManager.CheckForNewClients();
        MetaverseServerConnectionManager.CheckForNewClients();

        ProcessDBInterfaceMessages();
        CheckForServerFileAgentMessage();

        CheckForClientMessages();
        CheckForConsoleMessages();
        //DEBUG(" animator.ThisTimeIntervala " << animator.ThisTimeInterval );
        animator.AnimateWorld();
        CollisionBufSize = 0;
        //DEBUG(" animator.ThisTimeIntervalb " << animator.ThisTimeInterval );
        char sSkyboxReference[33];
        CollisionAndPhysicsEngine.HandleCollisionsAndPhysics( animator.GetThisTimeInterval(), World, Collisions, CollisionBufSize, sSkyboxReference, -1);
        //DEBUG(" call scripts ");
        SendCollisionsToScripts();

        ManageDirtyCache();    // objects that have moved and not been written to db

        ServerConsoleConnectionManager.PurgeDisconnectedConnections();
        MetaverseServerConnectionManager.PurgeDisconnectedConnections();
    }
}

//! metaverseserver entry point.  Processes commandline arguments; starts dbinterface and serverfileagent components; handles initialization
int main(int argc, char *argv[])
{

    printf( "Metaverse Server\n\n" );

    mvConfig.ReadConfig();
#ifndef _NOLOGGINGLIB

    CMessageGroup::disableAllMsgGroups();
    if( mvConfig.DebugLevel == "3" )
    {
        cout << "DEBUG MODE ON" << endl;
        CMessageGroup::enableMsgGroups( "DEBUG" );
    }
    CMessageGroup::enableMsgGroups( "INFO" );
    CMessageGroup::enableMsgGroups( "WARNING" );
    CMessageGroup::enableMsgGroups( "ERROR" );
#endif

    //if( mvConfig.CollisionAndPhysicsEngine != "" )
    //{
    // cout << "loading collision and physics engine " << mvConfig.CollisionAndPhysicsEngine << " ..." << endl;

    //LoadDll( mvConfig.CollisionAndPhysicsEngine );
    CollisionAndPhysicsEngine.Init(); // physics dll; we shoudl namespace this or something soonish
    //}
    //else
    //{
    // cout << "No physics engine used" << endl;
    //}

    bool bRunningWithDB = true;
    bool bSpawnNewWindows = false;

    for( int argnum = 0; argnum < argc; argnum++ )
    {
        if( strcmp( argv[ argnum ], "--spawnnewwindows" ) == 0 )
        {
            cout << "option --spawnnewwindows activated" << endl;
            bSpawnNewWindows = true;
        }
        else if( strcmp( argv[argnum], "--nodb" ) == 0 )
        {
            printf( "Option --nodb selected: we will not be using the database for this session\n" );
            printf( "WARNING: objects will not be persistent beyond this session\n" );
            bRunningWithDB = false;
        }
    }

    DEBUG("Initing sockets");
    mvsocket::InitSocketSystem();
    DirtyCache.clear();

    if( mvConfig.iNumAuthServers > 0 )
    {
        DEBUG("Authenticating to " << mvConfig.iNumAuthServers << " servers");
        if( !ConnectToAuthServer() )
        {
            ERRORMSG( "Failed to connect to auth server" );
            mvSystem::mvExit(1);
        }
    }

    DEBUG(  "creating serverfileagent listener..." ); // DEBUG
    SocketFileServerAgentListener.Init();
    SocketFileServerAgent.Init();
    SocketFileServerAgentListener.Listen( inet_addr( "127.0.0.1" ), iPortFileServerAgentListen );

    DEBUG(  "Waiting for serverfileagent to connect on port " << iPortFileServerAgentListen ); // DEBUG

#ifdef _WIN32

    if( bSpawnNewWindows )
    {
        mvSpawn::mvAsyncSpawn( "spawntexturesender.bat", "spawntexturesender.bat", NULL);
    }
    else
    {
        mvSpawn::mvAsyncSpawn( "msvc\\serverfileagent.exe", "msvc\\serverfileagent.exe", NULL );
    }
#endif

    SocketFileServerAgent = SocketFileServerAgentListener.AcceptNewConnection();
    cout << "serverfileagent connected" << endl;

    DEBUG(  "creating dbinterface listener..." ); // DEBUG
    SocketDBInterfaceListener.Init();
    DEBUG("Creating dbInterface socket");
    SocketDBInterface.Init();
    SocketDBInterfaceListener.Listen( inet_addr( "127.0.0.1" ), iPortDBInterfaceListen );

    DEBUG(  "Waiting for DBInterface to connect on port " << iPortDBInterfaceListen ); // DEBUG

#ifdef _WIN32

    if( bSpawnNewWindows )
    {
        if( bRunningWithDB )
        {
            mvSpawn::mvAsyncSpawn( "spawndbint.bat", "spawndbint.bat", NULL );
        }
        else
        {
            mvSpawn::mvAsyncSpawn( "spawndbint.bat", "spawndbint.bat", "--nodb", NULL );
        }
    }
    else
    {
        if( bRunningWithDB )
        {
            mvSpawn::mvAsyncSpawn( "msvc\\databasemanager.exe", "msvc\\databasemanager.exe", NULL );
        }
        else
        {
            mvSpawn::mvAsyncSpawn( "msvc\\databasemanager.exe", "msvc\\databasemanager.exe", "--nodb", NULL );
        }
    }
#endif

    SocketDBInterface = SocketDBInterfaceListener.AcceptNewConnection();
    printf( "DBInterface connected\n" );

    sprintf( SendBuffer, "<requestworldstate />\n" );
    SocketDBInterface.Send( SendBuffer );

    printf( "Creating Metaverse Client listener on port %i...\n", iPortMetaverseServer );
    MetaverseServerConnectionManager.Init( INADDR_ANY, iPortMetaverseServer );

    ServerConsoleConnectionManager.Init( inet_addr("127.0.0.1"), iPortServerConsoleListen );

    printf( "Initialization complete\n\n" );

    MainLoop();

    CollisionAndPhysicsEngine.Cleanup(); // physics dll
    //CollisionAndPhysics.UnloadDll();

    return 0;
}

