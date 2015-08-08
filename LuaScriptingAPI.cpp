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
//! \brief Contains functions available to OSMP Lua scripts

// This module contains functions that are called *only* from Lua scripts
// there are other modules that also contain lua-script-called functions
// This paricular module holds miscellaneous functions that havent been classified
// into other modules yet.

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
#include "LuaScriptingPhysics.h"
#include "LuaDBAccess.h"
#include "LuaMath.h"
#include "LuaScriptingAPITimerProperties.h"
#include "LuaKeyboard.h"

extern lua_State *luaVM;
extern mvWorldStorage World;
extern mvsocket SocketMetaverseServer;
extern char SendBuffer[ 2048 ];
extern char ReadBuffer[ 4097 ];

extern pthread_mutex_t EngineMutex;

// This will be god function only at some point, but it's very useful so...
static int WriteToConsole( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    if( strcmp( lua_typename( L, lua_type( L, 1 ) ), "string" ) == 0 )
    {
        const char *message = lua_tostring( L, 1 );
        DEBUG(  "Message from script: " << message ); // DEBUG
    }

    pthread_mutex_unlock( &EngineMutex );
    return 0;
}

static int DoSmoothMove( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    bool bMovePos = false;
    bool bMoveRot = false;
    bool bMoveColor = false;
    bool bMoveScale = false;

    DEBUG(  "DoSmoothMove" ); // DEBUG

    int iReference = GetReferenceFromVMRegistry( L );

    int iDuration;

    Vector3 NewPos;
    Vector3 NewScale;
    Color NewColor;
    Rot NewRot;

    if( strcmp( lua_typename( L, lua_type( L, 1 ) ), "table" ) == 0 )
    {
        lua_pushnil(L);  /* first key */
        while (lua_next(L, 1) != 0)
        {
            /* `key' is at index -2 and `value' at index -1 */
            string keyname = lua_tostring( L, -2 );
            string valuetype = lua_typename(L, lua_type(L, -1));
            // DEBUG(  "[" << keyname << "] [" << valuetype << "]" ); // DEBUG
            if( keyname == "pos" && valuetype == "table" )
            {
                DEBUG(  "read pos change" ); // DEBUG
                bMovePos = true;
                TableToPos( L, NewPos );
            }
            else if( keyname == "scale" && valuetype == "table" )
            {
                bMoveScale = true;
                TableToScale( L, NewScale );
                //  DEBUG(  NewScale ); // DEBUG
            }
            else if( keyname == "color" && valuetype == "table" )
            {
                bMoveColor = true;
                TableToColor( L, NewColor );
                //  DEBUG(  NewColor ); // DEBUG
            }
            else if( keyname == "rot" && valuetype == "table" )
            {
                bMoveRot = true;
                TableToRot( L, NewRot );
                //  DEBUG(  NewRot ); // DEBUG
            }
            else if( keyname == "duration" && valuetype == "number" )
            {
                iDuration = (int)lua_tonumber( L, -1 );
            }
            lua_pop(L, 1);  /* removes `value'; keeps `key' for next iteration */
        }

        if( ( bMovePos || bMoveScale || bMoveColor || bMoveRot ) && iReference > 0 && iDuration > 0 )
        {
            DEBUG(  "got movedata" ); // DEBUG
            ostringstream messagestream;
            messagestream << "<objectmove ireference=\"" << iReference << "\">";
            if( bMovePos || bMoveScale || bMoveRot )
            {
                DEBUG(  "got pos movedata" ); // DEBUG
                messagestream << "<geometry>";
                if( bMovePos )
                {
                    messagestream << NewPos;
                }
                if( bMoveRot )
                {
                    messagestream << NewRot;
                }
                if( bMoveScale )
                {
                    messagestream << NewScale;
                }
                messagestream << "</geometry>";
            }
            if( bMoveColor )
            {
                messagestream << "<faces><face num=\"0\">" << NewColor << "</face></faces>";
            }
            messagestream << "<dynamics><duration milliseconds=\"" << iDuration << "\"/></dynamics>";
            messagestream << "</objectmove>" << endl;
            SocketMetaverseServer.Send( messagestream.str().c_str() );
        }
    }

    pthread_mutex_unlock( &EngineMutex );
    return 0;
}

