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
//! \brief Used by renderer/client to assign scritps to objects and similar
// see header file for documentation

#include <string>
#include <iostream>
#include <sstream>
using namespace std;

#include "tinyxml.h"

#include "ScriptMgmt.h"
#include "WorldStorage.h"
#include "Selection.h"
#include "Checksum.h"
#include "MetaverseClient.h"
//#include "scriptinfocache.h"

//extern ScriptInfoCacheClass ScriptInfoCache;
namespace MetaverseClient
{
    extern int iMyReference;
    extern mvWorldStorage World;
    extern mvSelection Selector;
}
using namespace MetaverseClient;

//extern void SendClientMessage( const char *message );

void ScriptMgmtClass::AssignScript( int iObjectReference, string sCheckSum )
{
    DEBUG(  "AssignScript()" ); // DEBUG

    int iArrayNum = World.GetArrayNumForObjectReference( iObjectReference );

    if(iArrayNum != -1 )
    {
        if( strcmp( World.GetObject( iArrayNum )->sDeepObjectType, "AVATAR" ) != 0 || iObjectReference == iMyReference )
        {

            ostringstream messagestream;
            messagestream << "<objectupdate ireference=\"" << iObjectReference
            << "\"><scripts><script sscriptreference=\"" << sCheckSum << "\"/></scripts>"
            << "</objectupdate>" << endl;
            string message = messagestream.str();
            DEBUG(  "sending to server [" << message << "]" ); // DEBUG
            SendToServer( message.c_str() );

            sprintf( World.GetObject( iArrayNum )->sScriptReference, sCheckSum.c_str() );
        }
    }
}

void ScriptMgmtClass::UploadScript( string ScriptPath )
{
    DEBUG(  "uploadscript " << ScriptPath ); // DEBUG
    string CheckSum = GenerateCheckString( ScriptPath );

    ostringstream messagestream;
    messagestream << "<loadersendfile type=\"SCRIPT\" path=\"" << ScriptPath << "\" checksum=\"" << CheckSum + "\"/>" << endl;
    string message = messagestream.str();
    DEBUG(  "sending to fileagent " << message );
    MetaverseClient::SendXMLStringToFileAgent( message.c_str() );
}

void ScriptMgmtClass::AssignScriptToOneObject( int iObjectReference, string sScriptPath )
{
    string CheckSum = GenerateCheckString( sScriptPath );
    UploadScript( sScriptPath );

    AssignScript( iObjectReference, CheckSum );
}

void ScriptMgmtClass::AssignScriptFromFilename( const string &filename )
{
    string CheckSum = GenerateCheckString( filename );
    UploadScript( filename );

    SelectionIteratorTypedef iterator;
    for( iterator = Selector.SelectedObjects.begin();
            iterator != Selector.SelectedObjects.end();
            iterator++ )
    {
        AssignScript( iterator->second.iReference, CheckSum );
    }
}

void ScriptMgmtClass::AssignScriptFromXML( TiXmlElement *pElement )
{
    ostringstream ScriptPathStream;
    ScriptPathStream << pElement->Attribute( "file" );
    AssignScriptFromFilename( ScriptPathStream.str() );
}

void ScriptMgmtClass::RemoveScript( int iObjectReference )
{
    DEBUG(  "RemoveScript()" ); // DEBUG
    ostringstream messagestream;
    messagestream << "<objectupdate ireference=\"" << iObjectReference
    << "\"><scripts><script sscriptreference=\"\"/></scripts>"
    << "</objectupdate>" << endl;
    string message = messagestream.str();
    DEBUG(  "sending to server [" << message << "]" ); // DEBUG
    SendToServer( message.c_str() );

    Object *p_Object = World.GetObjectByReference( iObjectReference );
    if( p_Object != NULL )
    {
        sprintf( p_Object->sScriptReference, "" );
    }
}

void ScriptMgmtClass::RemoveScriptFromSelectedObjects()
{
    DEBUG(  "RemoveScriptFromSelectedObjects()" ); // DEBUG
    SelectionIteratorTypedef iterator;
    for( iterator = Selector.SelectedObjects.begin();
            iterator != Selector.SelectedObjects.end();
            iterator++ )
    {
        RemoveScript( iterator->second.iReference );
    }
}
