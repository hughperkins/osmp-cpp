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
// see headerfile SocketsClass.h for documentation

// 20050330 Mark Wagner - turned into a proper class; made a cross-platform
//  socket wrapper class
// 20050416 Hugh Perkins - tweaked to build on Windows\
// 20050416 Hugh Perkins - constructor from SOCKET populates PeerIP now

// Thread safety: The class is thread-safe so long as:
// 1) InitSocketSystem() and EndSocketSystem() are only called from a single thread -- preferably the main one
// 2) mvsocket objects are not shared across threads
// 3) The underlying socket implementation is thread-safe

// TODO: If we're on Unix, and connecting locally, use Unix domain sockets

#ifdef _WIN32   // Windows vs Linux
#include "winsock2.h"
typedef int socklen_t;
#endif

#ifndef _WIN32 // Linux, cygwin
#include <sys/select.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#endif

#include <stdio.h>

#include "Diag.h"
#include "SocketsClass.h"
#include "CRuntimeNameCompat.h"

#ifndef _WIN32 // Linux, Cygwin

int WSAGetLastError()
{
    return -1;
}
void closesocket (SOCKET s)
{
    close(s);
}
#endif

#ifdef _WIN32
static void InitWinsock()
{
    // Initialize Winsock.
    WSADATA wsaData;
    int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
    if ( iResult != NO_ERROR )
    {
        ERRORMSG("Error at WSAStartup()");
        exit(1);
    }
}
static void ShutdownWinsock()
{
    WSACleanup();
}
#endif

// Input: None
//
// Returns: None
//
// Description: Does initialization of the socket system dependant on the platform
//
// Thread safety: Probably not thread-safe
//
// History: 20050330 Mark Wagner - Created
void mvsocket::InitSocketSystem()
{
#ifdef _WIN32
    InitWinsock();
#endif
}

// Input: None
//
// Returns: None
//
// Description: Does termination of the socket system dependant on the platform
//
// Thread safety: Probably not thread-safe
//
// History: 20050330 Mark Wagner - Created
void mvsocket::EndSocketSystem()
{
#ifdef _WIN32
    ShutdownWinsock();
#endif
}

// Generic constructor
mvsocket::mvsocket()
{
    ReadBuffer = NULL;
    SendBuffer = NULL;
    TempBuffer = NULL;
    SetBufferSizes(4096, 2048, 2048);
    ClearBuffers();
    socket = 0;
    mSocketOpen = false;

#ifdef _WIN32

    PeerIP.S_un.S_addr = 0;
#endif
}

// Constructor from an existing raw socket.
mvsocket::mvsocket( SOCKET newsocket )
{
    ReadBuffer = NULL;
    SendBuffer = NULL;
    TempBuffer = NULL;
    SetBufferSizes(4096, 2048, 2048);
    ClearBuffers();
    socket = newsocket;
    mSocketOpen = true;

    struct sockaddr* pSockAddr;
    pSockAddr = (sockaddr *)(new sockaddr_in);
    socklen_t iSockNameLen = sizeof( sockaddr_in );
    getpeername( newsocket, pSockAddr, &iSockNameLen );
    PeerIP = (( sockaddr_in * )pSockAddr)->sin_addr;
    free( pSockAddr );
}

// Copy constructor
mvsocket::mvsocket( const mvsocket& sourcesocket )
{
    PeerIP = sourcesocket.PeerIP;
    socket = sourcesocket.socket;
    mSocketOpen = sourcesocket.mSocketOpen;
    ReadBuffer = NULL;
    SendBuffer = NULL;
    TempBuffer = NULL;
    SetBufferSizes(sourcesocket.ReadBufferLen, sourcesocket.SendBufferLen, sourcesocket.TempBufferLen);
    memcpy( ReadBuffer, sourcesocket.ReadBuffer, ReadBufferLen );
    memcpy( SendBuffer, sourcesocket.SendBuffer, SendBufferLen );
    memcpy( TempBuffer, sourcesocket.TempBuffer, TempBufferLen );
    iBufferContentsLength = sourcesocket.iBufferContentsLength;
}

