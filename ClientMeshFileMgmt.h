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

#ifndef _CLIENTMESHFILEMGMT_H
#define _CLIENTMESHFILEMGMT_H

#include "ClientFileMgmtFunctions.h"

//! ClientMeshFileMgmtClass handles mesh uploading/downloading on the client

//! ClientMeshFileMgmtClass handles mesh uploading/downloading for rendererglutsplit
//! Its a derived class of ClientFileFunctionsClass, which handles the majority of hte work
class ClientMeshFileMgmtClass : public ClientFileFunctionsClass
{
public:
    virtual void UploadFile( string FilePath, string sType = "MESHFILE" );    //! call this to upload a mesh to the server
    virtual void CreateFile( const char *sFilePath, const float x, const float y, const float z );     //! call this to create a new mesh at specified coordinates from specified meshfile
    virtual const char *GetDirectoryName(); //!< returns subdirectory where these files are stored (eg meshes, textures etc)
};

#endif // _CLIENTMESHFILEMGMT_H
