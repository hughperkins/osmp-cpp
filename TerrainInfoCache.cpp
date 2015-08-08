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
//! \brief This module contains terrain management functions used by the renderer / metaverseclient
// see TerrainInfoCache.h for documentation

#include <sstream>
#include <string>
using namespace std;

#include "Diag.h"
#include "fileinfocache.h"
#include "TerrainInfoCache.h"
#include "IDBInterface.h"

string TerrainCacheClass::GetTerrainRetrievalSQL()
{
    string SQL = "select s_terrain_reference, i_owner, s_source_filename, s_server_filename from terrainfiles;";
    return SQL;
}

bool TerrainCacheClass::CheckIfTerrainChecksumPresent( string sTerrainChecksum )
{
    if( Terrains.find( sTerrainChecksum ) != Terrains.end() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

void TerrainCacheClass::LoadTerrainInfoFromDBRow( TerrainINFO &rTerrainInfo )
{
    if( FileInfoCacheClass::pdbinterface != 0 )
    {
        //rTerrainInfo.iTerrainReference = atoi( GetFieldValueByName( "i_reference" ) );
        rTerrainInfo.sChecksum = FileInfoCacheClass::pdbinterface->GetFieldValueByName( "s_terrain_reference" );
        rTerrainInfo.iOwner = atoi( FileInfoCacheClass::pdbinterface->GetFieldValueByName( "i_owner" ) );
        rTerrainInfo.sServerFilename = FileInfoCacheClass::pdbinterface->GetFieldValueByName( "s_server_filename" );
        rTerrainInfo.sSourceFilename = FileInfoCacheClass::pdbinterface->GetFieldValueByName( "s_source_filename" );
    }
}

void TerrainCacheClass::WriteTerrainInfoToXML( TiXmlElement *pElement, TerrainINFO &rTerrainInfo )
{
    pElement->SetAttribute("checksum", rTerrainInfo.sChecksum );
    pElement->SetAttribute("sourcefilename", rTerrainInfo.sSourceFilename );
    pElement->SetAttribute("serverfilename", rTerrainInfo.sServerFilename );
    pElement->SetAttribute("owner", rTerrainInfo.iOwner );
}

TerrainINFO &TerrainCacheClass::AddTerrainInfoFromXML( TiXmlElement *pElement )
{
    DEBUG(  "AddTerrainInfoFromXML" ); // DEBUG
    ostringstream sChecksumStream;
    sChecksumStream << pElement->Attribute("checksum" );
    DEBUG(  "checking for checksum " << sChecksumStream.str() ); // DEBUG
    if( Terrains.find( sChecksumStream.str() ) == Terrains.end() )
    {
        DEBUG(  "Adding terrain..." ); // DEBUG
        TerrainINFO NewTerrainInfo;
        NewTerrainInfo.sChecksum = sChecksumStream.str();
        NewTerrainInfo.iOwner = 0;
        NewTerrainInfo.sSourceFilename = pElement->Attribute("sourcefilename" );
        if( pElement->Attribute("serverfilename" ) != NULL )
        {
            NewTerrainInfo.sServerFilename = pElement->Attribute("serverfilename" );
        }
        else
        {
            NewTerrainInfo.sServerFilename = "";
        }
        Terrains.insert( terraininfopair( NewTerrainInfo.sChecksum, NewTerrainInfo ) );
    }
    return Terrains.find( sChecksumStream.str() )->second;
}

void TerrainCacheClass::Clear()
{
    Terrains.clear();
}
