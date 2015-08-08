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
//! \brief This module groups together functions for setting timer properties

#include <sstream>
#include <string>
#ifdef _WIN32
#include <windows.h>
#endif
using namespace std;

extern "C"
{
#include <lua.h>
   #include "lauxlib.h"
   #include "lualib.h"
}

#include "pthread.h"
#include "LuaScriptingAPITimerProperties.h"
#include "LuaScriptingAPIHelper.h"
#include "Diag.h"
#include "TickCount.h"

extern int totalTimers;

extern struct timerData
    {
        int duration;
        int repeats;
        int rcount;
        int tickcount;
        string function;
        bool running;
        int iRef;
        bool active;
    }
timerList[2048];

extern pthread_mutex_t EngineMutex;

namespace LuaScriptingAPITimerProperties
{

    static int SetTimer(lua_State *L)
    {
        pthread_mutex_lock( &EngineMutex );
        // SetTimer(duration, repeats, function)
        timerData thisTimer;
        string s;
        int iRef = GetReferenceFromVMRegistry( L );
        if ( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "number" )
        {
            thisTimer.iRef = iRef;
            thisTimer.duration = lua_tonumber(L, 1);
            thisTimer.active = true;
            thisTimer.running = true;
            thisTimer.tickcount = 0;
            DEBUG("timer duration is : " << thisTimer.duration);
            lua_remove(L, 1);
            if ( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "number" )
            {
                thisTimer.repeats = lua_tonumber(L,1);
                lua_remove(L,1);
            }
            else
            {
                thisTimer.repeats = 0;
            }
            thisTimer.rcount = thisTimer.repeats;
            if ( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "string" )
            {
                thisTimer.function = lua_tostring(L,1);
                lua_remove(L,1);
            }
            else
            {
                thisTimer.function = "Timer";
            }

            lua_getglobal( L, thisTimer.function.c_str() );
            if( LuaScriptingAPIHelper::GetTypename( L, -1 ) == "function" )
            {
                lua_remove(L, 1);
                int f = -1;
                int i;
                for (i = 0; i < totalTimers; i++)
                {
                    if (thisTimer.iRef == timerList[i].iRef && timerList[i].function == thisTimer.function)
                    {
                        f = i;
                    }
                }
                if (f == -1)
                {
                    if (totalTimers < 2048)
                    {
                        timerList[totalTimers] = thisTimer;
                        totalTimers++;
                    }
                    else
                    {
                        bool g = false;
                        for (int j = 0; j < totalTimers; j++)
                        {
                            if (timerList[j].active == false || timerList[j].running == false)
                            {
                                g = true;
                                timerList[j] = thisTimer;
                                break;
                            }
                        }
                        if (g == false)
                        {
                            LuaScriptingAPIHelper::SayFromObject(iRef,  "Server ran out of available timers, could not set timer");
                        }
                    }
                }
                else
                {
                    timerList[f] = thisTimer;
                }
            }
            else
            {
                LuaScriptingAPIHelper::SayFromObject(iRef,  "Could not find specified timer function: " + thisTimer.function);
            }
        }
        else
        {
            LuaScriptingAPIHelper::SayFromObject(iRef, "1st parameter to SetTimer was not a number");
        }

        pthread_mutex_unlock( &EngineMutex );

        return 0;
    }

    static int GetTimer(lua_State *L)
    {
        pthread_mutex_lock( &EngineMutex );
        // GetTimer(function) -> (time left, repeats left)
        string function = "";
        if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "string" )
        {
            function = lua_tostring(L,1);
            lua_remove(L, 1);
        }
        else
        {
            function = "Timer";
        }

        int iRef = GetReferenceFromVMRegistry( L );
        int f =-1;
        int i;
        for (i =0; i < totalTimers; i++)
        {
            if (iRef == timerList[i].iRef && timerList[i].function == function)
            {
                f = i;
            }
        }

        if (f > -1)
        {
            int iCurrentTickCount = MVGetTickCount();
            int d = timerList[f].duration - (iCurrentTickCount  - timerList[f].tickcount);
            lua_pushnumber(L, d);
            lua_pushnumber(L, timerList[i].rcount);
        }
        else
        {
            lua_pushnumber(L, 0);
            lua_pushnumber(L, 0);
        }

        pthread_mutex_unlock( &EngineMutex );

        return 2;
    }

    static int StopTimer(lua_State *L)
    {
        pthread_mutex_lock( &EngineMutex );
        // StopTimer(function)
        string function = "";
        if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "string" )
        {
            function = lua_tostring(L,1);
            lua_remove(L, 1);
        }
        else
        {
            function = "Timer";
        }

        int f =-1;
        int iRef = GetReferenceFromVMRegistry( L );
        int i;
        for (i =0; i < totalTimers; i++)
        {
            if (iRef == timerList[i].iRef && timerList[i].function == function)
            {
                f = i;
            }
        }

        if (f > -1)
        {
            timerList[f].running = false;
        }

        pthread_mutex_unlock( &EngineMutex );

        return 0;
    }

    static int StartTimer(lua_State *L)
    {
        pthread_mutex_lock( &EngineMutex );
        // StartTimer(function)
        string function = "";
        if( LuaScriptingAPIHelper::GetTypename( L, 1 ) == "string" )
        {
            function = lua_tostring(L,1);
            lua_remove(L, 1);
        }
        else
        {
            function = "Timer";
        }

        int f =-1;
        int iRef = GetReferenceFromVMRegistry( L );
        for (int i =0; i < totalTimers; i++)
        {
            if (iRef == timerList[i].iRef && timerList[i].function == function)
            {
                f = i;
            }
        }

        if (f > -1)
        {
            timerList[f].running = true;
        }

        pthread_mutex_unlock( &EngineMutex );

        return 0;
    }

    void RegisterLuaFunctions( lua_State *pluaVM )
    {
#define LUAREGISTER( f ) lua_register(pluaVM, #f, f )

        LUAREGISTER(SetTimer); // SetTimer(duration, repeats, function)
        LUAREGISTER(GetTimer); // GetTimer(function) -> (time left, repeats left)
        LUAREGISTER(StopTimer); // StopTimer(function)
        LUAREGISTER(StartTimer); // StartTimer(function)
    }

}
