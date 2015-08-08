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
//! \brief This module stores information on script files
//!
//! This module stores information on script files
//! see fileinfocache for more infomration on member functions (not base class of this, but could be
//! and probably will be in the future)

#ifndef _SCRIPTINFOCACHE_H
#define _SCRIPTINFOCACHE_H

#include <map>
#include <string>
#include <iostream>
using namespace std;

#include "tinyxml.h"

//! structure to hold information about a single script file, used by ScriptInfoCache
class SCRIPTINFO
{
public:
   string sChecksum;   //!< script checksum (reference id, md5 checksum)
   int iOwner;        //!< script owner
   string sSourceFilename;   //!< filename of original file
   string sServerFilename;   //!< filename on server
};

typedef map <string, SCRIPTINFO, less<string> >::iterator ScriptIteratorTypedef;

//! ScriptInfoCache stores information on script files

//! ScriptInfoCache stores information on script files
//! see fileinfocache for more infomration on member functions (not base class of this, but could be
//! and probably will be in the future)
class ScriptInfoCacheClass
{
public:
   map <string, SCRIPTINFO, less<string> > Scripts;
   
   static string GetRetrievalSQL();   //!< Gets SQL to retrieve list of stored files from database; used by dbinterface
   bool CheckIfChecksumPresent( string sChecksum ); //!< Checks if sChecksum is in the list of known file checksums (to see if the file is new or not)
   
   //! Loads information into rScriptInfo from a single database row.  Used in DBInterface (?)
   static void LoadInfoFromDBRow( SCRIPTINFO &rScriptInfo );
   
   //! Writes out rScriptInfo into pElement XML element
   static void WriteInfoToXML( TiXmlElement *pElement, SCRIPTINFO &rScriptInfo );
   
   //! returns reference to a scriptinfo generated from passed in pElement XML element
   SCRIPTINFO &AddInfoFromXML( TiXmlElement *pElement );
};

typedef pair < string, SCRIPTINFO > scriptinfopair;

#endif // _SCRIPTINFOCACHE_H
