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

#ifdef _WIN32
//#include <process.h>
#include <io.h>
#endif

#include <stdio.h>
#include <string>
//#include <stdarg.h>
#include <set>
#include <iostream>
#include <sstream>
//#include <math.h>
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

#include "SpawnWrap.h"
#include "TickCount.h"
#include "SocketsClass.h"
#include "SocketsConnectionManager.h"
#include "Diag.h"
#include "Config.h"
#include "System.h"

#define BUFSIZE 2047

//! \file
//! \brief AuthServer is the OSMP Authentiation server
//!
//! AuthServer is the OSMP Authentiation server
//! Sim servers authenticate to the authserver on startup
//!
//! Clients connect to the authserver on startup, and receive a list of currently
//! active sims, so the authserver(s) can centralize active world lists
//!
//! The sim uses the authserver to authenticate the client when the client connects,
//! so the authserver provides for transparent hyperlinking between worlds by
//! the client
//!
//! authserver contains a main, so it runs from the commandline
//! See documentation at http://manageddreams.com/osmpwiki for more details on usage

char ReadBuffer[4098];     //!< Used for socket reads
char SendBuffer[BUFSIZE + 1];     //!< Used for socket writes

mvsocket SocketAuthDBInterfaceListener;  //!< Socket to listen for authdbinterface component to initially connect to
mvsocket SocketAuthDBInterface;  //!< Socket for comms with authdbinterface

SocketsConnectionManagerClass SimConnectionManager; //!< Handles sockets connections of connected sims
SocketsConnectionManagerClass ClientConnectionManager;  //!< Handles sockets connections of connected clients

ConfigClass mvConfig;  //!< Provides config information from config.xml file

//! Closes connections with the authdbinterface component
void CloseSockets()
{
    SocketAuthDBInterface.Close();
    SocketAuthDBInterfaceListener.Close();
}

//! Sleeps until something happens on a socket, ie until client or sim connects
//! or authdbinterface component sends some data
void SocketsBlockTillNextFrame()
{
    vector<const mvsocket *> sockets;
    DEBUG( "SocketsBlockTillNextFrame" );

    SimConnectionManager.GetSocketList(sockets);
    ClientConnectionManager.GetSocketList(sockets);
    sockets.push_back(&SocketAuthDBInterface);
    sockets.push_back(&SocketAuthDBInterfaceListener);

    // select( iMaxFileDescriptor, &TargetSet, NULL, NULL, NULL );
    SocketsReadBlock(-1, sockets);
    DEBUG("end of SocketsBlockTillNextFrame" );
}

//! Checks whether the client identified by UserName and IPAddress
//! has been authenticated
//! Used to let a sim server know whether this user has been authenticated or not
//!
//! IPAddress is the ip address in format 127.0.0.1
bool ValidateUser( string UserName, string IPAddress )
{
    for ( ConnectionsIteratorTypedef iterator = ClientConnectionManager.Connections.begin( ) ;
            iterator != SimConnectionManager.Connections.end( );
            iterator++ )
    {
        if( iterator->second.name == UserName )
        {
            if( strcmp( inet_ntoa( iterator->second.connectionsocket.GetPeer() ), IPAddress.c_str() ) == 0 )
            {
                return true;
            }
        }
    }

    return false;
}

//! Handles xml messages received from a connected OSMP client
void HandleClientInput( int iConnectionRef, CONNECTION &rConnection, char *Message )
{
    if( Message[0] == '<' )  // XML IPC
    {
        DEBUG("Client message " << Message);
        TiXmlDocument IPC;
        IPC.Parse( Message );
        if( !rConnection.bAuthenticated )
        {
            if( strcmp( IPC.RootElement()->Value(), "login" ) == 0 )
            {
                IPC.RootElement()->SetAttribute("iconnectionref", iConnectionRef );
                IPC.RootElement()->SetAttribute("type", "user" );
                ostringstream messagestream;
                messagestream << IPC << endl;
                DEBUG( "Sending to db " << messagestream.str() );
                SocketAuthDBInterface.Send( messagestream.str().c_str() );
            }
        }
    }
    else  // legacy IPC
    {
        DEBUG( "received legacy IPC from sim " << iConnectionRef << " " << Message );
    }
}

