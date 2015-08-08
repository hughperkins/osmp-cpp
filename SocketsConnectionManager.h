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
//! \brief This module is for use on a server accepting connections from multiple clients
//!
//! This module is for use on a server accepting connections from multiple clients
//! It manages the new connections for you and purges the old ones
//! You can use it to send broadcasts to all teh connections, to populate your fd_sets for sockets select
//! and you can iterate through all the connections, sending and receiving messages as you go

#ifndef _SOCKETSCONNECTIONMANAGER_H
#define _SOCKETSCONNECTIONMANAGER_H

#include <string>
#include <sstream>
#include <ostream>
#include <map>
using namespace std;

#include "SocketsClass.h"

//! Holds information on a single client connection socket, used by SocketsConnectionManager
class CONNECTION
{
public:
    mvsocket connectionsocket;  //!< socket on which client is connected
    string name;   //!< name of client (eg username)
    int iForeignReference;
    bool bAuthenticated;  //!< has client authenticated?
    bool bConnected;    //!< is client connected?
    //bool bInternet;   // Deprecated; use IsLocalClient

    //! Copies from passed-in connection
    const CONNECTION &operator=( const CONNECTION &srcconnection )
    {
        this->connectionsocket = srcconnection.connectionsocket;
        this->name = srcconnection.name;
        this->iForeignReference = srcconnection.iForeignReference;
        this->bAuthenticated = srcconnection.bAuthenticated;
        this->bConnected = srcconnection.bConnected;
        return *this;
        // bInternet = srcconnection.bInternet;
    }

    //! Use to serialize to ostream, eg for debugging
    friend ostream& operator <<( ostream& outs, const CONNECTION &data )
    {
        outs << data.connectionsocket;
        return outs;
    }
};

//! SocketsConnectionManagerClass handles multiple client connections, used by server components

//! SocketsConnectionManagerClass handles multiple client connections, used by server components
//! It manages the new connections for you and purges the old ones
//! You can use it to send broadcasts to all the connections, to populate your fd_sets for sockets select
//! and you can iterate through all the connections, sending and receiving messages as you go
class SocketsConnectionManagerClass
{
public:
    map <int, CONNECTION, less<int> > Connections;  //!< All current connections / connection info
    map <int, CONNECTION, less<int> >::iterator ConnectionsIterator;

    int iNextConnectionRef; //!< next connection reference to assign to next connection

    mvsocket OurListenerSocket;  //!< socket on which we're listening for new connections

    unsigned long IPAddress;  //!< Our IP address (I think?)
    int iPort;      //!< Our port number (I think?)

    SocketsConnectionManagerClass()
    {
        iNextConnectionRef = 1;
        Connections.clear();
    }

    void Init( const unsigned long IPAddress, const int port );                  //!< create listener on port specified, for remote ip address
    //!< in IPAddress (eg inet_addr("127.0.0.1") for local only)
    void CheckForNewClients();                                       //!< What it says.  Call this regularly

    int ReceiveIteratorLineIfAvailable( char *ReadBuffer );          //!< Loop through the iterator ( for ( scm.ConnectionsIterator = scm.Connections.begin( ) ; scm.ConnectionsIterator != scm.Connections.end( ); scm.ConnectionsIterator++ ) )
    //!< in the loop, call this function to get any new data received on each connection
    //!< ReadBuffer needs to be char [4097]
    //   int SendOnCurrentIterator( const char *Message );                      //!< You can loop through as above, and send a message on current iterator
    void Broadcast( const string Message );                                //!< Sends Message to all connections
    int SendThruConnection( CONNECTION &rConnection, const char *Message );
    //!< Send a message on the connection specified, adding to list to purge
    //!< if the connection has been disconnteced

    void GetSocketList( vector<const mvsocket *> &pTargetSet );             //!< use with sockets select statement
    //!< pass in a vector, and it will add
    //!< the Connections sockets in
    void PurgeDisconnectedConnections();                             //!< cleans up old sessions; do this often

    bool GetConnectionForForeignReference( CONNECTION &rConnection, const int iForeignReference );   //!< Gets the connection info given the foreign reference number
    void ShowCurrentConnections();                                                          //!< writes description of current connections to stdout
    int NumClientConnectionsWithName( const string name );                                        //!< get number of client connections with name name
    void CloseConnectionNow( CONNECTION &rConnection );               //!< kills socket and marks connection object for purge

protected:
    void SocketsConnectionManagerClass::HandleLostConnection( const int iConnectionRef );
};

typedef pair <int, CONNECTION> connectionmappair;
typedef map <int, CONNECTION, less<int> >::iterator ConnectionsIteratorTypedef;

#endif // _SOCKETSCONNECTIONMANAGER_H
