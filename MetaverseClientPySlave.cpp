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
//! \brief This module is the main metaverseclient exe, which coordinates the gui, and clientfileagent and manages the 3d renderer
//!
//! This module is the main metaverseclient exe.
//!
//! It coordinates the gui, renderer and clientfileagent and it contains the 3d opengl part.
//! It is responsible for communicating with the server, authenticating and so on
//! If/when we put WAN compression in, it will go between this module and the Internet
//!
//! - it contains a World object to store the current world state
//! - drawing is done by the objects themselves (called by the display callback in this file)
//! - animator object is used to move the objects around in the world, with smooth interpolation for rots, scales and so on
//! - selector object is used for selection/deselection of objects
//! - rendererimpl module manages OpenGL initialization and drawing
//! - playermovement module manager player movement
//! - odephysicsengine.dll is loaded to handle physics and collisions
//! - various fileinfoacaches (eg meshinfocache) store information on texture, mesh files and so on
//! - some management files handle texture, mesh, scirpt etc upload/download
//! - clientediting handles in-place editing of texture and script files
//! - clientlinking handles linking/unlinking of objects/prims
//! - objectimportexport handles import and export from/to xml
//! the renderer talks with metaverseclient via XML Sockets IPC
//! static functions in keyandmouse.* are used to manage keyboard and mouse callbacks
//!
//! This module primarily does:
//! - dispatch of messages to/from other classes
//! - management of other modules


// Client port numbers are specified here.

// 20050330 Mark Wagner - modified to use the mvsocket class as a class, not a glorified struct
//                        modified to work under Linux

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#ifdef _WIN32
#include <windows.h>
#endif
#include "GL/glut.h"

#ifdef _WIN32  // msvc, mingw
#include <io.h>
//#include "winsock2.h"
//typedef int socklen_t;
#endif

#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <set>
using namespace std;

#include <string.h>
#include <stdio.h>
#include <stdio.h>

#ifdef _WIN32
#include <process.h>
#endif

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

#include "Config.h"
#include "Diag.h"
#include "SocketsClass.h"
//#include "clientlogin.h"
#include "SpawnWrap.h"
#include "TickCount.h"
#include "SocketsClass.h"
#include "WorldStorage.h"
#include "Animation.h"
#include "Selection.h"
// #include "keyandmouse.h"
#include "Constants.h"
#include "TextureInfoCache.h"
#include "ObjectImportExport.h"
#include "RendererImpl.h"
#include "RendererTexturing.h"
#include "ClientEditing.h"
// #include "scriptinfocache.h"
#include "ScriptMgmt.h"
#include "ClientTerrainFunctions.h"
#include "TerrainInfoCache.h"
#include "ClientLinking.h"
#include "PlayerMovement.h"
#include "OdePhysicsEngine.h"
#include "Config.h"
#include "ClientMeshFileMgmt.h"
#include "MeshInfoCache.h"
#include "Graphics.h"
#include "Camera.h"
#include "Collision.h"

#include "Object.h"
#include "ObjectGrouping.h"
#include "Avatar.h"

#include "MetaverseClient.h"

namespace MetaverseClient
{
    bool bNoSpawn = false;            //!< option so that metaverseclient doesnt launch gui, renderer and login dialog itself
    bool bSpawnNewWindows = false;    //!< option so that metaverseclient launches the .py files rather than frozen .exe files

    // char sAuthServerIP[64];    //!< IP address of Authentication server, as string such as "127.0.0.1"

    char sMetaverseServerIP[64];    //!< IP address of Metaverse/sim server, as string such as "127.0.0.1"

    char sAvatarName[64] = "Guest";   //!< Name of user's avatar
    char sAvatarPassword[64] = "";    //!< password for user's avatar to authenticate
    string sConnectionKey = "";       //!< Not used yet (I think)

    // const char sRendererExeName[] = "renderermain.exe";   //!< name of renderer executable, for auto-spawning

#define BUFSIZE 2047   //!< size of socket send buffer

    char SendBuffer[ BUFSIZE + 1 ];   //!< buffer for socket sends
    char ReadBuffer[ 4097 ];   //!< buffer for socket reads

