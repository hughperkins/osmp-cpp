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
//! \brief This module groups together functions for getting object properties

// For documentation on each function, see the document LuaScriptEngine.html in the cvs module "documentation"

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

#include "Prim.h"

#include "LuaScriptingAPIHelper.h"
#include "LuaScriptingAPIGetObjectPropertiesStandard.h"

extern lua_State *luaVM;
extern mvWorldStorage World;
extern mvsocket SocketMetaverseServer;
extern char SendBuffer[ 2048 ];
extern char ReadBuffer[ 4097 ];

extern pthread_mutex_t EngineMutex;

static int GetObjectTypeByReference( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "number" )
    {
        int iReference = (int)lua_tonumber( L, 1 );
        int iArrayNum;
        iArrayNum = World.GetArrayNumForObjectReference( iReference );
        if( iArrayNum != -1 )
        {
            //DEBUG(  "Getting object type of object " << iReference << " " << World.GetObject( iArrayNum )->sDeepObjectType ); // DEBUG
            lua_pushstring( L, World.GetObject( iArrayNum )->sDeepObjectType );
        }
        else
        {
            //DEBUG(  "Returning object type NULL for reference " << iReference ); // DEBUG
            lua_pushstring( L, "NULL" );
        }
    }
    else
    {
        lua_pushstring( L, "NULL" );
    }

    pthread_mutex_unlock( &EngineMutex );
    return 1;
}

static int GetObjectType( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    int iReference = GetReferenceFromVMRegistry( L );
    int iArrayNum;
    iArrayNum = World.GetArrayNumForObjectReference( iReference );
    if( iArrayNum != -1 )
    {
        //DEBUG(  "Getting object type of object " << iReference << " " << World.GetObject( iArrayNum )->sDeepObjectType ); // DEBUG
        lua_pushstring( L, World.GetObject( iArrayNum )->sDeepObjectType );
    }
    else
    {
        //DEBUG(  "Returning object type NULL for reference " << iReference ); // DEBUG
        lua_pushstring( L, "NULL" );
    }

    pthread_mutex_unlock( &EngineMutex );
    return 1;
}

static int GetObjectNameByReference( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "number" )
    {
        int iReference = (int)lua_tonumber( L, 1 );
        Object *p_Object;
        p_Object = World.GetObjectByReference( iReference );
        if( p_Object != NULL )
        {
            //DEBUG(  "Getting object type of object " << iReference << " " << World.GetObject( iArrayNum )->sDeepObjectType ); // DEBUG
            lua_pushstring( L, p_Object->sObjectName );
        }
        else
        {
            //DEBUG(  "Returning object type NULL for reference " << iReference ); // DEBUG
            lua_pushstring( L, "NULL" );
        }
    }
    else
    {
        lua_pushstring( L, "NULL" );
    }

    pthread_mutex_unlock( &EngineMutex );
    return 1;
}
static int GetObjectName( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    int iReference = GetReferenceFromVMRegistry( L );
    Object *p_Object;
    p_Object = World.GetObjectByReference( iReference );
    if( p_Object != NULL )
    {
        //DEBUG(  "Getting object type of object " << iReference << " " << World.GetObject( iArrayNum )->sDeepObjectType ); // DEBUG
        lua_pushstring( L, p_Object->sObjectName );
    }
    else
    {
        //DEBUG(  "Returning object type NULL for reference " << iReference ); // DEBUG
        lua_pushstring( L, "NULL" );
    }

    pthread_mutex_unlock( &EngineMutex );
    return 1;
}

static int GetObjectParentByReference( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "number" )
    {
        int iReference = (int)lua_tonumber( L, 1 );
        int iArrayNum;
        iArrayNum = World.GetArrayNumForObjectReference( iReference );
        if( iArrayNum != -1 )
        {
            //DEBUG(  "Getting parent of object " << iReference << " " << World.GetObject( iArrayNum )->sDeepObjectType ); // DEBUG
            lua_pushnumber( L, (double)World.GetObject( iArrayNum )->iParentReference );
        }
        else
        {
            //DEBUG(  "Returning parent 0 for reference " << iReference ); // DEBUG
            lua_pushnumber( L, 0.0 );
        }
    }
    else
    {
        lua_pushnumber( L, 0.0 );
    }

    pthread_mutex_unlock( &EngineMutex );
    return 1;
}

static int GetObjectParent( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    int iReference = GetReferenceFromVMRegistry( L );

    int iArrayNum;
    iArrayNum = World.GetArrayNumForObjectReference( iReference );
    if( iArrayNum != -1 )
    {
        //DEBUG(  "Getting parent of object " << iReference << " " << World.GetObject( iArrayNum )->sDeepObjectType ); // DEBUG
        lua_pushnumber( L, (double)World.GetObject( iArrayNum )->iParentReference );
    }
    else
    {
        //DEBUG(  "Returning parent 0 for reference " << iReference ); // DEBUG
        lua_pushnumber( L, 0.0 );
    }

    pthread_mutex_unlock( &EngineMutex );
    return 1;
}

