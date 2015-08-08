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
//! \brief responsible for transmission of files to clients, such as textures and meshes
//!
//! responsible for transmission of files to clients, such as textures and meshes
//! handles reception of new files and lets Metaverse server know about them
//! has a single listener to accept incoming connections from clients, and a permanent connection to
//! the Metaverse Server
//!
//! A connection is created, by the client, for each file uploaded/downloaded,
//! to facilitate error handling and reduce protocol management
//! The client makes a single connection at a time to prevent bandwidth domination over
//! the higher-priority main control socket that is used for avatar movement and so on.

// Modified 20050330 Mark Wagner - Converted to use the mvsocket class as a class
//                               - Converted CONNECTION into a class
//                               - Replaced an array with a vector
// Modified 20050416 Hugh Perkins - includes CRuntimeNameCompat, for Windows build

// TODO: Proper rate limiting

#include <stdio.h>
#include <sstream>
#include <string.h>
#include <stdarg.h>
#include <iostream>
using namespace std;

#include "tinyxml.h"

#include "CRuntimeNameCompat.h"
#include "Diag.h"
#include "SocketsClass.h"
//#include "filetrans.h"
#include "Checksum.h"
#include "System.h"

int iMetaverseServerPort = 22140;  //!< port used to connect to metaverseserver component
int iClientFileAgentListenPort = 22174;   //!< port on which to listen for connections from clientfileagent

#define BUFSIZE 2047   //! Size of sockets SendBuffer
char SendBuffer[BUFSIZE + 1];   //!< sockets SendBuffer
char ReadBuffer[4097];   //! sockets read buffer

mvsocket MetaverseServerSocket;   //!< socket used to connect with metaverseserver component
mvsocket ClientFileAgentListener;   //!< socket to listen for new client connections

//! Stores information about one connection from clientfileagent; used by serverfileagent
class CONNECTION
{
private:
    mvsocket mSocket;   //!< socket associated with this connection/client
    int      mConnectionState;   //!< connection state, ie sending data etc (see enum ConnectionStates)
    char     mFileType[17];  //!< File type (texture, object, etc)
    char     mFileName[65];  //!< File name
    char     mFilePath[256]; //!< File name with path
    size_t   mFileSize;   //!< File size expected
    size_t   mBytesReceived;   //!< Byte received so far  (to compare with mFileSize)
    string   mChecksum;   //!< checksum of file to be received (to allow check that it is ok)
    FILE *   mFileHandle;   //!< filehandle to operating sytem file object

public:
    //! possible connection states
    enum ConnectionStates
    {
        StateNone,
        StateNewConnection,  //!< connection open, not sending data
        StateSendingData,  //!< connection open, we are sending data
        StateReceivingData,   //!< connection open, we are receiving data
        StatePendingValidation,  //!< transfer done, pending verification (I think?)

        StateLastState
    };

    // Member accessors
    int  GetState()
    {
        return mConnectionState;
    }
    void  SetState(int state)
    {
        if(state > 0 && state < StateLastState)
        {
            mConnectionState = state;
        }
    }

    const char *GetFiletype()
    {
        return mFileType;
    }
    void SetFiletype(char *ftype)
    {
        if(ftype != NULL)
        {
            strcpy(mFileType, ftype);
        }
    }

    const char *GetFilename()
    {
        return mFileName;
    }
    void  SetFilename(char *fname)
    {
        if(fname != NULL)
            strcpy(mFileName, fname);
    }

    const char *GetFilepath()
    {
        return mFilePath;
    }
    void  SetFilepath(char *fpath)
    {
        if(fpath != NULL)
            strcpy(mFilePath, fpath);
    }

    size_t GetFilesize()
    {
        return mFileSize;
    }
    void  SetFilesize(size_t size)
    {
        mFileSize = size;
    }

    const mvsocket &GetSocket()
    {
        return mSocket;
    }

    size_t GetBytesReceived()
    {
        return mBytesReceived;
    }
    void SetBytesReceived(size_t bytes)
    {
        mBytesReceived = bytes;
    }