    bool bSendKeepAlives = true;   //!< send avatar position as and when it changes?  can set to false for debugging (theres a commandline option for this)

    long LastAvPosUpdate;         //!< last tickcount(milliseconds) that av position update was sent

#ifndef _NOVector3KEEPALIVE

    int iSendMovesEvery = 2000;    //!< how often av position keepalives are sent if enabled, in milliseconds; note that theyre only sent if the av moves
#else

    int iSendMovesEvery = 50000;    //!< how often av position keepalives are sent if enabled, in milliseconds; note that theyre only sent if the av moves
#endif

    float fHeight = -1.0;    //!< height of hardcoded land

    ConfigClass mvConfig;   //!< Loads configuration from config.xml, and makes it available as properties
    mvWorldStorage World;          //!< The World and container for all objects in it (except the hardcoded land)

    Animation animator( World );     //!< used to make the world move, specifically handles all object and avatar interpolation
    mvSelection Selector;          //!< handles object selection/deselection when you click it.  Usees OpenGL's selection routine
    TextureInfoCache textureinfocache;

    ClientsideEditingClass EditorMgr( World, mvConfig, textureinfocache );  //!< manages in-place editing of textures, scripts etc
    TerrainCacheClass TerrainCache;  //!< stores information about available terrains
    PlayerMovementClass PlayerMovement;  //!< manages player movement
    // CollisionAndPhysicsDllLoaderClass CollisionAndPhysics;  //!< physics dll wrapper class
    MeshInfoCacheClass MeshInfoCache;  //!< stores information about available meshes
    ClientMeshFileMgmtClass MeshFileMgmt;  //!< manages upload/download of meshes
    mvGraphicsClass mvGraphics;  //!< wraps OpengL specific functions; also contains utility graphics functions
    mvCameraClass Camera;   //!< manages camera movement, eg, orbit, pan etc

    Editing3DClass Editing3D;

    RendererTexturingClass RendererTexturing;
    ClientTerrainFunctionsClass ClientTerrainFunctions;
    ClientLinkingClass ClientLinking;
    ObjectImportExportClass ObjectImportExport;
    ScriptMgmtClass ScriptMgmt;

    CollisionAndPhysicsEngineClass CollisionAndPhysicsEngine;

    mvsocket SocketFileTransferListener;    //!< socket for clientfileagent component to connect into
    mvsocket SocketFileTransfer;   //!< socket for comms with clientfileagent component

    int iMyReference = 0;  //!< iReference of user's avatar

    COLLISION Collisions[2048];
    int CollisionBufSize;
    char SkyboxReference[33];

    bool bUsingCollisionAndPhysicsLoaded = false;  //!< did we load the physics dll?

    bool bDeleteClicked = false;
    bool bReturnClicked = false;
    bool bRightMouseButtonClicked = false;

    void Connect();
    void MainLoop();

    CallbackToPythonClass *pCallbackToPython = 0;

    void CloseListeners()
    {
        SocketFileTransferListener.Close();
    }

    void CloseAllSockets()
    {
        CloseListeners();
        SocketFileTransfer.Close();
    }


    //! waits for anything to happen on any socket.  Probably no longer used.

    // Input: None
    //
    // Returns: None
    //
    // Description:
    //
    // History: 20050330 Mark Wagner - modified to use the new class interface to mvsocket
    void WaitForSocketsAction()
    {
        DEBUG(  "wait for sockets action" ); // DEBUG
        vector<const mvsocket *> TargetSet;

        //TargetSet.push_back(&SocketMetaverseServer);
        //TargetSet.push_back(&SocketGUI);
        TargetSet.push_back(&SocketFileTransfer);
        SocketsReadBlock(-1, TargetSet);
        DEBUG(  "wait done" ); // DEBUG
    }

    //! Sends Message to server
    void SendToServer( const char *Message )
    {
        DEBUG( "Sending to server, via Python layer [" << Message << "]" );
        pCallbackToPython->SendToServer( Message );
    }

    void SendRawToFileAgent( const char *Message )
    {
        DEBUG( "Sending to fileagent [" << Message << "]" );
        if( SocketFileTransfer.Send( Message ) == SOCKETS_READ_SOCKETGONE )
        {
            printf( "Lost fileagent connection -> shutting down \n" );
            CloseListeners();
            exit(1);
        }
    }