mvsocket::~mvsocket()
{
    // TODO: Figure out a way to see if the socket should be closed or not
    if(ReadBuffer != NULL)
    {
        delete ReadBuffer;
        ReadBuffer = NULL;
        ReadBufferLen = 0;
    }
    if(SendBuffer != NULL)
    {
        delete SendBuffer;
        SendBuffer = NULL;
        SendBufferLen = 0;
    }
    if(TempBuffer != NULL)
    {
        delete TempBuffer;
        TempBuffer = NULL;
        TempBufferLen = 0;
    }
}

// Assignment operator
mvsocket &mvsocket::operator =(const mvsocket &old)
{
    if(this != &old)
    {
        PeerIP = old.PeerIP;
        socket = old.socket;
        mSocketOpen = old.mSocketOpen;
        SetBufferSizes(old.ReadBufferLen, old.SendBufferLen, old.TempBufferLen);
        memcpy(ReadBuffer, old.ReadBuffer, ReadBufferLen);
        memcpy(SendBuffer, old.SendBuffer, SendBufferLen);
        memcpy(TempBuffer, old.TempBuffer, TempBufferLen);
        iBufferContentsLength = old.iBufferContentsLength;
    }
    return *this;
}

//void mvsocket::CopyFrom( mvsocket sourcesocket )
//{
// DEBUG( "CopyFrom() old PeerIP = " << inet_ntoa( sourcesocket.PeerIP ) );
// PeerIP = sourcesocket.PeerIP;
// socket = sourcesocket.socket;
// mSocketOpen = sourcesocket.mSocketOpen;
// SetBufferSizes(sourcesocket.ReadBufferLen, sourcesocket.SendBufferLen, sourcesocket.TempBufferLen);
// memcpy( ReadBuffer, sourcesocket.ReadBuffer, ReadBufferLen );
// memcpy( SendBuffer, sourcesocket.SendBuffer, SendBufferLen );
// memcpy( TempBuffer, sourcesocket.TempBuffer, TempBufferLen );
// iBufferContentsLength = sourcesocket.iBufferContentsLength;
//}


// Input: buffer: a pointer to the data to send
//        bytes: the number of bytes to send
//
// Returns: The number of bytes actually sent, or -1 on error
//
// Side effects: On any error (Windows) or certain errors (Linux) that
//  indicate the socket isn't connected properly any more, closes the
//  socket.
//
// Description: The low-level wrapper for the send() function.  All
//  socket data output goes through here.  SIGPIPE errors are suppressed.
//  Writes the data to the socket, making sure that all the data actually
//  gets written.  Can block.
//
// Thread safety: Thread-safe if the OS-level socket system is
//
// History: 20050409 Mark Wagner - Created
//          20050410 Mark Wagner - Modified to handle short writes
ssize_t mvsocket::LowLevelSend(const char *buffer, size_t bytes)
{
    ssize_t result = -1;
    size_t BytesSent = 0;
    if(mSocketOpen)
    {
        while(BytesSent < bytes)
        {
            //   DEBUG("Sending on socket " << socket << " " << strlen(SendBuffer) << " bytes: " << SendBuffer );

#ifndef _WIN32 // linux
            result = send(socket, buffer, bytes, MSG_NOSIGNAL);
#else

            result = send(socket, buffer, bytes, 0);
#endif

            //   DEBUG("Socket " << socket << " send result " << result);
            if(-1 == result)
            {
                WARNING("Error " << strerror(errno) << " sending on socket " << socket);
#ifndef _WIN32 // linux

                switch(errno)
                {
                    // Errors that indicate a socket isn't connected properly anymore
                    case EACCES:
                    case EBADF:
                    case ECONNRESET:
                    case ENOTCONN:
                    case ENOTSOCK:
                    case EPIPE:
                    close(socket);
                    mSocketOpen = false;
                    break;
                }
#else // Windows
                DEBUG("Error code was " << WSAGetLastError() );
                Close();
#endif

                break;
            }
            else
            {
                BytesSent += result;
            }
        }
    }
    else
    {
        WARNING("Socket " << socket << " not open");
    }
    return result;
}

