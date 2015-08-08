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
//! \brief This class is the base class for classes that manage file downloading and uploading of meshes and so on on the client.
//!
//! This class is the base class for classes that manage file downloading and uploading
//! of meshes and so on on the client.
//! rendererglutsplit dialogs with derivatives of clientfilemgmtfunctions in order to
//! display meshes, downloading them from the server if necessary.
//! This module also handles uploading of new files to the server.
//! The actual downloads/uploads are handled by a separate process (clientfileagent).  This module manages when
//! and what to download/upload

#ifndef _CLIENTFILEMGMTFUNCTIONS_H
#define _CLIENTFILEMGMTFUNCTIONS_H

#include <string>
using namespace std;

#include "tinyxml.h"

#include "fileinfocache.h"

//! ClientFileFunctionsClass is the base class for classes that manage file downloading and uploading files
//!
//! ClientFileFunctionsClass is the base class for classes that manage file downloading and uploading
//! of meshes and so on on the client.
//! rendererglutsplit dialogs with derivatives of clientfilemgmtfunctions in order to
//! display meshes, downloading them from the server if necessary.
//! This module also handles uploading of new files to the server.
//! The actual downloads/uploads are handled by a separate process (clientfileagent).  This module manages when
//! and what to download/upload
class ClientFileFunctionsClass
{
public:
    FileInfoCacheClass *pFileInfoCache;  //!< holds a fileinfocache class; used to store details of files available, downloaded etc

    virtual void SetFileInfoCache( FileInfoCacheClass &rFileInfoCache )
    {
        pFileInfoCache = &rFileInfoCache;
    }  //!< pass in the appropriate fileinfocache object, eg a meshinfocache
    //!< just do this once at initialization

    virtual bool FileDownloaded( FILEINFO &rFileInfo );          //!< checks if file has been downloaded.  Returns true/false

    virtual void UploadFile( string FilePath, string sType );    //!< asks ClientFileAgent to upload a file to the server
    virtual void DownloadFile( FILEINFO &rFileInfo );              //!< Asks ClientFileAgent to download file from server

    virtual void LoadOrRequestFile( FILEINFO &rFileInfo );         //!< Loads file into OpenGL texture cache etc, or
    //!< asks ClientFileAgent to download it if we dont already have it

    virtual void RegisterFileFromXMLString( const char *XMLString );
    virtual void RegisterFileFromXML( TiXmlElement *pElement );          //!< registers a file with the fileinfo cache
    //!< Loads file into OpenGL cache texture etc if file is available

    virtual void CreateFile( const char *sFilePath, const float x, const float y, const float z ) = 0;  //!< use to create a new file, eg a new meshfile, pass in the coorindates of where the new mesh or equivalent should be rezed

    virtual const char *GetDirectoryName()
    {
        return "";
    } //!< returns subdirectory where these files are stored (eg meshes, textures etc)
};

#endif // _CLIENTFILEMGMTFUNCTIONS_H
