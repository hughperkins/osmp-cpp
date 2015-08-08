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
//! \brief This module contains helper functions used by the Lua Scripting API functions
// see headerfile luascriptingapihelper.h for documentation

#include <iostream>
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
#include "BasicTypes.h"
#include "Math.h"
#include "scriptingenginelua.h"
#include "LuaScriptingAPI.h"
#include "LuaScriptingAPIHelper.h"

extern map < int, ObjectVMInfoClass > ObjectVMs;

extern void SendClientMessage( const char *message );

lua_State *GetVMForReference( int iReference )
{
    for( ObjectVMIterator iterator = ObjectVMs.begin(); iterator != ObjectVMs.end(); iterator++ )
    {
        if( iterator->second.iObjectReference == iReference )
        {
            return iterator->second.pVM;
        }
    }
    return NULL;
}

void TableToPos( lua_State *L, Vector3 &NewPos )
{
    if( strcmp( lua_typename( L, lua_type( L, -1 ) ), "table" ) == 0 )
    {
        //cout << "tabletopos" << endl;
        lua_pushnil( L );
        while (lua_next(L, -2) != 0)
        {
            /* `key' is at index -2 and `value' at index -1 */
            string keyname = lua_tostring( L, -2 );
            string valuetype = lua_typename(L, lua_type(L, -1));
            //cout << keyname << " " << valuetype << endl;
            if( keyname == "x" )
            {
                NewPos.x = lua_tonumber( L, -1 );
            }
            else if( keyname == "y" )
            {
                NewPos.y = lua_tonumber( L, -1 );
            }
            else if( keyname == "z" )
            {
                NewPos.z = lua_tonumber( L, -1 );
            }
            lua_pop(L, 1);  /* removes `value'; keeps `key' for next iteration */
        }
    }
}

void TableToVector( lua_State *L, Vector3 &NewVector )
{
    if( strcmp( lua_typename( L, lua_type( L, -1 ) ), "table" ) == 0 )
    {
        //cout << "tabletopos" << endl;
        lua_pushnil( L );
        while (lua_next(L, -2) != 0)
        {
            /* `key' is at index -2 and `value' at index -1 */
            string keyname = lua_tostring( L, -2 );
            string valuetype = lua_typename(L, lua_type(L, -1));
            //cout << keyname << " " << valuetype << endl;
            if( keyname == "x" )
            {
                NewVector.x = lua_tonumber( L, -1 );
            }
            else if( keyname == "y" )
            {
                NewVector.y = lua_tonumber( L, -1 );
            }
            else if( keyname == "z" )
            {
                NewVector.z = lua_tonumber( L, -1 );
            }
            lua_pop(L, 1);  /* removes `value'; keeps `key' for next iteration */
        }
    }
}

void TableToScale( lua_State *L, Vector3 &NewScale )
{
    if( strcmp( lua_typename( L, lua_type( L, -1 ) ), "table" ) == 0 )
    {
        //cout << "tabletopos" << endl;
        lua_pushnil( L );
        while (lua_next(L, -2) != 0)
        {
            /* `key' is at index -2 and `value' at index -1 */
            string keyname = lua_tostring( L, -2 );
            string valuetype = lua_typename(L, lua_type(L, -1));
            //cout << keyname << " " << valuetype << endl;
            if( keyname == "x" )
            {
                NewScale.x = lua_tonumber( L, -1 );
            }
            else if( keyname == "y" )
            {
                NewScale.y = lua_tonumber( L, -1 );
            }
            else if( keyname == "z" )
            {
                NewScale.z = lua_tonumber( L, -1 );
            }
            lua_pop(L, 1);  /* removes `value'; keeps `key' for next iteration */
        }
    }
}