//! Handles xml messages received from a connected OSMP sim server
void HandleSimInput( int iConnectionRef, CONNECTION &rConnection, char *Message )
{
    if( Message[0] == '<' )  // XML IPC
    {
        TiXmlDocument IPC;
        IPC.Parse( Message );
        if( !rConnection.bAuthenticated )
        {
            if( strcmp( IPC.RootElement()->Value(), "login" ) == 0 )
            {
                IPC.RootElement()->SetAttribute("iconnectionref", iConnectionRef );
                IPC.RootElement()->SetAttribute("type", "sim" );
                ostringstream messagestream;
                messagestream << IPC << endl;
                DEBUG( "Sending to db " << messagestream.str() );
                SocketAuthDBInterface.Send( messagestream.str().c_str() );
            }
        }
        else
        {
            if( strcmp( IPC.RootElement()->Value(), "requestclientvalidate" ) == 0 )
            {
                if( ValidateUser( IPC.RootElement()->Attribute("login"),
                                  IPC.RootElement()->Attribute("ipaddress") ) )
                {
                    ostringstream messagestream;
                    IPC.RootElement()->SetValue("clientvalidateresponse" );
                    IPC.RootElement()->SetAttribute("result", "ACCEPT" );
                    messagestream << IPC << endl;
                    DEBUG( "sending to sim " << messagestream.str() );
                    SimConnectionManager.SendThruConnection( rConnection, messagestream.str().c_str() );
                }
                else
                {
                    ostringstream messagestream;
                    IPC.RootElement()->SetValue("clientvalidateresponse" );
                    IPC.RootElement()->SetAttribute("result", "REJECT" );
                    messagestream << IPC << endl;
                    DEBUG( "sending to sim " << messagestream.str() );
                    SimConnectionManager.SendThruConnection( rConnection, messagestream.str().c_str() );
                }
            }
        }
    }
    else  // legacy IPC
    {
        DEBUG( "received legacy IPC from sim " << iConnectionRef << " " << Message );
    }
}

//! Scans all connected sim sockets, via SimConnectionManager, checking
//! for new messages received
void CheckForSimMessages()
{
    int bytesRecv = 0;
    for ( SimConnectionManager.ConnectionsIterator = SimConnectionManager.Connections.begin( ) ;
            SimConnectionManager.ConnectionsIterator != SimConnectionManager.Connections.end( );
            SimConnectionManager.ConnectionsIterator++ )
    {
        sprintf( ReadBuffer, "" );
        int ReadResult = SimConnectionManager.ReceiveIteratorLineIfAvailable( ReadBuffer );
        if( ReadResult == SOCKETS_READ_OK )
        {
            DEBUG( "sim Read buffer " << ReadBuffer );
            HandleSimInput( SimConnectionManager.ConnectionsIterator->first,
                            SimConnectionManager.ConnectionsIterator->second,
                            ReadBuffer );
        }

    }
}

//! Scans all connected client sockets, via ClientConnectionManager, checking
//! for new messages received
void CheckForClientMessages()
{
    int bytesRecv = 0;
    for ( ClientConnectionManager.ConnectionsIterator = ClientConnectionManager.Connections.begin( ) ;
            ClientConnectionManager.ConnectionsIterator != ClientConnectionManager.Connections.end( );
            ClientConnectionManager.ConnectionsIterator++ )
    {
        strcpy( ReadBuffer, "" );
        int ReadResult = ClientConnectionManager.ReceiveIteratorLineIfAvailable( ReadBuffer );
        DEBUG("Client check " << ReadResult);
        if( ReadResult == SOCKETS_READ_OK )
        {
            DEBUG( "client Read buffer " << ReadBuffer );
            HandleClientInput( ClientConnectionManager.ConnectionsIterator->first,
                               ClientConnectionManager.ConnectionsIterator->second,
                               ReadBuffer );
        }

    }
}

