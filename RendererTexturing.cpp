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
//! \brief This module handles texturing objects including managing the upload and download of textures
// See headerfile "renderertexturing.h" for documentation

// Modified 20050415 Mark Wagner - uses wxBase for filename manipulation
//                               - Separated file existance checking from file loading.
//                                 This means a corrupted file on the server won't lead
//                                 to an infinite loop of downloads.
// 20050423 Hugh Perkins - Migrated into Tartan library
// 20050520 Hugh Perkins - migrated to be a class

#include <string>
#include <iostream>
#include <sstream>
using namespace std;

#include <wx/filename.h>
#include <wx/strconv.h>

#include "tinyxml.h"

#include "TextureLoader.h"

#include "WorldStorage.h"
#include "Selection.h"
#include "SocketsClass.h"
#include "RendererTexturing.h"
#include "TextureInfoCache.h"
#include "Checksum.h"
#include "Config.h"
#include "Terrain.h"
#include "MetaverseClient.h"

namespace MetaverseClient
{
    extern TextureInfoCache textureinfocache;
    //extern mvsocket MetaverseClientSocket;  // we should probably get rid of these nasty externs at some point.
    extern mvSelection Selector;            // but at least the namespaces are encapsulated so it's not *really* bad
    extern mvWorldStorage World;
    extern ConfigClass mvConfig;
}
using namespace MetaverseClient;

// Input: rTextureInfo: A reference to a TEXTUREINFO struct
//
// Returns: True on success.
//
// Description: Loads the "missing texture" placeholder
//
// Thread safety: Thread-safe if the OpenGL library is
//
// History: 20050415 Mark Wagner - Created
bool RendererTexturingClass::LoadMissingTexture( TEXTUREINFO &rTextureInfo )
{
    return Tartan::LoadMissingTexture( rTextureInfo.iTextureID );
}

// Input: rTextureInfo: A reference to a TEXTUREINFO struct
//
// Returns: True on success, false otherwise
//
// Description: Loads a PCX image into an OpenGL texture.
// This function based on NeHe texture loading tutorial (http://nehe.gamedev.net)
//
// Thread safety: Unknown
//
// History: 20050415 Mark Wagner - Modified to use wxFileName
bool RendererTexturingClass::LoadTexturePCX( TEXTUREINFO &rTextureInfo )
{
    //  char sFilePath[255];
    //  sprintf( sFilePath, ".\\clientdata\\cache\\textures\\%s", rTextureInfo.sServerFilename.c_str() );
    wxFileName FilePath;

    FilePath = wxString(rTextureInfo.sServerFilename.c_str(), wxConvUTF8);
    FilePath.PrependDir(L"textures");
    FilePath.PrependDir(L"cache");
    FilePath.PrependDir(L"clientdata");
    FilePath.Normalize( wxPATH_NORM_ALL, wxString( mvConfig.CacheDirectory.c_str()  ));

    DEBUG(  "Texture file is at: " << FilePath.GetFullPath().mb_str() ); // DEBUG

    return Tartan::LoadTexturePCX( rTextureInfo.iTextureID, FilePath.GetFullPath().mb_str() );
}

// Input: rTextureInfo: A reference to a TEXTUREINFO struct
//
// Returns: True on success, false otherwise
//
// Description: Loads a TGA image into an OpenGL texture.
// This function based on NeHe texture loading tutorial (http://nehe.gamedev.net)
//
// Thread safety: Unknown
//
// History: 20050415 Mark Wagner - Modified to use wxFileName
bool RendererTexturingClass::LoadTextureTGA( TEXTUREINFO &rTextureInfo )
{
    wxFileName FilePath;

    // char sFilePath[255];
    // sprintf( sFilePath, ".\\clientdata\\cache\\textures\\%s", rTextureInfo.sServerFilename.c_str() );

    FilePath = wxString(rTextureInfo.sServerFilename.c_str(), wxConvUTF8);
    FilePath.PrependDir(L"textures");
    FilePath.PrependDir(L"cache");
    FilePath.PrependDir(L"clientdata");
    FilePath.Normalize( wxPATH_NORM_ALL, wxString( mvConfig.CacheDirectory.c_str()  ));

    DEBUG(  "Loading TGA file " << FilePath.GetFullPath().mb_str() << " ... " ); // DEBUG

    return Tartan::LoadTextureTGA( rTextureInfo.iTextureID, FilePath.GetFullPath().mb_str() );
}