static int GetReference( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    int iReference = GetReferenceFromVMRegistry( L );
    lua_pushnumber( L, (float)iReference );

    pthread_mutex_unlock( &EngineMutex );
    return 1;
}

static int GetRot( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    int iReference = GetReferenceFromVMRegistry( L );

    Object *p_Object = World.GetObjectByReference( iReference );

    lua_newtable( L );

    lua_pushstring( L, "x" );
    lua_pushnumber( L, p_Object->rot.x );
    lua_settable( L, -3 );

    lua_pushstring( L, "y" );
    lua_pushnumber( L, p_Object->rot.y );
    lua_settable( L, -3 );

    lua_pushstring( L, "z" );
    lua_pushnumber( L, p_Object->rot.z );
    lua_settable( L, -3 );

    lua_pushstring( L, "s" );
    lua_pushnumber( L, p_Object->rot.s );
    lua_settable( L, -3 );


    pthread_mutex_unlock( &EngineMutex );
    return 1;
}

static int GetRotByReference( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "number" )
    {
        int iReference = (int)lua_tonumber( L, 1 );

        Object *p_Object = World.GetObjectByReference( iReference );

        if( p_Object != NULL )
        {
            lua_newtable( L );

            lua_pushstring( L, "x" );
            lua_pushnumber( L, p_Object->rot.x );
            lua_settable( L, -3 );

            lua_pushstring( L, "y" );
            lua_pushnumber( L, p_Object->rot.y );
            lua_settable( L, -3 );

            lua_pushstring( L, "z" );
            lua_pushnumber( L, p_Object->rot.z );
            lua_settable( L, -3 );

            lua_pushstring( L, "s" );
            lua_pushnumber( L, p_Object->rot.s );
            lua_settable( L, -3 );
        }
        else
        {
            lua_newtable( L );

            lua_pushstring( L, "x" );
            lua_pushnumber( L, 0 );
            lua_settable( L, -3 );

            lua_pushstring( L, "y" );
            lua_pushnumber( L, 0 );
            lua_settable( L, -3 );

            lua_pushstring( L, "z" );
            lua_pushnumber( L, 0 );
            lua_settable( L, -3 );

            lua_pushstring( L, "s" );
            lua_pushnumber( L, 0 );
            lua_settable( L, -3 );
        }
    }

    pthread_mutex_unlock( &EngineMutex );
    return 1;
}

static int GetPos( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    int iReference = GetReferenceFromVMRegistry( L );

    Object *p_Object = World.GetObjectByReference( iReference );

    lua_newtable( L );

    lua_pushstring( L, "x" );
    lua_pushnumber( L, p_Object->pos.x );
    lua_settable( L, -3 );

    lua_pushstring( L, "y" );
    lua_pushnumber( L, p_Object->pos.y );
    lua_settable( L, -3 );

    lua_pushstring( L, "z" );
    lua_pushnumber( L, p_Object->pos.z );
    lua_settable( L, -3 );

    pthread_mutex_unlock( &EngineMutex );
    return 1;
}

static int GetPosByReference( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "number" )
    {
        int iReference = (int)lua_tonumber( L, 1 );

        Object *p_Object = World.GetObjectByReference( iReference );

        if( p_Object != NULL )
        {
            lua_newtable( L );

            lua_pushstring( L, "x" );
            lua_pushnumber( L, p_Object->pos.x );
            lua_settable( L, -3 );

            lua_pushstring( L, "y" );
            lua_pushnumber( L, p_Object->pos.y );
            lua_settable( L, -3 );

            lua_pushstring( L, "z" );
            lua_pushnumber( L, p_Object->pos.z );
            lua_settable( L, -3 );
        }
        else
        {
            lua_newtable( L );

            lua_pushstring( L, "x" );
            lua_pushnumber( L, 0 );
            lua_settable( L, -3 );

            lua_pushstring( L, "y" );
            lua_pushnumber( L, 0 );
            lua_settable( L, -3 );

            lua_pushstring( L, "z" );
            lua_pushnumber( L, 0 );
            lua_settable( L, -3 );
        }
    }

    pthread_mutex_unlock( &EngineMutex );
    return 1;
}

static int GetScale( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    int iReference = GetReferenceFromVMRegistry( L );

    Object *p_Object = World.GetObjectByReference( iReference );

    lua_newtable( L );

    if( strcmp( p_Object->ObjectType, "PRIM" ) == 0 )
    {
        Prim *p_Prim = dynamic_cast< Prim *>( p_Object );
        lua_pushstring( L, "x" );
        lua_pushnumber( L, p_Prim->scale.x );
        lua_settable( L, -3 );

        lua_pushstring( L, "y" );
        lua_pushnumber( L, p_Prim->scale.y );
        lua_settable( L, -3 );

        lua_pushstring( L, "z" );
        lua_pushnumber( L, p_Prim->scale.z );
        lua_settable( L, -3 );
    }
    else
    {
        lua_pushstring( L, "x" );
        lua_pushnumber( L, 0 );
        lua_settable( L, -3 );

        lua_pushstring( L, "y" );
        lua_pushnumber( L, 0 );
        lua_settable( L, -3 );

        lua_pushstring( L, "z" );
        lua_pushnumber( L, 0 );
        lua_settable( L, -3 );
    }

    pthread_mutex_unlock( &EngineMutex );
    return 1;
}

