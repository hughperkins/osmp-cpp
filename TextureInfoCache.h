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
//!
//! This module stores information about texture files
//! see fileinfocache for information on member functions

#ifndef _TEXTUREMANAGER_H
#define _TEXTUREMANAGER_H

#include <map>
#include <string>
#include <iostream>
using namespace std;

#include "tinyxml.h"

#include "fileinfocache.h"

//! Information about one texture
class TEXTUREINFO : public FILEINFO
{
public:
   int iTextureID;
};

typedef map <string, TEXTUREINFO, less<string> >::iterator TextureIteratorTypedef;

//! TextureInfoCache stores information about texture files

//! TextureInfoCache stores information about texture files
//! see fileinfocache for information on member functions
class TextureInfoCache
{
public:
   map <string, TEXTUREINFO, less<string> > Textures;  //!< Information about all texture files

   static string GetTextureRetrievalSQL();
   bool CheckIfTextureChecksumPresent( string sTextureChecksum );
   static void LoadTextureInfoFromDBRow( TEXTUREINFO &rTextureInfo );
   static void WriteTextureInfoToXML( TiXmlElement *pElement, TEXTUREINFO &rTextureInfo );
   TEXTUREINFO &AddTextureInfoFromXML( TiXmlElement *pElement );
   int TextureReferenceToTextureId( char *sTextureReference );
   void Clear();
};

typedef pair < string, TEXTUREINFO > textureinfopair;

#endif // _TEXTUREMANAGER_H
