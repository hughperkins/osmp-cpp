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
//! \brief This module groups together functions for setting object properties

// Programming standards:
// - *Only* functions called directly by Lua scripts should be in this module
//   (helper functions go in the module LuaScriptingApiHelper.cpp/.h)
// - You MUST lock the mutex EngineMutex at the start of each function, and unlock it at the end

#include <sstream>
#include <string>
using namespace std;

extern "C"
{
#include <lua.h>
   #include "lauxlib.h"
   #include "lualib.h"
}
#include "pthread.h"

#include "WorldStorage.h"
#include "SocketsClass.h"
#include "Math.h"

#include "LuaScriptingAPIHelper.h"
#include "LuaScriptingAPISetObjectPropertiesStandard.h"

extern lua_State *luaVM;
extern mvWorldStorage World;
extern mvsocket SocketMetaverseServer;
extern char SendBuffer[ 2048 ];
extern char ReadBuffer[ 4097 ];

extern pthread_mutex_t EngineMutex;

static int SetObjectName(lua_State *L)
{
    pthread_mutex_lock( &EngineMutex );

    if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "string" )
    {
        int iReference = GetReferenceFromVMRegistry( L );

        ostringstream messagestream;
        messagestream << "<objectupdate ireference=\"" << iReference << "\" objectname=\"" << lua_tostring( L, 1 ) << "\">" << endl;
        DEBUG(  "Sending to server: " << messagestream.str().c_str() ); // DEBUG
        SocketMetaverseServer.Send( messagestream.str().c_str() );
    }

    pthread_mutex_unlock( &EngineMutex );
    return 0;
}

static int SetPos(lua_State *L)
{
    pthread_mutex_lock( &EngineMutex );

    if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "number" && LuaScriptingAPIHelper::GetTypename( L, 2 ) == "number" && LuaScriptingAPIHelper::GetTypename( L, 3 ) == "number" )
    {
        int iReference = GetReferenceFromVMRegistry( L );

        sprintf( SendBuffer, "<objectupdate ireference=\"%i\">"
                 "<geometry><pos x=\"%f\" y=\"%f\" z=\"%f\"/>"
                 "</geometry>"
                 "</objectupdate>\n",
                 iReference,
                 lua_tonumber( L, 1 ), lua_tonumber( L, 2 ), lua_tonumber( L, 3 )
               );
        printf( "Sending to server [%s]\n", SendBuffer );
        SocketMetaverseServer.Send( SendBuffer );
    }

    pthread_mutex_unlock( &EngineMutex );
    return 0;
}

static int SetScale(lua_State *L)
{
    pthread_mutex_lock( &EngineMutex );

    if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "number" && LuaScriptingAPIHelper::GetTypename( L, 2 ) == "number" && LuaScriptingAPIHelper::GetTypename( L, 3 ) == "number" )
    {
        int iReference = GetReferenceFromVMRegistry( L );

        sprintf( SendBuffer, "<objectupdate ireference=\"%i\">"
                 "<geometry><scale x=\"%f\" y=\"%f\" z=\"%f\"/>"
                 "</geometry>"
                 "</objectupdate>\n",
                 iReference,
                 lua_tonumber( L, 1 ), lua_tonumber( L, 2 ), lua_tonumber( L, 3 )
               );
        printf( "Sending to server [%s]\n", SendBuffer );
        SocketMetaverseServer.Send( SendBuffer );
    }

    pthread_mutex_unlock( &EngineMutex );
    return 0;
}

static int SetColor(lua_State *L)
{
    pthread_mutex_lock( &EngineMutex );

    if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "number" && LuaScriptingAPIHelper::GetTypename( L, 2 ) == "number" && LuaScriptingAPIHelper::GetTypename( L, 3 ) == "number" )
    {
        int iReference = GetReferenceFromVMRegistry( L );

        sprintf( SendBuffer, "<objectupdate ireference=\"%i\">"
                 "<geometry><color r=\"%f\" g=\"%f\" b=\"%f\"/>"
                 "</geometry>"
                 "</objectupdate>\n",
                 iReference,
                 lua_tonumber( L, 1 ), lua_tonumber( L, 2 ), lua_tonumber( L, 3 )
               );
        printf( "Sending to server [%s]\n", SendBuffer );
        SocketMetaverseServer.Send( SendBuffer );
    }

    pthread_mutex_unlock( &EngineMutex );
    return 0;
}

static int SetRot(lua_State *L)
{
    pthread_mutex_lock( &EngineMutex );

    if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "number" && LuaScriptingAPIHelper::GetTypename( L, 2 ) == "number" && LuaScriptingAPIHelper::GetTypename( L, 3 ) == "number" && LuaScriptingAPIHelper::GetTypename( L, 4 ) == "number" )
    {
        int iReference = GetReferenceFromVMRegistry( L );

        sprintf( SendBuffer, "<objectupdate ireference=\"%i\">"
                 "<geometry><rot x=\"%f\" y=\"%f\" z=\"%f\" s=\"%f\"/>"
                 "</geometry>"
                 "</objectupdate>\n",
                 iReference,
                 lua_tonumber( L, 1 ), lua_tonumber( L, 2 ), lua_tonumber( L, 3 ), lua_tonumber( L, 4 )
               );
        printf( "Sending to server [%s]\n", SendBuffer );
        SocketMetaverseServer.Send( SendBuffer );
    }

    pthread_mutex_unlock( &EngineMutex );
    return 0;
}

static int SetObjectType( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "string" )
    {
        int iReference = GetReferenceFromVMRegistry( L );
        const char *ObjectType = lua_tostring( L, 1 );

        if( strcmp( ObjectType, "CUBE" ) == 0
                ||  strcmp( ObjectType, "SPHERE" ) == 0
                ||  strcmp( ObjectType, "CONE" ) == 0
                ||  strcmp( ObjectType, "CYLINDER" ) == 0 )
            sprintf( SendBuffer, "<objectupdate ireference=\"%i\" type=\"%s\">"
                     "</objectupdate>\n",
                     iReference,
                     ObjectType
                   );
        printf( "Sending to server [%s]\n", SendBuffer );
        SocketMetaverseServer.Send( SendBuffer );
    }

    pthread_mutex_unlock( &EngineMutex );
    return 0;
}

void RegisterLuaSetObjectPropertiesStandard( lua_State *pluaVM )
{
#define LUAREGISTER( f ) lua_register(pluaVM, #f, f )

    LUAREGISTER(SetObjectType);
    LUAREGISTER(SetObjectName);
    LUAREGISTER(SetPos);
    LUAREGISTER(SetRot);
    LUAREGISTER(SetScale);
    LUAREGISTER(SetColor);
}
