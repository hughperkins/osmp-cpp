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
//! \brief the Lua scripting engine manages running scripts in Lua within the OSMP world

#ifndef _SCRIPTINGENGINELUA_H
#define _SCRIPTINGENGINELUA_H

#include <map>
using namespace std;

extern "C"
{
#include <lua.h>
}

#include "pthread.h"

#include "LuaEventClass.h"

//void QueueFunction( int iVMNum, string sFunctionName, string sData = "" );

//! Queues one event to the corresponding Lua VM
void QueueEvent( EventInfo *pEvent );

//! Stores information about a single Lua virtual machine

//! Stores information about a single Lua virtual machine, such as:
//! - whether it is running
//! - whether it is initialized
//! - refrence of running script
class ObjectVMInfoClass
{
public:
    lua_State *pVM;   //!< pointer to Lua VM
    int iObjectReference;   //!< reference of object associated with this VM
    string sScriptReference;  //!< reference of script running in this VM
    bool bVMInitialized;    //!< whether VM is initialized or not
    bool bVMIsRunning;    //!< whether VM is running or not
    pthread_t threadobject;  //!< thread associated with running VM
    const EventInfo *pEvent;   //!< currently executing event
    ObjectVMInfoClass()
    {
        pVM = NULL;
        iObjectReference = 0;
        sScriptReference = "";
        bVMInitialized = false;
        bVMIsRunning = false;
        pEvent = 0;
    }
};

typedef map< int, ObjectVMInfoClass >::iterator ObjectVMIterator;

#endif // _SCRIPTINGENGINELUA_H