    //! Sends xmldocument IPC to clientfileagent; adding in serverip, serverport, and connectionkey first
    void SendXMLDocToFileAgent( TiXmlDocument &IPC )
    {
        IPC.RootElement()->SetAttribute("serverip", sMetaverseServerIP );
        IPC.RootElement()->SetAttribute("serverport", iPortFileTransferServerPort );
        IPC.RootElement()->SetAttribute("connectionkey", sConnectionKey );

        ostringstream messagestream;
        messagestream << IPC << endl;
        // SendToFileAgent( messagestream.str() );
        SendRawToFileAgent( messagestream.str().c_str() );
    }

    //! Sends xmldocument IPC to clientfileagent; adding in serverip, serverport, and connectionkey first
    void SendXMLStringToFileAgent( const char *xmlstring )
    {
        TiXmlDocument IPC;
        IPC.Parse( xmlstring );

        IPC.RootElement()->SetAttribute("serverip", sMetaverseServerIP );
        IPC.RootElement()->SetAttribute("serverport", iPortFileTransferServerPort );
        IPC.RootElement()->SetAttribute("connectionkey", sConnectionKey );

        ostringstream messagestream;
        messagestream << IPC << endl;
        // SendToFileAgent( messagestream.str() );
        SendRawToFileAgent( messagestream.str().c_str() );
    }

    //! Deletes everything currently selected
    //void DeleteCurrentSelection()
    //{
    //  pCallbackToPython->DeleteCurrentSelection();
    //}

    //! Sends avatar keepalive to server; letting server know where we are
    void SendAvatarKeepAlive()
    {
        DEBUG(  "sending avatar keepalive avref=" << iMyReference ); // DEBUG

        Rot rAvatar;
        //Vector3 ZAXIS( 0.0, 0.0, 1.0 );
        //Vector3 YAXIS = { 0.0, 1.0, 0.0 };

        Rot rTwist, rPitch;
        AxisAngle2Rot( rTwist, ZAXIS, ((float)PlayerMovement.avatarzrot * mvConstants::piover180 ) );
        AxisAngle2Rot( rPitch, YAXIS, ((float)PlayerMovement.avataryrot * mvConstants::piover180) );
        RotMultiply( rAvatar, rTwist, rPitch );


        ostringstream MessageStream;
        MessageStream << "<objectmove ireference=\"" << iMyReference << "\"><geometry>"
        << "<pos x=\"" << PlayerMovement.avatarxpos << "\" y=\"" << PlayerMovement.avatarypos << "\" z=\"0.0\" />"
        //<< AvRot
        << rAvatar
        << "</geometry>"
        << "<dynamics><duration milliseconds=\"" << iSendMovesEvery << "\"/></dynamics></objectmove>\n";
        string Message = MessageStream.str();
        DEBUG(  "avatar keepalive: " << Message.c_str() ); // DEBUG
        SendToServer( Message.c_str() );
        PlayerMovement.bAvatarMoved = false;
    }

    //! Sends updates to server for any objects we are in the process of editing, things like position, rot, color etc
    void SendObjectEditMessages()
    {
        int iArrayNum;
        int iReference;
        DEBUG(  "sending object edit messages" ); // DEBUG

        int iSelectedObjectWorldArrayNum;

        for ( Selector.SelectedObjectIterator = Selector.SelectedObjects.begin( ) ; Selector.SelectedObjectIterator != Selector.SelectedObjects.end( ); Selector.SelectedObjectIterator++ )
        {
            iReference = Selector.SelectedObjectIterator->second.iReference;
            iArrayNum = World.GetArrayNumForObjectReference( iReference );
            ostringstream MessageStream;
            if( iArrayNum != -1 )
            {
                if( strcmp( World.GetObject( iArrayNum )->ObjectType, "PRIM" ) == 0 )
                {
                    MessageStream << "<objectmove ireference=\"" << iReference << "\">"
                    << "<geometry>"
                    << World.GetObject( iArrayNum )->pos
                    << World.GetObject( iArrayNum )->rot
                    << dynamic_cast<Prim*>(World.GetObject( iArrayNum ))->scale
                    << "</geometry>"
                    << "<dynamics><duration milliseconds=\"" << iSendMovesEvery << "\"/></dynamics></objectmove>\n";
                }
                else
                {
                    MessageStream << "<objectmove ireference=\"" << iReference << "\">"
                    "<geometry>"
                    << World.GetObject( iArrayNum )->pos
                    << World.GetObject( iArrayNum )->rot
                    << "</geometry>"
                    "<dynamics><duration milliseconds=\"" << iSendMovesEvery << "\"/></dynamics></objectmove>\n";
                }

                string Message = MessageStream.str();

                DEBUG(  "sending [" << Message << "]" ); // DEBUG
                SendToServer( Message.c_str()  );
            }
        }
        Selector.bSendObjectMoveForSelection = false;
        DEBUG(  "messages sent for edited objects" ); // DEBUG
    }