void TableToColor( lua_State *L, Color &NewColor )
{
    if( strcmp( lua_typename( L, lua_type( L, -1 ) ), "table" ) == 0 )
    {
        //cout << "tabletopos" << endl;
        lua_pushnil( L );
        while (lua_next(L, -2) != 0)
        {
            /* `key' is at index -2 and `value' at index -1 */
            string keyname = lua_tostring( L, -2 );
            string valuetype = lua_typename(L, lua_type(L, -1));
            //cout << keyname << " " << valuetype << endl;
            if( keyname == "r" )
            {
                NewColor.r = lua_tonumber( L, -1 );
            }
            else if( keyname == "g" )
            {
                NewColor.g = lua_tonumber( L, -1 );
            }
            else if( keyname == "b" )
            {
                NewColor.b = lua_tonumber( L, -1 );
            }
            lua_pop(L, 1);  /* removes `value'; keeps `key' for next iteration */
        }
    }
}

void TableToRot( lua_State *L, Rot &NewRot )
{
    if( strcmp( lua_typename( L, lua_type( L, -1 ) ), "table" ) == 0 )
    {
        //cout << "tabletopos" << endl;
        lua_pushnil( L );
        while (lua_next(L, -2) != 0)
        {
            /* `key' is at index -2 and `value' at index -1 */
            string keyname = lua_tostring( L, -2 );
            string valuetype = lua_typename(L, lua_type(L, -1));
            //cout << keyname << " " << valuetype << endl;
            if( keyname == "x" )
            {
                NewRot.x = lua_tonumber( L, -1 );
            }
            else if( keyname == "y" )
            {
                NewRot.y = lua_tonumber( L, -1 );
            }
            else if( keyname == "z" )
            {
                NewRot.z = lua_tonumber( L, -1 );
            }
            else if( keyname == "s" )
            {
                NewRot.s = lua_tonumber( L, -1 );
            }
            lua_pop(L, 1);  /* removes `value'; keeps `key' for next iteration */
        }
    }
}

// Retrieves iREference of object, which we prevoiusly stored in VM's local registry
int GetReferenceFromVMRegistry( lua_State *L )
{
    int iReference;

    iReference = 0;

    //cout << "reading registry..." << lua_typename(L, lua_type(L, LUA_REGISTRYINDEX )) << endl;
    lua_pushnil( L );
    while (lua_next(L, LUA_REGISTRYINDEX ) != 0)
    {
        string keyname = lua_tostring( L, -2 );
        string valuetype = lua_typename(L, lua_type(L, -1));
        DEBUG("[" << keyname << "] [" << valuetype << "]" << endl);
        if( keyname == "iReference" && valuetype == "number" )
        {
            iReference = (int)lua_tonumber( L, -1 );
            DEBUG("Got iReference: " << iReference << endl);
        }
        lua_pop(L, 1);  /* removes `value'; keeps `key' for next iteration */
    }

    return iReference;
}

namespace LuaScriptingAPIHelper  // Add new functions to this namespace
{
    string GetTypename( lua_State *L, int iStackPos )
    {
        return lua_typename( L, lua_type( L, iStackPos ) );
    }

    bool DoPCall(lua_State *L, int nargs, int nresults, int errfunc)
    {
        int iVMNum = GetReferenceFromVMRegistry( L );
        bool ret = false;
        string errmsg = "";
        switch (lua_pcall( L, nargs, nresults, errfunc ))
        {
            case 0:
            ret = true;
            break;
            case LUA_ERRRUN:
            errmsg = lua_tostring(L, -1 );
            LuaScriptingAPIHelper::SayFromObject(iVMNum, "Event returned Runtime Error: " + errmsg);
            break;
            case LUA_ERRMEM:
            errmsg = lua_tostring(L, -1 );
            LuaScriptingAPIHelper::SayFromObject(iVMNum, "Event ran out of memory: " + errmsg);
            break;
            case LUA_ERRERR:
            errmsg = lua_tostring(L, -1 );
            LuaScriptingAPIHelper::SayFromObject(iVMNum, "Error while running error handler : " + errmsg);
            break;
            default:
            errmsg = lua_tostring(L, -1 );
            LuaScriptingAPIHelper::SayFromObject(iVMNum, "Event returned unknown error: " + errmsg);
            break;
        }

        return ret;
    }

