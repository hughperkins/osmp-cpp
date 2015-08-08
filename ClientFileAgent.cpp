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
//! \brief responsible for downloading files to the client on demand, and for uploading them to the server
//!
//! responsible for downloading files to the client on demand, and for uploading them to the server
//! dialogs with serverfileagent, and with metaverseclient

// Modified 20050409 Mark Wagner - uses wxBase for filename manipulation
//  Made thread-safe for sending
// Modified 20050410 - Began UTF-8 support

#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
using namespace std;

#include <wx/filename.h>
#include <wx/strconv.h>

#include "tinyxml.h"

#include "SocketsClass.h"
#include "FileTrans.h"
#include "File.h"
#include "port_list.h"
#include "Config.h"

//#define BUFSIZE 2047
//char SendBuffer[ BUFSIZE + 1 ];
//extern char ReadBuffer[ 4097 ];

mvsocket SocketServerFileAgent;
mvsocket MetaverseClientSocket;

//int iServerPort = 0;
//int iMetaverseClientPort = 22169;

ConfigClass mvConfig;
    
//char sMetaverseServerIP[64];

void TellClientFailed( const char *sType, const char *sFilename )
{
	  printf( "file load failed\n" );
}

void TellClientWereDone( const char *sType, const char *sFilename )
{
	  printf( "file loaded\n" );
}

bool GetFileItselfFromServer( mvsocket *RecvSocket, const char *sType, const char *sLocalFilename, const char *sFilename, const char *sFullFilePath )
{
	wxFileName FilePath;
	
	if( NULL == sFullFilePath )
	{
		FilePath = wxString(sLocalFilename, wxConvUTF8);
		if(strcmp( sType, "TEXTURE" ) == 0)
		{
			FilePath.PrependDir(L"textures");
		}
		else if( strcmp( sType, "TERRAIN" ) == 0 )
		{
			FilePath.PrependDir(L"terrains");
		}
		else if( strcmp( sType, "MESHFILE" ) == 0 )
		{
			FilePath.PrependDir(L"meshes");
		}
		else if( strcmp( sType, "SCRIPT" ) == 0 )
		{
			FilePath.PrependDir(L"scripts");
		}
		else
		{
			WARNING("GetFileItselfFromServer(): Unknown file type " << sType);
			return false;
		}
		
		FilePath.PrependDir(L"cache");
		FilePath.PrependDir(L"clientdata");
	}
	else
	{
		FilePath = wxString(sFullFilePath, wxConvUTF8);
	}
	FilePath.Normalize( wxPATH_NORM_ALL, wxString( mvConfig.CacheDirectory.c_str()  ));
	DEBUG(  "launching filetransgetfile..." ); // DEBUG
	if( FileTransGetFile( RecvSocket, FilePath.GetFullPath().mb_str() ) )
	{
		return true;
	}
	else
	{
		return false;
	}

}
   
bool SendFile( mvsocket *SendSocket, wxFileName *FileName, TiXmlElement *pElement)
{
	int iFileSize;
	iFileSize = GetFileSize( FileName->GetFullPath().mb_str() );

	if( iFileSize <= 0 )
	{
		DEBUG(  "problem opening file " << FileName->GetFullPath().mb_str() ); // DEBUG
		return false;
	}

	ostringstream messagestream;
	messagestream << "<loadersendfile type=\"" << pElement->Attribute("type" ) << "\" sourcefilename=\"" << FileName->GetFullName().mb_str() 
		<< "\" checksum=\"" << pElement->Attribute("checksum" ) << "\" filesize=\"" << iFileSize << "\" />" << endl;
	DEBUG(  "sending to server " << messagestream.str() ); // DEBUG
	SendSocket->Send( messagestream.str().c_str() );

	return FileTransSendFile( SendSocket, FileName->GetFullPath().mb_str() );	
}	

// Input: pIPC: a pointer to the load request
//
// Returns: None
//
// Description: Retrieves a file from the metaverse server, saving it
//  in the specified file.  Sends a message to the metaverse client
//  indicating success or failure.
//
// Thread safety: Unknown
//
// History:   
void LoadFile( TiXmlDocument *pIPC )
{
	bool iResult = false;
	int iServerPort;
	mvsocket *RecvSocket = new mvsocket;
	
//	sprintf( sMetaverseServerIP, pIPC->RootElement()->Attribute("serverip") );
//	iServerPort = atoi( pIPC->RootElement()->Attribute("serverport") );

	RecvSocket->Init();
	RecvSocket->ConnectToServer( inet_addr( pIPC->RootElement()->Attribute("serverip") ), atoi( pIPC->RootElement()->Attribute("serverport") ) );   

	if( !RecvSocket->IsOpen() )
	{
		iResult = false;
		DEBUG(  "failed to connect to server" ); // DEBUG
	}
	else
	{

		DEBUG(  "connected to server" ); // DEBUG

		ostringstream messagestream;
		messagestream << "<loadergetfile type=\"" << pIPC->RootElement()->Attribute("type" ) << "\" "
		"sourcefilename=\"" << pIPC->RootElement()->Attribute("sourcefilename" ) << "\""
		<< " checksum=\"" << pIPC->RootElement()->Attribute("checksum" ) << "\" serverfilename=\"" << pIPC->RootElement()->Attribute("serverfilename" )
		<< "\"/>" << endl;
		DEBUG(  "sending to server " << messagestream.str() ); // DEBUG
		RecvSocket->Send( messagestream.str().c_str() );

		if( pIPC->RootElement()->Attribute("ourlocalfilepath") == NULL )
		{
			iResult = GetFileItselfFromServer( RecvSocket, pIPC->RootElement()->Attribute("type" ), pIPC->RootElement()->Attribute("sourcefilename"), pIPC->RootElement()->Attribute("serverfilename" ), NULL );
		}
		else
		{
			iResult = GetFileItselfFromServer( RecvSocket, pIPC->RootElement()->Attribute("type" ), pIPC->RootElement()->Attribute("sourcefilename"), pIPC->RootElement()->Attribute("serverfilename" ), pIPC->RootElement()->Attribute("ourlocalfilepath") );
		}
	}
	ostringstream messagetoclientstream;
	DEBUG(  "before setvalue on root " << *pIPC ); // DEBUG
	pIPC->RootElement()->SetValue( "loaderfiledone" );
	DEBUG(  " after setvalue on root " << *pIPC ); // DEBUG
	if( iResult == true )
	{
		pIPC->RootElement()->SetAttribute("result", "SUCCESS" );
	}
	else
	{
		pIPC->RootElement()->SetAttribute("result", "FAIL" );
	}
	messagetoclientstream << *pIPC << endl;
	DEBUG(  "sending toclient " << messagetoclientstream.str() ); // DEBUG
	MetaverseClientSocket.Send( messagetoclientstream.str().c_str() );
	
	RecvSocket->Close();
	delete RecvSocket;
}