// Input: sMessage: A printf()-style formatting string
//        Other args: a variable number of arguments to be passed to vsnprintf
//
// Returns: The number of bytes sent, or -1 on error
//
// Description: Formats a message using vsnprintf with a maximum length of 4096 bytes
//  Sends that message over the connected socket
//
// Thread safety: Thread-safe if the OS-level socket system is
//
// History: 20050409 Mark Wagner - Modified to use LowLevelSend
ssize_t mvsocket::Send( const char *sMessage, ... )
{
    // DEBUG("Formatted send");
    va_list args;
    va_start( args, sMessage );
    vsnprintf( SendBuffer, SendBufferLen, sMessage, args );
    // DEBUG( "Sending value " << SendBuffer )
    return LowLevelSend( SendBuffer, strlen( SendBuffer ) );
}

// Input: buffer: A pointer to a blob of data to send
//        bytes: The number of bytes to send
//
// Returns: The number of bytes sent, or -1 on error
//
// Description: Sends a blob of data over the network
//
// Thread safety: Thread-safe if the OS-level socket system is
//
// History: 20040330 Mark Wagner - Created
//          20050409 Mark Wagner - Modified to use LowLevelSend
int mvsocket::Send( const char *buffer, size_t bytes )
{
    // DEBUG("Unformatted send");
    return LowLevelSend(buffer, bytes);
}

// Input: buffer: a pointer to where to store the data
//        bytes: The number of bytes to fetch
//
// Returns: The number of bytes read, or -1 on error
//
// Description: If the socket is open, calls recv() to do a blocking
//  fetch of the requested amount of data, unless the socket is specified
//  as a non-blocking socket.
//  NOTE: Does no buffer-overflow checking.  Make sure the buffer
//  really is big enough for the requested amount of data.
//
// Thread safety: Thread-safe if the OS-level socket system is
//
// History: 20050409 Mark Wagner - Created
ssize_t mvsocket::LowLevelReceive(char *buffer, size_t bytes)
{
    ssize_t result = -1;
    if(mSocketOpen)
    {
        //  DEBUG("Reading " << bytes << " bytes on socket " << socket);
        result = recv(socket, buffer, bytes, 0);
        //  result = read(socket, buffer, bytes);
        //  DEBUG("Socket " << socket << " recv result " << result);
        if(-1 == result)
        {
            ERRORMSG("Error " << strerror(errno) << " reading on socket " << socket);

#ifndef _WIN32 // linux

            switch(errno)
            {
                case EBADF:
                case ENOTCONN:
                case ENOTSOCK:
                mSocketOpen = false;
                break;
            }
#else // Windows
            mSocketOpen = false;
#endif

        }
    }
    else
    {
        WARNING("Reading: socket " << socket << " not open");
    }
    return result;
}

// Input: buffer: a pointer to where to store incoming data
//        bytes : The maximum number of bytes to read
//
// Returns: The number of bytes read, or -1 on error
//
// Description: Retrieves "bytes" bytes from the read buffer and (if neccessary) the network
//  Copies them to "buffer"
//
// Thread safety: Thread-safe if the OS-level socket system is
//
// History: 20050330 Mark Wagner - Created
//          20050409 Mark Wagner - Modified to use LowLevelReceive
int mvsocket::Receive( char *buffer, size_t bytes )
{
    int ReturnCode = 0;
    if(iBufferContentsLength > 0)
    {
        // There's data in the buffer.  Try to satisfy the request with it
        ReturnCode = ReadBufferRemove(buffer, bytes);
        //  DEBUG("Read " << ReturnCode << " bytes out of buffer");
        bytes -= ReturnCode;
        buffer += ReturnCode;
    }
    if(bytes > 0)
    {
        // Couldn't fill the request completely out of the buffer.  Need to get data
        //  off the network
        //  DEBUG("Need to get " << bytes << " from network");
        bytes = LowLevelReceive( buffer, bytes );
        if(bytes >= 0) // Success
        {
            ReturnCode += bytes;
        }
        else   // Error
        {
            ReturnCode = bytes;
        }
    }
    return ReturnCode;
}

