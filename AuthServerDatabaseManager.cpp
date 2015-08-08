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
//! \brief AuthServerDBInterface handles read/write to the database for the AuthServer component
//!
//! AuthServerDBInterface handles read/write to the database for the AuthServer component
//!
//! The AuthServer database contains information on user and sim accounts
//!
//! AuthServerDBInterface contains a main, and runs as a standalone component
//! See the documentation at http://hughperkins.com/osmpwiki for usage instructions

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>
#include <sstream>
using namespace std;

#include "tinyxml.h"

#include "MySQLDBInterface.h"
#include "Diag.h"
#include "Config.h"
#include "System.h"

#include "SocketsClass.h"

#define BUFSIZE 2047
char SendBuffer[BUFSIZE + 1];  //!< Buffer for socket sends
char ReadBuffer[4097];  //!< Buffer for socket sends

char SQLCommands[10][512];  //!< Buffer for SQL Commands

bool bRunningWithDB = true;  //!< if true, use DB, otherwise just pretend there is a db

ConfigClass mvConfig;  //!< Reads config from config.xml

mvsocket SocketAuthServer;  //!< Socket for connecting to AuthServer component
int iAuthServerPort = 25001;  //!< Port for connection to AuthServer

MySQLDBInterface DBAbstractionLayer;  //!< Class to handle MySQL-specific operations

//! Accepts authentication information for a client
//! and checks whether it is valid or not
//! Sends the result as XML through AuthServer component socket
void AuthenticateClient( int iConnectionRef, const char *AvatarName, const char *AvatarPassword )
{
    int iAvatarReference;
    int iPrimReference;

    DEBUG( "Authenticating " << AvatarName << " on connectionref " << iConnectionRef );

    if( bRunningWithDB )
    {
        DEBUG( "running query..." );
        DBAbstractionLayer.RunOneRowQuery( "select password from clientaccounts where account_name = '%s'", AvatarName  );
        if( DBAbstractionLayer.RowAvailable() )
        {
            DEBUG( "processing row..." );
            if( strcmp( AvatarPassword, DBAbstractionLayer.GetFieldValueByName( "password" ) ) == 0 )
            {
                DEBUG( "avatar " << AvatarName << " authenticated" );
                DEBUG( "authenticated, sending reply to server..." );
                ostringstream messagestream;
                messagestream << "<loginaccept type=\"user\" name=\"" << AvatarName << "\" iconnectionref=\"" << iConnectionRef << "\"/>" << endl;
                DEBUG( "Sending to server " << messagestream.str() );
                SocketAuthServer.Send( messagestream.str().c_str() );
            }
            else
            {
                Debug( "avatar %s failed authentication!\n", AvatarName );
                sprintf( SendBuffer, "<loginreject type=\"user\" name=\"%s\" iconnectionref=\"%i\"/>\n", AvatarName, iConnectionRef );
                DEBUG( "Sending to server " << SendBuffer );
                SocketAuthServer.Send( SendBuffer );
            }
        }
        else
        {
            DEBUG( "new user [" << AvatarName << "].  Registering..." );
            DBAbstractionLayer.ExecuteSQL( "insert into clientaccounts ( account_name, password ) values ( '%s', '%s' )", AvatarName, AvatarPassword );

            ostringstream messagestream;
            messagestream << "<loginaccept type=\"user\" name=\"" << AvatarName << "\" iconnectionref=\"" << iConnectionRef << "\"/>" << endl;
            DEBUG( "Sending to server " << messagestream.str() );
            SocketAuthServer.Send( messagestream.str().c_str() );
        }
    }
    else
    {
        DEBUG( "new user [" << AvatarName << "].  Registering..." );

        ostringstream messagestream;
        messagestream << "<loginaccept type=\"user\" name=\"" << AvatarName << "\" iconnectionref=\"" << iConnectionRef << "\"/>" << endl;
        DEBUG( "Sending to server " << messagestream.str() );
        SocketAuthServer.Send( messagestream.str().c_str() );
    }
}

//! Accepts authentication information for a sim server
//! and checks whether it is valid or not
//! Sends the result as XML through AuthServer component socket
void AuthenticateSim( int iConnectionRef, const char *SimName, const char *SimPassword )
{
    int iAvatarReference;
    int iPrimReference;

    DEBUG( "Authenticating " << SimName << " on connectionref " << iConnectionRef );

    if( bRunningWithDB )
    {
        DEBUG( "Running query..." );
        DBAbstractionLayer.RunOneRowQuery( "select password from simaccounts where account_name = '%s'", SimName  );
        if( DBAbstractionLayer.RowAvailable() )
        {
            DEBUG( "processing row b..." );
            const char *testvalue = DBAbstractionLayer.GetFieldValueByName("password" );
            DEBUG( testvalue );
            if( strcmp( SimPassword, DBAbstractionLayer.GetFieldValueByName( "password" ) ) == 0 )
            {
                DEBUG( "sim " << SimName << " authenticated" );
                ostringstream messagestream;
                messagestream << "<loginaccept type=\"sim\" name=\"" << SimName << "\" iconnectionref=\"" << iConnectionRef << "\"/>" << endl;
                DEBUG( "Sending to server " << messagestream.str() );
                SocketAuthServer.Send( messagestream.str().c_str() );
            }
            else
            {
                Debug( "sim %s failed authentication!\n", SimName );
                sprintf( SendBuffer, "<loginreject type=\"sim\" name=\"%s\" iconnectionref=\"%i\"/>\n", SimName, iConnectionRef );
                DEBUG( "Sending to server " << SendBuffer );
                SocketAuthServer.Send( SendBuffer );
            }
        }
        else
        {
            DEBUG( "new sim [" << SimName << "].  Registering..." );
            DBAbstractionLayer.ExecuteSQL( "insert into simaccounts ( account_name, password ) values ( '%s', '%s' )", SimName, SimPassword );

            ostringstream messagestream;
            messagestream << "<loginaccept type=\"sim\" name=\"" << SimName << "\" iconnectionref=\"" << iConnectionRef << "\"/>" << endl;
            DEBUG( "Sending to server " << messagestream.str() );
            SocketAuthServer.Send( messagestream.str().c_str() );
        }
    }
    else
    {
        DEBUG( "new sim [" << SimName << "].  Registering..." );

        ostringstream messagestream;
        messagestream << "<loginaccept type=\"sim\" name=\"" << SimName << "\" iconnectionref=\"" << iConnectionRef << "\"/>" << endl;
        DEBUG( "Sending to server " << messagestream.str() );
        SocketAuthServer.Send( messagestream.str().c_str() );
    }
}

