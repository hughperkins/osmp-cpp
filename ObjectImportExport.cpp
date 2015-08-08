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
// see header file for documentation

#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
using namespace std;

#include "tinyxml.h"

#include "WorldStorage.h"
#include "ObjectImportExport.h"
#include "MetaverseClient.h"

namespace MetaverseClient
{
    extern mvWorldStorage World;
}
using namespace MetaverseClient;

//void ObjectImportMessageCallback( string message );    // needs to be instantiated somewhere in calling code

void ObjectImportExportClass::ExportObject( string sFilename, Object *p_Object )
{
    TiXmlDocument HierarchicalObject;
    HierarchicalObject.Parse( "<object/>" );
    p_Object->WriteToDeepXML( HierarchicalObject.RootElement() );

    DEBUG(  HierarchicalObject );

    ofstream ostream;
    ostream.rdbuf()->open( sFilename.c_str(), ios::out );
    //ostream << *p_Object;

    ostream << HierarchicalObject << endl;

    ostream.rdbuf()->close();
}

int ObjectImportExportClass::GenerateTempReference()
{
    int genref = (int)( rand() * 2000000000 );
    DEBUG(  "generated ref: " << genref ); // DEBUG
    return genref;
}

void ObjectImportExportClass::SendObjectCreateMessageForSingleObject( TiXmlElement *pElement )
{
    DEBUG(  "creating object..." ); // DEBUG
    DEBUG(  "parent ref = " << atoi( pElement->Attribute("i_temp_parentreference" ) ) ); // DEBUG
    Object *pObjectToSend = Object::CreateNewObjectFromXML( pElement );

    DEBUG(  "parsing into xml..."); // DEBUG
    TiXmlDocument IPC;
    IPC.Parse("<objectimport/>" );
    pObjectToSend->WriteToXMLDoc( IPC.RootElement() );

    IPC.RootElement()->RemoveAttribute("ireference");
    IPC.RootElement()->RemoveAttribute("iparentreference" );
    IPC.RootElement()->SetAttribute("i_temp_reference", atoi( pElement->Attribute("i_temp_reference" ) ) );
    IPC.RootElement()->SetAttribute("i_temp_parentreference", atoi( pElement->Attribute("i_temp_parentreference" ) ) );

    ostringstream messagestream;
    messagestream << IPC << endl;
    string message = messagestream.str();
    ObjectImportMessageCallback( message );
}

void ObjectImportExportClass::SendCreateObjectMessages( TiXmlElement *pElement, int iParentReference )
{
    DEBUG(  "SendCreateObjectMessages()" ); // DEBUG
    TiXmlElement *p_ChildElement;
    DEBUG(  *pElement ); // DEBUG

    int iTempReference = GenerateTempReference();
    pElement->SetAttribute( "i_temp_reference", iTempReference );
    pElement->SetAttribute( "i_temp_parentreference", iParentReference );

    SendObjectCreateMessageForSingleObject( pElement );

    TiXmlHandle docHandle( pElement );
    if( docHandle.FirstChild("subobjects").Element() )
    {
        p_ChildElement = pElement->FirstChildElement("subobjects")->FirstChildElement("object");
        while( p_ChildElement != NULL )
        {
            DEBUG(  "processing subobject" ); // DEBUG
            SendCreateObjectMessages( p_ChildElement,iTempReference );
            p_ChildElement = p_ChildElement->NextSiblingElement("object");
        }
    }
}

void ObjectImportExportClass::ImportObject( string sFilename )
{
    Vector3 RezRoot;

    TiXmlDocument IPC;

    IPC.LoadFile( sFilename );

    SendCreateObjectMessages( IPC.RootElement(), 0 );

}

