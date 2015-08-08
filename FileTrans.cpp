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
//! \brief helper functions for client file agent
// see headerfile for documentation

// 20050409 Mark Wagner - Made thread-safe, modified so all error messages
//  use the DEBUG framework, vastly improved the error-handling
// 20050416 Hugh Perkins - renamed ERROR to ERRORMSG (naming conflict on Windows)

#include <stdio.h>
#include <iostream>
using namespace std;

#include "FileTrans.h"
#include "SocketsClass.h"

// Input: sockettouse: a pointer to a socket to read data from
//        sFilePath: the absolute path to the file to write data to
//
// Returns: True if the file was fully read, false otherwise
//
// Description: Reads a file from the specified socket, and writes
//  it to the file specified in "sFilePath", in chunks of 4096 bytes
//  per pass.  The socket remains open if the file was read successfully;
//  if an error occurred, the socket state is undefined; the file will
//  contain a partial file.
//
// Thread safety: Function is thread-safe, assuming that mvsocket and the
//  file-access calls are.
//
// History: 20050409 Mark Wagner - made thread-safe
bool FileTransGetFile( mvsocket *sockettouse, const char *sFilePath )
{
	char ReadBuffer[ 4097 ];
	FILE *fIncomingFile = NULL;
	bool bFileOpen = false;
	bool result = false;
	int BytesRecv;
	
	// Sanity-check input
	if( NULL == sockettouse || NULL == sFilePath )
	{
		ERRORMSG("Invalid parameter in FileTransGetFile");
		return false;
	}

	fIncomingFile = fopen( sFilePath, "wb" );
	if(NULL == fIncomingFile)
	{
		ERRORMSG("Receive error: Unable to open file " << sFilePath << " for writing");
		return false;
	}
	
	bFileOpen = true;
	DEBUG( "Beginning transfer of file " << sFilePath );
	
	while( bFileOpen )
	{
		BytesRecv = sockettouse->Receive( ReadBuffer, 4096 );

		if( BytesRecv == SOCKET_ERROR )
		{
			// A transfer error occurred
			ERRORMSG( "Transfer error " << strerror(errno) );
			bFileOpen = false;
			result = false;
		}
		else if( BytesRecv > 0 )
		{
			// Got data
			fwrite( ReadBuffer, sizeof( char ), BytesRecv, fIncomingFile );
		}
		else if( BytesRecv == 0 )
		{
			// Connection cleanly closed; assume a successful transfer
			bFileOpen = false;
			result = true;
		}
		else
		{
			// Unknown error
			ERRORMSG( "Unknown socket result " << BytesRecv << " errno " << strerror(errno) );
			bFileOpen = false;
			result = false;
		}
	}
	fclose( fIncomingFile );
	DEBUG( "Transfer finished" ); // DEBUG
	return result;
}	
   
// Input: sockettouse: a pointer to an mvsocket to send data on
//        sFilePath: a string containing the path of the file to send
//
// Returns: True if the file was sent successfully, false otherwise
//
// Description: Sends the file "sFilePath" to whatever's on the other end
//  of "sockettouse" at a rate of 1200 bytes per pass.  The socket remains
//  open when the function returns, assuming no errors occurred.  In the
//  event of a transfer error, the state of the socket is undefined.
//
// Thread safety: thread-safe, assuming that mvsocket and the 
//  file-access functions are.
//
// History: 20050409 Mark Wagner - Modified to return success/failure
//                               - Made thread-safe
//          20050413 Mark Wagner - Modified to handle errors when sending the end of the file
bool FileTransSendFile( mvsocket *sockettouse, const char *sFilePath )
{
	size_t bytes =  0;
	ssize_t bytes_sent = 0;
	bool result = false;
	char ReadBuffer[ 4097 ];
	FILE *fOutgoingFile;

	// Sanity-check input
	if( NULL == sockettouse || NULL == sFilePath )
	{
		ERRORMSG("Invalid parameter in FileTransSendFile");
		return false;
	}

	fOutgoingFile = fopen( sFilePath, "rb" );
	if( fOutgoingFile == NULL )
	{
		DEBUG(  "couldnt open file " << sFilePath ); // DEBUG
		return false;
	}

	bool bFileOpen = true;

	DEBUG("Beginning transfer for file " << sFilePath );

	int BytesRead;
	while( bFileOpen )
	{
		BytesRead = fread( ReadBuffer, sizeof( char), 1200, fOutgoingFile );

		if( feof( fOutgoingFile ) != 0 )
		{
			bFileOpen = false;
			DEBUG( "File " << sFilePath << " completely read, sending remainder of data" );
			bytes_sent = sockettouse->Send( ReadBuffer, BytesRead );
			if(SOCKET_ERROR == bytes_sent)
			{
				result = false;
			}
			else
			{
				result = true;
			}
		}
		else if( ferror( fOutgoingFile ) != 0 )
		{
			ERRORMSG( "Read error " << strerror(errno) << " reading file " << sFilePath );
//			sockettouse.Close();
			result = false;
			bFileOpen = false;
		}
		else
		{
			bytes_sent = sockettouse->Send( ReadBuffer, BytesRead );
			if( SOCKET_ERROR == bytes_sent )
			{
				ERRORMSG( "Socket transfer error " << strerror(errno) << " on file " << sFilePath );
				result = false;
				bFileOpen = false;
			}
		}
		bytes += bytes_sent;
		DEBUG(bytes << " bytes sent" );

	}
	fclose( fOutgoingFile );
	if(result)
		DEBUG(  "File " << sFilePath << " sent ok " ); // DEBUG
	return result;
}	
   