//! Checks for new messages from authdbinterface component
void CheckDBMessages()
{
    int bytesRecv = 0;

    sprintf( ReadBuffer, "" );
    int ReadResult = SocketAuthDBInterface.ReceiveLineIfAvailable( ReadBuffer );

    if( ReadResult == SOCKETS_READ_OK )
    {
        if( ReadBuffer[0] == '<' )
        {
            DEBUG( "received xml from Authdbinterface, socket " << SocketAuthDBInterface.GetSocket() << " " << ReadBuffer );
            TiXmlDocument IPC;
            IPC.Parse( ReadBuffer );
            if( strcmp( IPC.RootElement()->Value(), "loginaccept" ) == 0 )
            {
                if( strcmp( IPC.RootElement()->Attribute("type"), "user" ) == 0 )
                {
                    ConnectionsIteratorTypedef iterator;
                    iterator = ClientConnectionManager.Connections.find( atoi( IPC.RootElement()->Attribute("iconnectionref") ) );
                    if( iterator != ClientConnectionManager.Connections.end() )
                    {
                        CONNECTION &rConnection = iterator->second;

                        if( ClientConnectionManager.NumClientConnectionsWithName( IPC.RootElement()->Attribute("name") ) == 0 )
                        {
                            rConnection.name = IPC.RootElement()->Attribute("name");
                            rConnection.bAuthenticated = true;

                            ostringstream messagestream;
                            messagestream << "<loginaccept type=\"user\" name=\"" <<
                            IPC.RootElement()->Attribute("name") << "\" >"
                            << "<sims>";

                            for ( ConnectionsIteratorTypedef iterator = SimConnectionManager.Connections.begin( ) ;
                                    iterator != SimConnectionManager.Connections.end( );
                                    iterator++ )
                            {
                                if( strcmp( inet_ntoa( iterator->second.connectionsocket.GetPeer() ), "0.0.0.0" ) != 0 )
                                {
                                    messagestream << "<sim serverip=\"" << inet_ntoa( iterator->second.connectionsocket.GetPeer() )
                                    << "\" serverport=\"22165\" "
                                    << "servername=\"" << iterator->second.name << "\"/>";
                                }
                                else
                                {
                                    messagestream << "<sim serverip=\"127.0.0.1\""
                                    << " serverport=\"22165\" "
                                    << "servername=\"" << iterator->second.name << "\"/>";
                                }
                            }

                            messagestream << "</sims>";
                            messagestream << "</loginaccept>" << endl;

                            DEBUG(  "sending to client " << messagestream.str() ); // DEBUG
                            ClientConnectionManager.SendThruConnection( rConnection, messagestream.str().c_str() );
                        }
                        else
                        {
                            sprintf( SendBuffer, "<loginreject type=\"user\" name=\"%s\" reason=\"alreadyconnected\"/>\n", IPC.RootElement()->Attribute("name") );
                            printf( "sending to client [%s]\n", SendBuffer );
                            ClientConnectionManager.SendThruConnection( rConnection, SendBuffer );

                            DEBUG(  "Kicking client " << IPC.RootElement()->Attribute("name") << " : avatar already connected" ); // DEBUG
                            ClientConnectionManager.CloseConnectionNow( rConnection );
                        }
                    }
                }
                else if( strcmp( IPC.RootElement()->Attribute("type"), "sim" ) == 0 )
                {
                    ConnectionsIteratorTypedef iterator;
                    iterator = SimConnectionManager.Connections.find( atoi( IPC.RootElement()->Attribute("iconnectionref") ) );
                    if( iterator != SimConnectionManager.Connections.end() )
                    {
                        CONNECTION &rConnection = iterator->second;

                        if( SimConnectionManager.NumClientConnectionsWithName( IPC.RootElement()->Attribute("name") ) == 0 )
                        {
                            rConnection.name = IPC.RootElement()->Attribute("name");
                            rConnection.bAuthenticated = true;
                            sprintf( SendBuffer, "<loginaccept type=\"sim\" name=\"%s\"/>\n", IPC.RootElement()->Attribute("name") );
                            DEBUG(  "sending to sim " << SendBuffer ); // DEBUG
                            SimConnectionManager.SendThruConnection( rConnection, SendBuffer );
                        }
                        else
                        {
                            sprintf( SendBuffer, "<loginreject type=\"sim\" name=\"%s\" reason=\"alreadyconnected\"/>\n", IPC.RootElement()->Attribute("name") );
                            printf( "sending to sim [%s]\n", SendBuffer );
                            SimConnectionManager.SendThruConnection( rConnection, SendBuffer );

                            DEBUG(  "Kicking sim " << IPC.RootElement()->Attribute("name") << " : avatar already connected" ); // DEBUG
                            SimConnectionManager.CloseConnectionNow( rConnection );
                        }
                    }
                }
            }
            else if( strcmp( IPC.RootElement()->Value(), "loginreject" ) == 0 )
            {
                if( strcmp( IPC.RootElement()->Attribute("type"), "user" ) == 0 )
                {
                    ConnectionsIteratorTypedef iterator;
                    iterator = ClientConnectionManager.Connections.find( atoi( IPC.RootElement()->Attribute("iconnectionref") ) );
                    if( iterator != ClientConnectionManager.Connections.end() )
                    {
                        CONNECTION &rConnection = iterator->second;

                        sprintf( SendBuffer, "<loginreject type=\"user\" name=\"%s\" reason=\"failedauth\"/>\n", IPC.RootElement()->Attribute("name") );
                        DEBUG(  "sending to client " << SendBuffer ); // DEBUG
                        ClientConnectionManager.SendThruConnection( rConnection, SendBuffer );

                        DEBUG(  "Kicking client " << IPC.RootElement()->Attribute("name") << " for failed auth" ); // DEBUG
                        ClientConnectionManager.CloseConnectionNow( rConnection );
                    }
                }
                if( strcmp( IPC.RootElement()->Attribute("type"), "sim" ) == 0 )
                {
                    ConnectionsIteratorTypedef iterator;
                    iterator = SimConnectionManager.Connections.find( atoi( IPC.RootElement()->Attribute("iconnectionref") ) );
                    if( iterator != SimConnectionManager.Connections.end() )
                    {
                        CONNECTION &rConnection = iterator->second;

                        sprintf( SendBuffer, "<loginreject type=\"sim\" name=\"%s\" reason=\"failedauth\"/>\n", IPC.RootElement()->Attribute("name") );
                        DEBUG(  "sending to sim " << SendBuffer ); // DEBUG
                        SimConnectionManager.SendThruConnection( rConnection, SendBuffer );

                        DEBUG(  "Kicking sim " << IPC.RootElement()->Attribute("name") << " for failed auth" ); // DEBUG
                        SimConnectionManager.CloseConnectionNow( rConnection );
                    }
                }
            }
        }
        else
        {
            DEBUG( "received legacy IPC from authdbinterface " << SendBuffer );
        }
    }
    else if( ReadResult == SOCKETS_READ_SOCKETGONE )
    {
        CloseSockets();
        INFO( "Lost DBInterface connection, dieing..." );
        mvSystem::mvExit( 1 );
        // printf( "Lost DBInterface connection, waiting for new connection...\n" );
        // SocketDBInterface.CopyFrom( SocketDBInterfaceListener.AcceptNewConnection() );
        // printf( "DBInterface connection restored\n" );
    }
}

