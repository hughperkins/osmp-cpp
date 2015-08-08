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
//! \brief Stores file info on mesh files

#ifndef _MESHINFOCACHE_H
#define _MESHINFOCACHE_H

#include "fileinfocache.h"

//! used to store file info on mesh files

//! used to store file info on mesh files
//! see baseclass in fileinfocache.h for more information
class MeshInfoCacheClass : public FileInfoCacheClass
{
public:
   static string GetFileInfoRetrievalSQL();                  //!< returns the SQL command to retrieve all meshfiles from the database
   virtual FILEINFO &AddFileInfoFromXML( TiXmlElement *pElement, string sType = "MESHFILE" );       //!< creates a new fileinfo structure for the passed-in XML describing a new meshfile
};

#endif // _MESHINFOCACHE_H