// Input: Line: A pointer to a buffer to store one line of data in
//        bBlocking: A boolean indicating whether to block or not
//
// Output: A null-terminated line of data in "*Line", including the trailing line terminator
//         On error, *Line = '\0'
//
// Returns: A socket error code
//
// Description: Checks the read buffer to see if it's got a full line of data.  If it does,
//  returns that line.  If not, reads data from the socket to try to make up a full line.
//  NOTE: May return up to ReadBufferLen bytes; make sure that *Line can handle that.
//
// Thread safety: Thread-safe if the OS-level socket system is
//
// History: 20050330 Mark Wagner - modified to use the real network line ending codes
//          20050402 Mark Wagner - modified to allow any line ending
//          20050409 Mark Wagner - modified to use LowLevelReceive
//          20050410 Mark Wagner - Re-wrote from scratch to handle things like
//                                 binary data and data past end-of-line
//          20050415 Mark Wagner - More sensible handling of the contents of "Line" on error
int mvsocket::ReceiveLineGeneric( char *Line, bool bBlocking )
{
    char *pReadUntil = NULL;
    ssize_t recvlen = 0;
    *Line = '\0';
    // Can we satisfy the request out of the read buffer?
    pReadUntil = GetLineEnd( ReadBuffer );
    if(NULL != pReadUntil)
    {
        // Return the line, including newline
        recvlen = ReadBufferRemove(Line, (pReadUntil - ReadBuffer + 1));
        //  DEBUG("Line found in buffer " << Line << " bytes " << recvlen);
        return SOCKETS_READ_OK;
    }
    else
    {
        // If this is a blocking request, or if there's data available, fill the buffer as far as possible
        if(bBlocking || DataAvailable())
        {
            recvlen = LowLevelReceive(ReadBuffer + iBufferContentsLength, ReadBufferLen - iBufferContentsLength);
            iBufferContentsLength += recvlen;
            if(recvlen != SOCKET_ERROR && recvlen > 0)
            {
                pReadUntil = GetLineEnd( ReadBuffer );
                if(pReadUntil != NULL)
                {
                    // If we have a line, return it, including the newline
                    recvlen = ReadBufferRemove(Line, (pReadUntil - ReadBuffer + 1));
                    // Tack on a '\0', as ReadBufferRemove only does a memcpy
                    Line[recvlen] = '\0';
                    //     DEBUG("Got line " << Line << " bytes " << recvlen);
                    return SOCKETS_READ_OK;
                }
                else
                {
                    // If we don't have a line, and the buffer's full, return an error
                    DEBUG("Socket  " << socket << " Buffer full, no line found");
                    return SOCKETS_READ_NODATA;
                }
            }
            else
            {
                WARNING("Read failed on socket " << socket << " error " << strerror(errno));
                return SOCKETS_READ_SOCKETGONE;
            }
        }
        else
        {
            //   DEBUG("No data available");
            return SOCKETS_READ_NODATA;
        }
    }

#if 0 // Old-style method
    ssize_t recvlen = 0;
    char *pReadUntil;
    pReadUntil  = GetLineEnd( ReadBuffer + iReadBufferIndex );
    if( iBufferContentsLength == 0 || iReadBufferIndex >= iBufferContentsLength || pReadUntil == NULL )
    {
        //DEBUG("Socket " << socket << " need to get data off network");
        if( bBlocking || DataAvailable() )
        {
            DEBUG("Socket " << socket << " getting data");
            recvlen = LowLevelReceive( TempBuffer, 2046 );
            DEBUG("Socket " << socket << " recvlen " << recvlen);
            DEBUG("Buffer " << TempBuffer << " END");
            if( recvlen != SOCKET_ERROR && recvlen > 0 )
            {
                DEBUG("Socket " << socket << " data got" << TempBuffer);
                //     Debug( "recvlen = %i\n", recvlen );

                TempBuffer[ recvlen ] = '\0';
                if( iReadBufferIndex < iBufferContentsLength )
                {
                    //    Debug( "%i %i Joining buffers [%s] and [%s]\n", iReadBufferIndex, iBufferContentsLength, ReadBuffer + iReadBufferIndex, TempBuffer );
                    sprintf( ReadBuffer, "%s%s", ReadBuffer + iReadBufferIndex, TempBuffer );
                }
                else
                {
                    //  Debug( "Copying buffer direct [%s]\n", TempBuffer );
                    sprintf( ReadBuffer, "%s", TempBuffer );
                }
                iReadBufferIndex = 0;
                iBufferContentsLength = strlen( ReadBuffer );
                pReadUntil  = GetLineEnd( ReadBuffer + iReadBufferIndex );
            }
            else
            {
                DEBUG("Socket " << socket << " recv error " << strerror(errno));
                return SOCKETS_READ_SOCKETGONE;
            }
        }
        else
        {
            //DEBUG("Socket " << socket << " no data on network");
            return SOCKETS_READ_NODATA;
        }
    }

    if( pReadUntil != NULL )
    {
        sprintf( Line, "%.*s", pReadUntil - ReadBuffer - iReadBufferIndex, ReadBuffer + iReadBufferIndex );
        iReadBufferIndex = pReadUntil - ReadBuffer + 1;
        return SOCKETS_READ_OK;
    }
    else
    {
        return SOCKETS_READ_NODATA;
    }
#endif
}

