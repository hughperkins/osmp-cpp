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
//! \brief Contains Lua scripting engine event classes, such as EventInfoData

#ifndef _LUAEVENTCLASS_H
#define _LUAEVENTCLASS_H

#include <string>
using namespace std;

//! Base class for all Lua engine events; holds VM reference, eventname, and eventclass name
class EventInfo
{
public:
    int iVMNum;   //!< reference number of targeted VM
    string sEventName;  //!< Name of event
    //string sData;
    string sEventClass;   //!< Class of event

    virtual int DummyFunction()
    {
        return 0;
    }  //!< We need this for compilation
};

//! EventInfoData is for data returned from the database; used by Lua scripting engine
class EventInfoData : public EventInfo
{
public:

};

//! EventCollisionStart is for collision start events; used by Lua scripting engine
class EventCollisionStart : public EventInfo
{
public:
    int iReference; //!< iReference of colliding object
};

//! EventCollisionEnd is for collision end events; used by Lua scripting engine
class EventCollisionEnd : public EventInfo
{
public:
    int iReference; //!< iReference of colliding object
};

//! EventKeyUp is for key up events; used by Lua scripting engine
class EventKeyUp : public EventInfo
{
public:
    string sValue;  //!< key code(?)
    int iReference;    //!< iReference of avatar typing (?)
};

//! Key down events; used by Lua scripting engine
class EventKeyDown : public EventInfo
{
public:
    string sValue;   //!< key code(?)
    int iReference;    //!< iReference of avatar typing (?)
};

//! Time events; used by lua scripting engine
class EventTimer : public EventInfo
{
public:

};

//! initialization event (at script startup); used by lua scripting engine
class EventInit : public EventInfo
{
public:

};

//! mouseclick event; used by lua scripting engine
class EventClick : public EventInfo
{
public:
    int iClickerReference;  //!< iReference of avatar who clicked
};

//! userdata event; used to return userdata from db; used by lua scripting engine
class EventInfoUserData : public EventInfoData
{
public:
    int iOwner;    //!< owner of data (?)
    string sStore;  //!< select public or private data store (?)
    string sKey;    //!< key to identify the data
    string sData;    //!< returned data
    string sClientSideReference;  //!< arbitrary query reference sent by client
};

//! script RPC event; used by lua scripting engine
class EventInfoRPC : public EventInfo
{
public:

};

//! script multicast rpc event; used by lua scripting engine
class EventInfoMulticastRPC : public EventInfoRPC
{
public:
    int iSenderReference;  //!< iReference of sending object
    //string sRPCType;
    string sMessage;   //!< message
};

#endif // _LUAEVENTCLASS_H