    // File functions
    bool IsFilehandleValid()
    {
        return mFileHandle != NULL;
    }
    bool FileOpen(char *mode)
    {
        mFileHandle = fopen(mFilePath, mode);
        if( mFileHandle == NULL )
        {
            ERRORMSG( "Unable to open file " << mFilePath );
        }
        return (mFileHandle != NULL);
    }
    size_t FileRead(char *buffer, size_t bytes)
    {
        if(IsFilehandleValid())
        {
            return fread(buffer, sizeof(char), bytes, mFileHandle);
        }
        else
        {
            return 0;
        }
    }
    size_t FileWrite(char *buffer, size_t bytes)
    {
        if(IsFilehandleValid())
        {
            mBytesReceived += bytes;
            return fwrite(buffer, sizeof(char), bytes, mFileHandle);
        }
        return 0;
    }
    void FileClose()
    {
        if(IsFilehandleValid())
            fclose(mFileHandle);
        mFileHandle = NULL;
    }
    bool FileEOF()
    {
        if(IsFilehandleValid())
            return feof(mFileHandle);
        else
            return true;
    }
    bool FileError()
    {
        if(IsFilehandleValid())
            return ferror(mFileHandle);
        else
            return false;
    }

    // Socket functions
    void Close()
    {
        mSocket.Close();
    }
    int  Send(const char *buffer, size_t bytes)
    {
        return mSocket.Send(buffer, bytes);
    } // Send a blob
    int  Send(const char *buffer)
    {
        return Send(buffer, strlen(buffer) + 1);
    }   // Send a null-terminated string
    int  Receive(char *buffer, size_t bytes)
    {
        return mSocket.Receive(buffer, bytes);
    }
    bool DataAvailable()
    {
        return mSocket.DataAvailable();
    }

    int ReceiveLineIfAvailable( char *buffer)
    {
        int result = mSocket.ReceiveLineIfAvailable(buffer);
        return result;
    }

    // Checksum functions
    bool ValidateChecksum(string checksum)
    {
        return mChecksum == checksum;
    }
    string GetChecksum()
    {
        return mChecksum;
    }
    void SetChecksum( const char *checksum )
    {
        mChecksum = checksum;
    }

    CONNECTION()
    {
        DEBUG( "CONNECTION()" );
        mConnectionState = StateNone;
        sprintf(mFileType, "" );
        sprintf(mFileName, "" );
        sprintf(mFilePath, "" );
        mFileSize = 0;
        mBytesReceived = 0;
        mChecksum = "";
        mFileHandle = NULL;
    }

    CONNECTION(mvsocket &socket)
    {
        DEBUG( "CONNECTION(mvsocket &socket)" );
        mConnectionState = StateNone;
        sprintf(mFileType, "" );
        sprintf(mFileName, "" );
        sprintf(mFilePath, "" );
        mFileSize = 0;
        mBytesReceived = 0;
        mChecksum = "";
        mFileHandle = NULL;
        this->mSocket = socket;
    }

    ~CONNECTION()
    {
        DEBUG( "~CONNECTION()" );
        // Ditching this to fix file transfer agent.  Hugh
        //if(IsFilehandleValid())
        //{
        // FileClose();
        //}
        //if(mSocket.IsOpen())
        //{
        // mSocket.Close();
        //}
    }

    const CONNECTION &operator=( const CONNECTION &newconnection )
    {
        this->mSocket = newconnection.mSocket;
        this->mFileHandle = newconnection.mFileHandle;
        this->mFileSize = newconnection.mFileSize;
        this->mBytesReceived = newconnection.mBytesReceived;
        this->mConnectionState = newconnection.mConnectionState;
        this->mChecksum = newconnection.mChecksum;
        strcpy( this->mFileType, newconnection.mFileType );
        strcpy( this->mFileName, newconnection.mFileName );
        strcpy( this->mFilePath, newconnection.mFilePath );
        return *this;
    }
};

vector<CONNECTION> Connections;
int iNumConnections = 0;
int iNextConnectionRef = 0;

//! Input: sFilename: a filename to validate
//!
//! Returns: True if the filename is safe, false otherwise
//!
//! Description: Checks a proposed filename for length and for
//!  unsafe or otherwise questionable characters
//
// History:
bool ValidateFilename( const char *sFilename )
{
    DEBUG(  "Validating untrusted filename..." ); // DEBUG
    if( strlen( sFilename ) > 32 )
    {
        WARNING(  "Invalid path incoming!  Filename too long.  Dropping connection." ); // DEBUG
        return false;
    }
    if( strstr( sFilename, ".." ) != NULL )
    {
        WARNING(  "Invalid path incoming!  Path contains relative directory.  Dropping connection." << sFilename ); // DEBUG
        return false;
    }
    if( strstr( sFilename, "/" ) != NULL )
    {
        WARNING(  "Invalid path incoming!  Path contains Unix path separator.  Dropping connection." << sFilename ); // DEBUG
        return false;
    }
    if( strstr( sFilename, "\\" ) != NULL )
    {
        WARNING(  "Invalid path incoming!  Path contains DOS path separator.  Dropping connection." << sFilename ); // DEBUG
        return false;
    }
    DEBUG(  "...validated" ); // DEBUG
    return true;
}