//! Process commandline arguments, ie:
//!  -- nodb    Runs without a database; no postsession persistence
//!
//! - Connects to AuthServer component
//! - Handles messages from AuthServer as they arrive (loop forever)
int main( int argc, char *argv[] )
{
    bRunningWithDB = true;

    mvConfig.ReadConfig();

#ifndef _NOLOGGINGLIB

    CMessageGroup::disableAllMsgGroups();
    if( mvConfig.DebugLevel == "3" )
    {
        CMessageGroup::enableMsgGroups( "DEBUG" );
    }
#endif

    float fLoadFloat = 1.0 * 5.0;

    INFO( "AuthServerDBInterface module\n" );
    INFO( "==================\n" );
    INFO( "" );
    //INFO( "    Options available:\n" );
    //INFO( "       --nodb    runs without db; no post-session persistance\n" );
    //INFO( "" );
    //INFO( "" );

    mvsocket::InitSocketSystem();
    DEBUG( "Connecting to AuthServer on port " << iAuthServerPort );
    SocketAuthServer.ConnectToServer( inet_addr( "127.0.0.1" ), iAuthServerPort );
    DEBUG( "Connected to AuthServer." );

    if( argc>1 )
    {
        if( strcmp( argv[1], "--nodb" ) == 0 )
        {
            printf( "Option --nodb selected: we will not be using the database for this session\n" );
            printf( "WARNING: passwords will not be persistent beyond this session\n" );
            bRunningWithDB = false;
        }
        else
        {
            cout << "Unknown commandline option " << argv[1] << endl;
        }
    }

    if( bRunningWithDB )
    {
        DEBUG("Connecting to local MySQL database \"" << mvConfig.AuthServerDatabaseInfo.DatabaseName << "\"..." );
        DBAbstractionLayer.DBConnect( mvConfig.AuthServerDatabaseInfo.Host.c_str(),
                                      mvConfig.AuthServerDatabaseInfo.DatabaseName.c_str(),
                                      mvConfig.AuthServerDatabaseInfo.UserName.c_str(),
                                      mvConfig.AuthServerDatabaseInfo.Password.c_str() );
    }

    INFO( "Initialization completed" );

    int ReadResult = 0;

    while( 1 )
    {
        DEBUG( "Waiting for message from authserver..." );
        ReadResult = SocketAuthServer.ReceiveLineBlocking( ReadBuffer );
        if( ReadResult == SOCKETS_READ_OK )
        {
            if( ReadBuffer[0] =='<' )
            {
                DEBUG( "received xml from authserver " << ReadBuffer );
                TiXmlDocument IPC;
                IPC.Parse( ReadBuffer );
                if( strcmp( IPC.RootElement()->Value(), "login" ) == 0 )
                {
                    if( strcmp( IPC.RootElement()->Attribute( "type" ), "user" ) == 0 )
                    {
                        DEBUG( "received client authentication request for " << IPC.RootElement()->Attribute("name") );
                        AuthenticateClient( atoi( IPC.RootElement()->Attribute("iconnectionref") ),
                                            IPC.RootElement()->Attribute("name"),
                                            IPC.RootElement()->Attribute("password")
                                          );
                    }
                    else if( strcmp( IPC.RootElement()->Attribute( "type" ), "sim" ) == 0 )
                    {
                        DEBUG( "received sim authentication request for " << IPC.RootElement()->Attribute("name") );
                        AuthenticateSim( atoi( IPC.RootElement()->Attribute("iconnectionref") ),
                                         IPC.RootElement()->Attribute("name"),
                                         IPC.RootElement()->Attribute("password")
                                       );
                    }
                }
            }
            else
            {
                DEBUG( "received legacy IPC from authserver " << ReadBuffer );
            }
        }
        else if( ReadResult == SOCKETS_READ_SOCKETGONE )
        {
            DEBUG( "AuthServer socket gone.  Dieing..." );

            if( bRunningWithDB )
            {
                DBAbstractionLayer.DisconnectDB();
            }
            mvSystem::mvExit(1);
        }
    }

    if( bRunningWithDB )
    {
        DBAbstractionLayer.DisconnectDB();
    }

    return 0;
}