bool RendererTexturingClass::LoadTexture( TEXTUREINFO &rTextureInfo )
{
    DEBUG(  "loading texture..."); // DEBUG

    DEBUG(  rTextureInfo.sServerFilename.c_str() + strlen( rTextureInfo.sServerFilename.c_str() - 4 ) ); // DEBUG
    char filetypestring[4];
    sprintf( filetypestring, "%.4s", rTextureInfo.sServerFilename.substr( rTextureInfo.sServerFilename.length() - 4, 4 ).c_str() );
    DEBUG(  "filetypestring " << filetypestring ); // DEBUG
    if( toupper( filetypestring[0] ) == '.' && toupper( filetypestring[1] ) == 'T' && toupper( filetypestring[2] ) == 'G' && toupper( filetypestring[3] ) == 'A' )
    {
        return LoadTextureTGA( rTextureInfo );
    }
    else
    {
        return LoadTexturePCX( rTextureInfo );
    }
}

void RendererTexturingClass::DownloadTexture( TEXTUREINFO &rTextureInfo )
{
    DEBUG(" ** SENDING LOADER GET FILE XML");
    ostringstream messagetoclientfileagentstream;
    messagetoclientfileagentstream << "<loadergetfile type=\"TEXTURE\" checksum=\"" << rTextureInfo.sChecksum
    << "\" sourcefilename=\"" << rTextureInfo.sSourceFilename << "\" serverfilename=\"" << rTextureInfo.sServerFilename
    << "\"/>" << endl;
    //DEBUG(  "sending to server " << messagetoclientfileagentstream.str() ); // DEBUG
    TiXmlDocument IPC;
    IPC.Parse( messagetoclientfileagentstream.str().c_str() );
    //MetaverseClientSocket.Send( messagetoclientfileagentstream.str().c_str() );
    SendXMLDocToFileAgent( IPC );
}


// Input: rTextureInfo: A reference to a TEXTUREINFO struct
//
// Returns: True if the file is in the cache, false otherwise
//
// Description: Checks the cache to see if the specified texture is
//  there.
//
// Thread safety: Thread-safe
//
// History: 20050415 Mark Wagner - Created
// 20050520 Hugh Perkins renamed from TextureInCache to TextureIsInCache
bool RendererTexturingClass::TextureIsInCache( TEXTUREINFO &rTextureInfo )
{
    wxFileName FilePath;

    DEBUG( rTextureInfo.sServerFilename.c_str() );

    FilePath = wxString(rTextureInfo.sServerFilename.c_str(), wxConvUTF8);
    FilePath.PrependDir(L"textures");
    FilePath.PrependDir(L"cache");
    FilePath.PrependDir(L"clientdata");
    FilePath.Normalize( wxPATH_NORM_ALL, wxString( mvConfig.CacheDirectory.c_str()  ));

    DEBUG( "Checking for path: " << FilePath.GetFullPath() );
    bool bFileExists = FilePath.FileExists();

    DEBUG( "Filepath exists = " << bFileExists << ", checking checkums..." );

    string CheckSum = GenerateCheckString( FilePath.GetFullPath().c_str() );
    DEBUG( "Checksum is " << CheckSum << " compares to: " << rTextureInfo.sChecksum );

    bool bInCache = ( CheckSum == rTextureInfo.sChecksum );

    DEBUG( "So, in cache = " << bInCache );

    return bInCache;
}

// Input: rTextureInfo: A TEXTUREINFO struct
//
// Returns: True if the texture or a proxy was loaded
//          False if the texture was requested
//
// Description: Checks to see if the requested texture is in cache. If
//  it is, loads it.  If loading fails, loads a "Missing Image" proxy.
//  If the file isn't in cache, sends a load request to the metaverse
//  client.
//
// Thread safety: Unknown
//
// History: 20050415 Mark Wagner - separated file existance checking
//                                 from file loading
void RendererTexturingClass::LoadOrRequestTexture( TEXTUREINFO &rTextureInfo )
{
    DEBUG(" ** INSIDE LOAD OR REQUEST TEXTURE");
    if( !TextureIsInCache( rTextureInfo ) )
    {
        DownloadTexture( rTextureInfo );
    }
    else
    {
        if(!LoadTexture( rTextureInfo ) )
        {
            //   WARNING("Loading proxy texture for " << rTextureInfo.sServerFilename.c_str());
            LoadMissingTexture( rTextureInfo );
        }
    }
}

