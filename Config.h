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
//!
//! Used to read config from config.xml
//! currently reads values of script,texture and text editors, of the F1 helpfile name, and
//! of the physicsandcollision dll
//! Used by both metaverseserver and rendererglutsplit

#ifndef _MVCONFIG_H
#define _MVCONFIG_H

#include <string>
#include <map>
using namespace std;

//! Database connection info for one database. Contained by mvConfig class
struct DatabaseConnectionInfo
{
	string Host;
	 string DatabaseName;
	 string UserName;
	 string Password;
};

//! Connection info for the authentication server. Contained by mvConfig class
struct AuthServerConnectInfo
{
	 string sIPAddress;
	 int iPort;
	 string sPassword;
};

//! ConfigClass reads configuration information from the config.xml configuration file

//! ConfigClass reads configuration information from the config.xml configuration file
//!
//! Prerequisites for use:
//! - ensure you have a valid config.xml file in the current directory
//!
//! To use:
//! - call ReadConfig()
//! - read appropriate values as class properties
class ConfigClass
{
public:
   typedef map < string, string, less< string > > StringMap;
   map < string, string, less< string > > KeyMappingsKeyToCommand;
   map < string, string, less< string > > KeyMappingsCommandToKey;

   string TempDirectory;
   string CacheDirectory;
   string TextureEditor;  //!< Editor for editing texture files (used by ClientEditing)
   string TextEditor; //!< Editor for editing text files (not used yet)
   string ScriptEditor; //!< Editor for editing script files (used by ClientEditing)
   
   string F1HelpFile; //!< File to open when user presses F1
   
   string CollisionAndPhysicsEngine; //!< Filename of collision and physics engine dll
   
   string DebugLevel;  //!< Debug level; 0 is no debug; 3 is lots of debug
   
   string sSimName;  //!< Name of our sim; used by metaverseserver
   
   DatabaseConnectionInfo SimDatabaseInfo;  //!< database connection info for sim database, used by metaverseserver
   DatabaseConnectionInfo AuthServerDatabaseInfo; //!< database connecdtion info for auth server, used by authserver
   
   int iNumAuthServers;
   AuthServerConnectInfo AuthServers[10];

   map < string, string, less< string > > ClientConfig;	//!< holds client app configuration data

   ConfigClass()
   {
   	  TempDirectory = "";
   	  TextureEditor = "";
   	  TextEditor = ""; 
   	  ScriptEditor = ""; 
   	  CacheDirectory = "";
   	  
   	  F1HelpFile = ""; 
   	  
   	  CollisionAndPhysicsEngine = ""; 
   	  
   	  KeyMappingsKeyToCommand.clear();
   	  KeyMappingsCommandToKey.clear();
   	  
   	  DebugLevel = "";
   	  
   	  sSimName = "";
   	  
   	  SimDatabaseInfo.DatabaseName = "";
   	  SimDatabaseInfo.UserName = "";
   	  SimDatabaseInfo.Password = "";

   	  AuthServerDatabaseInfo.DatabaseName = "";
   	  AuthServerDatabaseInfo.UserName = "";
   	  AuthServerDatabaseInfo.Password = "";
   	  
   	  iNumAuthServers = 0;
	  ClientConfig.clear();
   }
   void ReadConfig();  //!< Reads config from config.xml file (just need to do this once; or if the file changes or might have changed)
   
   string GetCommandForKeycode( string sKeycode );  //!<  Returns the command for a particular keycoard (eg "UP")
   string GetKeycodeForCommand( string sCommand );  //!< Returns the keycode for a particular command string such as "UP"
};

#endif // _MVCONFIG_H