//! generates filepath for server-side files portably
//!
//! generates filepath for server-side files
//! subdirectoryname is typically Textures, Scripts, or something like that
//! filename is the actual filename
//! This function is basically to ensure Windows/Linux portability
void GenerateServerDataFilePath( char *targetbuffer, const char *subdirectoryname, const char *filename )
{
#ifdef _WIN32
    snprintf( targetbuffer, 1024, ".\\ServerData\\%s\\%s", subdirectoryname, filename );
#else // Linux

    snprintf( targetbuffer, 1024, "./ServerData/%s/%s", subdirectoryname, filename );
#endif
}

//! Input: None
//!
//! Returns: None
//!
//! Description: Block until data is available on a read socket, or for one second
//
// History: 20050330 Mark Wagner - Converted to use the mvsocket class as a class
void SocketsBlockTillSomethingHappens()
{
    vector<CONNECTION>::iterator i;
    vector<const mvsocket *> readsocks;

    readsocks.clear();

    if( MetaverseServerSocket.IsOpen() )
    {
        //     DEBUG( "select: adding socket for MetaverseServerSocket" );
        readsocks.push_back( &MetaverseServerSocket );
    }
    //  DEBUG( "ClientFileAgentListener socket: " << ClientFileAgentListener.GetSocket() );
    if( ClientFileAgentListener.IsOpen() )
    {
        //     DEBUG( "select: adding socket for ClientFileAgentListener" );
        readsocks.push_back( &ClientFileAgentListener);
    }
    //  DEBUG( "socket in vector : " << readsocks.front()->GetSocket() );
    //  DEBUG( "socket in vector : " << readsocks.back()->GetSocket() );
    //  DEBUG( "ClientFileAgentListener socket: " << ClientFileAgentListener.GetSocket() );
    vector< const CONNECTION * > SocketsToClose;
    for( i = Connections.begin(); i != Connections.end(); i++ )
    {
        if( i->GetSocket().IsOpen() )
        {
            //       DEBUG( "select: adding connected client socket " << i->GetSocket().GetSocket() );
            readsocks.push_back( &( i->GetSocket() ) );
        }
        else
        {
            INFO( "Removing connection from vector" );
            i->Close();
            i = Connections.erase(i);
            i--;
        }
    }

    SocketsReadBlock(10000, readsocks);
    //Debug("wait finished\n" );
}

void ShowCurrentConnections()
{
    Debug( "Current sockets:\n" );
    vector<CONNECTION>::iterator i;
    for( i = Connections.begin(); i != Connections.end(); i++ )
    {
        const mvsocket &thissocket = (*i).GetSocket();
        DEBUG( "Connection state=" << (*i).GetState() << "  socket " << thissocket.GetSocket() << " isopen: " << thissocket.IsOpen() );

        //      Debug( "socket[%i]\n", Connections[i].thissocket.socket );
        //  TODO: Figure out a good way to do this
    }
}

void RandomSelectCheckForDebugging( const SOCKET &socket, char * comment )
{
    DEBUG( "" );
    DEBUG( "RandomSelectCheckForDebugging() " << comment );

    fd_set ReadTargetSet;
    FD_ZERO( &ReadTargetSet );
    FD_SET( socket, &ReadTargetSet);
    int result = select( 0, &ReadTargetSet, NULL, NULL, NULL );
    cout << result << endl;
    if( result == SOCKET_ERROR )
    {
#ifdef _WIN32
        INFO("ERROR: " << WSAGetLastError() );
#endif

    }
    else
    {
        INFO("PASS" );
    }
}