void RendererTexturingClass::ApplySkyboxTexture( string sCheckSum )
{
    DEBUG(  "2ApplySkyboxTexture()" ); // DEBUG

    ostringstream messagestream;
    messagestream << "<skyboxupdate stexturereference=\"" << sCheckSum << "\"/>" << endl;
    string message = messagestream.str();
    DEBUG(  "sending to server [" << message << "]" ); // DEBUG
    SendToServer( message.c_str() );

    if( textureinfocache.Textures.find( sCheckSum ) != textureinfocache.Textures.end() )
    {

        if( textureinfocache.Textures.find( sCheckSum )->second.iTextureID != -1 )
        {
            DEBUG(  "Texture is loaded in memory, id " << textureinfocache.Textures.find( sCheckSum )->second.iTextureID << ", assigning..." ); // DEBUG
        }
        else
        {
            DEBUG(  "Texture not in memory, requesting download from server, or load into memory if we already have it" ); // DEBUG
            LoadOrRequestTexture( textureinfocache.Textures.find( sCheckSum )->second );
            if( textureinfocache.Textures.find( sCheckSum )->second.iTextureID != -1 )
            {
                DEBUG(  "Texture is loaded in memory, id " << textureinfocache.Textures.find( sCheckSum )->second.iTextureID << ", assigning..." ); // DEBUG
            }
        }
    }
}

void RendererTexturingClass::ApplySkyboxTerrain( int iObjectReference, string sCheckSum )
{
    DEBUG(  "ApplySkyboxTerrain()" ); // DEBUG

    int iArrayNum = World.GetArrayNumForObjectReference( iObjectReference );

    if(iArrayNum != -1 )
    {
        if( strcmp( World.GetObject( iArrayNum )->ObjectType, "PRIM" ) == 0 )
        {
            dynamic_cast< Terrain *>( World.GetObject( iArrayNum ) )->SetSkybox( sCheckSum.c_str() );

            ostringstream messagestream;
            messagestream << "<objectupdate ireference=\"" << iObjectReference
            << "\"><faces><face num=\"0\"><skybox stexturereference=\"" << sCheckSum << "\"/></face></faces>"
            << "</objectupdate>" << endl;
            string message = messagestream.str();
            DEBUG(  "sending to server [" << message << "]" ); // DEBUG
            SendToServer( message.c_str() );

            if( textureinfocache.Textures.find( sCheckSum ) != textureinfocache.Textures.end() )
            {

                if( textureinfocache.Textures.find( sCheckSum )->second.iTextureID != -1 )
                {
                    DEBUG(  "Texture is loaded in memory, id " << textureinfocache.Textures.find( sCheckSum )->second.iTextureID << ", assigning..." ); // DEBUG
                }
                else
                {
                    DEBUG(  "Texture not in memory, requesting download from server, or load into memory if we already have it" ); // DEBUG
                    LoadOrRequestTexture( textureinfocache.Textures.find( sCheckSum )->second );
                    if( textureinfocache.Textures.find( sCheckSum )->second.iTextureID != -1 )
                    {
                        DEBUG(  "Texture is loaded in memory, id " << textureinfocache.Textures.find( sCheckSum )->second.iTextureID << ", assigning..." ); // DEBUG
                    }
                }
            }
        }
    }
}


void RendererTexturingClass::ApplyTexture( int iObjectReference, string sCheckSum )
{
    DEBUG(  "ApplyTexture()" ); // DEBUG

    int iArrayNum = World.GetArrayNumForObjectReference( iObjectReference );

    if(iArrayNum != -1 )
    {
        if( strcmp( World.GetObject( iArrayNum )->ObjectType, "PRIM" ) == 0 )
        {

            dynamic_cast< Prim *>( World.GetObject( iArrayNum ) )->SetTexture( 0, sCheckSum.c_str() );

            ostringstream messagestream;
            messagestream << "<objectupdate ireference=\"" << iObjectReference
            << "\"><faces><face num=\"0\"><texture stexturereference=\"" << sCheckSum << "\"/></face></faces>"
            << "</objectupdate>" << endl;
            string message = messagestream.str();
            DEBUG(  "sending to server [" << message << "]" ); // DEBUG
            SendToServer( message.c_str() );

            if( textureinfocache.Textures.find( sCheckSum ) != textureinfocache.Textures.end() )
            {

                if( textureinfocache.Textures.find( sCheckSum )->second.iTextureID != -1 )
                {
                    DEBUG(  "Texture is loaded in memory, id " << textureinfocache.Textures.find( sCheckSum )->second.iTextureID << ", assigning..." ); // DEBUG
                }
                else
                {
                    DEBUG(  "Texture not in memory, requesting download from server, or load into memory if we already have it" ); // DEBUG
                    LoadOrRequestTexture( textureinfocache.Textures.find( sCheckSum )->second );
                    if( textureinfocache.Textures.find( sCheckSum )->second.iTextureID != -1 )
                    {
                        DEBUG(  "Texture is loaded in memory, id " << textureinfocache.Textures.find( sCheckSum )->second.iTextureID << ", assigning..." ); // DEBUG
                    }
                }
            }
        }
    }
}

