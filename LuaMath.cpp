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
//! \brief Contains maths functions for OSMP Lua scripts; mostly quaternions and matrix maths

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
#include "LuaMath.h"

extern lua_State *luaVM;
extern mvWorldStorage World;
extern mvsocket SocketMetaverseServer;
extern char SendBuffer[ 2048 ];
extern char ReadBuffer[ 4097 ];

extern int iMyReference;

extern pthread_mutex_t EngineMutex;

namespace LuamvMath
{
    static int RotBetween( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        Vector3 v1, v2;

        lua_pushvalue( L, 1 );
        TableToVector( L, v1 );
        lua_pop( L, 1 );

        lua_pushvalue( L, 2 );
        TableToVector( L, v2 );
        lua_pop( L, 1 );

        Rot ResultRot;
        RotBetween( ResultRot, v1, v2 );

        LuaScriptingAPIHelper::PushRotAsTable( L, ResultRot );

        pthread_mutex_unlock( &EngineMutex );
        return 1;
    }

    static int VectorMag( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        Vector3 incomingvector;

        lua_pushvalue( L, 1 );
        TableToVector( L, incomingvector );
        lua_pop( L, 1 );

        float fResult = VectorMag( incomingvector );

        lua_pushnumber( L, fResult );

        pthread_mutex_unlock( &EngineMutex );
        return 1;
    }

    static int VectorNormalize( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        Vector3 incomingvector;

        lua_pushvalue( L, 1 );
        TableToVector( L, incomingvector );
        lua_pop( L, 1 );

        VectorNormalize( incomingvector );

        LuaScriptingAPIHelper::PushVectorAsTable( L, incomingvector );

        pthread_mutex_unlock( &EngineMutex );
        return 1;
    }

    static int VectorDot( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        Vector3 v1, v2;
        float fResult;

        lua_pushvalue( L, 1 );
        TableToVector( L, v1 );
        lua_pop( L, 1 );

        lua_pushvalue( L, 2 );
        TableToVector( L, v2 );
        lua_pop( L, 1 );

        fResult = VectorDot( v1, v2 );

        lua_pushnumber( L, fResult );

        pthread_mutex_unlock( &EngineMutex );
        return 1;
    }

    static int VectorCross( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        Vector3 v1, v2, vresult;

        lua_pushvalue( L, 1 );
        TableToVector( L, v1 );
        lua_pop( L, 1 );

        lua_pushvalue( L, 2 );
        TableToVector( L, v2 );
        lua_pop( L, 1 );

        VectorCross( vresult, v1, v2 );

        LuaScriptingAPIHelper::PushVectorAsTable( L, vresult );

        pthread_mutex_unlock( &EngineMutex );
        return 1;
    }

    static int RotMultiply( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        Rot rot1, rot2;

        lua_pushvalue( L, 1 );
        TableToRot( L, rot1 );
        lua_pop( L, 1 );

        lua_pushvalue( L, 2 );
        TableToRot( L, rot2 );
        lua_pop( L, 1 );

        Rot resultrot;
        RotMultiply( resultrot, rot1, rot2 );

        LuaScriptingAPIHelper::PushRotAsTable( L, resultrot );

        pthread_mutex_unlock( &EngineMutex );
        return 1;
    }

    static int Rot2AxisAngle( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        Vector3 Axis;
        float Angle;
        Rot IncomingRotation;

        lua_pushvalue( L, 1 );
        TableToRot( L, IncomingRotation );
        lua_pop( L, 1 );

        Rot2AxisAngle( Axis, Angle, IncomingRotation );

        LuaScriptingAPIHelper::PushVectorAsTable( L, Axis );
        lua_pushnumber( L, Angle );

        pthread_mutex_unlock( &EngineMutex );
        return 2;
    }

    static int MultiplyVectorByRot( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        Vector3 vTargetVector;
        Rot rot;

        lua_pushvalue( L, 1 );
        TableToVector( L, vTargetVector );
        lua_pop( L, 1 );

        lua_pushvalue( L, 2 );
        TableToRot( L, rot );
        lua_pop( L, 1 );

        Vector3 vResultVector;
        vResultVector = vTargetVector * rot;

        LuaScriptingAPIHelper::PushVectorAsTable( L, vResultVector );

        pthread_mutex_unlock( &EngineMutex );
        return 1;
    }

    static int InverseRot( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        Rot IncomingRotation;

        lua_pushvalue( L, 1 );
        TableToRot( L, IncomingRotation );
        lua_pop( L, 1 );

        IncomingRotation.s = - IncomingRotation.s;

        LuaScriptingAPIHelper::PushRotAsTable( L, IncomingRotation );

        pthread_mutex_unlock( &EngineMutex );
        return 1;
    }

    static int AxisAngle2Rot( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        Vector3 Axis;
        float Angle;
        Rot RotationResult;

        lua_pushvalue( L, 1 );
        TableToVector( L, Axis );
        lua_pop( L, 1 );

        Angle = lua_tonumber( L, 2 );

        DEBUG(  "axisangle2rot: " << Axis << " " << Angle ); // DEBUG
        AxisAngle2Rot( RotationResult, Axis, Angle );

        LuaScriptingAPIHelper::PushRotAsTable( L, RotationResult );

        pthread_mutex_unlock( &EngineMutex );
        return 1;
    }

    void RegisterLuaFunctions( lua_State *pluaVM )
    {
#define LUAREGISTER( f ) lua_register(pluaVM, #f, f )

        LUAREGISTER(RotBetween);
        LUAREGISTER(VectorMag);
        LUAREGISTER(VectorNormalize);
        LUAREGISTER(VectorDot);
        LUAREGISTER(VectorCross);
        LUAREGISTER(RotMultiply);
        LUAREGISTER(InverseRot);
        LUAREGISTER(Rot2AxisAngle);
        LUAREGISTER(AxisAngle2Rot);
        LUAREGISTER(MultiplyVectorByRot);
    }
}