void CheckForNewClients()
{
    if( ClientFileAgentListener.NewConnectionAvailable() )
    {
        INFO( "New connection available.  Accepting..." );

        mvsocket AcceptedSocket;
        AcceptedSocket = ClientFileAgentListener.AcceptNewConnection();

        // RandomSelectCheckForDebugging( AcceptedSocket.GetSocket(), "CheckForNewClients a" );

        if( AcceptedSocket.IsOpen() )
        {
            CONNECTION newconnection = CONNECTION(AcceptedSocket);
            // RandomSelectCheckForDebugging( AcceptedSocket.GetSocket(), "CheckForNewClients bblah" );
            Connections.push_back( newconnection );
            // RandomSelectCheckForDebugging( AcceptedSocket.GetSocket(), "CheckForNewClients b" );
            Connections.back().SetState(CONNECTION::StateNewConnection);
            //iNextConnectionRef++;
            //  SendCurrentData( iNumConnections );
        }
        else
        {
            Debug( "Error opening socket :-/" );
        }

        // RandomSelectCheckForDebugging( AcceptedSocket.GetSocket(), "CheckForNewClients c" );

        ShowCurrentConnections();
        //      INFO( "blocking till something happens, as test" );
        //     SocketsBlockTillSomethingHappens();
        //    INFO( "block ended" );
    }
}

//! Input: None
//!
//! Returns: None
//!
//! Description: For each connection, if it's a "send" connection, send up to 1200 bytes of data
//!  to the server.
//
// History: 20050330 Mark Wagner - converted to use Connections as a vector
//                               - converted to use CONNECTION as a class
void ProcessDataSendConnections()
{
    int BytesRead, i;
    char buffer[1025];
    vector<CONNECTION>::iterator Connection;
    for( Connection = Connections.begin(), i = 0; Connection != Connections.end(); Connection++, i++ )
    {
        if( Connection->GetState() == CONNECTION::StateSendingData )
        {
            DEBUG(  "Processing connection " << i ); // DEBUG
            if( !Connection->IsFilehandleValid() )
            {
                if( ValidateFilename( Connection->GetFilename() ) )
                {
                    if( strcmp( Connection->GetFiletype(), "TEXTURE" ) == 0 )
                    {
                        GenerateServerDataFilePath( buffer, "textures", Connection->GetFilename() );
                        Connection->SetFilepath(buffer);
                    }
                    else if( strcmp( Connection->GetFiletype(), "SCRIPT" ) == 0 )
                    {
                        GenerateServerDataFilePath( buffer, "scripts", Connection->GetFilename() );
                        Connection->SetFilepath(buffer);
                    }
                    else if( strcmp( Connection->GetFiletype(), "TERRAIN" ) == 0 )
                    {
                        GenerateServerDataFilePath( buffer, "terrains", Connection->GetFilename() );
                        Connection->SetFilepath(buffer);
                    }
                    else if( strcmp( Connection->GetFiletype(), "MESHFILE" ) == 0 )
                    {
                        GenerateServerDataFilePath( buffer, "meshes", Connection->GetFilename() );
                        Connection->SetFilepath(buffer);
                    }
                    else
                    {
                        DEBUG(  "unknown type " << Connection->GetFiletype() ); // DEBUG
                        Connection->Close();
                        Connection = Connections.erase(Connection);
                        Connection--;
                        DEBUG( "socket closed, connection removed" ); // DEBUG
                        break;
                    }
                    Connection->FileOpen("rb");
                    DEBUG( "Setting up 'send' connection for file " << Connection->GetFilepath());
                    if( !Connection->IsFilehandleValid() )
                    {
                        Debug( "could not open file %s\n", Connection->GetFilepath() );
                        Connection->Close();
                        Connection = Connections.erase(Connection);
                        Connection--;
                        DEBUG(  "socket closed, connection removed" ); // DEBUG
                        break;
                    }
                }
                else
                {
                    DEBUG(  "Filename not validated -> dropping connection" ); // DEBUG
                    Connection->Close();
                    Connection = Connections.erase(Connection);
                    Connection--;
                    break;
                }
            }

            DEBUG(  "reading file if open..." ); // DEBUG
            if( Connection->IsFilehandleValid() )
            {
                DEBUG(  "File open, reading..." ); // DEBUG
                BytesRead = Connection->FileRead( ReadBuffer, 1200 );
                if( Connection->FileEOF() )
                {
                    Debug( "File %s completely read, sending remainder of data, then closing socket...\n", Connection->GetFilename() );
                    Connection->Send(ReadBuffer, BytesRead);
                    Connection->Close();
                    Connection = Connections.erase(Connection);
                    Connection--;
                }
                else if( Connection->FileError() )
                {
                    Debug( "File %s read ERROR , closing socket...\n", Connection->GetFilename() );
                    Connection->Close();
                    Connection = Connections.erase(Connection);
                    Connection--;
                }
                else
                {
                    if( Connection->Send( ReadBuffer, BytesRead ) == SOCKET_ERROR )
                    {
                        Debug( "socket connection file %s closed by remote host\n", Connection->GetFilename() );
                        Connection = Connections.erase(Connection);
                        Connection--;
                    }
                }
            }
        }
    }
}