    //! Sends click event to server, if appropriate; for processing by the scripting engines
    // void SendClickEventToServer( int mousex, int mousey )
    // {
    //   int iReference = Selector.GetClickedPrimReference( mousex, mousey );
    //   if( iReference > 0 )
    //   {
    //        DEBUG(  "click event" ); // DEBUG/
    //          ostringstream MessageStream;
    //          MessageStream << "<event type=\"clickdown\" ireference=\"" << iReference << "\"/>" << endl;
    //          string Message = MessageStream.str();
    //          DEBUG(  "sending [" << Message << "]" ); // DEBUG
    //          SendToServer( Message.c_str()  );
    //   }
    // }

    //! Displays help file
    // void DisplayHelp()
    // {
    //   DEBUG(  "Displaying helpfile " << mvConfig.F1HelpFile << " ..." ); // DEBUG
    //   ostringstream filenamestream;
    //   filenamestream << "\"" << mvConfig.F1HelpFile << "\"";
    //#ifdef _WIN32
    //    spawnlp( _P_NOWAIT, "./launchhelp.bat", "launchhelp.bat", filenamestream.str().c_str(), NULL );
    //#else
    //  char buffer[1024];
    //  snprintf(buffer, 1024, "%s \"%s\"", "lynx", filenamestream.str().c_str());  // For now, hard-encode Lynx as the helpfile viewer
    //  system(buffer);
    //#endif
    // }

    //! Sends out object edit messages and avatar movement messages to server, if appropriate
    void SendOutAnyMessages()
    {
        if( bSendKeepAlives )
        {
            if( MVGetTickCount() - LastAvPosUpdate > iSendMovesEvery )
            {
                if( PlayerMovement.bAvatarMoved )
                {
                    SendAvatarKeepAlive();
                }

                if( Selector.bSendObjectMoveForSelection )
                {
                    SendObjectEditMessages();
                }

                // DEBUG(  "all messages sent" ); // DEBUG
                LastAvPosUpdate = MVGetTickCount();
            }
        }
    }

    //! Asks gui to activate chat; and sends it to the foreground (possible on Windows, because we started its process)
    //void ActivateChat()
    //{
    //  pCallbackToPython->ActivateChat();
    // }

    //! Asks GUI to display a context menu at screen position x and y
    // void DisplayContextMenu( const int x, const int y )
    // {
    //    pCallbackToPython->DisplayContextMenu( RendererImpl::GetWindowXPos() + x, RendererImpl::GetWindowYPos() + y );
    // }

    //! callback for ObjectImport module, to allow it to sends messages to the server
    void ObjectImportMessageCallback( string message )
    {
        DEBUG(  "sending to server: " << message ); // DEBUG
        SendToServer( message.c_str() );
    }

