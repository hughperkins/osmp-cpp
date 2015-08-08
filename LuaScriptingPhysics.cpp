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
//! \brief Physics functions callable from OSMP Lua scripts

// This module contains functions that are called *only* from Lua scripts

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
#include "LuaScriptingAPI.h"
#include "LuaScriptingAPIGetObjectPropertiesStandard.h"
#include "LuaScriptingAPISetObjectPropertiesStandard.h"
#include "LuaScriptingStandardRPC.h"

extern lua_State *luaVM;
extern mvWorldStorage World;
extern mvsocket SocketMetaverseServer;
extern char SendBuffer[ 2048 ];
extern char ReadBuffer[ 4097 ];

extern pthread_mutex_t EngineMutex;

namespace LuaScriptingPhysics
{
    static int SetLocalForce( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "number" && LuaScriptingAPIHelper::GetTypename( L, 2 ) == "number" && LuaScriptingAPIHelper::GetTypename( L, 3 ) == "number" )
        {
            int iReference = GetReferenceFromVMRegistry( L );

            ostringstream messagestream;
            messagestream << "<objectmove ireference=\"" << iReference << "\">"
            "<physics>"
            "<localforce x=\"" << lua_tonumber( L, 1 ) << "\" y=\"" << lua_tonumber( L, 2 ) << "\" z=\"" << lua_tonumber( L, 3 ) << "\"/>"
            "</physics>"
            "</objectmove>" << endl;
            DEBUG(  "Sending to server " << messagestream.str() );
            SocketMetaverseServer.Send( messagestream.str().c_str() );
        }

