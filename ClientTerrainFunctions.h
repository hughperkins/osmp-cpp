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
//! \brief This module handles texturing objects including managing the upload and download of terrains

#ifndef _CLIENTTerrainFUNCTIONS_H
#define _CLIENTTerrainFUNCTIONS_H

#include <string>
using namespace std;

#include "tinyxml.h"

#include "TerrainInfoCache.h"

class ClientTerrainFunctionsClass
{
public:
    void UploadTerrain( string TerrainPath );                        //!< If terrain unknown, asks ClientFileAgent to upload a terrain to the server
    void DownloadTerrain( TerrainINFO &rTerrainInfo );              //!< Asks ClientFileAgent to download file from server

    // bool LoadTerrain( TerrainINFO &rTerrainInfo );                  //!< Loads a terrain from file into OpenGL terrain cache
    void LoadOrRequestTerrain( TerrainINFO &rTerrainInfo );         //!< Loads terrain from file into OpenGL terrain cache, or
    //!< asks ClientFileAgent to download it

    //void ApplyTerrain( int iObjectReference, string sCheckSum );       //!< Applies a terrain to an object, setting the sTerrainReference in the object's properties,
    //!< If the file exists locally, loads it into OpenGL terrain cache and assigns TerrainID to object's properties
    //!< otherwise reqeusts download of file from server (via DownloadTerrain function)
    //void ApplyTerrainToOneObject( int iObjectReference, string sTerrainPath );
    //void ApplyTerrainFromXML( TiXmlElement *pElement );             //!< Applies a terrain to an object, uploading it to server if necessary, uses
    //!< UploadTerrain and ApplyTerrain

    void RegisterTerrainFromXMLString( const char *XMLString );
    void RegisterTerrainFromXML( TiXmlElement *pElement );          //!< registers a terrain with the TerrainManager terrain cache
    //!< Loads terrain into OpenGL cache if necessary
    //void UploadTerrainFromXML( TiXmlElement *pElement );            //!< If terrain unknown, asks ClientFileAgent to upload a terrain to the server, uses UploadTerrain


    void CreateTerrain( const char *sFilePath, const float x, const float y, const float z );  //!< creates a new terrain at specified coordinates from specified .raw filepath
protected:
    bool TerrainDownloaded( TerrainINFO &rTerrainInfo );
};

#endif // _CLIENTTerrainFUNCTIONS_H
