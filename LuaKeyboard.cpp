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
//! \brief Contains functions for capturing keyboard in OSMP Lua scripts

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
#include "SocketsClass.h"
#include "Diag.h"
//#include "ThreadWrapper.h"

//#include "LuaScriptingAPI.h"
#include "LuaScriptingAPIHelper.h"

#include "scriptingenginelua.h"
//#include "LuaKeyboard.h"
//#include "LuaEventClass.h"



extern lua_State *luaVM;
extern mvsocket SocketMetaverseServer;
extern map < int, ObjectVMInfoClass > ObjectVMs;

extern pthread_mutex_t EngineMutex;

extern int totalCaptures;

//! Used for keyboard input from client to scripting engine(s)
extern struct captureData
    {
        int iRef;    //!< iReference number of object capturing keyboard
        int iowner;   //!< Object owner (?)
        bool active;    //!< If active, capture is on
    }
captureList[2048];


namespace LuaKeyboard
{

    static int CaptureKeyboard( lua_State *L )
    {
        pthread_mutex_lock( &EngineMutex );

        int iRef = GetReferenceFromVMRegistry( L );

        captureData thisCapture;

        if ( LuaScriptingAPIHelper::GetTypename( L, 1 ) != "number")
        {
            LuaScriptingAPIHelper::SayFromObject(iRef,  "First parameter must be a id of av");
            return 0;
        }

        int iReference = GetReferenceFromVMRegistry( L );
        int iowner = lua_tonumber( L, 1 );

        thisCapture.iRef = iRef;
        thisCapture.iowner = iowner;
        thisCapture.active = true;
        DEBUG("Calling captureKeyboard " << totalCaptures << " captures so far");
        int f = -1;
        int i;
        for (i = 0; i < totalCaptures; i++)
        {
            if ((thisCapture.iRef == captureList[i].iRef) && (thisCapture.iowner == captureList[i].iowner))
            {
                f = i;
            }
        }
        bool g = false;
        if (f == -1)
        {
            if (totalCaptures < 2048)
            {
                captureList[totalCaptures] = thisCapture;
                totalCaptures++;
                g = true;
            }
            else
            {
                for (int j = 0; j < totalCaptures; j++)
                {
                    if (captureList[j].active == false)
                    {
                        g = true;
                        captureList[j] = thisCapture;
                        break;
                    }
                }
            }
        }
        else
        {
            g = true;
            captureList[f] = thisCapture;
        }
        if (g == true)
        {
            ostringstream messagestream;
            messagestream << "<capture what=\"wholekeyboard\" ireference=\"" << iRef << "\" iowner=\"" << iowner << "\"/>" << endl;
            DEBUG(  "Sending to server " << messagestream.str().c_str());
            SocketMetaverseServer.Send( messagestream.str().c_str() );
        }
        else
        {
            LuaScriptingAPIHelper::SayFromObject(iRef,  "Server ran out of available keyboard captures");
        }

        pthread_mutex_unlock( &EngineMutex );

        return 0;
    }

    static int StopCaptureKeyboard(lua_State *L)
    {
        pthread_mutex_lock( &EngineMutex );

        int f =-1;
        int iRef = GetReferenceFromVMRegistry( L );
        int i;
        for (i = 0; i < totalCaptures; i++)
        {
            if (iRef == captureList[i].iRef)
            {
                f = i;
                break;
            }
        }

        if (f > -1)
        {
            captureList[f].active = false;
            ostringstream messagestream;
            messagestream << "<capture what=\"wholekeyboardoff\" ireference=\"" << iRef << "\"/>" << endl;
            DEBUG(  "Sending to server " << messagestream.str() );
            SocketMetaverseServer.Send( messagestream.str().c_str() );
        }

        pthread_mutex_unlock( &EngineMutex );

        return 0;
    }

    void RegisterLuaFunctions( lua_State *pluaVM )
    {
#define LUAREGISTER( f ) lua_register(pluaVM, #f, f )

        LUAREGISTER(CaptureKeyboard);
        LUAREGISTER(StopCaptureKeyboard);
    }
}
