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

#ifndef _FILEINFOCACHE_H
#define _FILEINFOCACHE_H

#include <map>
#include <string>
#include <iostream>
using namespace std;

#include "tinyxml.h"

#include "IDBInterface.h"

//! stores information about a single file, eg a texture file or mesh file
class FILEINFO
{
public:
   string sType;        //!< type of file: MESHFILE, TEXTURE, etc
   string sChecksum;    //!< md5 checksum of file, used as unique identifier/key
   int iOwner;            //!< iReference of person who created file
   string sSourceFilename;    //!< original filename when first uploaded to server
   string sServerFilename;    //!< filename of file on the Metaverse Server
   string sOurFilePath;       //!< local filepath of file
   bool bFilePresent;        //!< does the file exist on our PC yet?
   FILEINFO()
   {
   	 sType = "";
   	 bFilePresent = false;
   	 sSourceFilename = "";
   	 sServerFilename = "";
   	 sOurFilePath = "";
   	 iOwner = 0;
   	 sChecksum = "";
   }
};

typedef map <string, FILEINFO, less<string> >::iterator FileInfoIteratorTypedef;

//! Base class for file info cache classes; used to manage upload/download of files such as textures
class FileInfoCacheClass
{
public:
   map <string, FILEINFO, less<string> > Files;

   static IDBInterface *pdbinterface;  //!< added this to reduce dependencies somewhat
   static void SetDBAbstractionLayer( IDBInterface &rdbinterface )
   {
   	  FileInfoCacheClass::pdbinterface = &rdbinterface;
   }

   //static string GetFileInfoRetrievalSQL() = 0;
   virtual bool CheckIfChecksumPresent( string sChecksum );           //!< checks if we already know about this file, keyed by checksum
   static void LoadFileInfoFromDBRow( FILEINFO &rFileInfo );            //!< loads fileinfo from a database row
   static void WriteFileInfoToXML( TiXmlElement *pElement, FILEINFO &rFileInfo );      //!< writes the passed-in fileinfo into the pElement XML
   virtual FILEINFO &AddFileInfoFromXML( TiXmlElement *pElement, string sType = "" );    //!< registers the file information passed-in as XML
   void Clear();      //!< empties the cache
};

typedef pair < string, FILEINFO > fileinfopair;

#endif // _FILEINFOCACHE_H