int mvsocket::ReceiveLineIfAvailable( char *Line )
{
    return ReceiveLineGeneric( Line, false );
}

int mvsocket::ReceiveLineBlocking( char *Line )
{
    return ReceiveLineGeneric( Line, true );
}


// Input: None
//
// Returns: None
//
// Description: If the socket associated with the object is open, close it
//
// Thread safety: Thread-safe if the OS-level socket system is
void mvsocket::Close()
{
    if(mSocketOpen)
    {
#ifdef _WIN32
        closesocket( socket );
#else

        shutdown( socket, SHUT_RDWR );
#endif

    }
    mSocketOpen = false;
}

bool mvsocket::NewConnectionAvailable()
{
    int result;
    timeval TimeOut;
    TimeOut.tv_sec = 0;
    TimeOut.tv_usec = 0;

    fd_set TargetSet;
    FD_ZERO( &TargetSet );
    FD_SET( socket, &TargetSet);
    result = select( socket + 1, &TargetSet, NULL, NULL, &TimeOut );
    if(result > 0)
    {
        return true;
    }
    else if(result == SOCKET_ERROR)
    {
        ERRORMSG("Socket " << socket << " connection error: " << strerror(errno) << " in NewConnectionAvailable()");
        return false;
    }
    else
    {
        return false;
    }
}

// Input: IPAddress: An IP address to connect to, in network byte order
//        iPort: A port number to connect to, in host byte order
//
// Returns: True on success, false otherwise
//
// Description: Creats a new socket and attempts to connect to the specified
//  address on the specified port.
//
// Thread safety: Thread-safe if the OS socket system is
//
// History: 20050410 Mark Wagner - added error return
bool mvsocket::ConnectToServer( unsigned long IPAddress, int iPort )
{
    socket = SOCKET_ERROR;

    socket = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if ( socket == INVALID_SOCKET )
    {
        ERRORMSG( "Socket " << socket << " error at socket():" << strerror(errno) );
        return false;
    }
    mSocketOpen = true;

    sockaddr_in clientService;
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = IPAddress;
    clientService.sin_port = htons( iPort );
    if ( connect( socket, (SOCKADDR*) &clientService, sizeof(clientService) ) == SOCKET_ERROR)
    {
        ERRORMSG( "Socket " << socket << " failed to connect: " << strerror(errno) );
        closesocket(socket);
        mSocketOpen = false;
        return false;
    }

    // DEBUG("Connected to " << IPAddress << ":" << iPort << " socket " << socket);
    return true;
}

