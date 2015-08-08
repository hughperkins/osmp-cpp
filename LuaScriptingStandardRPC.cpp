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
//! \brief This module groups together functions for script RPC

// This module groups together functions for script RPC

// For documentation on each function, see the document LuaScriptEngine.html in the cvs module "documentation"

// Programming standards:
// - *Only* functions called directly by Lua scripts should be in this module
//   (helper functions go in the module LuaScriptingApiHelper.cpp/.h)
// - You MUST lock the mutex EngineMutex at the start of each function, and unlock it at the end

#include <set>
#include <map>
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
#include "ThreadWrapper.h"

#include "scriptingenginelua.h"
#include "LuaScriptingAPIHelper.h"
#include "LuaScriptingStandardRPC.h"
#include "LuaEventClass.h"

extern lua_State *luaVM;
extern mvWorldStorage World;
extern mvsocket SocketMetaverseServer;
extern char SendBuffer[ 2048 ];
extern char ReadBuffer[ 4097 ];

//! a multicast group; used by Lua scripting engine for RPC multicast calls
class MULTICASTGROUP
{
public:
    string sGroupName;
    set < int >
    iMemberReferences;
};

map < string, MULTICASTGROUP > MulticastGroups;  //!< All registered multicast groups

extern map < int, ObjectVMInfoClass > ObjectVMs;

extern pthread_mutex_t EngineMutex;

static int RegisterForMulticastGroup( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    DEBUG(  "registerformulticastgroup()" ); // DEBUG
    int iReference = GetReferenceFromVMRegistry( L );
    if( strcmp( lua_typename( L, lua_type( L, 1 ) ), "string" ) == 0 )
    {
        string sGroup = lua_tostring( L, 1 );
        DEBUG(  "ref " << iReference << " trying to register for multicast gorup " << sGroup ); // DEBUG
        if( MulticastGroups.find( sGroup ) == MulticastGroups.end() )
        {
            MULTICASTGROUP Multicastgroup;
            Multicastgroup.sGroupName = sGroup;
            MulticastGroups.insert( pair < string, MULTICASTGROUP >( sGroup, Multicastgroup ) );
        }
        MulticastGroups.find( sGroup )->second.iMemberReferences.insert( iReference );
    }

    pthread_mutex_unlock( &EngineMutex );
    return 0;
}

static int SendSynchroRPC( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    int iReference = GetReferenceFromVMRegistry( L );
    int args = 0;
    if (LuaScriptingAPIHelper::GetTypename( L, 1 ) == "number")
    {
        int iTargetReference = lua_tonumber( L, 1 );
        lua_remove( L, 1 );
        ObjectVMInfoClass &ObjectVMInfo = ObjectVMs.find( iTargetReference )->second;

        //struct timespec delay;
        //delay.tv_sec = 0;
        //delay.tv_nsec = 1000 * 1000 * 1000;

        DEBUG(  "synchroRPC() Checking for target vm running or not..." );
        while( ObjectVMInfo.bVMIsRunning == true )
        {
            pthread_mutex_unlock( &EngineMutex );
            PauseThreadMilliseconds( 1000 );
            //pthread_delay_np( &delay );
            pthread_mutex_lock( &EngineMutex );
        }
        DEBUG(  "synchroRPC() target vm not running, we have mutex, send rpc..." );
        args = 0;
        lua_State *pluaVM = ObjectVMInfo.pVM;
        lua_settop(pluaVM, 0);
        lua_getglobal( pluaVM, "SynchroRPC" );
        lua_pushnumber( pluaVM, (float)iReference );

        args = LuaScriptingAPIHelper::SwapParams(L, pluaVM);
        args++;
        pthread_mutex_unlock( &EngineMutex );
        LuaScriptingAPIHelper::DoPCall(pluaVM, args, LUA_MULTRET, 0);
        pthread_mutex_lock( &EngineMutex );
        args = LuaScriptingAPIHelper::SwapParams(pluaVM, L);
    }
    else
    {
        LuaScriptingAPIHelper::SayFromObject(iReference,  "First parameter to sendSyncroRPC was not a number");
    }

    pthread_mutex_unlock( &EngineMutex );
    DEBUG("RPC returned " << args << " results.");
    return args;
}

static int SendMulticastRPC( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    int iReference = GetReferenceFromVMRegistry( L );
    if( strcmp( lua_typename( L, lua_type( L, 1 ) ), "string" ) == 0 && strcmp( lua_typename( L, lua_type( L, 2 ) ), "string" ) == 0 )
    {
        string sGroupname = lua_tostring( L, 1 );
        string sMessage = lua_tostring( L, 2 );

        if( MulticastGroups.find(  sGroupname ) != MulticastGroups.end() )
        {
            MULTICASTGROUP &rMulticastgroup = MulticastGroups.find(  sGroupname )->second;

            for( set < int >::iterator iterator = rMulticastgroup.iMemberReferences.begin(); iterator != rMulticastgroup.iMemberReferences.end(); iterator++ )
            {
                lua_State *pluaVM = GetVMForReference( *iterator );
                if( pluaVM != NULL )
                {
                    EventInfoMulticastRPC *pEvent = new EventInfoMulticastRPC;

                    pEvent->sEventName = "AsyncRPC";
                    pEvent->sEventClass = "EventInfoMulticastRPC";
                    pEvent->iVMNum = *iterator;
                    pEvent->iSenderReference = iReference;
                    pEvent->sMessage = sMessage;

                    // QueueFunction( *iterator, "AsyncRPC", sMessage );
                    QueueEvent( pEvent );
                }
            }
        }
    }

    pthread_mutex_unlock( &EngineMutex );
    return 0;
}

void RegisterLuaStandardRPC( lua_State *pluaVM )
{
#define LUAREGISTER( f ) lua_register(pluaVM, #f, f )

    LUAREGISTER(SendSynchroRPC);
    LUAREGISTER(RegisterForMulticastGroup);
    LUAREGISTER(SendMulticastRPC);
}
