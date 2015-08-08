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
//!
//! This module handles texturing objects including managing the upload and download of textures
//! and the upload of textures into the local OpenGL/GLUT texture cache.

#ifndef _RENDERERTEXTURING_H
#define _RENDERERTEXTURING_H

#include <string>
using namespace std;

#include "tinyxml.h"

#include "TextureInfoCache.h"

class RendererTexturingClass
{
public:
   void UploadTexture( string TexturePath );                        //!< If texture unknown, asks ClientFileAgent to upload a texture to the server
   void DownloadTexture( TEXTUREINFO &rTextureInfo );              //!< Asks ClientFileAgent to download file from server

   bool LoadTexture( TEXTUREINFO &rTextureInfo );                  //!< Loads a texture from file into OpenGL texture cache
   void LoadOrRequestTexture( TEXTUREINFO &rTextureInfo );         //!< Loads texture from file into OpenGL texture cache, or
                                                                   //!< asks ClientFileAgent to download it

   void ApplyTexture( int iObjectReference, string sCheckSum );       //!< Applies a texture to an object, setting the sTextureReference in the object's properties,
                                                                   //!< If the file exists locally, loads it into OpenGL texture cache and assigns TextureID to object's properties
                                                                   //!< otherwise reqeusts download of file from server (via DownloadTexture function)
   void ApplySkyboxTerrain( int iObjectReference, string sCheckSum );       //!< Applies a texture to an object, setting the sTextureReference in the object's properties,
   void ApplyTextureToOneObject( int iObjectReference, string sTexturePath );
   void ApplyTextureFromFilename( const string &filename );
   void ApplyTextureFromXML( TiXmlElement *pElement );             //!< Applies a texture to an object, uploading it to server if necessary, uses
                                                                   //!< UploadTexture and ApplyTexture
   void ApplySkyboxTerrainFromFilename( const string &filename );
   void ApplySkyboxTerrainFromXML( TiXmlElement *pElement );             //!< Applies a texture to an object, uploading it to server if necessary, uses
   void ApplySkyboxTextureFromFilename( const string &filename );
   void ApplySkyboxTextureFromXML( TiXmlElement *pElement );             //!< Applies a texture to an object, uploading it to server if necessary, uses

   void RegisterTextureFromXMLString( const char *XMLString );
   void RegisterTextureFromXML( TiXmlElement *pElement );          //!< registers a texture with the textureinfocache texture cache
                                                                   //!< Loads texture into OpenGL cache if necessary

   void UploadTextureFromXML( TiXmlElement *pElement );            //!< If texture unknown, asks ClientFileAgent to upload a texture to the server, uses UploadTexture

protected:
   bool LoadMissingTexture( TEXTUREINFO &rTextureInfo );
   bool LoadTexturePCX( TEXTUREINFO &rTextureInfo );
   bool LoadTextureTGA( TEXTUREINFO &rTextureInfo );
   bool TextureIsInCache( TEXTUREINFO &rTextureInfo );
   void ApplySkyboxTexture( string sCheckSum );
};

#endif // _RENDERERTEXTURING_H