// Input: None
//
// Returns: An mvsocket object representing the connection on success, or
//  an mvsocket object with a socket number of 0 on error
//
// Description: Accepts a new connection on the socket.  Creates an mvsocket
//  object representing that connection and returns it to the caller
//
// Thread safety: Thread-safe if the OS socket system is
//
// History: 20050410 Mark Wagner - Added an error return
//          20050413 Mark Wagner - Made it do something sensible with PeerIP
mvsocket mvsocket::AcceptNewConnection()
{
    // DEBUG("Socket " << socket << " AcceptNewConnection");
    SOCKET AcceptSocket;
    AcceptSocket = SOCKET_ERROR;
    AcceptSocket = accept( socket, NULL, NULL );
    if(  AcceptSocket == SOCKET_ERROR )
    {
        ERRORMSG("Socket " << socket << " accept error: " << strerror(errno));
        return mvsocket();
    }
    mvsocket NewSocket(AcceptSocket);

    struct sockaddr* pSockAddr;
    pSockAddr = (sockaddr *)(new sockaddr_in);
    socklen_t iSockNameLen = sizeof( sockaddr_in );
    getpeername( AcceptSocket, pSockAddr, &iSockNameLen );
    PeerIP = (( sockaddr_in * )pSockAddr)->sin_addr;
    free( pSockAddr );

    DEBUG("Socket " << socket << " AcceptNewConnection done: " << AcceptSocket );
    return NewSocket;
}

// Input: IPAddress: An IP address in network byte order
//        iPort: A port number in native byte order
//
// Returns: True on success, false otherwise
//
// Description: Creates a socket, binds it to the specified IP address and port,
//  and sets it to listen for new connections
//
// Thread safety: Thread-safe if the OS socket system is
//
// History:
bool mvsocket::Listen( unsigned long IPAddress, int iPort )
{
    socket = SOCKET_ERROR;

    socket = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if ( socket == INVALID_SOCKET )
    {
        //  Debug( "Error at socket(): %ld\n", WSAGetLastError() );
        ERRORMSG( "Error at socket(): " << strerror(errno));
        return false;
    }

    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = IPAddress;
    service.sin_port = htons( iPort );
    if ( bind( socket, (SOCKADDR*) &service, sizeof(service) ) == SOCKET_ERROR )
    {
        ERRORMSG( "bind() failed: " << strerror(errno) );
        closesocket(socket);
        mSocketOpen = false;
        return false;
    }

    if ( listen( socket, SOMAXCONN ) == SOCKET_ERROR )
    {
        ERRORMSG( "Error listening on socket: " << strerror(errno));
        return false;
    }

    mSocketOpen = true;
    return true;
}

bool mvsocket::DataAvailable()
{
    int result = 0;
    timeval TimeOut;
    TimeOut.tv_sec = 0;
    TimeOut.tv_usec = 0;

    fd_set TargetSet;
    FD_ZERO( &TargetSet );
    FD_SET(socket, &TargetSet);
    //DEBUG("Data check on " << socket);
    result = select( socket + 1, &TargetSet, NULL, NULL, &TimeOut );

#ifdef _WIN32

    if( result > 0 )
    {
        //  DEBUG("Socket " << socket << " data available");
        return true;
    }
    else if( result == SOCKET_ERROR)
    {
        int errnum = WSAGetLastError();
        WARNING("Socket " << socket << " error: " << errnum << " in DataAvailable()");
        cout << "mSocketOpen = " << mSocketOpen << endl;
        Close();
        return false;
    }
    else
    {
        //  DEBUG("Socket " << socket << " no data available");
        return false;
    }

#else // linux

    if( result > 0 )
    {
        //  DEBUG("Socket " << socket << " data available");
        return true;
    }
    else if( result == -1)
    {
        WARNING("Socket " << socket << " error " << strerror(errno) << " in DataAvailable()");
        Close();
        return false;
    }
    else
    {
        //  DEBUG("Socket " << socket << " no data available");
        return false;
    }

#endif // linux
}