    //! Handles XML messages from fileagent
    void HandleFileAgentInput()
    {
        //DEBUG(  "handlefileagentinput" ); // DEBUG
        int ReadResult = 0;

        if( SocketFileTransfer.DataAvailable() )
        {
            DEBUG(  "fileagent data available" ); // DEBUG
            ReadResult = SocketFileTransfer.ReceiveLineIfAvailable( ReadBuffer );
            if( ReadResult == SOCKETS_READ_OK )
            {
                printf( "FileAgent says: [%s]\n", ReadBuffer );

                if( ReadBuffer[0] == '<' )
                {
                    Debug( "XML IPC received from SocketFileTransfer [%s]\n", ReadBuffer );

                    TiXmlDocument IPC;
                    IPC.Parse( ReadBuffer );
                    TiXmlElement *pElement = IPC.RootElement();
                    if( strcmp( pElement->Value(), "loaderfiledone" ) == 0 )
                    {
                        DEBUG(  "loaderfiledone command received from fileagent " ); // DEBUG
                        if( strcmp( pElement->Attribute("result"), "SUCCESS" ) == 0 )
                        {
                            if( pElement->Attribute("oureditreference" ) == NULL )
                            {
                                DEBUG(  "loaderfiledone SUCCESS command received from fileagent " ); // DEBUG

                                if( strcmp( pElement->Attribute("type"), "TEXTURE" ) == 0 )
                                {
                                    RendererTexturing.RegisterTextureFromXML( pElement );
                                }
                                else if( strcmp( pElement->Attribute("type"), "TERRAIN" ) == 0 )
                                {
                                    ClientTerrainFunctions.RegisterTerrainFromXML( pElement );
                                }
                                else if( strcmp( pElement->Attribute("type"), "MESHFILE" ) == 0 )
                                {
                                    MeshFileMgmt.RegisterFileFromXML( pElement );
                                }
                                else
                                {
                                    DEBUG(  "HandleFileAgentInput() WARNING: unknown file type " << pElement->Attribute("type") ); // DEBUG
                                }
                            }
                            else
                            {
                                EditorMgr.OpenDownloadedFileForEditingFromXML( pElement );
                            }
                        }
                    }
                }
                else
                {
                    Debug( "Legacy IPC received from SocketFileTransfer [%s]\n", ReadBuffer );
                }
            }
            else if( ReadResult == SOCKETS_READ_SOCKETGONE )
            {
                printf( "Lost SocketFileTransfer connection, dieing...\n" );
                CloseListeners();
                exit(1);
            }
        }
    }

    //! Manages world processing; things like: moving avatar, managing camera, animation, physics, etc...
    void ProcessWorld()
    {
        //DEBUG( "ProcessWorld()" );
        int iTimeTicks = MVGetTickCount();
        int iLastCount = MVGetTickCount();

        int iClientInputTime = MVGetTickCount() - iLastCount;
        iLastCount = MVGetTickCount();

        Object *p_Avatar = World.GetObjectByReference( iMyReference );
        if( p_Avatar != NULL )
        {
            //DEBUG("x: " << int(PlayerMovement.avatarxpos) << " y: " << int(PlayerMovement.avatarypos) << " z: " << int(PlayerMovement.avatarzpos));
            World.GetObjectByReference( iMyReference )->pos.x = PlayerMovement.avatarxpos;
            World.GetObjectByReference( iMyReference )->pos.y = PlayerMovement.avatarypos;
            World.GetObjectByReference( iMyReference )->pos.z = PlayerMovement.avatarzpos;
            p_Avatar->bPhysicsEnabled = true;
            if( PlayerMovement.bJumping )
            {
                PlayerMovement.bJumping = false;

                if( p_Avatar->pos.z <= 0.1 )
                {
                    DEBUG(  "Jump!" ); // DEBUG
                    p_Avatar->vVelocity.z = 10.0;
                }
            }
        }

        animator.AnimateWorld();
        int iAnimateTime = MVGetTickCount() - iLastCount;
        iLastCount = MVGetTickCount();

        if( bUsingCollisionAndPhysicsLoaded )
        {
            for( int i = 0; i < animator.GetNumMovingObjects(); i++ )
            {
                Object *p_Object = World.GetObjectByReference( animator.GetMovingObjectiReference( i ) );
                if( p_Object != NULL )
                {
                    CollisionAndPhysicsEngine.ObjectModify( p_Object );
                }
            }
            for( map< int, SELECTION >::iterator iterator = Selector.SelectedObjects.begin(); iterator != Selector.SelectedObjects.end(); iterator++ )
            {
                Object *p_Object = World.GetObjectByReference( iterator->second.iReference );
                if( p_Object != NULL )
                {
                    CollisionAndPhysicsEngine.ObjectModify( p_Object );
                }
            }
            CollisionBufSize = -1;
            CollisionAndPhysicsEngine.HandleCollisionsAndPhysics( animator.GetThisTimeInterval(), World, Collisions, CollisionBufSize, SkyboxReference, iMyReference );
        }
        int iPhysicsTime = MVGetTickCount() - iLastCount;
        iLastCount = MVGetTickCount();

        if( p_Avatar != NULL )
        {
            PlayerMovement.avatarxpos = World.GetObjectByReference( iMyReference )->pos.x;
            PlayerMovement.avatarypos = World.GetObjectByReference( iMyReference )->pos.y;
            PlayerMovement.avatarzpos = World.GetObjectByReference( iMyReference )->pos.z;
        }

        PlayerMovement.MovePlayer();

        int iPlayerMovementTime = MVGetTickCount() - iLastCount;
        iLastCount = MVGetTickCount();

        SendOutAnyMessages();
        EditorMgr.UploadChangedFiles();
        int iEditmgrTime = MVGetTickCount() - iLastCount;
        iLastCount = MVGetTickCount();
        TIMING( "Timings for mainloop: " << iClientInputTime << " " << iAnimateTime << " " << iPhysicsTime << " " << iPlayerMovementTime );
    }

