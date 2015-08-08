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
//! \brief This module is used to create and manage a single TCP/IP sockets connection

//! This module is used to create a single TCP/IP sockets connection
//! and to manage transfer of data to/from this
//! It returns the data line by line
//!
//! see metaverseserver and metaverseclient for examples of how to use it

// Modified 20050330 Mark Wagner - major re-write to turn it into a
//  cross-platform socket wrapper for both TCP/IP and Unix sockets,
//  and a proper class.  May break Windows compatibility.
// 20050416 Hugh Perkins - tweaked to build on Windows

#ifndef _SOCKETS_H
#define _SOCKETS_H

#ifdef _WIN32
#ifndef _WINDOWS_
#include <winsock2.h>
#endif
#endif

#include <string.h>
#include <stdio.h>
#include <vector>
#include <ostream>

#include "Diag.h"

#ifndef _WIN32  // Linux, Cygwin
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef int SOCKET;
typedef sockaddr SOCKADDR;
const  int SOCKET_ERROR = -1;
const int INVALID_SOCKET = -1;

#else // Windows
#ifndef __GNUC__ // msvc
typedef int ssize_t;
#endif
#endif

enum SocketReadResult
{
    SOCKETS_READ_NODATA,
    SOCKETS_READ_SOCKETGONE,
    SOCKETS_READ_OK
};

//! mvsocket is used to create and manage a single TCP/IP sockets connection

//! mvsocket is used to create a single TCP/IP sockets connection
//! and to manage transfer of data to/from this
//! It returns the data line by line
//!
//! see metaverseserver and metaverseclient for examples of how to use it
class mvsocket
{
private:
    char *ReadBuffer;
    char *SendBuffer;
    char *TempBuffer;
    size_t ReadBufferLen;
    size_t SendBufferLen;
    size_t TempBufferLen;

    int iBufferContentsLength;

    SOCKET socket;  //!< actual underlying socket
    in_addr PeerIP;  //!< ip address of connected machine

    bool mSocketOpen;   //!< whether socket is open or not

    mvsocket( SOCKET newsocket );

    // Low-level wrappers for reading and writing data to the network
    ssize_t LowLevelSend(const char *buffer, size_t bytes); //!< Low-level wrappers for writing data to the network
    ssize_t LowLevelReceive(char *buffer, size_t bytes);  //!< Low-level wrappers for reading data from the network

    //! Search a string for any form of line-ending
    char *GetLineEnd(char *buffer);

    //! Internal buffer-maintainence functions
    size_t ReadBufferRemove(char *buffer, size_t bytes);
public:

    static void InitSocketSystem(); //!< Initialize the TCP sockets system
    static void EndSocketSystem(); //!< Terminate the TCP sockets system

    mvsocket();
    mvsocket( const mvsocket &oldsocket ); //!< Copy constructor
    ~mvsocket();

    mvsocket &operator =(const mvsocket &old); //!< Assignment operator
    //void CopyFrom( mvsocket source );

    void Init()
    {
        ClearBuffers();
        socket = 0;
    }

    bool ConnectToServer( unsigned long IPAddress, int iPort );     //!< Call this to connect to a server from a client

    // Data-transmission functions
    ssize_t Send( const char *message, ... );                           //!< Sends data on our socket (client or server)
    int Send( const char *buffer, size_t bytes );     //!< Sends pre-formatted data on our socket

    // Data-reception functions
    bool DataAvailable();                                           //!< Is there any data waiting?  true/false
    int Receive( char *buffer, size_t bytes );      //!< Receives a blob of data
    int ReceiveLineIfAvailable( char *Line );                       //!< gets a line of data, returns immediately if no data
    //!< returns SOCKETS_READ_NODATA, SOCKETS_READ_SOCKETGONE, SOCKETS_READ_OK
    //!< according to what happened
    int ReceiveLineBlocking( char *Line );                          //!< like ReceiveLineIfAvailable but blocks until data arrives
    int ReceiveLineGeneric( char *Line, bool bBlocking = false );

    // Buffer-related functions
    void ClearBuffers();
    void SetBufferSizes(size_t ReadSize, size_t SendSize, size_t TempSize);

    // Connection-related functions
    bool Listen( unsigned long IPAddress, int iPort );              //!< Call this to listen for clients on a server
    bool NewConnectionAvailable();                                  //!< Are there any clients waiting to connect
    class mvsocket AcceptNewConnection();                           //!< Call this to accept a new client that just connected
    void Close();
    bool IsOpen() const
    {
        return mSocketOpen;
    }

    // Accessors
    in_addr GetPeer() const
    {
        return PeerIP;
    }
    ;   //!< Gets PeerIP
    void SetPeer(in_addr addr)
    {
        PeerIP = addr;
    }
    ; //!< Sets PeerIP

    const SOCKET GetSocket() const
    {
        return socket;
    }   //!< Gets socket


    // Friend functions
    void friend SocketsReadBlock(int timeout, vector<const mvsocket *> readsocks);
    friend ostream& operator <<( ostream& outs, const mvsocket data )
    {
        outs << "Socket: " << data.socket << " ReadBuffer " << data.iBufferContentsLength;
        return outs;
    }
};

#endif