// Input: buffer: A null-terminated string
//
// Returns: A pointer to the byte before the first byte after the line terminator,
//  or NULL if no line terminator occurs
//
// Description: Searches the string for line endings consisting of '\015', '\012',
//  '\015\012', or '\012\015'.  Returns a pointer to the first one found, or NULL
//  if none are found.
//
// Thread safety: Thread-safe
//
// History: 20050402 Mark Wagner - Created
char *mvsocket::GetLineEnd(char *buffer)
{
    char *end15 = strchr(buffer, '\015');
    char *end12 = strchr(buffer, '\012');
    if(end15 && !end12)
    {
        // Only '\015'.  Return it
        //  DEBUG("End: 15");
        return end15;
    }
    else if(end12 && !end15)
    {
        // Only '\012'.  Return it
        //  DEBUG("End: 12");
        return end12;
    }
    else if(!end12 && !end15)
    {
        // Neither.  Return NULL
        //  DEBUG("End: None");
        return NULL;
    }
    else
    {
        // The hard case: both
        if(abs(end15 - end12) > 1)
        {
            // They don't occur in succession.  Return the first one.
            //   DEBUG("End: Mismatch");
            return (end12 > end15)?end15:end12;
        }
        else
        {
            // They occur in succession.  Return the second one
            //   DEBUG("End: Both");
            return (end12 > end15)?end12:end15;
        }
    }
}

// Input: None
//
// Returns: None
//
// Description: Discards all data in the buffers
//
// Thread safety: Thread-safe
//
// History: 20050410 Mark Wagner - Created
void mvsocket::ClearBuffers()
{
    if(NULL != ReadBuffer)
        ReadBuffer[0] = '\0';
    if(NULL != SendBuffer)
        SendBuffer[0] = '\0';
    if(NULL != TempBuffer)
        TempBuffer[0] = '\0';
    iBufferContentsLength = 0;
}

// Input: ReadSize: The size for the read buffer, or 0 for no change
//        SendSize: The size for the send buffer, or 0 for no change
//        TempSize: The size for the temp buffer, or 0 for no change
//
// Returns: None
//
// Side effects: Sets ReadBufferLen, SendBufferLen, and TempBufferLen if the
//  respective buffers have changed size.
//
// Description: For each buffer, if the buffer exists, resizes it and copies
//  as much data from the old buffer as fits in the new one.  The new buffer
//  is one char larger than the specified size, and the final char is set to
//  '\0' to make sure that string functions don't crash the program.
//
// Thread safety: Thread-safe
//
// History: 20050410 Mark Wagner - Created
//          20050412 Mark Wagner - Modified to not resize if no size change
void mvsocket::SetBufferSizes(size_t ReadSize, size_t SendSize, size_t TempSize)
{
    char *NewBuffer;
    if(ReadSize != 0 && ReadSize != ReadBufferLen)
    {
        if(NULL != ReadBuffer)
        {
            NewBuffer = new char[ReadSize + 1];
            if(ReadSize > ReadBufferLen)
            {
                memcpy(NewBuffer, ReadBuffer, ReadBufferLen);
            }
            else
            {
                memcpy(NewBuffer, ReadBuffer, ReadSize);
            }
            delete ReadBuffer;
            ReadBuffer = NewBuffer;
        }
        else
        {
            ReadBuffer = new char[ReadSize + 1];
        }
        ReadBufferLen = ReadSize;
        ReadBuffer[ReadBufferLen] = '\0';
    }
    if(SendSize != 0 && SendSize != SendBufferLen)
    {
        if(NULL != SendBuffer)
        {
            NewBuffer = new char[SendSize + 1];
            if(SendSize > SendBufferLen)
            {
                memcpy(NewBuffer, SendBuffer, SendBufferLen);
            }
            else
            {
                memcpy(NewBuffer, SendBuffer, SendSize);
            }
            delete SendBuffer;
            SendBuffer = NewBuffer;
        }
        else
        {
            SendBuffer = new char[SendSize + 1];
        }
        SendBufferLen = SendSize;
        SendBuffer[SendBufferLen] = '\0';
    }
    if(TempSize != 0 && TempSize != TempBufferLen)
    {
        if(NULL != TempBuffer)
        {
            NewBuffer = new char[TempSize + 1];
            if(TempSize > TempBufferLen)
            {
                memcpy(NewBuffer, TempBuffer, TempBufferLen);
            }
            else
            {
                memcpy(NewBuffer, TempBuffer, TempSize);
            }
            delete TempBuffer;
            TempBuffer = NewBuffer;
        }
        else
        {
            TempBuffer = new char[TempSize + 1];
        }
        TempBufferLen = TempSize;
        TempBuffer[TempBufferLen] = '\0';
    }
}