// Input: pIPC: A TinyXML document tree containing the IO request
//
// Returns: True if the file was sent successfully, false otherwise
//
// Description: Parses pIPC to get the details of the request, connects
//  to the metaverse file server, and uploads the file.
//
// Thread safety: unknown
//
// History: 20050410 Mark Wagner - Modified to use wxFileName, beginnings of Unicode support
//          20050413 Mark Wagner - Modified to return success or failure
bool ConnectAndSendFile( TiXmlDocument *pIPC )
{
	bool result = false;
	int iServerPort;
	char ServerReply[4097] = "";
	TiXmlDocument ServerReplyXml;
	
	mvsocket SendSocket;
	wxFileName FileName( wxString(pIPC->RootElement()->Attribute("path"), wxConvUTF8));
	FileName.Normalize();
	
	DEBUG(  "SendFile " ); // DEBUG
	iServerPort = atoi( pIPC->RootElement()->Attribute("serverport") );

	DEBUG(  "Filename is: [" << FileName.GetFullName().mb_str() << "]" ); // DEBUG

	DEBUG(  "connecting to server... " ); // DEBUG
	SendSocket.Init();
	SendSocket.ConnectToServer( inet_addr( pIPC->RootElement()->Attribute("serverip") ),  atoi(pIPC->RootElement()->Attribute("serverport")) );

	if( !SendSocket.IsOpen() )
	{
		DEBUG( "failed to connect to server" ); // DEBUG
		result = false;
	}
	else
	{
		DEBUG( "connected to server." ); // DEBUG

		if(SendFile( &SendSocket, &FileName, pIPC->RootElement() ))
		{
			if(SendSocket.ReceiveLineBlocking(ServerReply) == SOCKETS_READ_OK)
			{			
				ServerReplyXml.Parse(ServerReply);
				if(strcmp(ServerReplyXml.RootElement()->Value(), "fileuploadsuccess") == 0)
				{
					result = true;
				}
				else
				{
					result = false;
				}
			}
			else
			{
				result = false;
			}
		}
		else
		{
			result = false;
		}
		SendSocket.Close();
	}
	
	return result;
}

// Input:
//
// Returns:
//
// Description:
//
// History: 20050409 Mark Wagner: Added support for clean shutdown
int main(int argc, char *argv[])
{
	char ReadBuffer[4096];
	mvConfig.ReadConfig();
	mvsocket::InitSocketSystem();
	MetaverseClientSocket.Init();
	MetaverseClientSocket.ConnectToServer( inet_addr( "127.0.0.1" ), iPortFileTransfer );   

	int ReadResult = 0;

	while( 1 )
	{
		DEBUG( "Waiting for request from metaverseclient ..." );
		ReadResult = MetaverseClientSocket.ReceiveLineBlocking( ReadBuffer );
		if( ReadResult == SOCKETS_READ_OK )
		{
			if( ReadBuffer[0] =='<' )
			{
				Debug( "received xml from metaverseclient [%s]\n", ReadBuffer );
				TiXmlDocument IPC;
				IPC.Parse( ReadBuffer );
				if( strcmp( IPC.RootElement()->Value(), "loadergetfile" ) == 0 )
				{
					LoadFile( &IPC );
				}
				else if( strcmp( IPC.RootElement()->Value(), "loadersendfile" ) == 0 )
				{
					ConnectAndSendFile( &IPC );
				}
				else if( strcmp( IPC.RootElement()->Value(), "shutdown" ) == 0)
				{
					// Need to shut down
					DEBUG("Shutdown request received");
					break;
				}
			}
			else
			{
				DEBUG( "received legacy IPC from metaverseclient [" << ReadBuffer << "]" );
			}
		}
		else if( ReadResult == SOCKETS_READ_SOCKETGONE )
		{
			DEBUG( "Server socket gone.  Shutting down." );
			break;
		}
	}
	// Shut down
	MetaverseClientSocket.Close();
	mvsocket::EndSocketSystem();
	
	return 0;
}
