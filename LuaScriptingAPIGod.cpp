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
//! \brief This module groups together god functions called by Lua scripts
//!
//! This module groups together god functions called by Lua scripts
//! Currently it's not used by the luascriptingengine, but we could imagine that
//! certain console/admin scripts could access functions here.

// Programming standards:
// - *Only* functions called directly by Lua scripts should be in this module
//   (helper functions go in the module LuaScriptingApiHelper.cpp/.h)
// - You MUST lock the mutex EngineMutex at the start of each function, and unlock it at the end
//   (note: need to migrate current functions to comply with this standard)

#include <sstream>
#include <string>
using namespace std;

extern "C"
{
#include <lua.h>
   #include "lauxlib.h"
   #include "lualib.h"
}

#include "Diag.h"
#include "WorldStorage.h"
#include "LuaScriptingAPI.h"
#include "SocketsClass.h"
#include "LuaScriptingAPIHelper.h"
#include "Math.h"

extern lua_State *luaVM;
extern mvWorldStorage World;
extern mvsocket SocketMetaverseServer;
extern char SendBuffer[ 2048 ];
extern char ReadBuffer[ 4097 ];

//=========================================================================================
// God functions
//
// God functions can affect anything, which makes it hard to work out where the script is
// without access to the server console, or else they are otherwise a bit too powerful
// for standard use
//======================================================================================

static int MoveObject(lua_State *L)
{
    /* get number of arguments */
    int n = lua_gettop(L);
    int iReference = (int)lua_tonumber( L, 1 );
    Vector3 newpos;
    newpos.x = lua_tonumber( L, 2 );
    newpos.y = lua_tonumber( L, 3 );
    newpos.z = lua_tonumber( L, 4 );
    int iDuration = (int)lua_tonumber( L, 5 );
    DEBUG(  "moving object " << iReference << " to " << newpos ); // DEBUG
    sprintf( SendBuffer, "<objectmove ireference=\"%i\">"
             "<geometry><pos x=\"%f\" y=\"%f\" z=\"%f\"/>"
             "</geometry>"
             "<dynamics><duration milliseconds=\"%i\"/></dynamics>"
             "</objectmove>\n",
             iReference,
             newpos.x, newpos.y, newpos.z,
             iDuration
           );
    printf( "Sending to server [%s]\n", SendBuffer );
    SocketMetaverseServer.Send( SendBuffer );
    return 0;
}

static int MoveObjectTableVersion( lua_State *L )
{
    bool bMovePos = false;
    bool bMoveRot = false;
    bool bMoveColor = false;
    bool bMoveScale = false;

    int iReference;

    iReference = (int)lua_tonumber( L, 1 );

    int iDuration;

    Vector3 NewPos;
    Vector3 NewScale;
    Color NewColor;
    Rot NewRot;

    if( strcmp( lua_typename( L, lua_type( L, 2 ) ), "table" ) == 0 )
    {
        lua_pushnil(L);  /* first key */
        while (lua_next(L, 2) != 0)
        {
            /* `key' is at index -2 and `value' at index -1 */
            string keyname = lua_tostring( L, -2 );
            string valuetype = lua_typename(L, lua_type(L, -1));
            // DEBUG(  "[" << keyname << "] [" << valuetype << "]" ); // DEBUG
            if( keyname == "pos" && valuetype == "table" )
            {
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

        if( bMoveRot || bMoveScale || bMoveColor || bMoveRot && iReference > 0 && iDuration > 0 )
        {
            ostringstream messagestream;
            messagestream << "<objectmove ireference=\"" << iReference << "\">";
            if( bMoveRot || bMoveScale || bMoveRot )
            {
                messagestream << "<geometry>";
                if( bMoveRot )
                {
                    messagestream << NewRot;
                }
                if( bMovePos )
                {
                    messagestream << NewPos;
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

    return 0;
}

static int SmoothScaleObject(lua_State *L)
{
    /* get number of arguments */
    int n = lua_gettop(L);
    int iReference = (int)lua_tonumber( L, 1 );
    Vector3 newscale;
    newscale.x = lua_tonumber( L, 2 );
    newscale.y = lua_tonumber( L, 3 );
    newscale.z = lua_tonumber( L, 4 );
    int iDuration = (int)lua_tonumber( L, 5 );
    //DEBUG(  "moving object " << iReference << " to " << newpos ); // DEBUG
    sprintf( SendBuffer, "<objectmove ireference=\"%i\">"
             "<geometry><scale x=\"%f\" y=\"%f\" z=\"%f\"/>"
             "</geometry>"
             "<dynamics><duration milliseconds=\"%i\"/></dynamics>"
             "</objectmove>\n",
             iReference,
             newscale.x, newscale.y, newscale.z,
             iDuration
           );
    printf( "Sending to server [%s]\n", SendBuffer );
    SocketMetaverseServer.Send( SendBuffer );
    return 0;
}

static int SmoothColorObject(lua_State *L)
{
    /* get number of arguments */
    int n = lua_gettop(L);
    int iReference = (int)lua_tonumber( L, 1 );
    Color newcolor;
    newcolor.r = lua_tonumber( L, 2 );
    newcolor.g = lua_tonumber( L, 3 );
    newcolor.b = lua_tonumber( L, 4 );
    int iDuration = (int)lua_tonumber( L, 5 );
    //DEBUG(  "moving object " << iReference << " to " << newpos ); // DEBUG
    sprintf( SendBuffer, "<objectmove ireference=\"%i\">"
             "<faces><face num=\"0\"><color r=\"%f\" g=\"%f\" b=\"%f\"/></face></faces>"
             "<dynamics><duration milliseconds=\"%i\"/></dynamics>"
             "</objectmove>\n",
             iReference,
             newcolor.r, newcolor.g, newcolor.b,
             iDuration
           );
    printf( "Sending to server [%s]\n", SendBuffer );
    SocketMetaverseServer.Send( SendBuffer );
    return 0;
}

static int DeleteObject(lua_State *L)
{
    /* get number of arguments */
    int n = lua_gettop(L);
    int iReference = (int)lua_tonumber( L, 1 );
    DEBUG(  "deleting object " << iReference ); // DEBUG
    sprintf( SendBuffer, "<objectdelete ireference=\"%i\"/>\n",
             iReference
           );
    printf( "Sending to server [%s]\n", SendBuffer );
    SocketMetaverseServer.Send( SendBuffer );
    return 0;
}

void RegisterLuaGodFunctions( lua_State *pluaVM )
{
#define LUAREGISTER( f ) lua_register(pluaVM, #f, f )

    LUAREGISTER(MoveObject);
    LUAREGISTER(DeleteObject);
    LUAREGISTER(SmoothScaleObject);
    LUAREGISTER(SmoothColorObject);
    LUAREGISTER(MoveObjectTableVersion);
}