static int CreateObject(lua_State *L)
{
    pthread_mutex_lock( &EngineMutex );

    /* get number of arguments */
    int n = lua_gettop(L);
    Vector3 newpos;
    string type = lua_tostring( L, 1 );
    newpos.x = lua_tonumber( L, 2 );
    newpos.y = lua_tonumber( L, 3 );
    newpos.z = lua_tonumber( L, 4 );
    DEBUG(  "rezing " << type << " at " << newpos ); // DEBUG
    sprintf( SendBuffer, "<objectcreate type=\"%s\" iparentreference=0>"
             "<geometry><pos x=\"%f\" y=\"%f\" z=\"%f\"/>"
             "</geometry>"
             "</objectcreate>\n",
             type.c_str(),
             newpos.x, newpos.y, newpos.z
           );
    printf( "Sending to server [%s]\n", SendBuffer );
    SocketMetaverseServer.Send( SendBuffer );

    pthread_mutex_unlock( &EngineMutex );
    return 0;
}

static int DeleteMe(lua_State *L)
{
    pthread_mutex_lock( &EngineMutex );

    /* get number of arguments */
    int n = lua_gettop(L);
    int iReference = GetReferenceFromVMRegistry( L );
    DEBUG(  "deleting object " << iReference ); // DEBUG
    sprintf( SendBuffer, "<objectdelete ireference=\"%i\"/>\n",
             iReference
           );
    printf( "Sending to server [%s]\n", SendBuffer );
    SocketMetaverseServer.Send( SendBuffer );

    pthread_mutex_unlock( &EngineMutex );
    return 0;
}

static int Say( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    /* get number of arguments */
    int n = lua_gettop(L);

    int iReference = GetReferenceFromVMRegistry( L );

    const char *message = lua_tostring( L, 1 );
    // DEBUG(  "Saying " << message ); // DEBUG

    sprintf( SendBuffer, "<comm type=\"say\" message=\"%s\" iowner=\"%i\"/>\n",
             message,
             iReference
           );
    // printf( "Sending to server [%s]\n", SendBuffer );
    SocketMetaverseServer.Send( SendBuffer );

    pthread_mutex_unlock( &EngineMutex );
    return 0;
}

static int GetNumObjects( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    /* get number of arguments */
    int n = lua_gettop(L);
    lua_pushnumber(L, (double)World.iNumObjects );

    pthread_mutex_unlock( &EngineMutex );
    return 1;
}

static int GetObjectReference( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    int iArrayNum = (int)lua_tonumber( L, 1 );

    if( iArrayNum > 0 && iArrayNum < World.iNumObjects )
    {
        lua_pushnumber(L, (double)World.GetObject( iArrayNum )->iReference );
    }
    else
    {
        // invalid iarraynum, returning -1
        lua_pushnumber(L, (double)-1 );
    }

    pthread_mutex_unlock( &EngineMutex );
    return 1;
}

static int HyperlinkAgent( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    int iSourceReference = GetReferenceFromVMRegistry( L );
    if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "number"
            && LuaScriptingAPIHelper::GetTypename( L, 2 ) == "string"
            && LuaScriptingAPIHelper::GetTypename( L, 3 ) == "number" )
    {
        int iTargetReference = (int)lua_tonumber( L, 1 );
        string sTargetServer = lua_tostring( L, 2 );
        int iTargetPort = (int)lua_tonumber( L, 3 );
        ostringstream messagestream;
        messagestream << "<hyperlinkagent itargetreference=\"" << iTargetReference << "\" serverip=\"" << sTargetServer << "\" "
        << "serverport=\"" << iTargetPort << "\" iowner=\"0\"/>" << endl;
        printf( "Sending to server [%s]\n", messagestream.str().c_str() );
        SocketMetaverseServer.Send( messagestream.str().c_str() );
    }

    pthread_mutex_unlock( &EngineMutex );
    return 0;
}

void RegisterMiscellaneousFunctions( lua_State *pluaVM )
{
#define LUAREGISTER( f ) lua_register(pluaVM, #f, f )

    LUAREGISTER(GetNumObjects);
    LUAREGISTER(GetObjectReference);

    LUAREGISTER(DeleteMe);
    LUAREGISTER(CreateObject);

    LUAREGISTER(WriteToConsole);
    LUAREGISTER(Say);

    LUAREGISTER(DoSmoothMove);

    LUAREGISTER(HyperlinkAgent);
}

void RegisterLuaStandardFunctions( lua_State *pluaVM )
{
    RegisterMiscellaneousFunctions( pluaVM );

    RegisterLuaGetObjectPropertiesStandard( pluaVM );
    RegisterLuaSetObjectPropertiesStandard( pluaVM );
    RegisterLuaStandardRPC( pluaVM );
    LuaScriptingAPITimerProperties::RegisterLuaFunctions( pluaVM );
    LuaScriptingPhysics::RegisterLuaFunctions( pluaVM );
    LuaDBAccess::RegisterLuaFunctions( pluaVM );
    LuamvMath::RegisterLuaFunctions( pluaVM );
    LuaKeyboard::RegisterLuaFunctions( pluaVM );
}
