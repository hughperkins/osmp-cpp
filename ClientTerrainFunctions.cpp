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
//! \brief This module handles terrain objects including upload and download
// See headerfile "ClientTerrainFunctions.h" for documentation

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
#include "ClientTerrainFunctions.h"
#include "Checksum.h"
#include "File.h"
#include "MetaverseClient.h"

namespace MetaverseClient
{
    extern TerrainCacheClass TerrainCache;
    //extern mvsocket MetaverseClientSocket;  // we should probably get rid of these nasty externs at some point.
    extern mvSelection Selector;            // but at least the namespaces are encapsulated so it's not *really* bad
    extern mvWorldStorage World;
    extern ConfigClass mvConfig;
}
using namespace MetaverseClient;

bool ClientTerrainFunctionsClass::TerrainDownloaded( TerrainINFO &rTerrainInfo )
{
    DEBUG(  "checking if terrain downloaded..."); // DEBUG

    wxFileName FilePath;

    FilePath = wxString(rTerrainInfo.sServerFilename.c_str(), wxConvUTF8);
    FilePath.PrependDir(L"terrains");
    FilePath.PrependDir(L"cache");
    FilePath.PrependDir(L"clientdata");
    FilePath.Normalize( wxPATH_NORM_ALL, wxString( mvConfig.CacheDirectory.c_str()  ));

    DEBUG(  "Terrain file is at: " << FilePath.GetFullPath().mb_str() ); // DEBUG

    bool bIsDownloaded = FileExists( FilePath.GetFullPath().c_str() );

    if( bIsDownloaded )
    {
        rTerrainInfo.bFilePresent = true;
        rTerrainInfo.sOurFilePath = FilePath.GetFullPath().c_str();
        DEBUG(  "file downloaded ok " << rTerrainInfo.sOurFilePath ); // DEBUG
    }
    else
    {
        DEBUG(  "... not yet" ); // DEBUG
    }

    return bIsDownloaded;
}

void ClientTerrainFunctionsClass::DownloadTerrain( TerrainINFO &rTerrainInfo )
{
    ostringstream messagetoclientfileagentstream;
    messagetoclientfileagentstream << "<loadergetfile type=\"Terrain\" checksum=\"" << rTerrainInfo.sChecksum
    << "\" sourcefilename=\"" << rTerrainInfo.sSourceFilename << "\" serverfilename=\"" << rTerrainInfo.sServerFilename
    << "\"/>" << endl;
    DEBUG(  "sending to clientfileagent " << messagetoclientfileagentstream.str() ); // DEBUG
    //RendererMain::SendClientMessage( messagetoclientfileagentstream.str().c_str() );
    TiXmlDocument IPC;
    IPC.Parse( messagetoclientfileagentstream.str().c_str() );
    //MetaverseClientSocket.Send( message.c_str() );
    MetaverseClient::SendXMLDocToFileAgent( IPC );
}

void ClientTerrainFunctionsClass::LoadOrRequestTerrain( TerrainINFO &rTerrainInfo )
{
    if( !TerrainDownloaded( rTerrainInfo ) )
    {
        DownloadTerrain( rTerrainInfo );
    }
}
void ClientTerrainFunctionsClass::UploadTerrain( string TerrainPath )
{
    DEBUG(  "uploadterrain " << TerrainPath ); // DEBUG
    string CheckSum = GenerateCheckString( TerrainPath );
    if( !TerrainCache.CheckIfTerrainChecksumPresent( CheckSum ) )
    {
        ostringstream messagestream;
        messagestream << "<loadersendfile type=\"Terrain\" path=\"" << TerrainPath << "\" checksum=\"" << CheckSum + "\"/>" << endl;
        string message = messagestream.str();
        DEBUG(  "sending to clientfileagent " << message );
        //RendererMain::SendClientMessage( message.c_str() );
        //  SetNewTerrain( TerrainPath, CheckSum );
        TiXmlDocument IPC;
        IPC.Parse( messagestream.str().c_str() );
        //MetaverseClientSocket.Send( message.c_str() );
        MetaverseClient::SendXMLDocToFileAgent( IPC );
    }
}

void ClientTerrainFunctionsClass::RegisterTerrainFromXMLString( const char *XMLString )
{
    TiXmlDocument IPC;
    IPC.Parse( XMLString );
    RegisterTerrainFromXML( IPC.RootElement() );
}

void ClientTerrainFunctionsClass::RegisterTerrainFromXML( TiXmlElement *pElement )
{
    DEBUG(  "RegisterTerrainFromXML" ); // DEBUG
    TerrainINFO &rTerrainInfo = TerrainCache.AddTerrainInfoFromXML( pElement );
    //LoadOrRequestTerrain( TerrainManager.Terrains.find( pElement->Attribute("checksum") )->second );
    LoadOrRequestTerrain( rTerrainInfo );
}

void ClientTerrainFunctionsClass::CreateTerrain( const char *sFilePath, const float x, const float y, const float z )
{
    string CheckSum = GenerateCheckString( sFilePath );

    Vector3 terrainpos;
    terrainpos.x = x;
    terrainpos.y = y;
    terrainpos.z = z;
    ostringstream messagestream;
    messagestream << "<objectcreate type=\"Terrain\" path=\"" << sFilePath << "\" iparentreference=\"0\">"
    "<geometry>"
    << terrainpos
    << "<terrain sterrainreference=\"" << CheckSum << "\"/>"
    << "</geometry>"
    "</objectcreate>" << endl;
    DEBUG(  "ClientTerrainFunctions::CreateTerrain() sending to client " << messagestream.str() ); // DEBUG
    MetaverseClient::SendToServer( messagestream.str().c_str() );

    UploadTerrain( sFilePath );
}