//! Input: None
//!
//! Returns: None
//!
//! Description: For each connection, if it's a "receive" connection, get up to 4096 bytes of data
//!  from the server.
//
// History: 20050330 Mark Wagner - converted to use Connections as a vector
//                               - converted to use CONNECTION as a class
//          20050413 Mark Wagner - modified to leave the socket open on success
void ProcessDataReceiveConnections()
{
    int BytesRead;
    char buffer[1025];
    vector<CONNECTION>::iterator Connection;
    for( Connection = Connections.begin(); Connection != Connections.end(); Connection++ )
    {
        if( Connection->GetState() == CONNECTION::StateReceivingData )
        {
            if( Connection->DataAvailable() )
            {
                if( !Connection->IsFilehandleValid() )
                {
                    if( ValidateFilename( Connection->GetFilename() ) )
                    {
                        if( strcmp( Connection->GetFiletype(), "TEXTURE" ) == 0 )
                        {
                            GenerateServerDataFilePath( buffer, "textures", Connection->GetFilename() );
                            Connection->SetFilepath(buffer);
                        }
                        else if( strcmp( Connection->GetFiletype(), "SCRIPT" ) == 0 )
                        {
                            GenerateServerDataFilePath( buffer, "scripts", Connection->GetFilename() );
                            Connection->SetFilepath(buffer);
                        }
                        else if( strcmp( Connection->GetFiletype(), "TERRAIN" ) == 0 )
                        {
                            GenerateServerDataFilePath( buffer, "terrains", Connection->GetFilename() );
                            Connection->SetFilepath(buffer);
                        }
                        else if( strcmp( Connection->GetFiletype(), "MESHFILE" ) == 0 )
                        {
                            GenerateServerDataFilePath( buffer, "meshes", Connection->GetFilename() );
                            Connection->SetFilepath(buffer);
                        }
                        else
                        {
                            DEBUG(  "unknown type " << Connection->GetFiletype() ); // DEBUG
                        }

                        DEBUG(  "opening file " << Connection->GetFilename() << " at " << Connection->GetFilepath()); // DEBUG
                        if( Connection->FileOpen( "wb" ) )
                        {
                            DEBUG( "file opened ok" );
                        }
                    }
                    else
                    {
                        DEBUG(  "WARNING: invalid path incoming!  Dropping connection..." ); // DEBUG
                        Connection->Close();
                        Connection = Connections.erase(Connection);
                        Connection--;
                        continue;
                    }
                }

                if( Connection->IsFilehandleValid() )
                {
                    BytesRead = Connection->Receive( ReadBuffer, 4096 );

                    if( BytesRead == SOCKET_ERROR )
                    {
                        WARNING(  "Read error, aborting transfer of " << Connection->GetFilename() ); // DEBUG
                        Connection->FileClose();
                        try
                        {
                            Connection->Close();
                        }
                        catch( ... )
                        {
                            DEBUG(  "exception caught closing socket" ); // DEBUG
                        }
                        Connection = Connections.erase(Connection);
                        Connection--;
                        continue;
                    }
                    else if( BytesRead > 0 )
                    {
                        DEBUG(  "writing " << BytesRead << " bytes to file, total " << Connection->GetBytesReceived() ); // DEBUG
                        Connection->FileWrite( ReadBuffer, BytesRead );
                    }
                    else if( BytesRead == 0 )
                    {
                        DEBUG( "Zero bytes received.  Socket closed?" );
                        Connection->Close();
                        Connection->FileClose();
                        Connection->SetState( CONNECTION::StatePendingValidation );
                    }
                    else
                    {
                        ERRORMSG( "Weird condition " << BytesRead << " not handled in FileTransGetFile; closing connection" );
                        Connection->FileClose();
                        Connection->Close();
                        Connection = Connections.erase(Connection);
                        Connection--;
                    }
                    if( Connection->GetBytesReceived() >= Connection->GetFilesize() )
                    {
                        DEBUG(  "File received completely. " << Connection->GetBytesReceived() << " bytes of " << Connection->GetFilesize() << " closing socket..." ); // DEBUG
                        Connection->FileClose();
                        Connection->SetState( CONNECTION::StatePendingValidation );
                    }
                }
                else
                {
                    ERRORMSG( "ERROR: invalid file handle");
                }
            }
        }
    }
}

