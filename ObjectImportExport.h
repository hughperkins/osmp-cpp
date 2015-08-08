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
//! \brief This module handles the export and import of Objects to/from xml files
//!
//! This is used to export/import objects to and from local file storage

#ifndef _ObjectIMPORTEXPORT_H
#define _ObjectIMPORTEXPORT_H

#include <string>
using namespace std;

#include "tinyxml.h"

#include "Object.h"
#include "ObjectGrouping.h"

class ObjectImportExportClass
{
public:
   void ExportObject( string sFilename, Object *p_Object );               //!< exports object *p_Object to file sFilename
   void ImportObject( string sFilename );                                 //!< imports an object from sFilename
                                                                    //!< and calls  void ObjectImportMessageCallback( string message );
                                                                    //!< to send objectimport messages back to the caller (serialized XML)                                                                    
protected:
   int GenerateTempReference();
   void SendObjectCreateMessageForSingleObject( TiXmlElement *pElement );
   void SendCreateObjectMessages( TiXmlElement *pElement, int iParentReference );
};

#endif // _ObjectIMPORTEXPORT_H