    void procTable(lua_State *L, lua_State *pluaVM, int top, int depth)
    {
        int t = 0;
        double td = 0;
        string ts = "";
        bool tb;
        lua_newtable( pluaVM );
        lua_pushnil(L);
        bool ret = false;
        while (lua_next(L, top)  != 0)
        {
            /* `key' is at index -2 and `value' at index -1 */
            if (lua_checkstack( pluaVM, 6))
            {
                switch(lua_type(L, -2))
                {
                    case LUA_TNUMBER:
                    td = lua_tonumber(L, -2);
                    //DEBUG("table key, num: " << td);
                    lua_pushnumber( pluaVM, td );
                    break;
                    case LUA_TSTRING:
                    ts = lua_tostring(L, -2);
                    //DEBUG("table key, str: " << ts);
                    lua_pushstring( pluaVM,  ts.c_str());
                    break;
                    case LUA_TBOOLEAN:
                    tb = lua_toboolean(L, -2);
                    //DEBUG("table key, boo: " << tb);
                    lua_pushboolean( pluaVM, tb );
                    break;
                    default:
                    WARNING("passed unknown type in table key: " << lua_typename(L, -2) << endl);
                    lua_pushnil(pluaVM);
                    break;
                }

                switch(lua_type(L, -1))
                {
                    case LUA_TNUMBER:
                    t++;
                    ret = true;
                    td = lua_tonumber(L, -1);
                    //DEBUG("table val, num : " << td);
                    lua_pushnumber( pluaVM, td );
                    break;
                    case LUA_TSTRING:
                    t++;
                    ret = true;
                    ts = lua_tostring(L, -1) ;
                    //DEBUG("table val, str: " << ts);
                    lua_pushstring( pluaVM, ts.c_str());
                    break;
                    case LUA_TBOOLEAN:
                    t++;
                    ret = true;
                    tb = lua_toboolean(L, -1);
                    //DEBUG("table val, boo: " << tb);
                    lua_pushboolean( pluaVM, tb );
                    break;
                    case LUA_TTABLE:
                    if (depth < 255)
                    {
                        t++;
                        ret = true;
                        //DEBUG("passed a sub table value: " << endl);
                        depth++;
                        procTable(L, pluaVM, lua_gettop(L), depth);
                    }
                    else
                    {
                        int iRef = GetReferenceFromVMRegistry( pluaVM );
                        LuaScriptingAPIHelper::SayFromObject(iRef,  "Max number of sub-tables exceeded, recursion ending");
                        DEBUG("Max number of sub-tables exceeded, recursion ending");
                    }
                    break;
                    default:
                    t++;
                    ret = true;
                    //DEBUG("passed unknown type in table val: " << lua_typename(L, -1) << endl);
                    lua_pushnil(pluaVM);
                    break;
                }
            }
            else
            {
                int iRef = GetReferenceFromVMRegistry( pluaVM );
                LuaScriptingAPIHelper::SayFromObject(iRef,  "Ran out of stack space");
            }

            if (ret == true)
            {
                //DEBUG("setting table");
                lua_rawset(pluaVM, -3);
            }
            else
            {
                WARNING("could not add entry to table");
            }
            lua_pop(L, 1);  /* removes `value'; keeps `key' for next iteration */
        }

        //DEBUG("closing table, " << t << " entries");
        lua_pushliteral(pluaVM, "n");        /* Pushes the literal */
        lua_pushnumber(pluaVM, t);       /* Pushes the total number of cells */
        lua_rawset(pluaVM, -3);              /* Stores the pair in the table */
    }

