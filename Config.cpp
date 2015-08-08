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
//! \brief Used to read config from config.xml

// see headerfile Config.h for docuemntation

#include <string>
#include <iostream>
#include <fstream>
#include <map>
using namespace std;

#include "Diag.h"

#include "stdio.h"

#include "tinyxml.h"

#include "Config.h"

void ConfigClass::ReadConfig()
{
    string sFilePath = "config.xml";

    // ifstream configfile;
    // configfile.open( sFilePath.c_str() );
    // TiXmlDocument IPC;
    // configfile >> IPC;
    TiXmlDocument IPC("config.xml");
    IPC.LoadFile();
    // DEBUG(  IPC ); // DEBUG
    DEBUG("reading config.xml ...");

    TiXmlHandle docHandle( IPC.RootElement() );
    if( docHandle.FirstChild("editors").FirstChild("texteditor").Element() )
    {
        TextEditor = IPC.RootElement()->FirstChildElement("editors")->FirstChildElement("texteditor")->Attribute("path");
    }
    if( docHandle.FirstChild("editors").FirstChild("textureeditor").Element() )
    {
        TextureEditor = IPC.RootElement()->FirstChildElement("editors")->FirstChildElement("textureeditor")->Attribute("path");
    }
    if( docHandle.FirstChild("editors").FirstChild("scripteditor").Element() )
    {
        ScriptEditor = IPC.RootElement()->FirstChildElement("editors")->FirstChildElement("scripteditor")->Attribute("path");
    }

    if( docHandle.FirstChild("help").FirstChild("f1helpfile").Element() )
    {
        F1HelpFile = IPC.RootElement()->FirstChildElement("help")->FirstChildElement("f1helpfile")->Attribute("path");
    }

    if( docHandle.FirstChild("directories").FirstChild("temp").Element() )
    {
        TempDirectory = IPC.RootElement()->FirstChildElement("directories")->FirstChildElement("temp")->Attribute("path");
        DEBUG("Temp directory " << TempDirectory);
    }

    if( docHandle.FirstChild("directories").FirstChild("cache").Element() )
    {
        CacheDirectory = IPC.RootElement()->FirstChildElement("directories")->FirstChildElement("cache")->Attribute("path");
        DEBUG("Cache directory " << CacheDirectory);
    }

    if( docHandle.FirstChild("system").FirstChild("debuglevel").Element() )
    {
        DebugLevel = IPC.RootElement()->FirstChildElement("system")->FirstChildElement("debuglevel")->Attribute("value");
        DEBUG("DebugLevel " << DebugLevel);
    }

    if( docHandle.FirstChild("engines").FirstChild("collisionandphysicsdll").Element() )
    {
        CollisionAndPhysicsEngine = IPC.RootElement()->FirstChildElement("engines")->FirstChildElement("collisionandphysicsdll")->Attribute("filename" );
        DEBUG("Collision/physics engine " << CollisionAndPhysicsEngine );
    }

    if( docHandle.FirstChild("keymappings").Element() )
    {
        DEBUG("Key mappings");
        TiXmlElement *pKeyMappingXml = IPC.RootElement()->FirstChildElement("keymappings")->FirstChildElement( "key" );
        KeyMappingsKeyToCommand.clear();
        KeyMappingsCommandToKey.clear();
        while( pKeyMappingXml != NULL )
        {
            string sCommand = pKeyMappingXml->Attribute("command");
            string sKeyCode = pKeyMappingXml->Attribute("keycode");

            // cout << sCommand << " " << sKeyCode << endl;
            KeyMappingsKeyToCommand.insert( pair< string, string >( sKeyCode, sCommand ) );
            KeyMappingsCommandToKey.insert( pair< string, string >( sCommand, sKeyCode ) );

            pKeyMappingXml = pKeyMappingXml->NextSiblingElement("key" );
        }
    }

    if( docHandle.FirstChild("simconfig").FirstChild("database").Element() )
    {
        DEBUG("Sim config database");
        TiXmlElement *pelement = IPC.RootElement()->FirstChildElement("simconfig")->FirstChildElement( "database" );
        DatabaseConnectionInfo &rDBInfo = SimDatabaseInfo;
        if( pelement->Attribute("name") != NULL )
        {
            rDBInfo.DatabaseName = pelement->Attribute("name");
        }
        if( pelement->Attribute("host") != NULL )
        {
            rDBInfo.Host = pelement->Attribute("host");
        }
        if( pelement->Attribute("user") != NULL )
        {
            rDBInfo.UserName = pelement->Attribute("user");
        }
        if( pelement->Attribute("password") != NULL )
        {
            rDBInfo.Password = pelement->Attribute("password");
        }
    }

    if( docHandle.FirstChild("authserver").FirstChild("database").Element() )
    {
        DEBUG("Auth server database");
        TiXmlElement *pelement = IPC.RootElement()->FirstChildElement("authserver")->FirstChildElement( "database" );
        DatabaseConnectionInfo &rDBInfo = AuthServerDatabaseInfo;
        if( pelement->Attribute("host") != NULL )
        {
            rDBInfo.Host = pelement->Attribute("host");
        }
        if( pelement->Attribute("name") != NULL )
        {
            rDBInfo.DatabaseName = pelement->Attribute("name");
        }
        if( pelement->Attribute("user") != NULL )
        {
            rDBInfo.UserName = pelement->Attribute("user");
        }
        if( pelement->Attribute("password") != NULL )
        {
            rDBInfo.Password = pelement->Attribute("password");
        }
    }

    if( docHandle.FirstChild("simconfig").Element() )
    {
        if( IPC.RootElement()->FirstChildElement("simconfig")->Attribute("name" ) != NULL )
        {
            sSimName = IPC.RootElement()->FirstChildElement("simconfig")->Attribute("name" );
            DEBUG("Sim name " << sSimName);
        }
    }

    if( docHandle.FirstChild("simconfig").FirstChild("authservers").FirstChild("authserver").Element() )
    {
        DEBUG("Reading sim auth servers");
        TiXmlElement *pauthserverelement = IPC.RootElement()->FirstChildElement("simconfig")->FirstChildElement( "authservers" )->FirstChildElement( "authserver" );
        while( pauthserverelement != NULL && iNumAuthServers < 10 )
        {
            AuthServerConnectInfo &rAuthServer = AuthServers[ iNumAuthServers ];
            rAuthServer.sIPAddress = "";
            rAuthServer.iPort = 0;
            rAuthServer.sPassword = "";

            if( pauthserverelement->Attribute("serverip" ) != NULL )
            {
                rAuthServer.sIPAddress = pauthserverelement->Attribute("serverip" );
            }
            if( pauthserverelement->Attribute("serverport" ) != NULL )
            {
                rAuthServer.iPort = atoi( pauthserverelement->Attribute("serverport" ) );
                cout << pauthserverelement->Attribute("serverport" ) << endl;
                cout << rAuthServer.iPort << endl;
            }
            if( pauthserverelement->Attribute("password" ) != NULL )
            {
                rAuthServer.sPassword = pauthserverelement->Attribute("password" );
            }
            DEBUG("Sim auth server " << iNumAuthServers << ": " << rAuthServer.sIPAddress << ":" << rAuthServer.iPort);
            iNumAuthServers++;

            pauthserverelement = pauthserverelement->NextSiblingElement("authserver" );
        }
    }

    if( docHandle.FirstChild("clientconfig").Element() )
    {
        DEBUG("Client config:");
        TiXmlElement *pKey = IPC.RootElement()->FirstChildElement("clientconfig")->FirstChildElement();
        while( pKey != NULL )
        {
            TiXmlAttribute *pAttrib = pKey->FirstAttribute();
            while( pAttrib != NULL )
            {
                string sName = pAttrib->Name();
                string sVal = pAttrib->Value();
                string sMapKey = string( pKey->Value() ) + "." + sName;
                ClientConfig.insert( pair< string, string >( sMapKey, sVal ) );
                DEBUG( "read data " << sMapKey << "=" << sVal );

                pAttrib = pAttrib->Next();
            }
            pKey = pKey->NextSiblingElement();
        }
    }

    //  configfile.close();

    DEBUG("... config.xml read");
}

string ConfigClass::GetCommandForKeycode( string sKeycode )
{
    if( KeyMappingsKeyToCommand.find( sKeycode ) != KeyMappingsKeyToCommand.end() )
    {
        return KeyMappingsKeyToCommand.find( sKeycode )->second;
    }
    else
    {
        return "";
    }
}

string ConfigClass::GetKeycodeForCommand( string sCommand )
{
    if( KeyMappingsCommandToKey.find( sCommand ) != KeyMappingsCommandToKey.end() )
    {
        return KeyMappingsCommandToKey.find( sCommand )->second;
    }
    else
    {
        return "";
    }
}
