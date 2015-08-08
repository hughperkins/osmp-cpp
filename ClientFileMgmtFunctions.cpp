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
//! \brief ClientFileFunctionsClass is the base class for classes that manage file downloading and uploading files
// See headerfile "clientfilemgmtfunctions.h" for documentation

// Modified 20050415 Mark Wagner - Uses wxFileName for path handling

#include <string>
#include <iostream>
#include <sstream>
using namespace std;

#include <wx/filename.h>
#include <wx/strconv.h>

#include "WorldStorage.h"
#include "Selection.h"
#include "SocketsClass.h"
#include "RendererTexturing.h"
#include "TerrainInfoCache.h"
#include "ClientFileMgmtFunctions.h"
#include "Checksum.h"
#include "File.h"
#include "MetaverseClient.h"
#include "Config.h"

namespace MetaverseClient
{
    // extern FileInfoCacheClass FileInfoCache;
    //extern mvsocket MetaverseClientSocket;  // we should probably get rid of these nasty externs at some point.
    extern mvSelection Selector;            // but at least the namespaces are encapsulated so it's not *really* bad
    extern mvWorldStorage World;
    extern ConfigClass mvConfig;
}
using namespace MetaverseClient;

//extern void SendClientMessage( const char *message );

// Input: rFileInfo: A FILEINFO struct
//
// Returns: True if the file is cached, false otherwise
//
// Description: Checks to see if the file is in the local cache or not
//
// Thread safety: Mostly.  There's a race condition where a file download
//  starts after the function checks for the existance of the file, but
//  before the function returns.
//
// History: 20050415 Mark Wagner - Modified to use wxFileName for path handling
bool ClientFileFunctionsClass::FileDownloaded( FILEINFO &rFileInfo )
{
    DEBUG(  "checking if file downloaded..."); // DEBUG

    // char sFilePath[255];
    // sprintf( sFilePath, ".\\clientdata\\cache\\%ss\\%s", rFileInfo.sType.c_str(), rFileInfo.sServerFilename.c_str() );

    wxFileName FilePath;
    FilePath = wxString(rFileInfo.sServerFilename.c_str(), wxConvUTF8);
    DEBUG("Getting subdirectory name..." );
    const char *DirectoryName = GetDirectoryName();
    DEBUG("Converting to wxString..." );
    const wxString &wxstringpath = wxString(DirectoryName, wxConvUTF8);
    DEBUG("prepending to path" );
    FilePath.PrependDir(wxstringpath);
    FilePath.PrependDir(L"cache");
    FilePath.PrependDir(L"clientdata");
    FilePath.Normalize( wxPATH_NORM_ALL, wxString( mvConfig.CacheDirectory.c_str()  ));

    DEBUG(  "File path to check is: [" << FilePath.GetFullPath().mb_str() << "]" ); // DEBUG

    bool bIsDownloaded = FileExists( FilePath.GetFullPath().mb_str() );

    if( bIsDownloaded )
    {
        rFileInfo.bFilePresent = true;
        rFileInfo.sOurFilePath = FilePath.GetFullPath().mb_str();
        DEBUG(  "file downloaded ok " << rFileInfo.sOurFilePath ); // DEBUG
    }
    else
    {
        DEBUG(  "... not yet" ); // DEBUG
    }

    return bIsDownloaded;
}

void ClientFileFunctionsClass::DownloadFile( FILEINFO &rFileInfo )
{
    ostringstream messagetoclientfileagentstream;
    messagetoclientfileagentstream << "<loadergetfile type=\"" << rFileInfo.sType << "\" checksum=\"" << rFileInfo.sChecksum
    << "\" sourcefilename=\"" << rFileInfo.sSourceFilename << "\" serverfilename=\"" << rFileInfo.sServerFilename
    << "\"/>" << endl;
    DEBUG(  "sending to clientfileagent " << messagetoclientfileagentstream.str() ); // DEBUG
    //endererMain::SendClientMessage( messagetoclientfileagentstream.str().c_str() );
    TiXmlDocument IPC;
    IPC.Parse( messagetoclientfileagentstream.str().c_str() );
    //MetaverseClientSocket.Send( message.c_str() );
    MetaverseClient::SendXMLDocToFileAgent( IPC );
}

void ClientFileFunctionsClass::LoadOrRequestFile( FILEINFO &rFileInfo )
{
    if( !FileDownloaded( rFileInfo ) )
    {
        DownloadFile( rFileInfo );
    }
}

void ClientFileFunctionsClass::UploadFile( string FilePath, string sType )
{
    DEBUG(  "uploadfile " << FilePath ); // DEBUG
    string CheckSum = GenerateCheckString( FilePath );
    if( !pFileInfoCache->CheckIfChecksumPresent( CheckSum ) )
    {
        ostringstream messagestream;
        messagestream << "<loadersendfile type=\"" << sType << "\" path=\"" << FilePath << "\" checksum=\"" << CheckSum + "\"/>" << endl;
        string message = messagestream.str();
        DEBUG(  "sending to clientfileagent " << message );
        //RendererMain::SendClientMessage( message.c_str() );
        //  SetNewFile( FilePath, CheckSum );
        TiXmlDocument IPC;
        IPC.Parse( messagestream.str().c_str() );
        //MetaverseClientSocket.Send( message.c_str() );
        MetaverseClient::SendXMLDocToFileAgent( IPC );
    }
}

void ClientFileFunctionsClass::RegisterFileFromXMLString( const char *XMLString )
{
    TiXmlDocument IPC;
    IPC.Parse( XMLString );
    RegisterFileFromXML( IPC.RootElement() );
}

void ClientFileFunctionsClass::RegisterFileFromXML( TiXmlElement *pElement )
{
    DEBUG(  "RegisterFileFromXML" ); // DEBUG
    FILEINFO &rFileInfo = pFileInfoCache->AddFileInfoFromXML( pElement );
    //LoadOrRequestFile( FileManager.Files.find( pElement->Attribute("checksum") )->second );
    LoadOrRequestFile( rFileInfo );
}

