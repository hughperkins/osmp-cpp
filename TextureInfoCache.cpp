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
//! \brief This module stores information about texture files
// see texturemanager.h for documentation

#include <sstream>
#include <string>
using namespace std;

#include "Diag.h"
#include "fileinfocache.h"
#include "TextureInfoCache.h"
#include "IDBInterface.h"

string TextureInfoCache::GetTextureRetrievalSQL()
{
    string SQL = "select s_texture_reference, i_owner, s_source_filename, s_server_filename from textures;";
    return SQL;
}

bool TextureInfoCache::CheckIfTextureChecksumPresent( string sTextureChecksum )
{
    if( Textures.find( sTextureChecksum ) != Textures.end() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

int TextureInfoCache::TextureReferenceToTextureId( char *sTextureReference )
{
    ostringstream refstream;
    refstream << sTextureReference;
    // DEBUG(  "looking for texture reference " << sTextureReferenceString ); // DEBUG
    TextureIteratorTypedef iterator = Textures.find( refstream.str() );
    if( iterator != Textures.end() )
    {
        return iterator->second.iTextureID;
    }
    else
    {
        return -1;
    }
}

void TextureInfoCache::LoadTextureInfoFromDBRow( TEXTUREINFO &rTextureInfo )
{
    if( FileInfoCacheClass::pdbinterface != 0 )
    {
        //rTextureInfo.iTextureReference = atoi( GetFieldValueByName( "i_reference" ) );
        rTextureInfo.sChecksum = FileInfoCacheClass::pdbinterface->GetFieldValueByName( "s_texture_reference" );
        rTextureInfo.iOwner = atoi( FileInfoCacheClass::pdbinterface->GetFieldValueByName( "i_owner" ) );
        rTextureInfo.sServerFilename = FileInfoCacheClass::pdbinterface->GetFieldValueByName( "s_server_filename" );
        rTextureInfo.sSourceFilename = FileInfoCacheClass::pdbinterface->GetFieldValueByName( "s_source_filename" );
    }
}

void TextureInfoCache::WriteTextureInfoToXML( TiXmlElement *pElement, TEXTUREINFO &rTextureInfo )
{
    pElement->SetAttribute("checksum", rTextureInfo.sChecksum );
    pElement->SetAttribute("sourcefilename", rTextureInfo.sSourceFilename );
    pElement->SetAttribute("serverfilename", rTextureInfo.sServerFilename );
    pElement->SetAttribute("owner", rTextureInfo.iOwner );
}

TEXTUREINFO &TextureInfoCache::AddTextureInfoFromXML( TiXmlElement *pElement )
{
    DEBUG(  "AddTextureInfoFromXML" ); // DEBUG
    ostringstream sChecksumStream;
    sChecksumStream << pElement->Attribute("checksum" );
    DEBUG(  "checking for checksum " << sChecksumStream.str() ); // DEBUG
    if( Textures.find( sChecksumStream.str() ) == Textures.end() )
    {
        DEBUG(  "Adding texture..." ); // DEBUG
        TEXTUREINFO NewTextureInfo;
        NewTextureInfo.sChecksum = sChecksumStream.str();
        NewTextureInfo.iTextureID = 0;
        NewTextureInfo.iOwner = 0;
        NewTextureInfo.sSourceFilename = pElement->Attribute("sourcefilename" );
        if( pElement->Attribute("serverfilename" ) != NULL )
        {
            NewTextureInfo.sServerFilename = pElement->Attribute("serverfilename" );
        }
        else
        {
            NewTextureInfo.sServerFilename = "";
        }
        Textures.insert( textureinfopair( NewTextureInfo.sChecksum, NewTextureInfo ) );
    }
    return Textures.find( sChecksumStream.str() )->second;
}

void TextureInfoCache::Clear()
{
    Textures.clear();
}