    int SwapParams(lua_State *L, lua_State *pluaVM)
    {
        int args = 0;
        double n = 0;
        string s = "";
        bool b = true;

        double td = 0;
        string ts = "";
        bool tb;

        //DEBUG("swapping stacks" << endl);
        while (lua_gettop(L))
        {
            if (lua_checkstack( pluaVM, 6))
            {
                switch (lua_type( L, 1))
                {
                    case LUA_TNUMBER:
                    args++;
                    n = lua_tonumber( L, 1 );
                    //DEBUG("passed a number: " << n << endl);
                    lua_pushnumber( pluaVM, n);
                    break;
                    case LUA_TTABLE:
                    args++;
                    //DEBUG("passed a table: " << endl);
                    procTable(L, pluaVM, 1, 0);
                    break;
                    case LUA_TSTRING:
                    args++;
                    s = lua_tostring( L, 1);
                    //DEBUG("passed a string : " << s << endl);
                    lua_pushstring( pluaVM, s.c_str());
                    break;
                    case LUA_TBOOLEAN:
                    args++;
                    b = lua_toboolean( L, 1 );
                    //DEBUG("passed a boolean: " << b << endl);
                    lua_pushboolean( pluaVM, b);
                    break;
                    default:
                    //DEBUG("passed unknown type : " << lua_typename(L, 1) << endl);
                    break;
                }
                lua_remove(L,1);
            }
            else
            {
                int iRef = GetReferenceFromVMRegistry( pluaVM );
                LuaScriptingAPIHelper::SayFromObject(iRef,  "Ran out of stack space");
                DEBUG("Ran out of stack space");
            }
        }
        //DEBUG("swaped " << args << " args" << endl);
        return args;
    }

    string makeSafeForXML(string unsafeText)
    {
        string ret;

        int pos = unsafeText.find('<');
        while (pos > -1)
        {
            unsafeText.replace(pos,1,"[",1);
            pos = unsafeText.find('<');
        }

        pos = unsafeText.find('>');
        while (pos > -1)
        {
            unsafeText.replace(pos,1,"]",1);
            pos = unsafeText.find('>');
        }

        return unsafeText;
    }

    void SayFromObject( int iObjectReference, string sMessage )
    {
        ostringstream messagestream;
        messagestream << "<comm type=\"say\" message=\"" << makeSafeForXML(sMessage) << "\" iowner=\"" << iObjectReference << "\"/>" << endl;
        DEBUG("SayFromObj: " << iObjectReference << " : " << messagestream.str().c_str() );
        SendClientMessage( messagestream.str().c_str() );
    }

    void AddObjectReferenceToVMRegistry( int iObjectReference, lua_State *pluaVM )
    {
        DEBUG("AddObjectReferenceToVMRegistry()" << endl);
        DumpVMRegistry( pluaVM );

        lua_pushstring( pluaVM, "iReference" );
        lua_pushnumber( pluaVM, iObjectReference );
        lua_settable( pluaVM, LUA_REGISTRYINDEX );

        DumpVMRegistry( pluaVM );
    }

    void PushRotAsTable( lua_State *L, const Rot &rot )
    {
        lua_newtable( L );

        lua_pushstring( L, "x" );
        lua_pushnumber( L, rot.x );
        lua_settable( L, -3 );

        lua_pushstring( L, "y" );
        lua_pushnumber( L, rot.y );
        lua_settable( L, -3 );

        lua_pushstring( L, "z" );
        lua_pushnumber( L, rot.z );
        lua_settable( L, -3 );

        lua_pushstring( L, "s" );
        lua_pushnumber( L, - rot.s );
        lua_settable( L, -3 );
    }

    void PushVectorAsTable( lua_State *L, const Vector3 &vector )
    {
        lua_newtable( L );

        lua_pushstring( L, "x" );
        lua_pushnumber( L, vector.x );
        lua_settable( L, -3 );

        lua_pushstring( L, "y" );
        lua_pushnumber( L, vector.y );
        lua_settable( L, -3 );

        lua_pushstring( L, "z" );
        lua_pushnumber( L, vector.z );
        lua_settable( L, -3 );
    }

    void DumpVMRegistry( lua_State *pluaVM )
    {
        DEBUG("reading registry..." << endl);
        lua_pushnil( pluaVM );
        while (lua_next(pluaVM, LUA_REGISTRYINDEX ) != 0)
        {
            string keyname = lua_tostring( pluaVM, -2 );
            string valuetype = lua_typename(pluaVM, lua_type(pluaVM, -1));
            DEBUG("[" << keyname << "] [" << valuetype << "]" << endl);
            lua_pop(pluaVM, 1);  /* removes `value'; keeps `key' for next iteration */
        }
        DEBUG("...done" << endl);
    }
}