// Input: dest: A pointer to the location to copy the data to, or NULL to
//         discard the data
//        bytes: The amount of data requested from the buffer
//
// Returns: The amount of data actually removed
//
// Description: Removes the specified amount of data from the buffer or as
//  much data as the buffer actually contains, then copies the rest of the
//  buffer contents to the beginning, appending a '\0'.
//
// Thread safety: Thread-safe
//
// History: 20050410 Mark Wagner - Created
size_t mvsocket::ReadBufferRemove(char *dest, size_t bytes)
{
    size_t BytesRemoved;
    if(bytes > iBufferContentsLength)
        BytesRemoved = iBufferContentsLength;
    else
        BytesRemoved = bytes;

    if(NULL != dest)
    {
        memcpy(dest, ReadBuffer, BytesRemoved);
    }
    memmove(ReadBuffer, ReadBuffer + BytesRemoved, iBufferContentsLength - BytesRemoved);
    iBufferContentsLength -= BytesRemoved;
    ReadBuffer[iBufferContentsLength] = '\0';
    return BytesRemoved;
}

// Input: timeout:    a timeout in milliseconds
//        readsock:   an STL vector of mvsocket pointers to check for reading
//
// Returns: None
//
// Description: Block until a socket event occurs, or until the timeout has expired
//              If timeout is 0, does not block.  This is useful for polling
//              If timeout is negative, blocks indefinitely
//
// History: 20050330 Mark Wagner - Moved from metaverseserver.cpp
//                                 Made a friend function for the mvsocket class
//                                 Made more general-purpose
void SocketsReadBlock(int timeout, vector<const mvsocket *> readsocks)
{
    //DEBUG( "ticks till next frame: %i\n", iTicksTillNextFrame );
    vector<const mvsocket *>::const_iterator i;
    int  MaxFileDescriptor = 0;
    timeval TimeOut;
    TimeOut.tv_sec = 0;
    TimeOut.tv_usec = timeout * 1000;
    fd_set ReadTargetSet;

    FD_ZERO( &ReadTargetSet );

    for(i = readsocks.begin(); i != readsocks.end(); i++)
    {
        SOCKET thissocket = (*i)->socket;
        //DEBUG("Blocking on socket " << thissocket );
        FD_SET( thissocket, &ReadTargetSet);
        if( thissocket > MaxFileDescriptor)
        {
            MaxFileDescriptor = thissocket;
        }
    }

    int result;
    if(timeout >= 0)
    {
        result = select( MaxFileDescriptor + 1, &ReadTargetSet, NULL, NULL, &TimeOut );
    }
    else
    {
        result = select( MaxFileDescriptor + 1, &ReadTargetSet, NULL, NULL, NULL );
    }
    if( result == SOCKET_ERROR )
    {
#ifdef _WIN32
        INFO("Sockets select error: " << WSAGetLastError() );
#endif

    }
}