        pthread_mutex_unlock( &EngineMutex );
        return 0;
    }

    static int GetLocalForce( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        int iReference = GetReferenceFromVMRegistry( L );

        Object *p_Object = World.GetObjectByReference( iReference );

        lua_newtable( L );

        lua_pushstring( L, "x" );
        lua_pushnumber( L, p_Object->vLocalForce.x );
        lua_settable( L, -3 );

        lua_pushstring( L, "y" );
        lua_pushnumber( L, p_Object->vLocalForce.y );
        lua_settable( L, -3 );

        lua_pushstring( L, "z" );
        lua_pushnumber( L, p_Object->vLocalForce.z );
        lua_settable( L, -3 );

        pthread_mutex_unlock( &EngineMutex );
        return 1;
    }

    static int GetLocalTorque( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        int iReference = GetReferenceFromVMRegistry( L );

        Object *p_Object = World.GetObjectByReference( iReference );

        lua_newtable( L );

        lua_pushstring( L, "x" );
        lua_pushnumber( L, p_Object->vLocalTorque.x );
        lua_settable( L, -3 );

        lua_pushstring( L, "y" );
        lua_pushnumber( L, p_Object->vLocalTorque.y );
        lua_settable( L, -3 );

        lua_pushstring( L, "z" );
        lua_pushnumber( L, p_Object->vLocalTorque.z );
        lua_settable( L, -3 );

        pthread_mutex_unlock( &EngineMutex );
        return 1;
    }

    static int SetLocalTorque( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "number" && LuaScriptingAPIHelper::GetTypename( L, 2 ) == "number" && LuaScriptingAPIHelper::GetTypename( L, 3 ) == "number" )
        {
            int iReference = GetReferenceFromVMRegistry( L );

            ostringstream messagestream;
            messagestream << "<objectmove ireference=\"" << iReference << "\">"
            "<physics>"
            "<localtorque x=\"" << lua_tonumber( L, 1 ) << "\" y=\"" << lua_tonumber( L, 2 ) << "\" z=\"" << lua_tonumber( L, 3 ) << "\"/>"
            "</physics>"
            "</objectmove>" << endl;
            DEBUG(  "Sending to server " << messagestream.str() );
            SocketMetaverseServer.Send( messagestream.str().c_str() );
        }

        pthread_mutex_unlock( &EngineMutex );
        return 0;
    }

    static int SetAngularVelocity( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "number" && LuaScriptingAPIHelper::GetTypename( L, 2 ) == "number" && LuaScriptingAPIHelper::GetTypename( L, 3 ) == "number" )
        {
            int iReference = GetReferenceFromVMRegistry( L );

            ostringstream messagestream;
            messagestream << "<objectmove ireference=\"" << iReference << "\">"
            "<physics>"
            "<angularvelocity x=\"" << lua_tonumber( L, 1 ) << "\" y=\"" << lua_tonumber( L, 2 ) << "\" z=\"" << lua_tonumber( L, 3 ) << "\"/>"
            "</physics>"
            "</objectmove>" << endl;
            DEBUG(  "Sending to server " << messagestream.str() );
            SocketMetaverseServer.Send( messagestream.str().c_str() );
        }

        pthread_mutex_unlock( &EngineMutex );
        return 0;
    }

    static int GetAngularVelocity( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        int iReference = GetReferenceFromVMRegistry( L );

        Object *p_Object = World.GetObjectByReference( iReference );

        lua_newtable( L );

        lua_pushstring( L, "x" );
        lua_pushnumber( L, p_Object->vAngularVelocity.x );
        lua_settable( L, -3 );

        lua_pushstring( L, "y" );
        lua_pushnumber( L, p_Object->vAngularVelocity.y );
        lua_settable( L, -3 );

        lua_pushstring( L, "z" );
        lua_pushnumber( L, p_Object->vAngularVelocity.z );
        lua_settable( L, -3 );

        pthread_mutex_unlock( &EngineMutex );
        return 1;
    }

    static int SetLinearVelocity( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "number" && LuaScriptingAPIHelper::GetTypename( L, 2 ) == "number" && LuaScriptingAPIHelper::GetTypename( L, 3 ) == "number" )
        {
            int iReference = GetReferenceFromVMRegistry( L );

            ostringstream messagestream;
            messagestream << "<objectmove ireference=\"" << iReference << "\">"
            "<physics>"
            << "<linearvelocity x=\"" << lua_tonumber( L, 1 ) << "\" y=\"" << lua_tonumber( L, 2 ) << "\" z=\"" << lua_tonumber( L, 3 ) << "\"/>"
            << "</physics>"
            "</objectmove>" << endl;
            DEBUG(  "Sending to server " << messagestream.str() );
            SocketMetaverseServer.Send( messagestream.str().c_str() );
        }

        pthread_mutex_unlock( &EngineMutex );
        return 0;
    }

    static int GetLinearVelocity( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        int iReference = GetReferenceFromVMRegistry( L );

        Object *p_Object = World.GetObjectByReference( iReference );

        lua_newtable( L );

        lua_pushstring( L, "x" );
        lua_pushnumber( L, p_Object->vVelocity.x );
        lua_settable( L, -3 );

        lua_pushstring( L, "y" );
        lua_pushnumber( L, p_Object->vVelocity.y );
        lua_settable( L, -3 );

        lua_pushstring( L, "z" );
        lua_pushnumber( L, p_Object->vVelocity.z );
        lua_settable( L, -3 );

        pthread_mutex_unlock( &EngineMutex );
        return 1;
    }

    static int SetForce( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        int iReference = GetReferenceFromVMRegistry( L );

        pthread_mutex_unlock( &EngineMutex );
        return 0;
    }

    static int GetForce( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        int iReference = GetReferenceFromVMRegistry( L );

        pthread_mutex_unlock( &EngineMutex );
        return 1;
    }

    static int SetTorque( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        int iReference = GetReferenceFromVMRegistry( L );

        pthread_mutex_unlock( &EngineMutex );
        return 0;
    }

    static int GetTorque( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        int iReference = GetReferenceFromVMRegistry( L );

        pthread_mutex_unlock( &EngineMutex );
        return 1;
    }

    static int SetPhysics( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "boolean" )
        {
            int iReference = GetReferenceFromVMRegistry( L );
            bool bPhysicsOn = lua_toboolean( L, 1 );

            if( bPhysicsOn )
            {
                sprintf( SendBuffer, "<objectupdate ireference=\"%i\">"
                         "<physics state=\"on\"/>"
                         "</objectupdate>\n",
                         iReference
                       );
            }
            else
            {
                sprintf( SendBuffer, "<objectupdate ireference=\"%i\">"
                         "<physics state=\"off\"/>"
                         "</objectupdate>\n",
                         iReference
                       );
            }
            DEBUG( "Sending to server " << SendBuffer );
            SocketMetaverseServer.Send( SendBuffer );
        }

        pthread_mutex_unlock( &EngineMutex );
        return 0;
    }

    static int SetPhantom( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );
        int iReference = GetReferenceFromVMRegistry( L );
        if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "boolean" )
        {
            bool bPhantomOn = lua_toboolean( L, 1 );

            if( bPhantomOn )
            {
                sprintf( SendBuffer, "<objectupdate ireference=\"%i\">"
                         "<phantom state=\"on\"/>"
                         "</objectupdate>\n",
                         iReference
                       );
            }
            else
            {
                sprintf( SendBuffer, "<objectupdate ireference=\"%i\">"
                         "<phantom state=\"off\"/>"
                         "</objectupdate>\n",
                         iReference
                       );
            }
            DEBUG( "Sending to server " << SendBuffer );
            SocketMetaverseServer.Send( SendBuffer );
        }
        else
        {
            LuaScriptingAPIHelper::SayFromObject(iReference, "1st param to SetPhantom must be boolean");
        }

        pthread_mutex_unlock( &EngineMutex );
        return 0;
    }

    static int SetTerrain( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );
        int iReference = GetReferenceFromVMRegistry( L );
        if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "boolean" )
        {
            bool bTerrainOn = lua_toboolean( L, 1 );

            if( bTerrainOn )
            {
                sprintf( SendBuffer, "<objectupdate ireference=\"%i\">"
                         "<terrain state=\"on\"/>"
                         "</objectupdate>\n",
                         iReference
                       );
            }
            else
            {
                sprintf( SendBuffer, "<objectupdate ireference=\"%i\">"
                         "<terrain state=\"off\"/>"
                         "</objectupdate>\n",
                         iReference
                       );
            }
            DEBUG( "Sending to server " << SendBuffer );
            SocketMetaverseServer.Send( SendBuffer );
        }
        else
        {
            LuaScriptingAPIHelper::SayFromObject(iReference, "1st param to SetTerrain must be boolean");
        }

        pthread_mutex_unlock( &EngineMutex );
        return 0;
    }

    static int SetGravity( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "boolean" )
        {
            int iReference = GetReferenceFromVMRegistry( L );
            bool bGravityOn = lua_toboolean( L, 1 );

            ostringstream messagestream;
            if( bGravityOn )
            {
                messagestream << "<objectupdate ireference=\"" << iReference << "\">"
                "<physics gravity=\"on\"/>"
                "</objectupdate>" << endl;
            }
            else
            {
                messagestream << "<objectupdate ireference=\"" << iReference << "\">"
                "<physics gravity=\"off\"/>"
                "</objectupdate>" << endl;
            }
            DEBUG(  "Sending to server: " << messagestream.str() ); // DEBUG
            SocketMetaverseServer.Send( messagestream.str().c_str() );
        }

        pthread_mutex_unlock( &EngineMutex );
        return 0;
    }

    static int GetPhantom( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        /* get number of arguments */
        int n = lua_gettop(L);
        int iReference = GetReferenceFromVMRegistry( L );
        Object *p_Object;
        p_Object = World.GetObjectByReference( iReference );
        lua_pushboolean( L, p_Object->bPhantomEnabled );

        pthread_mutex_unlock( &EngineMutex );
        return 1;
    }

    static int GetTerrain( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        /* get number of arguments */
        int n = lua_gettop(L);
        int iReference = GetReferenceFromVMRegistry( L );
        Object *p_Object;
        p_Object = World.GetObjectByReference( iReference );
        lua_pushboolean( L, p_Object->bTerrainEnabled );

        pthread_mutex_unlock( &EngineMutex );
        return 1;
    }

    static int GetPhysics( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        /* get number of arguments */
        int n = lua_gettop(L);
        int iReference = GetReferenceFromVMRegistry( L );
        Object *p_Object;
        p_Object = World.GetObjectByReference( iReference );
        lua_pushboolean( L, p_Object->bPhysicsEnabled );

        pthread_mutex_unlock( &EngineMutex );
        return 1;
    }

    static int GetGravity( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        /* get number of arguments */
        int n = lua_gettop(L);
        int iReference = GetReferenceFromVMRegistry( L );
        Object *p_Object;
        p_Object = World.GetObjectByReference( iReference );
        lua_pushboolean( L, p_Object->bGravityEnabled );

        pthread_mutex_unlock( &EngineMutex );
        return 1;
    }

    void RegisterLuaFunctions( lua_State *pluaVM )
    {
#define LUAREGISTER( f ) lua_register(pluaVM, #f, f )

        LUAREGISTER(SetPhysics);
        LUAREGISTER(SetPhantom);
        LUAREGISTER(SetTerrain);
        LUAREGISTER(GetPhantom);
        LUAREGISTER(GetTerrain);
        LUAREGISTER(GetPhysics);
        LUAREGISTER(SetGravity);
        LUAREGISTER(GetGravity);
        LUAREGISTER(SetLocalForce);
        LUAREGISTER(GetLocalForce);
        LUAREGISTER(SetLocalTorque);
        LUAREGISTER(GetLocalTorque);
        LUAREGISTER(SetAngularVelocity);
        LUAREGISTER(GetAngularVelocity);
        LUAREGISTER(SetLinearVelocity);
        LUAREGISTER(GetLinearVelocity);
    }
}
