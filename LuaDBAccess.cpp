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
//! \brief provides database access functions to OSMP Lua scripts

// For documentation on each function, see the document LuaScriptEngine.html in the cvs module "documentation"

// Programming Standards:
// You MUST lock the mutex EngineMutex at the start of each function and unlock it when you exit


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
#include "LuaDBAccess.h"

extern lua_State *luaVM;
extern mvWorldStorage World;
extern mvsocket SocketMetaverseServer;
extern char SendBuffer[ 2048 ];
extern char ReadBuffer[ 4097 ];

extern int iMyReference;

extern pthread_mutex_t EngineMutex;

namespace LuaDBAccess
{
    static int WriteDBPrivateValue( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "string" && LuaScriptingAPIHelper::GetTypename( L, 2 ) == "string" )
        {
            int iReference = GetReferenceFromVMRegistry( L );

            string sValuename = lua_tostring( L, 1 );
            string sValue = lua_tostring( L, 2 );

            Object *p_Object = World.GetObjectByReference( iReference );
            if( p_Object != NULL )
            {
                int iOwner = p_Object->iownerreference;
                ostringstream messagestream;
                messagestream << "<setinfo type=\"DBUSERDATA\" ireference=\"" << iReference << "\" iowner=\"" << iOwner <<
                "\" store=\"private\" valuename=\"" << sValuename << "\" value=\"" << sValue << "\">" << endl;
                DEBUG(  "Sending to server " << messagestream.str() );
                SocketMetaverseServer.Send( messagestream.str().c_str() );
            }
        }

        pthread_mutex_unlock( &EngineMutex );
        return 0;
    }

    static int WriteDBPublicValue( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "string" && LuaScriptingAPIHelper::GetTypename( L, 2 ) == "string" )
        {
            int iReference = GetReferenceFromVMRegistry( L );

            string sValuename = lua_tostring( L, 1 );
            string sValue = lua_tostring( L, 2 );

            Object *p_Object = World.GetObjectByReference( iReference );
            if( p_Object != NULL )
            {
                int iOwner = p_Object->iownerreference;

                ostringstream messagestream;
                messagestream << "<setinfo type=\"DBUSERDATA\" ireference=\"" << iReference << "\" iowner=\"" << iOwner <<
                "\" store=\"public\" valuename=\"" << sValuename << "\" value=\"" << sValue << "\" />" << endl;
                DEBUG(  "Sending to server " << messagestream.str() );
                SocketMetaverseServer.Send( messagestream.str().c_str() );
            }
        }

        pthread_mutex_unlock( &EngineMutex );
        return 0;
    }

    static int GetDBPrivateValue( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "string" && LuaScriptingAPIHelper::GetTypename( L, 2 ) == "string" )
        {
            int iReference = GetReferenceFromVMRegistry( L );

            string sClientsidereference = lua_tostring( L, 1 );
            string sValuename = lua_tostring( L, 2 );

            Object *p_Object = World.GetObjectByReference( iReference );
            if( p_Object != NULL )
            {
                int iOwner = p_Object->iownerreference;

                ostringstream messagestream;
                messagestream << "<requestinfo type=\"DBUSERDATA\" ireplytoreference=\"" << iMyReference << "\" ireference=\"" << iReference << "\" idataowner=\"" << iOwner <<
                "\" store=\"private\" valuename=\"" << sValuename << "\" clientsidereference=\"" << sClientsidereference << "\" />" << endl;
                DEBUG(  "Sending to server " << messagestream.str() );
                SocketMetaverseServer.Send( messagestream.str().c_str() );
            }
        }

        pthread_mutex_unlock( &EngineMutex );
        return 0;
    }

    static int GetDBPublicValue( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "string" && LuaScriptingAPIHelper::GetTypename( L, 2 ) == "number" && LuaScriptingAPIHelper::GetTypename( L, 3 ) == "string" )
        {
            int iReference = GetReferenceFromVMRegistry( L );

            string sClientsidereference = lua_tostring( L, 1 );
            int iOwnerReference = lua_tonumber( L, 2 );
            string sValuename = lua_tostring( L, 3 );

            Object *p_Object = World.GetObjectByReference( iReference );
            if( p_Object != NULL )
            {
                ostringstream messagestream;
                messagestream << "<requestinfo type=\"DBUSERDATA\" ireplytoreference=\"" << iMyReference << "\" ireference=\"" << iReference << "\" idataowner=\"" << iOwnerReference <<
                "\" store=\"public\" valuename=\"" << sValuename << "\" clientsidereference=\"" << sClientsidereference << "\" />" << endl;
                DEBUG(  "Sending to server " << messagestream.str() );
                SocketMetaverseServer.Send( messagestream.str().c_str() );
            }
        }

        pthread_mutex_unlock( &EngineMutex );
        return 0;
    }

    void RegisterLuaFunctions( lua_State *pluaVM )
    {
#define LUAREGISTER( f ) lua_register(pluaVM, #f, f )

        LUAREGISTER(GetDBPublicValue);
        LUAREGISTER(GetDBPrivateValue);
        LUAREGISTER(WriteDBPublicValue);
        LUAREGISTER(WriteDBPrivateValue);
    }
}