    //! Main loop, called by GLUT once a frame
    void MainLoop( void )
    {
        pCallbackToPython->DoEvents();

        HandleFileAgentInput();
        ProcessWorld();
    }

} // namespace MetaverseClient

using namespace MetaverseClient;

//! Initializes GLUT renderer; creates window, registers keyboard and mouse callbacks
void InitializeRenderer( int argc, char *argv[] )
{
    RendererImpl::RendererInit(argc, argv);
    //  mvKeyboardAndMouse::Init();
}

//! Gets world state from server
void InitializeWorld()
{
    bSendKeepAlives = true;
    PlayerMovement.avatarxpos = -5;
    PlayerMovement.avatarypos = 0;
    PlayerMovement.avatarzpos = 0;

    PlayerMovement.avatarzrot = 0;
    PlayerMovement.avataryrot = 0;

    // LastTickCount = GetTickCount ();
    LastAvPosUpdate = 0;

    //char RequestWorldState[50] = "<requestworldstate />\n";
    // SendToServer( "<requestworldstate />\n" );

    //mvKeyboardAndMouse::RegisterCallbacks();
    //RendererRegisterVisibilityCallback( visibilitycallback );
}

void InitMetaverseClient( CallbackToPythonClass &callbackobject )
{
    pCallbackToPython = &callbackobject;

    Object::SetmvGraphics( mvGraphics );

    Object::Settextureinfocache( textureinfocache );
    textureinfocache.Clear();
    Object::SetTerrainCache( TerrainCache );

    MeshFileMgmt.SetFileInfoCache( MeshInfoCache );
    Object::SetMeshInfoCache( MeshInfoCache );

    //   cout << "test" << endl;

    mvConfig.ReadConfig();

#ifndef _NOLOGGINGLIB

    CMessageGroup::disableAllMsgGroups();
    if( mvConfig.DebugLevel == "3" )
    {
        CMessageGroup::enableMsgGroups( "DEBUG" );
    }
    CMessageGroup::enableMsgGroups( "INFO" );
    CMessageGroup::enableMsgGroups( "WARNING" );
    CMessageGroup::enableMsgGroups( "ERROR" );
#endif

    mvsocket::InitSocketSystem();

    //if( mvConfig.CollisionAndPhysicsEngine != "" )
    //{
    //  DEBUG(  "loading collision and physics engine " << mvConfig.CollisionAndPhysicsEngine << " ..." ); // DEBUG

    //CollisionAndPhysics.LoadDll( mvConfig.CollisionAndPhysicsEngine );
    CollisionAndPhysicsEngine.Init();
    bUsingCollisionAndPhysicsLoaded = true;
    // }

}

