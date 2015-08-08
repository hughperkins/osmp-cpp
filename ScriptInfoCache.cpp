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
//! \brief This module stores information on script files

// see scriptinfocache.h for documentation

#include <sstream>
#include <string>
using namespace std;

#include "Diag.h"
#include "fileinfocache.h"
#include "ScriptInfoCache.h"
#include "IDBInterface.h"

string ScriptInfoCacheClass::GetRetrievalSQL()
{
    string SQL = "select s_script_reference, i_owner, s_source_filename, s_server_filename from scripts;";
    return SQL;
}

bool ScriptInfoCacheClass::CheckIfChecksumPresent( string sChecksum )
{
    if( Scripts.find( sChecksum ) != Scripts.end() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

void ScriptInfoCacheClass::LoadInfoFromDBRow( SCRIPTINFO &rScriptInfo )
{
    if( FileInfoCacheClass::pdbinterface != 0 )
    {
        //rScriptInfo.iTextureReference = atoi( GetFieldValueByName( "i_reference" ) );
        rScriptInfo.sChecksum = FileInfoCacheClass::pdbinterface->GetFieldValueByName( "s_script_reference" );
        rScriptInfo.iOwner = atoi( FileInfoCacheClass::pdbinterface->GetFieldValueByName( "i_owner" ) );
        rScriptInfo.sServerFilename = FileInfoCacheClass::pdbinterface->GetFieldValueByName( "s_server_filename" );
        rScriptInfo.sSourceFilename = FileInfoCacheClass::pdbinterface->GetFieldValueByName( "s_source_filename" );
    }
}

void ScriptInfoCacheClass::WriteInfoToXML( TiXmlElement *pElement, SCRIPTINFO &rScriptInfo )
{
    pElement->SetAttribute("checksum", rScriptInfo.sChecksum );
    pElement->SetAttribute("sourcefilename", rScriptInfo.sSourceFilename );
    pElement->SetAttribute("serverfilename", rScriptInfo.sServerFilename );
    pElement->SetAttribute("owner", rScriptInfo.iOwner );
}

SCRIPTINFO &ScriptInfoCacheClass::AddInfoFromXML( TiXmlElement *pElement )
{
    DEBUG(  "AddScriptingInfoFromXML" ); // DEBUG
    ostringstream sChecksumStream;
    sChecksumStream << pElement->Attribute("checksum" );
    DEBUG(  "checking for checksum " << sChecksumStream.str() ); // DEBUG
    if( Scripts.find( sChecksumStream.str() ) == Scripts.end() )
    {
        DEBUG(  "Adding script..." ); // DEBUG
        SCRIPTINFO NewScriptInfo;
        NewScriptInfo.sChecksum = sChecksumStream.str();
        NewScriptInfo.iOwner = 0;
        NewScriptInfo.sSourceFilename = pElement->Attribute("sourcefilename" );
        if( pElement->Attribute("serverfilename" ) != NULL )
        {
            NewScriptInfo.sServerFilename = pElement->Attribute("serverfilename" );
        }
        else
        {
            NewScriptInfo.sServerFilename = "";
        }
        Scripts.insert( scriptinfopair( NewScriptInfo.sChecksum, NewScriptInfo ) );
    }
    return Scripts.find( sChecksumStream.str() )->second;
}
