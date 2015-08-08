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
//! \brief This module contains terrain management functions used by the renderer / metaverseclient

//! This module contains terrain management functions used by the renderer / metaverseclient
//! manages storage of information about terrain files
//! see fileinfocache for more information on member fucntions

#ifndef _TerrainCACHE_H
#define _TerrainCACHE_H

#include <map>
#include <string>
#include <iostream>
using namespace std;

#include "tinyxml.h"

//! Stores information on a single terrain file
class TerrainINFO
{
public:
   string sChecksum;  //!< checksum / reference of terrain file (ie md5 checksum)
   int iOwner;  //!< owner of file (avatar iReference number)
   string sSourceFilename;   //!< filename of original file
   string sServerFilename;   //!< filename on server
   string sOurFilePath;   //!< filepath on our system
   bool bFilePresent;   //!< whether we have the file or not yet
   TerrainINFO()
   {
   	 bFilePresent = false;
   	 sSourceFilename = "";
   	 sServerFilename = "";
   	 sOurFilePath = "";
   	 iOwner = 0;
   	 sChecksum = "";
   }
};

typedef map <string, TerrainINFO, less<string> >::iterator TerrainIteratorTypedef;

//! TerrainCacheClass contains terrain management functions used by the renderer / metaverseclient

//! TerrainCacheClass contains terrain management functions used by the renderer / metaverseclient
//! manages storage of information about terrain files
//! see fileinfocache for more information on member fucntions
class TerrainCacheClass
{
public:
   map <string, TerrainINFO, less<string> > Terrains;   //!< Information on all terrain files
   
   static string GetTerrainRetrievalSQL();
   bool CheckIfTerrainChecksumPresent( string sTerrainChecksum );
   static void LoadTerrainInfoFromDBRow( TerrainINFO &rTerrainInfo );
   static void WriteTerrainInfoToXML( TiXmlElement *pElement, TerrainINFO &rTerrainInfo );
   TerrainINFO &AddTerrainInfoFromXML( TiXmlElement *pElement );
   void Clear();
};

typedef pair < string, TerrainINFO > terraininfopair;

#endif // _TerrainCACHE_H