void MetaverseclientStartRenderer()
{
    InitializeWorld();

    DEBUG( "initializing renderer..." );
    char fakestring[1] = "";
    char *fakestringarray[1];

    fakestringarray[0] = fakestring;

    //char **argv = 0;
    InitializeRenderer( 0, fakestringarray );

    DEBUG( "initialized renderer, registering mainloop..." );

    RendererImpl::RendererRegisterMainLoop( MetaverseClient::MainLoop );

    DEBUG( "registered mainloop, starting..." );
    RendererImpl::RendererStartMainLoop();
    DEBUG( "done" );
}

void MetaverseclientShutdown()
{
    mvsocket::EndSocketSystem();
}

void SetCallbackToPython( CallbackToPythonClass &callbackobject )
{
    pCallbackToPython = &callbackobject;
}

void SetAvatariReference( int AvatariReference )
{
    DEBUG( "Got iMyReference: " << AvatariReference );
    iMyReference = AvatariReference;
}


// Input: None
//
// Returns: None
//

//! Description: Launches the file-transfer subsystems

//
// History: 20050330 Mark Wagner - Modified to work with Linux
void SpawnFileAgent()
{
    char buffer[1024];
    printf( "Connecting to server at IP %s port %i\n", sMetaverseServerIP, iPortMetaverseServer );

    SocketFileTransferListener.Init();
    SocketFileTransferListener.Listen( inet_addr( "127.0.0.1" ), iPortFileTransfer );
    if( !bNoSpawn )
    {
#ifdef _WIN32
        Debug( "Launching file transfer agent...\n" );
        if( bSpawnNewWindows )
        {
            SPAWNLP( _P_NOWAIT, "spawnclientfileagent.bat", "spawnclientfileagent.bat", NULL );
        }
        else
        {
            SPAWNLP( _P_NOWAIT, "clientfileagent.exe", "clientfileagent.exe", NULL );
        }
        if( bSpawnNewWindows )
        {}
        else
        {
            SPAWNLP( _P_NOWAIT, "msvc\\clientfileagent.exe", "msvc\\clientfileagent.exe", NULL );
        }
#else
        system("./clientfileagent &");
#endif

    }
    printf( "Waiting for file transfer agent to connect...\n" );
    SocketFileTransfer = SocketFileTransferListener.AcceptNewConnection();
    printf( "File transfer agent connected.\n" );

    printf( "Initialization complete\n" );
}

void SetSimIPAddress( const char *SimIPAddress, const int SimPort )
{
    strcpy( sMetaverseServerIP, SimIPAddress );
    iPortMetaverseServer = SimPort;
}

mvWorldStorage &GetWorld()
{
    return World;
}
mvSelection &GetSelector()
{
    return Selector;
}
Animation &GetAnimator()
{
    return animator;
}

TextureInfoCache &GetTextureInfoCache()
{
    return textureinfocache;
}
TerrainCacheClass &GetTerrainCache()
{
    return TerrainCache;
}
MeshInfoCacheClass &GetMeshInfoCache()
{
    return MeshInfoCache;
}
ClientsideEditingClass &GetEditorMgr()
{
    return EditorMgr;
}
ClientLinkingClass &GetClientLinking()
{
    return ClientLinking;
}
ObjectImportExportClass &GetObjectImportExport()
{
    return ObjectImportExport;
}

RendererTexturingClass &GetRendererTexturing()
{
    return RendererTexturing;
}
ClientTerrainFunctionsClass &GetClientTerrainFunctions()
{
    return ClientTerrainFunctions;
}
ClientMeshFileMgmtClass &GetMeshFileMgmt()
{
    return MeshFileMgmt;
}
ScriptMgmtClass &GetScriptMgmt()
{
    return ScriptMgmt;
}

mvCameraClass &GetCamera()
{
    return Camera;
}

ConfigClass &GetmvConfig()
{
    return mvConfig;
}

CollisionAndPhysicsEngineClass &GetCollisionAndPhysicsEngine()
{
    return CollisionAndPhysicsEngine;
}
mvGraphicsClass &GetmvGraphics()
{
    return mvGraphics;
}

PlayerMovementClass &GetPlayerMovement()
{
    return PlayerMovement;
}
Editing3DClass &GetEditing3D()
{
    return Editing3D;
}
