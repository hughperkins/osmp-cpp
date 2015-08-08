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
//! \brief This module contains generic fileinfo management functions used by the renderer / metaverseclient
// see header file fileinfocache.h for documentation

#include <sstream>
#include <string>
using namespace std;

#include "Diag.h"
#include "fileinfocache.h"
#include "IDBInterface.h"

IDBInterface *FileInfoCacheClass::pdbinterface = 0;

//string FileInfoCacheClass::GetTerrainRetrievalSQL()
//{
//  string SQL = "select s_terrain_reference, i_owner, s_source_filename, s_server_filename from terrainfiles;";
//  return SQL;
//}

bool FileInfoCacheClass::CheckIfChecksumPresent( string sFileChecksum )
{
    if( Files.find( sFileChecksum ) != Files.end() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

void FileInfoCacheClass::LoadFileInfoFromDBRow( FILEINFO &rFileInfo )
{
    if( pdbinterface != 0 )
    {
        //rFileInfo.iTerrainReference = atoi( GetFieldValueByName( "i_reference" ) );
        rFileInfo.sChecksum = pdbinterface->GetFieldValueByName( "s_file_reference" );
        rFileInfo.iOwner = atoi( pdbinterface->GetFieldValueByName( "i_owner" ) );
        rFileInfo.sServerFilename = pdbinterface->GetFieldValueByName( "s_server_filename" );
        rFileInfo.sSourceFilename = pdbinterface->GetFieldValueByName( "s_source_filename" );
    }
}

void FileInfoCacheClass::WriteFileInfoToXML( TiXmlElement *pElement, FILEINFO &rFileInfo )
{
    pElement->SetAttribute("checksum", rFileInfo.sChecksum );
    pElement->SetAttribute("sourcefilename", rFileInfo.sSourceFilename );
    pElement->SetAttribute("serverfilename", rFileInfo.sServerFilename );
    pElement->SetAttribute("owner", rFileInfo.iOwner );
}

FILEINFO &FileInfoCacheClass::AddFileInfoFromXML( TiXmlElement *pElement, string sType )
{
    DEBUG(  "AddFileInfoFromXML" ); // DEBUG
    ostringstream sChecksumStream;
    sChecksumStream << pElement->Attribute("checksum" );
    DEBUG(  "checking for checksum " << sChecksumStream.str() ); // DEBUG
    if( Files.find( sChecksumStream.str() ) == Files.end() )
    {
        DEBUG(  "Adding file info ..." ); // DEBUG
        FILEINFO NewFileInfo;
        NewFileInfo.sType = sType;
        NewFileInfo.sChecksum = sChecksumStream.str();
        NewFileInfo.iOwner = 0;
        NewFileInfo.sSourceFilename = pElement->Attribute("sourcefilename" );
        if( pElement->Attribute("serverfilename" ) != NULL )
        {
            NewFileInfo.sServerFilename = pElement->Attribute("serverfilename" );
        }
        else
        {
            NewFileInfo.sServerFilename = "";
        }
        Files.insert( fileinfopair( NewFileInfo.sChecksum, NewFileInfo ) );
    }
    return Files.find( sChecksumStream.str() )->second;
}

void FileInfoCacheClass::Clear()
{
    Files.clear();
}
