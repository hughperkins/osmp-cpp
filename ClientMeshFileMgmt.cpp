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
//! \brief This module handles mesh uploading/downloading for rendererglutsplit
//!
//! Its a derived class of ClientFileFunctionsClass, which handles the majority of hte work
// See headerfile "clientmeshfilemgmt.h" for documentation

#include <string>
#include <iostream>
#include <sstream>
using namespace std;

#include "WorldStorage.h"
#include "Selection.h"
#include "SocketsClass.h"
#include "RendererTexturing.h"
#include "TerrainInfoCache.h"
#include "Checksum.h"
#include "File.h"
#include "MetaverseClient.h"

#include "ClientMeshFileMgmt.h"

namespace MetaverseClient
{
    // extern FileInfoCacheClass FileInfoCache;
    //extern mvsocket MetaverseClientSocket;  // we should probably get rid of these nasty externs at some point.
    extern mvSelection Selector;            // but at least the namespaces are encapsulated so it's not *really* bad
    extern mvWorldStorage World;
}
using namespace MetaverseClient;

//extern void SendClientMessage( const char *message );

void ClientMeshFileMgmtClass::UploadFile( string FilePath, string sType )
{
    ClientFileFunctionsClass::UploadFile( FilePath, "MESHFILE" );
}

void ClientMeshFileMgmtClass::CreateFile( const char *sFilePath, const float x, const float y, const float z )
{
    string CheckSum = GenerateCheckString( sFilePath );

    Vector3 pos;
    pos.x = x;
    pos.y = y;
    pos.z = z;
    ostringstream messagestream;
    messagestream << "<objectcreate type=\"mvMd2Mesh\" path=\"" << sFilePath << "\" iparentreference=\"0\">"
    "<geometry>"
    << pos
    << "<mesh smeshreference=\"" << CheckSum << "\"/>"
    << "</geometry>"
    "</objectcreate>" << endl;
    DEBUG(  "ClientMeshFileMgmt::CreateFile() sending to server " << messagestream.str() ); // DEBUG
    SendToServer( messagestream.str().c_str() );

    UploadFile( sFilePath );
}

const char *ClientMeshFileMgmtClass::GetDirectoryName()
{
    return "meshes";
}