static int GetScaleByReference( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    bool bAnswerWritten = false;

    if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "number" )
    {
        int iReference = (int)lua_tonumber( L, 1 );
        Object *p_Object = World.GetObjectByReference( iReference );
        if( p_Object != NULL )
        {
            if( strcmp( p_Object->ObjectType, "PRIM" ) == 0 )
            {
                Prim *p_Prim = dynamic_cast< Prim *>( p_Object );

                lua_newtable( L );

                lua_pushstring( L, "x" );
                lua_pushnumber( L, p_Prim->scale.x );
                lua_settable( L, -3 );

                lua_pushstring( L, "y" );
                lua_pushnumber( L, p_Prim->scale.y );
                lua_settable( L, -3 );

                lua_pushstring( L, "z" );
                lua_pushnumber( L, p_Prim->scale.z );
                lua_settable( L, -3 );

                bAnswerWritten = true;
            }
        }
    }

    if( !bAnswerWritten )
    {
        lua_newtable( L );

        lua_pushstring( L, "x" );
        lua_pushnumber( L, 0 );
        lua_settable( L, -3 );

        lua_pushstring( L, "y" );
        lua_pushnumber( L, 0 );
        lua_settable( L, -3 );

        lua_pushstring( L, "z" );
        lua_pushnumber( L, 0 );
        lua_settable( L, -3 );
    }

    pthread_mutex_unlock( &EngineMutex );
    return 1;
}

static int GetColor( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    int iReference = GetReferenceFromVMRegistry( L );

    Object *p_Object = World.GetObjectByReference( iReference );

    lua_newtable( L );

    if( strcmp( p_Object->ObjectType, "PRIM" ) == 0 )
    {
        Prim *p_Prim = dynamic_cast< Prim *>( p_Object );

        Color color = p_Prim->GetColor( 0 );

        lua_pushstring( L, "r" );
        lua_pushnumber( L, color.r );
        lua_settable( L, -3 );

        lua_pushstring( L, "g" );
        lua_pushnumber( L, color.g );
        lua_settable( L, -3 );

        lua_pushstring( L, "b" );
        lua_pushnumber( L, color.b );
        lua_settable( L, -3 );
    }
    else
    {
        lua_pushstring( L, "r" );
        lua_pushnumber( L, 0 );
        lua_settable( L, -3 );

        lua_pushstring( L, "g" );
        lua_pushnumber( L, 0 );
        lua_settable( L, -3 );

        lua_pushstring( L, "b" );
        lua_pushnumber( L, 0 );
        lua_settable( L, -3 );
    }

    pthread_mutex_unlock( &EngineMutex );
    return 1;
}

static int GetColorByReference( lua_State *L )
{
    pthread_mutex_lock( &EngineMutex );

    bool bAnswerWritten = false;

    if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "number" )
    {
        int iReference = (int)lua_tonumber( L, 1 );
        Object *p_Object = World.GetObjectByReference( iReference );

        if( p_Object != NULL )
        {
            if( strcmp( p_Object->ObjectType, "PRIM" ) == 0 )
            {
                Prim *p_Prim = dynamic_cast< Prim *>( p_Object );

                Color color = p_Prim->GetColor( 0 );

                lua_newtable( L );

                lua_pushstring( L, "r" );
                lua_pushnumber( L, color.r );
                lua_settable( L, -3 );

                lua_pushstring( L, "g" );
                lua_pushnumber( L, color.g );
                lua_settable( L, -3 );

                lua_pushstring( L, "b" );
                lua_pushnumber( L, color.b );
                lua_settable( L, -3 );

                bAnswerWritten = true;
            }
        }
    }

    if( !bAnswerWritten )
    {
        lua_newtable( L );

        lua_pushstring( L, "r" );
        lua_pushnumber( L, 0 );
        lua_settable( L, -3 );

        lua_pushstring( L, "g" );
        lua_pushnumber( L, 0 );
        lua_settable( L, -3 );

        lua_pushstring( L, "b" );
        lua_pushnumber( L, 0 );
        lua_settable( L, -3 );
    }

    pthread_mutex_unlock( &EngineMutex );
    return 1;
}

void RegisterLuaGetObjectPropertiesStandard( lua_State *pluaVM )
{
#define LUAREGISTER( f ) lua_register(pluaVM, #f, f )

    LUAREGISTER(GetObjectParentByReference);
    LUAREGISTER(GetObjectTypeByReference);
    LUAREGISTER(GetObjectNameByReference);
    LUAREGISTER(GetPosByReference);
    LUAREGISTER(GetRotByReference);
    LUAREGISTER(GetScaleByReference);
    LUAREGISTER(GetColorByReference);

    LUAREGISTER(GetReference);
    LUAREGISTER(GetObjectType);
    LUAREGISTER(GetObjectName);
    LUAREGISTER(GetObjectParent);
    LUAREGISTER(GetPos);
    LUAREGISTER(GetRot);
    LUAREGISTER(GetScale);
    LUAREGISTER(GetColor);
}