//! Input: None
//!
//! Returns: None
//!
//! Description:
//
// History: 20050330 Mark Wagner - Converted to use Connections as a vector
//                               - converted to use CONNECTION as a class
void ProcessReceivedFiles()
{
    vector<CONNECTION>::iterator Connection;
    for( Connection = Connections.begin(); Connection != Connections.end(); Connection++ )
    {
        if( Connection->GetState() == CONNECTION::StatePendingValidation )
        {
            string ChecksumOfReceivedFile;
            ChecksumOfReceivedFile = GenerateCheckString( Connection->GetFilepath() );

            ostringstream resultstream;
            DEBUG("Checksum " << ChecksumOfReceivedFile);
            if( Connection->ValidateChecksum( ChecksumOfReceivedFile ) )
            {
                ostringstream servermessagestream;
                servermessagestream << "<newfileupload type=\"" << Connection->GetFiletype() << "\" sourcefilename=\"" << Connection->GetFilename()
                << "\" serverfilename=\"" << Connection->GetFilename() << "\" checksum=\"" << Connection->GetChecksum() << "\"/>" << endl;
                DEBUG(  "sending to server " << servermessagestream.str() ); // DEBUG
                MetaverseServerSocket.Send( servermessagestream.str().c_str() );
                resultstream << "<fileuploadsuccess type=\"" << Connection->GetFiletype() << "\" sourcefilename=\"" << Connection->GetFilename()
                << "\" checksum=\"" << Connection->GetChecksum() << "\"/>" << endl;
            }
            else
            {
                resultstream << "<fileuploadfail type=\"" << Connection->GetFiletype() << "\" sourcefilename=\"" << Connection->GetFilename()
                << "\" checksum=\"" << Connection->GetChecksum() << "\"/>" << endl;
            }
            DEBUG(  "sending to client " << resultstream.str() ); // DEBUG
            Connection->Send( resultstream.str().c_str() );
            DEBUG(  "(closing socket)" ); // DEBUG
            Connection->Close();
            //   Connections[ iNumConnections - 1 ].CopyTo( Connections[i] );
            //   i--;
            Connection = Connections.erase(Connection);
            Connection--;
        }
    }
}

//! TODO: Attempt to re-establish contact
void CheckServerAlive()
{
    //DEBUG(  "checking server alive..." ); // DEBUG
    if( MetaverseServerSocket.IsOpen() )
    {
        if( MetaverseServerSocket.ReceiveLineIfAvailable( ReadBuffer ) == SOCKETS_READ_SOCKETGONE )
        {
            DEBUG(  "Server connection gone.  Dieing..." ); // DEBUG
            mvSystem::mvExit(1);
        }
    }
}