//! MainLoop. Checks for new messages on sockets, and handles them
//! appropriately
//! Sleeps till more messages arrive
void MainLoop()
{
    while(1)
    {
        DEBUG("Blocking...");
        SocketsBlockTillNextFrame();
        DEBUG("Something happened");

        SimConnectionManager.CheckForNewClients();
        ClientConnectionManager.CheckForNewClients();

        CheckDBMessages();
        CheckForSimMessages();
        CheckForClientMessages();

        SimConnectionManager.PurgeDisconnectedConnections();
        ClientConnectionManager.PurgeDisconnectedConnections();
    }
}

//! Main.
//! Handles commandline arguments, ie:
//! --spawnnewwindows     spawns other processes (authdbinterface) in a new window
//!                       instead of in same cmd window.  Useful for debugging
//! --nospawn             doesnt spawn other processes (ie authdbinterface)
//!                       Useful for debugging (eg to run authdbinterface manually)
//! --nodb                Runs without backend database
//!
//! - Reads the config
//! - Initializes logging
//! - Spawns authdbinterface component, and waits for it to connect
int main(int argc, char *argv[])
{

    INFO("");
    INFO( "Metaverse Authentication Server" );
    INFO( "===============================" );
    INFO("");

    bool bRunningWithDB = true;
    bool bSpawnNewWindows = false;
    bool bNoSpawn = false;

    for( int argnum = 0; argnum < argc; argnum++ )
    {
        if( strcmp( argv[ argnum ], "--spawnnewwindows" ) == 0 )
        {
            cout << "option --spawnnewwindows activated" << endl;
            bSpawnNewWindows = true;
        }
        else if( strcmp( argv[ argnum ], "--nospawn" ) == 0 )
        {
            cout << "option --nospawn activated" << endl;
            bNoSpawn = true;
        }
        else if( strcmp( argv[argnum], "--nodb" ) == 0 )
        {
            printf( "Option --nodb selected: we will not be using the database for this session\n" );
            printf( "WARNING: objects will not be persistent beyond this session\n" );
            bRunningWithDB = false;
        }
    }

    mvConfig.ReadConfig();

#ifndef _NOLOGGINGLIB

    CMessageGroup::disableAllMsgGroups();

    CMessageGroup::enableMsgGroups( "INFO" );
    CMessageGroup::enableMsgGroups( "WARNING" );
    CMessageGroup::enableMsgGroups( "ERROR" );

    if( mvConfig.DebugLevel == "3" )
    {
        CMessageGroup::enableMsgGroups( "DEBUG" );
    }
#endif

    mvsocket::InitSocketSystem();

    DEBUG(  "creating authdbinterface listener..." ); // DEBUG
    SocketAuthDBInterfaceListener.Init();
    SocketAuthDBInterface.Init();
    SocketAuthDBInterfaceListener.Listen( inet_addr( "127.0.0.1" ), iAuthServerPortForDBInterface );

    DEBUG(  "Waiting for AuthDBInterface to connect on port " << iAuthServerPortForDBInterface ); // DEBUG

    if( !bNoSpawn )
    {
        if( bSpawnNewWindows )
        {
            if( bRunningWithDB )
            {
                mvSpawn::mvAsyncSpawn( "spawnauthdbint.bat", "spawnauthdbint.bat", NULL );
            }
            else
            {
                mvSpawn::mvAsyncSpawn( "spawnauthdbint.bat", "spawnauthdbint.bat", "--nodb", NULL );
            }
        }
        else
        {
            if( bRunningWithDB )
            {
                mvSpawn::mvAsyncSpawn( "msvc\\authserverdatabasemanager.exe", "msvc\\authserverdatabasemanager.exe", NULL );
            }
            else
            {
                mvSpawn::mvAsyncSpawn( "msvc\\authserverdatabasemanager.exe", "msvc\\authserverdatabasemanager.exe", "--nodb", NULL );
            }
        }
    }

    SocketAuthDBInterface = SocketAuthDBInterfaceListener.AcceptNewConnection();
    printf( "DBInterface connected\n" );


    INFO( "Creating Sim listener on port " << iPortAuthServerForSims );
    SimConnectionManager.Init( INADDR_ANY, iPortAuthServerForSims );

    INFO( "Creating User listener on port " << iPortAuthServerForClients );
    ClientConnectionManager.Init( INADDR_ANY, iPortAuthServerForClients );

    INFO("");
    INFO( "Initialization complete" );
    INFO("");

    MainLoop();

    return 0;
}