void RendererTexturingClass::ApplyTextureToOneObject( int iObjectReference, string sTexturePath )
{
    string CheckSum = GenerateCheckString( sTexturePath );
    if( !textureinfocache.CheckIfTextureChecksumPresent( CheckSum ) )
    {
        UploadTexture( sTexturePath );
    }

    ApplyTexture( iObjectReference, CheckSum );
}

void RendererTexturingClass::ApplyTextureFromFilename( const string &filename )
{
    string CheckSum = GenerateCheckString( filename );
    if( !textureinfocache.CheckIfTextureChecksumPresent( CheckSum ) )
    {
        UploadTexture( filename );
    }

    SelectionIteratorTypedef iterator;
    for( iterator = Selector.SelectedObjects.begin();
            iterator != Selector.SelectedObjects.end();
            iterator++ )
    {
        ApplyTexture( iterator->second.iReference, CheckSum );
    }
}

void RendererTexturingClass::ApplyTextureFromXML( TiXmlElement *pElement )
{
    ostringstream TexturePathStream;
    TexturePathStream << pElement->Attribute( "file" );
    ApplyTextureFromFilename( TexturePathStream.str() );
}

void RendererTexturingClass::ApplySkyboxTextureFromFilename( const string &filename )
{
    string CheckSum = GenerateCheckString( filename );
    if( !textureinfocache.CheckIfTextureChecksumPresent( CheckSum ) )
    {
        UploadTexture( filename );
    }

    ApplySkyboxTexture( CheckSum );
}

void RendererTexturingClass::ApplySkyboxTextureFromXML( TiXmlElement *pElement )
{
    ostringstream TexturePathStream;
    TexturePathStream << pElement->Attribute( "file" );
    ApplySkyboxTextureFromFilename( TexturePathStream.str() );
}

void RendererTexturingClass::ApplySkyboxTerrainFromFilename( const string &filename )
{
    string CheckSum = GenerateCheckString( filename );
    if( !textureinfocache.CheckIfTextureChecksumPresent( CheckSum ) )
    {
        UploadTexture( filename );
    }

    SelectionIteratorTypedef iterator;
    for( iterator = Selector.SelectedObjects.begin();
            iterator != Selector.SelectedObjects.end();
            iterator++ )
    {
        ApplySkyboxTerrain( iterator->second.iReference, CheckSum );
    }
}

void RendererTexturingClass::ApplySkyboxTerrainFromXML( TiXmlElement *pElement )
{
    ostringstream TexturePathStream;
    TexturePathStream << pElement->Attribute( "file" );
    ApplySkyboxTerrainFromFilename( TexturePathStream.str() );
}

void RendererTexturingClass::UploadTexture( string TexturePath )
{
    DEBUG(  "uploadtexture " << TexturePath ); // DEBUG
    string CheckSum = GenerateCheckString( TexturePath );
    if( !textureinfocache.CheckIfTextureChecksumPresent( CheckSum ) )
    {
        ostringstream messagestream;
        messagestream << "<loadersendfile type=\"TEXTURE\" path=\"" << TexturePath << "\" checksum=\"" << CheckSum + "\"/>" << endl;
        string message = messagestream.str();
        DEBUG(  "sending to clientfileagent " << message );

        TiXmlDocument IPC;
        IPC.Parse( messagestream.str().c_str() );
        //MetaverseClientSocket.Send( message.c_str() );
        SendXMLDocToFileAgent( IPC );

        //  SetNewTexture( TexturePath, CheckSum );
    }
}

void RendererTexturingClass::UploadTextureFromXML( TiXmlElement *pElement )
{
    DEBUG(  "uploadtexturefromxml " << pElement->Attribute( "path" ) ); // DEBUG
    string TexturePath = pElement->Attribute( "path" );
    UploadTexture( TexturePath );
}

void RendererTexturingClass::RegisterTextureFromXMLString( const char *XMLString )
{
    TiXmlDocument IPC;
    IPC.Parse( XMLString );
    RegisterTextureFromXML( IPC.RootElement() );
}

void RendererTexturingClass::RegisterTextureFromXML( TiXmlElement *pElement )
{
    DEBUG(  "RegisterTextureFromXML" ); // DEBUG
    TEXTUREINFO &rTextureInfo = textureinfocache.AddTextureInfoFromXML( pElement );

    //LoadOrRequestTexture( textureinfocache.Textures.find( pElement->Attribute("checksum") )->second );
    DEBUG(" ** CALL loadorrequestTexture from RegisterTextureFromXML");
    LoadOrRequestTexture( rTextureInfo );
}

