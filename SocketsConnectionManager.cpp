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
// see SocketsConnectionManager.h for documentation

#include <iostream>
#include <map>

#include "SocketsConnectionManager.h"
#include "SocketsClass.h"

bool SocketsConnectionManagerClass::GetConnectionForForeignReference( CONNECTION &rConnection, const int iForeignReference )
{
    for ( ConnectionsIterator = Connections.begin( ) ; ConnectionsIterator != Connections.end( ); ConnectionsIterator++ )
    {
        if( ( ConnectionsIterator->second ).iForeignReference == iForeignReference )
        {
            rConnection = ConnectionsIterator->second;
            return true;
        }
    }
    return false;
}

// Input: pTargetSet: a vector of mvsocket pointers
//
// Returns: None
//
// Description: Returns a list of sockets this object is managing, usually for use with a "select" statement
//
// History: 20050330 Mark Wagner - created from PopulateFDSet
void SocketsConnectionManagerClass::GetSocketList( vector<const mvsocket *> &pTargetSet )
{
    for ( ConnectionsIterator = Connections.begin( ) ; ConnectionsIterator != Connections.end( ); ConnectionsIterator++ )
    {
        /*       FD_SET( ConnectionsIterator->second.connectionsocket.socket, pTargetSet);
               if( ConnectionsIterator->second.connectionsocket.socket > *p_iMaxFileDescriptor - 1 )
               {
                   *p_iMaxFileDescriptor = ConnectionsIterator->second.connectionsocket.socket + 1;
               }*/
        pTargetSet.push_back(&(ConnectionsIterator->second.connectionsocket));
    }
    pTargetSet.push_back(&OurListenerSocket);
    /*   FD_SET( OurListenerSocket.socket, pTargetSet );
       if( OurListenerSocket.socket > *p_iMaxFileDescriptor - 1 )
       {
           *p_iMaxFileDescriptor = OurListenerSocket.socket + 1;
       }*/
}

void SocketsConnectionManagerClass::CloseConnectionNow( CONNECTION &rConnection )
{
    rConnection.connectionsocket.Close();
    rConnection.bConnected = false;
}

void SocketsConnectionManagerClass::Broadcast( const string Message )
{
    int result;
    for ( ConnectionsIterator = Connections.begin( ) ; ConnectionsIterator != Connections.end( ); ConnectionsIterator++ )
    {
        result = ConnectionsIterator->second.connectionsocket.Send( Message.c_str() );
        if( result == SOCKET_ERROR )
        {
            DEBUG(  "Client connref " << ConnectionsIterator->first << " disconnected" ); // DEBUG
            ConnectionsIterator->second.bConnected = false;
        }
    }
}

void SocketsConnectionManagerClass::ShowCurrentConnections()
{
    DEBUG(  "Current connections on port " << iPort << ":" ); // DEBUG

    for ( ConnectionsIterator = Connections.begin( ) ; ConnectionsIterator != Connections.end( ); ConnectionsIterator++ )
    {
        DEBUG(  "connectionref " << ConnectionsIterator->first << " foreign ref " << ConnectionsIterator->second.iForeignReference
                << " name " << ConnectionsIterator->second.name << " socketnum " << ConnectionsIterator->second.connectionsocket.GetSocket() << " "
                << inet_ntoa( ConnectionsIterator->second.connectionsocket.GetPeer() ) ); // DEBUG
    }
}

/*
int SocketsConnectionManagerClass::SendOnCurrentIterator( const char *Message )
{
  int result;
  result = ConnectionsIterator->second.connectionsocket.Send( Message );
  if( result == SOCKETS_READ_SOCKETGONE )
  {
      ConnectionsIterator->second.bConnected = false;
  }
  return result;
}
*/

int SocketsConnectionManagerClass::SendThruConnection( CONNECTION &rConnection, const char *Message )
{
    int result;
    result = rConnection.connectionsocket.Send( Message );
    if( result == SOCKETS_READ_SOCKETGONE )
    {
        rConnection.bConnected = false;
    }
    return result;
}

int SocketsConnectionManagerClass::ReceiveIteratorLineIfAvailable( char *ReadBuffer )
{
    int result;
    result = ConnectionsIterator->second.connectionsocket.ReceiveLineIfAvailable( ReadBuffer );
    if( result == SOCKETS_READ_SOCKETGONE )
    {
        ConnectionsIterator->second.bConnected = false;
    }
    return result;
}

void SocketsConnectionManagerClass::CheckForNewClients()
{
    while( OurListenerSocket.NewConnectionAvailable() )
    {
        DEBUG(  "New connection available, port " << iPort << "  Accepting..." ); // DEBUG
        CONNECTION NewConnection;
        NewConnection.connectionsocket = OurListenerSocket.AcceptNewConnection();
        NewConnection.bAuthenticated = false;
        NewConnection.name = "";
        NewConnection.iForeignReference = -1;
        NewConnection.bConnected = true;
        DEBUG("New connection " << NewConnection << " PeerIP " << inet_ntoa( NewConnection.connectionsocket.GetPeer() ) );
        DEBUG("New connection socket " << NewConnection.connectionsocket.GetSocket());
        Connections.insert( connectionmappair( iNextConnectionRef, NewConnection ) );
        iNextConnectionRef++;
        ShowCurrentConnections();
    }
}

int SocketsConnectionManagerClass::NumClientConnectionsWithName( const string name )
{
    int iNum = 0;
    for ( ConnectionsIterator = Connections.begin( ) ; ConnectionsIterator != Connections.end( ); ConnectionsIterator++ )
    {
        if( ConnectionsIterator->second.name == name )
        {
            iNum++;
        }
    }
    return iNum;
}

void SocketsConnectionManagerClass::Init( const unsigned long IPAddress, const int port )
{
    this->IPAddress = IPAddress;
    this->iPort = port;
    OurListenerSocket.Listen( IPAddress, port );
}

void SocketsConnectionManagerClass::PurgeDisconnectedConnections()
{
    //DEBUG("PurgeDisconnectedConnections()" );
    map< int, CONNECTION >::iterator iterator = Connections.begin();
    while( iterator != Connections.end() )
    {
        if( iterator->second.bConnected == false )
        {
            DEBUG(  "Purging connection of " << iterator->second.name ); // DEBUG
            //          ConnectionsIterator = Connections.erase( ConnectionsIterator, ConnectionsIterator );
            map< int, CONNECTION >::iterator nextiterator = iterator;
            nextiterator++;
            Connections.erase( iterator );
            iterator = nextiterator;
        }
        if( iterator != Connections.end() )
        {
            iterator++;
        }
    }
    //  DEBUG(  "purge done" ); // DEBUG
}