//! Input: None
//!
//! Returns: None
//!
//! Description:
//
// History: 20050331 Mark Wagner - Converted to use Connections as a vector
//                               - Converted to use CONNECTION as a class
void ProcessNewConnections()
{
    //INFO( "blocking till something happens, as test" );
    //SocketsBlockTillSomethingHappens();
    //INFO( "block ended" );

    int i;
    int bytesRecv = 0;
    char buffer[1025];

    vector<CONNECTION>::iterator Connection;
    for( Connection = Connections.begin(), i = 0; Connection != Connections.end(); Connection++, i++ )
    {
        if( Connection->GetState() == CONNECTION::StateNewConnection )
        {
            INFO( "processing connection for socket " << Connection->GetSocket().GetSocket() );
            strcpy( ReadBuffer, "" );
            int ReadResult = Connection->ReceiveLineIfAvailable( ReadBuffer );
            if( ReadResult == SOCKETS_READ_OK )
            {
                Debug( "client Read buffer [%.2048s]\n", ReadBuffer );

                TiXmlDocument IPC;
                IPC.Parse( ReadBuffer );
                if( strcmp( IPC.RootElement()->Value(), "loadergetfile" ) == 0 )
                {
                    snprintf( buffer, 1024, "%.33s", IPC.RootElement()->Attribute("serverfilename") );
                    Connection->SetFilename(buffer);
                    snprintf( buffer, 1024, "%.16s", IPC.RootElement()->Attribute("type") );
                    Connection->SetFiletype(buffer);
                    //     Connections[i].FileHandle = NULL;
                    Connection->SetState(CONNECTION::StateSendingData);
                }
                else if( strcmp( IPC.RootElement()->Value(), "loadersendfile" ) == 0 )
                {
                    snprintf( buffer, 1024, "%.33s", IPC.RootElement()->Attribute("sourcefilename") );
                    Connection->SetFilename(buffer);
                    snprintf( buffer, 1024, "%.16s", IPC.RootElement()->Attribute("type") );
                    Connection->SetFiletype(buffer);
                    /*     char sFilesize[33] = "";
                         sprintf( sFilesize, "%.32s", IPC.RootElement()->Attribute("filesize") );
                         Connections[i].iFileSize = atoi( sFilesize );
                         Connections[i].iBytesReceived = 0;*/

                    Connection->SetFilesize(atoi(IPC.RootElement()->Attribute("filesize")));
                    Connection->SetBytesReceived(0);

                    DEBUG(  "Filesize anticipated: " << Connection->GetFilesize() ); // DEBUG
                    /*     char sChecksum[33] = "";
                         sprintf( sChecksum, "%.32s", IPC.RootElement()->Attribute("checksum") );
                         Connections[i].Checksum = sChecksum; */
                    Connection->SetChecksum(IPC.RootElement()->Attribute("checksum"));
                    Connection->SetState(CONNECTION::StateReceivingData);
                }
            }
            else if( ReadResult == SOCKETS_READ_SOCKETGONE )
            {
                printf( "Client %i disconnected\n", i );
                //    Connections[ iNumConnections - 1 ].CopyTo( Connections[i] );
                //    i--;
                Connection = Connections.erase(Connection);
                Connection--;
                DEBUG("Connection removed");
            }
        }
    }
}

int NumDataClients()
{
    int iNumDataClients = 0;
    for( int i = 0; i < Connections.size(); i++ )
    {
        if( Connections[i].GetState() == CONNECTION::StateSendingData )
        {
            iNumDataClients++;
        }
    }
    return iNumDataClients;
}

int main( int argc, char *argv[] )
{
    mvsocket::InitSocketSystem();
    //printf( "Connecting to Metaverse Server on port %i...\n", iMetaverseServerPort );
    //SocketMetaverseServer.ConnectToServer( inet_addr( "127.0.0.1" ), iMetaverseServerPort );
    //printf( "Connected to Metaverse Server.\n" );

    bool bStandalone = false;
    if( argc >= 2 )
    {
        if( strcmp( argv[1], "--standalone" ) == 0 )
        {
            bStandalone = true;
            INFO( "Activating mode --standalone; we wont connect to server component." );
        }
    }

    printf( "Creating Metaverse Client listener on port %i...\n", iClientFileAgentListenPort );
    ClientFileAgentListener.Init();
    ClientFileAgentListener.Listen( INADDR_ANY, iClientFileAgentListenPort );

    if( !bStandalone )
    {
        DEBUG(  "connecting to server on port " << iMetaverseServerPort ); // DEBUG
        MetaverseServerSocket.ConnectToServer( inet_addr("127.0.0.1"), iMetaverseServerPort );
        printf( "Initialization complete\n\n" );
    }

    while(1)
    {
        try
        {
            //  DEBUG("1");
            if( NumDataClients() == 0 )
            {
                SocketsBlockTillSomethingHappens();
            }
            //  DEBUG("2");
            CheckServerAlive();
            //    DEBUG("3");
            CheckForNewClients();
            /// DEBUG("4");
            ProcessDataSendConnections();
            // DEBUG("5");
            ProcessDataReceiveConnections();
            // DEBUG("6");
            ProcessReceivedFiles();
            // DEBUG("7");
            ProcessNewConnections();
        }
        catch( ... )
        {
            DEBUG(  "Warning! Unknown exception!" ); // DEBUG
        }
    }

    return 0;
}


